/* Transformation pass for OpenACC kernels regions.  Converts a kernels
   region into a series of smaller parallel regions.  There is a parallel
   region for each parallelizable loop nest, as well as a "gang-single"
   parallel region for each non-parallelizable piece of code.

   Contributed by Gergö Barany <gergo@codesourcery.com> and
                  Thomas Schwinge <thomas@codesourcery.com>

   Copyright (C) 2019 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "target.h"
#include "tree.h"
#include "cp/cp-tree.h"
#include "gimple.h"
#include "tree-pass.h"
#include "cgraph.h"
#include "fold-const.h"
#include "gimplify.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "gomp-constants.h"

/* This is a preprocessing pass to be run immediately before lower_omp.  It
   will convert OpenACC "kernels" regions into sequences of "parallel"
   regions.
   For now, the translation is as follows:
   - The entire kernels region is turned into a data region with clauses
     taken from the kernels region.  New "create" clauses are added for all
     variables declared at the top level in the kernels region.
   - Any loop annotated with an OpenACC loop directive is wrapped in a new
     parallel region.  Gang/worker/vector annotations are copied from the
     original kernels region if present.
     * Loops without an explicit "independent" or "seq" annotation get an
       "auto" annotation; other annotations are preserved on the loop or
       moved to the new surrounding parallel region.  Which annotations are
       moved is determined by the constraints in the OpenACC spec; for
       example, loops in the kernels region may have a gang clause, but
       such annotations must now be moved to the new parallel region.
   - Any sequences of other code (non-loops, non-OpenACC loops) are wrapped
     in new "gang-single" parallel regions: Worker/vector annotations are
     copied from the original kernels region if present, but num_gangs is
     explicitly set to 1.  */

/* Helper function for decompose_kernels_region_body.  If STMT contains a
   "top-level" OMP_FOR statement, returns a pointer to that statement;
   returns NULL otherwise.

   A "top-level" OMP_FOR statement is one that is possibly accompanied by
   small snippets of setup code.  Specifically, this function accepts an
   OMP_FOR possibly wrapped in a singleton bind and a singleton try
   statement to allow for a local loop variable, but not an OMP_FOR
   statement nested in any other constructs.  Alternatively, it accepts a
   non-singleton bind containing only assignments and then an OMP_FOR
   statement at the very end.  The former style can be generated by the C
   frontend, the latter by the Fortran frontend.  */

static gimple *
top_level_omp_for_in_stmt (gimple *stmt)
{
  if (gimple_code (stmt) == GIMPLE_OMP_FOR)
    return stmt;

  if (gimple_code (stmt) == GIMPLE_BIND)
    {
      gimple_seq body = gimple_bind_body (as_a <gbind *> (stmt));
      if (gimple_seq_singleton_p (body))
        {
          /* Accept an OMP_FOR statement, or a try statement containing only
             a single OMP_FOR.  */
          gimple *maybe_for_or_try = gimple_seq_first_stmt (body);
          if (gimple_code (maybe_for_or_try) == GIMPLE_OMP_FOR)
            return maybe_for_or_try;
          else if (gimple_code (maybe_for_or_try) == GIMPLE_TRY)
            {
              gimple_seq try_body = gimple_try_eval (maybe_for_or_try);
              if (!gimple_seq_singleton_p (try_body))
                return NULL;
              gimple *maybe_omp_for_stmt = gimple_seq_first_stmt (try_body);
              if (gimple_code (maybe_omp_for_stmt) == GIMPLE_OMP_FOR)
                return maybe_omp_for_stmt;
            }
        }
      else
        {
          gimple_stmt_iterator gsi;
          /* Accept only a block of optional assignments followed by an
             OMP_FOR at the end.  No other kinds of statements allowed.  */
          for (gsi = gsi_start (body); !gsi_end_p (gsi); gsi_next (&gsi))
            {
              gimple *body_stmt = gsi_stmt (gsi);
              if (gimple_code (body_stmt) == GIMPLE_ASSIGN)
                continue;
              else if (gimple_code (body_stmt) == GIMPLE_OMP_FOR
                        && gsi_one_before_end_p (gsi))
                return body_stmt;
              else
                return NULL;
            }
        }
    }

  return NULL;
}

/* Construct a "gang-single" OpenACC parallel region at LOC containing the
   STMTS.  The newly created region is annotated with CLAUSES, which must
   not contain a num_gangs clause, and an additional "num_gangs(1)" clause
   to force gang-single execution.  */

static gimple *
make_gang_single_region (location_t loc, gimple_seq stmts, tree clauses)
{
  /* This correctly unshares the entire clause chain rooted here.  */
  clauses = unshare_expr (clauses);
  /* Make a num_gangs(1) clause.  */
  tree gang_single_clause = build_omp_clause (loc, OMP_CLAUSE_NUM_GANGS);
  OMP_CLAUSE_OPERAND (gang_single_clause, 0) = integer_one_node;
  OMP_CLAUSE_CHAIN (gang_single_clause) = clauses;

  /* Build the gang-single region.  */
  gimple *single_region
    = gimple_build_omp_target (
        NULL,
        GF_OMP_TARGET_KIND_OACC_PARALLEL_KERNELS_GANG_SINGLE,
        gang_single_clause);
  gimple_set_location (single_region, loc);
  gbind *single_body = gimple_build_bind (NULL, stmts, make_node (BLOCK));
  gimple_omp_set_body (single_region, single_body);

  return single_region;
}

/* Helper for make_region_loop_nest.  Transform OpenACC 'kernels'/'loop'
   construct clauses into OpenACC 'parallel'/'loop' construct ones.  */

static tree
transform_kernels_loop_clauses (gimple *omp_for,
				tree num_gangs_clause,
				tree clauses)
{
  /* If this loop in a kernels region does not have an explicit
     "independent", "seq", or "auto" clause, we must give it an explicit
     "auto" clause. */
  bool add_auto_clause = true;
  tree loop_clauses = gimple_omp_for_clauses (omp_for);
  for (tree c = loop_clauses; c; c = OMP_CLAUSE_CHAIN (c))
    {
      if (OMP_CLAUSE_CODE (c) == OMP_CLAUSE_AUTO
          || OMP_CLAUSE_CODE (c) == OMP_CLAUSE_INDEPENDENT
          || OMP_CLAUSE_CODE (c) == OMP_CLAUSE_SEQ)
        {
          add_auto_clause = false;
          break;
        }
    }
  if (add_auto_clause)
    {
      tree auto_clause = build_omp_clause (gimple_location (omp_for),
                                           OMP_CLAUSE_AUTO);
      OMP_CLAUSE_CHAIN (auto_clause) = loop_clauses;
      gimple_omp_for_set_clauses (omp_for, auto_clause);
    }

  /* If the kernels region had a num_gangs clause, add that to this new
     parallel region.  */
  if (num_gangs_clause != NULL)
    {
      tree parallel_num_gangs_clause = unshare_expr (num_gangs_clause);
      OMP_CLAUSE_CHAIN (parallel_num_gangs_clause) = clauses;
      clauses = parallel_num_gangs_clause;
    }

  return clauses;
}

/* Construct a possibly gang-parallel OpenACC parallel region containing the
   STMT, which must be identical to, or a bind containing, the loop OMP_FOR
   with OpenACC loop annotations.

   The newly created region is annotated with the optional NUM_GANGS_CLAUSE
   as well as the other CLAUSES, which must not contain a num_gangs clause.  */

static gimple *
make_gang_parallel_loop_region (gimple *omp_for, gimple *stmt,
                                tree num_gangs_clause, tree clauses)
{
  /* This correctly unshares the entire clause chain rooted here.  */
  clauses = unshare_expr (clauses);

  clauses = transform_kernels_loop_clauses (omp_for,
					    num_gangs_clause,
					    clauses);

  /* Now build the parallel region containing this loop.  */
  gimple_seq parallel_body = NULL;
  gimple_seq_add_stmt (&parallel_body, stmt);
  gimple *parallel_body_bind
    = gimple_build_bind (NULL, parallel_body, make_node (BLOCK));
  gimple *parallel_region
    = gimple_build_omp_target (
        parallel_body_bind,
        GF_OMP_TARGET_KIND_OACC_PARALLEL_KERNELS_PARALLELIZED,
        clauses);
  gimple_set_location (parallel_region, gimple_location (stmt));

  return parallel_region;
}

/* Eliminate any binds directly inside BIND by adding their statements to
   BIND (i.e., modifying it in place), excluding binds that hold only an
   OMP_FOR loop and associated setup/cleanup code.  Recurse into binds but
   not other statements.  Return a chain of the local variables of eliminated
   binds, i.e., the local variables found in nested binds.  If
   INCLUDE_TOPLEVEL_VARS is true, this also includes the variables belonging
   to BIND itself. */

static tree
flatten_binds (gbind *bind, bool include_toplevel_vars = false)
{
  tree vars = NULL, last_var = NULL;

  if (include_toplevel_vars)
    {
      vars = gimple_bind_vars (bind);
      last_var = vars;
    }

  gimple_seq new_body = NULL;
  gimple_seq body_sequence = gimple_bind_body (bind);
  gimple_stmt_iterator gsi, gsi_n;
  for (gsi = gsi_start (body_sequence); !gsi_end_p (gsi); gsi = gsi_n)
    {
      /* Advance the iterator here because otherwise it would be invalidated
         by moving statements below.  */
      gsi_n = gsi;
      gsi_next (&gsi_n);

      gimple *stmt = gsi_stmt (gsi);
      /* Flatten bind statements, except the ones that contain only an
         OpenACC for loop.  */
      if (gimple_code (stmt) == GIMPLE_BIND
          && !top_level_omp_for_in_stmt (stmt))
        {
          gbind *inner_bind = as_a <gbind *> (stmt);
          /* Flatten recursively, and collect all variables.  */
          tree inner_vars = flatten_binds (inner_bind, true);
          gimple_seq inner_sequence = gimple_bind_body (inner_bind);
          gcc_assert (gimple_code (inner_sequence) != GIMPLE_BIND
                      || top_level_omp_for_in_stmt (inner_sequence));
          gimple_seq_add_seq (&new_body, inner_sequence);
          /* Find the last variable; we will append others to it.  */
          while (last_var != NULL && TREE_CHAIN (last_var) != NULL)
            last_var = TREE_CHAIN (last_var);
          if (last_var != NULL)
            {
              TREE_CHAIN (last_var) = inner_vars;
              last_var = inner_vars;
            }
          else
            {
              vars = inner_vars;
              last_var = vars;
            }
        }
      else
        gimple_seq_add_stmt (&new_body, stmt);
    }

  /* Put the possibly transformed body back into the bind.  */
  gimple_bind_set_body (bind, new_body);
  return vars;
}

/* Helper function for places where we construct data regions.  Wraps the BODY
   inside a try-finally construct at LOC that calls __builtin_GOACC_data_end
   in its cleanup block.  Returns this try statement.  */

static gimple *
make_data_region_try_statement (location_t loc, gimple *body)
{
  tree data_end_fn = builtin_decl_explicit (BUILT_IN_GOACC_DATA_END);
  gimple *call = gimple_build_call (data_end_fn, 0);
  gimple_seq cleanup = NULL;
  gimple_seq_add_stmt (&cleanup, call);
  gimple *try_stmt = gimple_build_try (body, cleanup, GIMPLE_TRY_FINALLY);
  gimple_set_location (body, loc);
  return try_stmt;
}

/* If INNER_BIND_VARS holds variables, build an OpenACC data region with
   location LOC containing BODY and having "create(var)" clauses for each
   variable.  If INNER_CLEANUP is present, add a try-finally statement with
   this cleanup code in the finally block.  Return the new data region, or
   the original BODY if no data region was needed.  */

static gimple *
maybe_build_inner_data_region (location_t loc, gimple *body,
                               tree inner_bind_vars, gimple *inner_cleanup)
{
  /* Build data "create(var)" clauses for these local variables.
     Below we will add these to a data region enclosing the entire body
     of the decomposed kernels region.  */
  tree prev_mapped_var = NULL, next = NULL, artificial_vars = NULL,
       inner_data_clauses = NULL;
  for (tree v = inner_bind_vars; v; v = next)
    {
      next = TREE_CHAIN (v);
      if (DECL_ARTIFICIAL (v)
          || TREE_CODE (v) == CONST_DECL
          || (DECL_LANG_SPECIFIC (current_function_decl)
              && DECL_TEMPLATE_INSTANTIATION (current_function_decl)))
        {
          /* If this is an artificial temporary, it need not be mapped.  We
             move its declaration into the bind inside the data region.
             Also avoid mapping variables if we are inside a template
             instantiation; the code does not contain all the copies to
             temporaries that would make this legal.  */
          TREE_CHAIN (v) = artificial_vars;
          artificial_vars = v;
          if (prev_mapped_var != NULL)
            TREE_CHAIN (prev_mapped_var) = next;
          else
            inner_bind_vars = next;
        }
      else
        {
          /* Otherwise, build the map clause.  */
          tree new_clause = build_omp_clause (loc, OMP_CLAUSE_MAP);
          OMP_CLAUSE_SET_MAP_KIND (new_clause, GOMP_MAP_ALLOC);
          OMP_CLAUSE_DECL (new_clause) = v;
          OMP_CLAUSE_SIZE (new_clause) = DECL_SIZE_UNIT (v);
          OMP_CLAUSE_CHAIN (new_clause) = inner_data_clauses;
          inner_data_clauses = new_clause;

          prev_mapped_var = v;
        }
    }

  if (artificial_vars)
    body = gimple_build_bind (artificial_vars, body, make_node (BLOCK));

  /* If we determined above that there are variables that need to be created
     on the device, construct a data region for them and wrap the body
     inside that.  */
  if (inner_data_clauses != NULL)
    {
      gcc_assert (inner_bind_vars != NULL);
      gimple *inner_data_region
        = gimple_build_omp_target (NULL, GF_OMP_TARGET_KIND_OACC_DATA_KERNELS,
                                   inner_data_clauses);
      gimple_set_location (inner_data_region, loc);
      /* Make sure __builtin_GOACC_data_end is called at the end.  */
      gimple *try_stmt = make_data_region_try_statement (loc, body);
      gimple_omp_set_body (inner_data_region, try_stmt);
      gimple *bind_body;
      if (inner_cleanup != NULL)
          /* Clobber all the inner variables that need to be clobbered.  */
          bind_body = gimple_build_try (inner_data_region, inner_cleanup,
                                        GIMPLE_TRY_FINALLY);
      else
          bind_body = inner_data_region;
      body = gimple_build_bind (inner_bind_vars, bind_body, make_node (BLOCK));
    }

  return body;
}

/* Decompose the body of the KERNELS_REGION, which was originally annotated
   with the KERNELS_CLAUSES, into a series of parallel regions.  */

static gimple *
decompose_kernels_region_body (gimple *kernels_region, tree kernels_clauses)
{
  location_t loc = gimple_location (kernels_region);

  /* The kernels clauses will be propagated to the child clauses unmodified,
     except that that num_gangs clause will only be added to loop regions.
     The other regions are "gang-single" and get an explicit num_gangs(1)
     clause.  So separate out the num_gangs clause here.  */
  tree num_gangs_clause = NULL, prev_clause = NULL;
  tree parallel_clauses = kernels_clauses;
  for (tree c = parallel_clauses; c; c = OMP_CLAUSE_CHAIN (c))
    {
      if (OMP_CLAUSE_CODE (c) == OMP_CLAUSE_NUM_GANGS)
        {
          /* Cut this clause out of the chain.  */
          num_gangs_clause = c;
          if (prev_clause != NULL)
            OMP_CLAUSE_CHAIN (prev_clause) = OMP_CLAUSE_CHAIN (c);
          else
            kernels_clauses = OMP_CLAUSE_CHAIN (c);
          OMP_CLAUSE_CHAIN (num_gangs_clause) = NULL;
          break;
        }
      else
        prev_clause = c;
    }

  gimple *kernels_body = gimple_omp_body (kernels_region);
  gbind *kernels_bind = as_a <gbind *> (kernels_body);

  /* The body of the region may contain other nested binds declaring inner
     local variables.  Collapse all these binds into one to ensure that we
     have a single sequence of statements to iterate over; also, collect all
     inner variables.  */
  tree inner_bind_vars = flatten_binds (kernels_bind);
  gimple_seq body_sequence = gimple_bind_body (kernels_bind);

  /* All these inner variables will get allocated on the device (below, by
     calling maybe_build_inner_data_region).  Here we create "present"
     clauses for them and add these clauses to the list of clauses to be
     attached to each inner parallel region.  */
  tree present_clauses = kernels_clauses;
  for (tree var = inner_bind_vars; var; var = TREE_CHAIN (var))
    {
      if (!DECL_ARTIFICIAL (var) && TREE_CODE (var) != CONST_DECL)
        {
          tree present_clause = build_omp_clause (loc, OMP_CLAUSE_MAP);
          OMP_CLAUSE_SET_MAP_KIND (present_clause, GOMP_MAP_FORCE_PRESENT);
          OMP_CLAUSE_DECL (present_clause) = var;
          OMP_CLAUSE_SIZE (present_clause) = DECL_SIZE_UNIT (var);
          OMP_CLAUSE_CHAIN (present_clause) = present_clauses;
          present_clauses = present_clause;
        }
    }
  kernels_clauses = present_clauses;

  /* In addition to nested binds, the "real" body of the region may be
     nested inside a try-finally block.  Find its cleanup block, which
     contains code to clobber the local variables that must be clobbered.  */
  gimple *inner_cleanup = NULL;
  if (body_sequence != NULL && gimple_code (body_sequence) == GIMPLE_TRY)
    {
      if (gimple_seq_singleton_p (body_sequence))
        {
          /* The try statement is the only thing inside the bind.  */
          inner_cleanup = gimple_try_cleanup (body_sequence);
          body_sequence = gimple_try_eval (body_sequence);
        }
      else
        {
          /* The bind's body starts with a try statement, but it is followed
             by other things.  */
          gimple_stmt_iterator gsi = gsi_start (body_sequence);
          gimple *try_stmt = gsi_stmt (gsi);
          inner_cleanup = gimple_try_cleanup (try_stmt);
          gimple *try_body = gimple_try_eval (try_stmt);

          gsi_remove (&gsi, false);
          /* Now gsi indicates the sequence of statements after the try
             statement in the bind.  Append the statement in the try body and
             the trailing statements from gsi.  */
          gsi_insert_seq_before (&gsi, try_body, GSI_CONTINUE_LINKING);
          body_sequence = gsi_stmt (gsi);
        }
    }

  /* This sequence will collect all the top-level statements in the body of
     the data region we are about to construct.  */
  gimple_seq region_body = NULL;
  /* This sequence will collect consecutive statements to be put into a
     gang-single region.  */
  gimple_seq gang_single_seq = NULL;
  /* Flag recording whether the gang_single_seq only contains copies to
     local variables.  These may be loop setup code that should not be
     separated from the loop.  */
  bool only_simple_assignments = true;

  /* Iterate over the statements in the kernels region's body.  */
  gimple_stmt_iterator gsi, gsi_n;
  for (gsi = gsi_start (body_sequence); !gsi_end_p (gsi); gsi = gsi_n)
    {
      /* Advance the iterator here because otherwise it would be invalidated
         by moving statements below.  */
      gsi_n = gsi;
      gsi_next (&gsi_n);

      gimple *stmt = gsi_stmt (gsi);
      gimple *omp_for = top_level_omp_for_in_stmt (stmt);
      if (omp_for != NULL)
        {
          /* This is an OMP for statement, put it into a parallel region.
             But first, construct a gang-single region containing any
             complex sequential statements we may have seen.  */
          if (gang_single_seq != NULL && !only_simple_assignments)
            {
              gimple *single_region
                = make_gang_single_region (loc, gang_single_seq,
                                           kernels_clauses);
              gimple_seq_add_stmt (&region_body, single_region);
            }
          else if (gang_single_seq != NULL && only_simple_assignments)
            {
              /* There is a sequence of sequential statements preceding this
                 loop, but they are all simple assignments.  This is
                 probably setup code for the loop; in particular, Fortran DO
                 loops are preceded by code to copy the loop limit variable
                 to a temporary.  Group this code together with the loop
                 itself.  */
              gimple_seq_add_stmt (&gang_single_seq, stmt);
              stmt = gimple_build_bind (NULL, gang_single_seq,
                                        make_node (BLOCK));
            }
          gang_single_seq = NULL;
          only_simple_assignments = true;

          gimple *parallel_region
            = make_gang_parallel_loop_region (omp_for, stmt,
                                              num_gangs_clause,
                                              kernels_clauses);
          gimple_seq_add_stmt (&region_body, parallel_region);
        }
      else
        {
          /* This is not an OMP for statement, so it will be put into a
             gang-single region.  */
          gimple_seq_add_stmt (&gang_single_seq, stmt);
          /* Is this a simple assignment? We call it simple if it is an
             assignment to an artificial local variable.  This captures
             Fortran loop setup code computing loop bounds and offsets.  */
          bool is_simple_assignment
            = (gimple_code (stmt) == GIMPLE_ASSIGN
                && TREE_CODE (gimple_assign_lhs (stmt)) == VAR_DECL
                && DECL_ARTIFICIAL (gimple_assign_lhs (stmt)));
          if (!is_simple_assignment)
            only_simple_assignments = false;
        }
    }

  /* If we did not emit a new region, and are not going to emit one now
     (that is, the original region was empty), prepare to emit a dummy so as
     to preserve the original construct, which other processing (at least
     test cases) depend on.  */
  if (region_body == NULL && gang_single_seq == NULL)
    {
      gimple *stmt = gimple_build_nop ();
      gimple_set_location (stmt, loc);
      gimple_seq_add_stmt (&gang_single_seq, stmt);
    }

  /* Gather up any remaining gang-single statements.  */
  if (gang_single_seq != NULL)
    {
      gimple *single_region
        = make_gang_single_region (loc, gang_single_seq, kernels_clauses);
      gimple_seq_add_stmt (&region_body, single_region);
    }

  tree kernels_locals = gimple_bind_vars (as_a <gbind *> (kernels_body));
  gimple *body = gimple_build_bind (kernels_locals, region_body,
                                    make_node (BLOCK));

  /* If we found variables declared in nested scopes, build a data region to
     map them to the device.  */
  body = maybe_build_inner_data_region (loc, body, inner_bind_vars,
                                        inner_cleanup);

  return body;
}

/* Transform KERNELS_REGION, which is an OpenACC kernels region, into a data
   region containing the original kernels region's body cut up into a
   sequence of parallel regions.  */

static gimple *
transform_kernels_region (gimple *kernels_region)
{
  gcc_checking_assert (gimple_omp_target_kind (kernels_region)
                        == GF_OMP_TARGET_KIND_OACC_KERNELS);
  location_t loc = gimple_location (kernels_region);

  /* Collect the kernels region's data clauses and create the new data
     region with those clauses.  */
  tree kernels_clauses = gimple_omp_target_clauses (kernels_region);
  tree data_clauses = NULL;
  for (tree c = kernels_clauses; c; c = OMP_CLAUSE_CHAIN (c))
    {
      /* Certain map clauses are copied to the enclosing data region.  Any
         non-data clause remains on the kernels region.  */
      if (OMP_CLAUSE_CODE (c) == OMP_CLAUSE_MAP)
        {
          tree decl = OMP_CLAUSE_DECL (c);
          HOST_WIDE_INT kind = OMP_CLAUSE_MAP_KIND (c);
          switch (kind)
            {
            default:
              if (kind == GOMP_MAP_ALLOC &&
                  integer_zerop (OMP_CLAUSE_SIZE (c)))
                /* ??? This is an alloc clause for mapping a pointer whose
                   target is already mapped.  We leave these on the inner
                   parallel regions because moving them to the outer data
                   region causes runtime errors.  */
                break;

              /* For non-artificial variables, and for non-declaration
                 expressions like A[0:n], copy the clause to the data
                 region.  */
              if ((DECL_P (decl) && !DECL_ARTIFICIAL (decl))
                  || !DECL_P (decl))
                {
                  tree new_clause = build_omp_clause (OMP_CLAUSE_LOCATION (c),
                                                      OMP_CLAUSE_MAP);
                  OMP_CLAUSE_SET_MAP_KIND (new_clause, kind);
                  /* This must be unshared here to avoid "incorrect sharing
                     of tree nodes" errors from verify_gimple.  */
                  OMP_CLAUSE_DECL (new_clause) = unshare_expr (decl);
                  OMP_CLAUSE_SIZE (new_clause) = OMP_CLAUSE_SIZE (c);
                  OMP_CLAUSE_CHAIN (new_clause) = data_clauses;
                  data_clauses = new_clause;

                  /* Now that this data is mapped, the inner data clause on
                     the kernels region can become a present clause.  */
                  OMP_CLAUSE_SET_MAP_KIND (c, GOMP_MAP_FORCE_PRESENT);
                }
              break;

            case GOMP_MAP_POINTER:
            case GOMP_MAP_TO_PSET:
            case GOMP_MAP_FORCE_TOFROM:
            case GOMP_MAP_FIRSTPRIVATE_POINTER:
            case GOMP_MAP_FIRSTPRIVATE_REFERENCE:
              /* ??? Copying these map kinds leads to internal compiler
                 errors in later passes.  */
              break;
            }
        }
      else if (OMP_CLAUSE_CODE (c) == OMP_CLAUSE_IF)
        {
          /* If there is an if clause, it must also be present on the
             enclosing data region.  Temporarily remove the if clause's
             chain to avoid copying it.  */
          tree saved_chain = OMP_CLAUSE_CHAIN (c);
          OMP_CLAUSE_CHAIN (c) = NULL;
          tree new_if_clause = unshare_expr (c);
          OMP_CLAUSE_CHAIN (c) = saved_chain;
          OMP_CLAUSE_CHAIN (new_if_clause) = data_clauses;
          data_clauses = new_if_clause;
        }
    }
  /* Restore the original order of the clauses.  */
  data_clauses = nreverse (data_clauses);

  gimple *data_region
    = gimple_build_omp_target (NULL, GF_OMP_TARGET_KIND_OACC_DATA_KERNELS,
                               data_clauses);
  gimple_set_location (data_region, loc);

  /* Transform the body of the kernels region into a sequence of parallel
     regions.  */
  gimple *body = decompose_kernels_region_body (kernels_region,
                                                kernels_clauses);

  /* Put the transformed pieces together.  The entire body of the region is
     wrapped in a try-finally statement that calls __builtin_GOACC_data_end
     for cleanup.  */
  gimple *try_stmt = make_data_region_try_statement (loc, body);
  gimple_omp_set_body (data_region, try_stmt);

  return data_region;
}

/* Helper function of convert_oacc_kernels for walking the tree, calling
   transform_kernels_region on each kernels region found.  */

static tree
scan_kernels (gimple_stmt_iterator *gsi_p, bool *handled_ops_p,
              struct walk_stmt_info *)
{
  gimple *stmt = gsi_stmt (*gsi_p);
  *handled_ops_p = false;

  int kind;
  switch (gimple_code (stmt))
    {
    case GIMPLE_OMP_TARGET:
      kind = gimple_omp_target_kind (stmt);
      if (kind == GF_OMP_TARGET_KIND_OACC_KERNELS)
        {
          gimple *new_region = transform_kernels_region (stmt);
          gsi_replace (gsi_p, new_region, false);
          *handled_ops_p = true;
        }
      break;

    default:
      break;
    }

  return NULL;
}

/* Find and transform OpenACC kernels regions in the current function.  */

static unsigned int
convert_oacc_kernels (void)
{
  struct walk_stmt_info wi;
  gimple_seq body = gimple_body (current_function_decl);

  memset (&wi, 0, sizeof (wi));
  walk_gimple_seq_mod (&body, scan_kernels, NULL, &wi);

  gimple_set_body (current_function_decl, body);

  return 0;
}

namespace {

const pass_data pass_data_convert_oacc_kernels =
{
  GIMPLE_PASS, /* type */
  "convert_oacc_kernels", /* name */
  OPTGROUP_OMP, /* optinfo_flags */
  TV_NONE, /* tv_id */
  PROP_gimple_any, /* properties_required */
  0, /* properties_provided */
  0, /* properties_destroyed */
  0, /* todo_flags_start */
  0, /* todo_flags_finish */
};

class pass_convert_oacc_kernels : public gimple_opt_pass
{
public:
  pass_convert_oacc_kernels (gcc::context *ctxt)
    : gimple_opt_pass (pass_data_convert_oacc_kernels, ctxt)
  {}

  /* opt_pass methods: */
  virtual bool gate (function *)
  {
    return (flag_openacc
	    && flag_openacc_kernels == OPENACC_KERNELS_SPLIT);
  }
  virtual unsigned int execute (function *)
  {
    return convert_oacc_kernels ();
  }

}; // class pass_convert_oacc_kernels

} // anon namespace

gimple_opt_pass *
make_pass_convert_oacc_kernels (gcc::context *ctxt)
{
  return new pass_convert_oacc_kernels (ctxt);
}
