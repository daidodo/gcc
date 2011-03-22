/* Definitions of target machine for GNU compiler, for DEC Alpha.
   Copyright (C) 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
   2000, 2001, 2002, 2004, 2005, 2007, 2008, 2009, 2010
   Free Software Foundation, Inc.
   Contributed by Richard Kenner (kenner@vlsi1.ultra.nyu.edu)

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/* Target CPU builtins.  */
#define TARGET_CPU_CPP_BUILTINS()			\
  do							\
    {							\
	builtin_define ("__alpha");			\
	builtin_define ("__alpha__");			\
	builtin_assert ("cpu=alpha");			\
	builtin_assert ("machine=alpha");		\
	if (TARGET_CIX)					\
	  {						\
	    builtin_define ("__alpha_cix__");		\
	    builtin_assert ("cpu=cix");			\
	  }						\
	if (TARGET_FIX)					\
	  {						\
	    builtin_define ("__alpha_fix__");		\
	    builtin_assert ("cpu=fix");			\
	  }						\
	if (TARGET_BWX)					\
	  {						\
	    builtin_define ("__alpha_bwx__");		\
	    builtin_assert ("cpu=bwx");			\
	  }						\
	if (TARGET_MAX)					\
	  {						\
	    builtin_define ("__alpha_max__");		\
	    builtin_assert ("cpu=max");			\
	  }						\
	if (alpha_cpu == PROCESSOR_EV6)			\
	  {						\
	    builtin_define ("__alpha_ev6__");		\
	    builtin_assert ("cpu=ev6");			\
	  }						\
	else if (alpha_cpu == PROCESSOR_EV5)		\
	  {						\
	    builtin_define ("__alpha_ev5__");		\
	    builtin_assert ("cpu=ev5");			\
	  }						\
	else	/* Presumably ev4.  */			\
	  {						\
	    builtin_define ("__alpha_ev4__");		\
	    builtin_assert ("cpu=ev4");			\
	  }						\
	if (TARGET_IEEE || TARGET_IEEE_WITH_INEXACT)	\
	  builtin_define ("_IEEE_FP");			\
	if (TARGET_IEEE_WITH_INEXACT)			\
	  builtin_define ("_IEEE_FP_INEXACT");		\
	if (TARGET_LONG_DOUBLE_128)			\
	  builtin_define ("__LONG_DOUBLE_128__");	\
							\
	/* Macros dependent on the C dialect.  */	\
	SUBTARGET_LANGUAGE_CPP_BUILTINS();		\
} while (0)

#ifndef SUBTARGET_LANGUAGE_CPP_BUILTINS
#define SUBTARGET_LANGUAGE_CPP_BUILTINS()		\
  do							\
    {							\
      if (preprocessing_asm_p ())			\
	builtin_define_std ("LANGUAGE_ASSEMBLY");	\
      else if (c_dialect_cxx ())			\
	{						\
	  builtin_define ("__LANGUAGE_C_PLUS_PLUS");	\
	  builtin_define ("__LANGUAGE_C_PLUS_PLUS__");	\
	}						\
      else						\
	builtin_define_std ("LANGUAGE_C");		\
      if (c_dialect_objc ())				\
	{						\
	  builtin_define ("__LANGUAGE_OBJECTIVE_C");	\
	  builtin_define ("__LANGUAGE_OBJECTIVE_C__");	\
	}						\
    }							\
  while (0)
#endif

/* Print subsidiary information on the compiler version in use.  */
#define TARGET_VERSION

/* Run-time compilation parameters selecting different hardware subsets.  */

/* Which processor to schedule for. The cpu attribute defines a list that
   mirrors this list, so changes to alpha.md must be made at the same time.  */

enum processor_type
{
  PROCESSOR_EV4,			/* 2106[46]{a,} */
  PROCESSOR_EV5,			/* 21164{a,pc,} */
  PROCESSOR_EV6,			/* 21264 */
  PROCESSOR_MAX
};

extern enum processor_type alpha_cpu;
extern enum processor_type alpha_tune;

enum alpha_trap_precision
{
  ALPHA_TP_PROG,	/* No precision (default).  */
  ALPHA_TP_FUNC,      	/* Trap contained within originating function.  */
  ALPHA_TP_INSN		/* Instruction accuracy and code is resumption safe.  */
};

enum alpha_fp_rounding_mode
{
  ALPHA_FPRM_NORM,	/* Normal rounding mode.  */
  ALPHA_FPRM_MINF,	/* Round towards minus-infinity.  */
  ALPHA_FPRM_CHOP,	/* Chopped rounding mode (towards 0).  */
  ALPHA_FPRM_DYN	/* Dynamic rounding mode.  */
};

enum alpha_fp_trap_mode
{
  ALPHA_FPTM_N,		/* Normal trap mode.  */
  ALPHA_FPTM_U,		/* Underflow traps enabled.  */
  ALPHA_FPTM_SU,	/* Software completion, w/underflow traps */
  ALPHA_FPTM_SUI	/* Software completion, w/underflow & inexact traps */
};

extern enum alpha_trap_precision alpha_tp;
extern enum alpha_fp_rounding_mode alpha_fprm;
extern enum alpha_fp_trap_mode alpha_fptm;

/* Invert the easy way to make options work.  */
#define TARGET_FP	(!TARGET_SOFT_FP)

/* These are for target os support and cannot be changed at runtime.  */
#define TARGET_ABI_OPEN_VMS	0
#define TARGET_ABI_OSF		(!TARGET_ABI_OPEN_VMS)

#ifndef TARGET_AS_CAN_SUBTRACT_LABELS
#define TARGET_AS_CAN_SUBTRACT_LABELS TARGET_GAS
#endif
#ifndef TARGET_AS_SLASH_BEFORE_SUFFIX
#define TARGET_AS_SLASH_BEFORE_SUFFIX TARGET_GAS
#endif
#ifndef TARGET_CAN_FAULT_IN_PROLOGUE
#define TARGET_CAN_FAULT_IN_PROLOGUE 0
#endif
#ifndef TARGET_HAS_XFLOATING_LIBS
#define TARGET_HAS_XFLOATING_LIBS TARGET_LONG_DOUBLE_128
#endif
#ifndef TARGET_PROFILING_NEEDS_GP
#define TARGET_PROFILING_NEEDS_GP 0
#endif
#ifndef TARGET_LD_BUGGY_LDGP
#define TARGET_LD_BUGGY_LDGP 0
#endif
#ifndef TARGET_FIXUP_EV5_PREFETCH
#define TARGET_FIXUP_EV5_PREFETCH 0
#endif
#ifndef HAVE_AS_TLS
#define HAVE_AS_TLS 0
#endif

#define TARGET_DEFAULT MASK_FPREGS

#ifndef TARGET_CPU_DEFAULT
#define TARGET_CPU_DEFAULT 0
#endif

#ifndef TARGET_DEFAULT_EXPLICIT_RELOCS
#ifdef HAVE_AS_EXPLICIT_RELOCS
#define TARGET_DEFAULT_EXPLICIT_RELOCS MASK_EXPLICIT_RELOCS
#define TARGET_SUPPORT_ARCH 1
#else
#define TARGET_DEFAULT_EXPLICIT_RELOCS 0
#endif
#endif

#ifndef TARGET_SUPPORT_ARCH
#define TARGET_SUPPORT_ARCH 0
#endif

/* Support for a compile-time default CPU, et cetera.  The rules are:
   --with-cpu is ignored if -mcpu is specified.
   --with-tune is ignored if -mtune is specified.  */
#define OPTION_DEFAULT_SPECS \
  {"cpu", "%{!mcpu=*:-mcpu=%(VALUE)}" }, \
  {"tune", "%{!mtune=*:-mtune=%(VALUE)}" }


/* target machine storage layout */

/* Define the size of `int'.  The default is the same as the word size.  */
#define INT_TYPE_SIZE 32

/* Define the size of `long long'.  The default is the twice the word size.  */
#define LONG_LONG_TYPE_SIZE 64

/* The two floating-point formats we support are S-floating, which is
   4 bytes, and T-floating, which is 8 bytes.  `float' is S and `double'
   and `long double' are T.  */

#define FLOAT_TYPE_SIZE 32
#define DOUBLE_TYPE_SIZE 64
#define LONG_DOUBLE_TYPE_SIZE (TARGET_LONG_DOUBLE_128 ? 128 : 64)

/* Define this to set long double type size to use in libgcc2.c, which can
   not depend on target_flags.  */
#ifdef __LONG_DOUBLE_128__
#define LIBGCC2_LONG_DOUBLE_TYPE_SIZE 128
#else
#define LIBGCC2_LONG_DOUBLE_TYPE_SIZE 64
#endif

/* Work around target_flags dependency in ada/targtyps.c.  */
#define WIDEST_HARDWARE_FP_SIZE 64

#define	WCHAR_TYPE "unsigned int"
#define	WCHAR_TYPE_SIZE 32

/* Define this macro if it is advisable to hold scalars in registers
   in a wider mode than that declared by the program.  In such cases,
   the value is constrained to be within the bounds of the declared
   type, but kept valid in the wider mode.  The signedness of the
   extension may differ from that of the type.

   For Alpha, we always store objects in a full register.  32-bit integers
   are always sign-extended, but smaller objects retain their signedness.

   Note that small vector types can get mapped onto integer modes at the
   whim of not appearing in alpha-modes.def.  We never promoted these
   values before; don't do so now that we've trimmed the set of modes to
   those actually implemented in the backend.  */

#define PROMOTE_MODE(MODE,UNSIGNEDP,TYPE)			\
  if (GET_MODE_CLASS (MODE) == MODE_INT				\
      && (TYPE == NULL || TREE_CODE (TYPE) != VECTOR_TYPE)	\
      && GET_MODE_SIZE (MODE) < UNITS_PER_WORD)			\
    {								\
      if ((MODE) == SImode)					\
	(UNSIGNEDP) = 0;					\
      (MODE) = DImode;						\
    }

/* Define this if most significant bit is lowest numbered
   in instructions that operate on numbered bit-fields.

   There are no such instructions on the Alpha, but the documentation
   is little endian.  */
#define BITS_BIG_ENDIAN 0

/* Define this if most significant byte of a word is the lowest numbered.
   This is false on the Alpha.  */
#define BYTES_BIG_ENDIAN 0

/* Define this if most significant word of a multiword number is lowest
   numbered.

   For Alpha we can decide arbitrarily since there are no machine instructions
   for them.  Might as well be consistent with bytes.  */
#define WORDS_BIG_ENDIAN 0

/* Width of a word, in units (bytes).  */
#define UNITS_PER_WORD 8

/* Width in bits of a pointer.
   See also the macro `Pmode' defined below.  */
#define POINTER_SIZE 64

/* Allocation boundary (in *bits*) for storing arguments in argument list.  */
#define PARM_BOUNDARY 64

/* Boundary (in *bits*) on which stack pointer should be aligned.  */
#define STACK_BOUNDARY 128

/* Allocation boundary (in *bits*) for the code of a function.  */
#define FUNCTION_BOUNDARY 32

/* Alignment of field after `int : 0' in a structure.  */
#define EMPTY_FIELD_BOUNDARY 64

/* Every structure's size must be a multiple of this.  */
#define STRUCTURE_SIZE_BOUNDARY 8

/* A bit-field declared as `int' forces `int' alignment for the struct.  */
#define PCC_BITFIELD_TYPE_MATTERS 1

/* No data type wants to be aligned rounder than this.  */
#define BIGGEST_ALIGNMENT 128

/* For atomic access to objects, must have at least 32-bit alignment
   unless the machine has byte operations.  */
#define MINIMUM_ATOMIC_ALIGNMENT ((unsigned int) (TARGET_BWX ? 8 : 32))

/* Align all constants and variables to at least a word boundary so
   we can pick up pieces of them faster.  */
/* ??? Only if block-move stuff knows about different source/destination
   alignment.  */
#if 0
#define CONSTANT_ALIGNMENT(EXP, ALIGN) MAX ((ALIGN), BITS_PER_WORD)
#define DATA_ALIGNMENT(EXP, ALIGN) MAX ((ALIGN), BITS_PER_WORD)
#endif

/* Set this nonzero if move instructions will actually fail to work
   when given unaligned data.

   Since we get an error message when we do one, call them invalid.  */

#define STRICT_ALIGNMENT 1

/* Set this nonzero if unaligned move instructions are extremely slow.

   On the Alpha, they trap.  */

#define SLOW_UNALIGNED_ACCESS(MODE, ALIGN) 1

/* Standard register usage.  */

/* Number of actual hardware registers.
   The hardware registers are assigned numbers for the compiler
   from 0 to just below FIRST_PSEUDO_REGISTER.
   All registers that the compiler knows about must be given numbers,
   even those that are not normally considered general registers.

   We define all 32 integer registers, even though $31 is always zero,
   and all 32 floating-point registers, even though $f31 is also
   always zero.  We do not bother defining the FP status register and
   there are no other registers.

   Since $31 is always zero, we will use register number 31 as the
   argument pointer.  It will never appear in the generated code
   because we will always be eliminating it in favor of the stack
   pointer or hardware frame pointer.

   Likewise, we use $f31 for the frame pointer, which will always
   be eliminated in favor of the hardware frame pointer or the
   stack pointer.  */

#define FIRST_PSEUDO_REGISTER 64

/* 1 for registers that have pervasive standard uses
   and are not available for the register allocator.  */

#define FIXED_REGISTERS  \
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, \
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }

/* 1 for registers not available across function calls.
   These must include the FIXED_REGISTERS and also any
   registers that can be used without being saved.
   The latter must include the registers where values are returned
   and the register where structure-value addresses are passed.
   Aside from that, you can include as many other registers as you like.  */
#define CALL_USED_REGISTERS  \
 {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, \
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, \
  1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, \
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }

/* List the order in which to allocate registers.  Each register must be
   listed once, even those in FIXED_REGISTERS.  */

#define REG_ALLOC_ORDER { \
   1, 2, 3, 4, 5, 6, 7, 8,	/* nonsaved integer registers */	\
   22, 23, 24, 25, 28,		/* likewise */				\
   0,				/* likewise, but return value */	\
   21, 20, 19, 18, 17, 16,	/* likewise, but input args */		\
   27,				/* likewise, but OSF procedure value */	\
									\
   42, 43, 44, 45, 46, 47,	/* nonsaved floating-point registers */	\
   54, 55, 56, 57, 58, 59,	/* likewise */				\
   60, 61, 62,			/* likewise */				\
   32, 33,			/* likewise, but return values */	\
   53, 52, 51, 50, 49, 48,	/* likewise, but input args */		\
									\
   9, 10, 11, 12, 13, 14,	/* saved integer registers */		\
   26,				/* return address */			\
   15,				/* hard frame pointer */		\
									\
   34, 35, 36, 37, 38, 39,	/* saved floating-point registers */	\
   40, 41,			/* likewise */				\
									\
   29, 30, 31, 63		/* gp, sp, ap, sfp */			\
}

/* Return number of consecutive hard regs needed starting at reg REGNO
   to hold something of mode MODE.
   This is ordinarily the length in words of a value of mode MODE
   but can be less for certain modes in special long registers.  */

#define HARD_REGNO_NREGS(REGNO, MODE)   \
  ((GET_MODE_SIZE (MODE) + UNITS_PER_WORD - 1) / UNITS_PER_WORD)

/* Value is 1 if hard register REGNO can hold a value of machine-mode MODE.
   On Alpha, the integer registers can hold any mode.  The floating-point
   registers can hold 64-bit integers as well, but not smaller values.  */

#define HARD_REGNO_MODE_OK(REGNO, MODE) 				\
  (IN_RANGE ((REGNO), 32, 62)						\
   ? (MODE) == SFmode || (MODE) == DFmode || (MODE) == DImode		\
     || (MODE) == SCmode || (MODE) == DCmode				\
   : 1)

/* A C expression that is nonzero if a value of mode
   MODE1 is accessible in mode MODE2 without copying.

   This asymmetric test is true when MODE1 could be put
   in an FP register but MODE2 could not.  */

#define MODES_TIEABLE_P(MODE1, MODE2) 				\
  (HARD_REGNO_MODE_OK (32, (MODE1))				\
   ? HARD_REGNO_MODE_OK (32, (MODE2))				\
   : 1)

/* Specify the registers used for certain standard purposes.
   The values of these macros are register numbers.  */

/* Alpha pc isn't overloaded on a register that the compiler knows about.  */
/* #define PC_REGNUM  */

/* Register to use for pushing function arguments.  */
#define STACK_POINTER_REGNUM 30

/* Base register for access to local variables of the function.  */
#define HARD_FRAME_POINTER_REGNUM 15

/* Base register for access to arguments of the function.  */
#define ARG_POINTER_REGNUM 31

/* Base register for access to local variables of function.  */
#define FRAME_POINTER_REGNUM 63

/* Register in which static-chain is passed to a function.

   For the Alpha, this is based on an example; the calling sequence
   doesn't seem to specify this.  */
#define STATIC_CHAIN_REGNUM 1

/* The register number of the register used to address a table of
   static data addresses in memory.  */
#define PIC_OFFSET_TABLE_REGNUM 29

/* Define this macro if the register defined by `PIC_OFFSET_TABLE_REGNUM'
   is clobbered by calls.  */
/* ??? It is and it isn't.  It's required to be valid for a given
   function when the function returns.  It isn't clobbered by
   current_file functions.  Moreover, we do not expose the ldgp
   until after reload, so we're probably safe.  */
/* #define PIC_OFFSET_TABLE_REG_CALL_CLOBBERED */

/* Define the classes of registers for register constraints in the
   machine description.  Also define ranges of constants.

   One of the classes must always be named ALL_REGS and include all hard regs.
   If there is more than one class, another class must be named NO_REGS
   and contain no registers.

   The name GENERAL_REGS must be the name of a class (or an alias for
   another name such as ALL_REGS).  This is the class of registers
   that is allowed by "g" or "r" in a register constraint.
   Also, registers outside this class are allocated only when
   instructions express preferences for them.

   The classes must be numbered in nondecreasing order; that is,
   a larger-numbered class must never be contained completely
   in a smaller-numbered class.

   For any two classes, it is very desirable that there be another
   class that represents their union.  */

enum reg_class {
  NO_REGS, R0_REG, R24_REG, R25_REG, R27_REG,
  GENERAL_REGS, FLOAT_REGS, ALL_REGS,
  LIM_REG_CLASSES
};

#define N_REG_CLASSES (int) LIM_REG_CLASSES

/* Give names of register classes as strings for dump file.  */

#define REG_CLASS_NAMES					\
 {"NO_REGS", "R0_REG", "R24_REG", "R25_REG", "R27_REG",	\
  "GENERAL_REGS", "FLOAT_REGS", "ALL_REGS" }

/* Define which registers fit in which classes.
   This is an initializer for a vector of HARD_REG_SET
   of length N_REG_CLASSES.  */

#define REG_CLASS_CONTENTS				\
{ {0x00000000, 0x00000000},	/* NO_REGS */		\
  {0x00000001, 0x00000000},	/* R0_REG */		\
  {0x01000000, 0x00000000},	/* R24_REG */		\
  {0x02000000, 0x00000000},	/* R25_REG */		\
  {0x08000000, 0x00000000},	/* R27_REG */		\
  {0xffffffff, 0x80000000},	/* GENERAL_REGS */	\
  {0x00000000, 0x7fffffff},	/* FLOAT_REGS */	\
  {0xffffffff, 0xffffffff} }

/* The following macro defines cover classes for Integrated Register
   Allocator.  Cover classes is a set of non-intersected register
   classes covering all hard registers used for register allocation
   purpose.  Any move between two registers of a cover class should be
   cheaper than load or store of the registers.  The macro value is
   array of register classes with LIM_REG_CLASSES used as the end
   marker.  */

#define IRA_COVER_CLASSES						     \
{									     \
  GENERAL_REGS, FLOAT_REGS, LIM_REG_CLASSES				     \
}

/* The same information, inverted:
   Return the class number of the smallest class containing
   reg number REGNO.  This could be a conditional expression
   or could index an array.  */

#define REGNO_REG_CLASS(REGNO)			\
 ((REGNO) == 0 ? R0_REG				\
  : (REGNO) == 24 ? R24_REG			\
  : (REGNO) == 25 ? R25_REG			\
  : (REGNO) == 27 ? R27_REG			\
  : IN_RANGE ((REGNO), 32, 62) ? FLOAT_REGS	\
  : GENERAL_REGS)

/* The class value for index registers, and the one for base regs.  */
#define INDEX_REG_CLASS NO_REGS
#define BASE_REG_CLASS GENERAL_REGS

/* Given an rtx X being reloaded into a reg required to be
   in class CLASS, return the class of reg to actually use.
   In general this is just CLASS; but on some machines
   in some cases it is preferable to use a more restrictive class.  */

#define PREFERRED_RELOAD_CLASS  alpha_preferred_reload_class

/* If we are copying between general and FP registers, we need a memory
   location unless the FIX extension is available.  */

#define SECONDARY_MEMORY_NEEDED(CLASS1,CLASS2,MODE) \
 (! TARGET_FIX && (((CLASS1) == FLOAT_REGS && (CLASS2) != FLOAT_REGS) \
                   || ((CLASS2) == FLOAT_REGS && (CLASS1) != FLOAT_REGS)))

/* Specify the mode to be used for memory when a secondary memory
   location is needed.  If MODE is floating-point, use it.  Otherwise,
   widen to a word like the default.  This is needed because we always
   store integers in FP registers in quadword format.  This whole
   area is very tricky! */
#define SECONDARY_MEMORY_NEEDED_MODE(MODE)		\
  (GET_MODE_CLASS (MODE) == MODE_FLOAT ? (MODE)		\
   : GET_MODE_SIZE (MODE) >= 4 ? (MODE)			\
   : mode_for_size (BITS_PER_WORD, GET_MODE_CLASS (MODE), 0))

/* Return the maximum number of consecutive registers
   needed to represent mode MODE in a register of class CLASS.  */

#define CLASS_MAX_NREGS(CLASS, MODE)				\
 ((GET_MODE_SIZE (MODE) + UNITS_PER_WORD - 1) / UNITS_PER_WORD)

/* Return the class of registers that cannot change mode from FROM to TO.  */

#define CANNOT_CHANGE_MODE_CLASS(FROM, TO, CLASS)		\
  (GET_MODE_SIZE (FROM) != GET_MODE_SIZE (TO)			\
   ? reg_classes_intersect_p (FLOAT_REGS, CLASS) : 0)

/* Define the cost of moving between registers of various classes.  Moving
   between FLOAT_REGS and anything else except float regs is expensive.
   In fact, we make it quite expensive because we really don't want to
   do these moves unless it is clearly worth it.  Optimizations may
   reduce the impact of not being able to allocate a pseudo to a
   hard register.  */

#define REGISTER_MOVE_COST(MODE, CLASS1, CLASS2)		\
  (((CLASS1) == FLOAT_REGS) == ((CLASS2) == FLOAT_REGS)	? 2	\
   : TARGET_FIX ? ((CLASS1) == FLOAT_REGS ? 6 : 8)		\
   : 4+2*alpha_memory_latency)

/* A C expressions returning the cost of moving data of MODE from a register to
   or from memory.

   On the Alpha, bump this up a bit.  */

extern int alpha_memory_latency;
#define MEMORY_MOVE_COST(MODE,CLASS,IN)  (2*alpha_memory_latency)

/* Provide the cost of a branch.  Exact meaning under development.  */
#define BRANCH_COST(speed_p, predictable_p) 5

/* Stack layout; function entry, exit and calling.  */

/* Define this if pushing a word on the stack
   makes the stack pointer a smaller address.  */
#define STACK_GROWS_DOWNWARD

/* Define this to nonzero if the nominal address of the stack frame
   is at the high-address end of the local variables;
   that is, each additional local variable allocated
   goes at a more negative offset in the frame.  */
/* #define FRAME_GROWS_DOWNWARD 0 */

/* Offset within stack frame to start allocating local variables at.
   If FRAME_GROWS_DOWNWARD, this is the offset to the END of the
   first local allocated.  Otherwise, it is the offset to the BEGINNING
   of the first local allocated.  */

#define STARTING_FRAME_OFFSET 0

/* If we generate an insn to push BYTES bytes,
   this says how many the stack pointer really advances by.
   On Alpha, don't define this because there are no push insns.  */
/*  #define PUSH_ROUNDING(BYTES) */

/* Define this to be nonzero if stack checking is built into the ABI.  */
#define STACK_CHECK_BUILTIN 1

/* Define this if the maximum size of all the outgoing args is to be
   accumulated and pushed during the prologue.  The amount can be
   found in the variable crtl->outgoing_args_size.  */
#define ACCUMULATE_OUTGOING_ARGS 1

/* Offset of first parameter from the argument pointer register value.  */

#define FIRST_PARM_OFFSET(FNDECL) 0

/* Definitions for register eliminations.

   We have two registers that can be eliminated on the Alpha.  First, the
   frame pointer register can often be eliminated in favor of the stack
   pointer register.  Secondly, the argument pointer register can always be
   eliminated; it is replaced with either the stack or frame pointer.  */

/* This is an array of structures.  Each structure initializes one pair
   of eliminable registers.  The "from" register number is given first,
   followed by "to".  Eliminations of the same "from" register are listed
   in order of preference.  */

#define ELIMINABLE_REGS				     \
{{ ARG_POINTER_REGNUM, STACK_POINTER_REGNUM},	     \
 { ARG_POINTER_REGNUM, HARD_FRAME_POINTER_REGNUM},   \
 { FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM},	     \
 { FRAME_POINTER_REGNUM, HARD_FRAME_POINTER_REGNUM}}

/* Round up to a multiple of 16 bytes.  */
#define ALPHA_ROUND(X) (((X) + 15) & ~ 15)

/* Define the offset between two registers, one to be eliminated, and the other
   its replacement, at the start of a routine.  */
#define INITIAL_ELIMINATION_OFFSET(FROM, TO, OFFSET) \
  ((OFFSET) = alpha_initial_elimination_offset(FROM, TO))

/* Define this if stack space is still allocated for a parameter passed
   in a register.  */
/* #define REG_PARM_STACK_SPACE */

/* Define how to find the value returned by a function.
   VALTYPE is the data type of the value (as a tree).
   If the precise function being called is known, FUNC is its FUNCTION_DECL;
   otherwise, FUNC is 0.

   On Alpha the value is found in $0 for integer functions and
   $f0 for floating-point functions.  */

#define FUNCTION_VALUE(VALTYPE, FUNC) \
  function_value (VALTYPE, FUNC, VOIDmode)

/* Define how to find the value returned by a library function
   assuming the value has mode MODE.  */

#define LIBCALL_VALUE(MODE) \
  function_value (NULL, NULL, MODE)

/* 1 if N is a possible register number for a function value
   as seen by the caller.  */

#define FUNCTION_VALUE_REGNO_P(N)  \
  ((N) == 0 || (N) == 1 || (N) == 32 || (N) == 33)

/* 1 if N is a possible register number for function argument passing.
   On Alpha, these are $16-$21 and $f16-$f21.  */

#define FUNCTION_ARG_REGNO_P(N) \
  (IN_RANGE ((N), 16, 21) || ((N) >= 16 + 32 && (N) <= 21 + 32))

/* Define a data type for recording info about an argument list
   during the scan of that argument list.  This data type should
   hold all necessary information about the function itself
   and about the args processed so far, enough to enable macros
   such as FUNCTION_ARG to determine where the next arg should go.

   On Alpha, this is a single integer, which is a number of words
   of arguments scanned so far.
   Thus 6 or more means all following args should go on the stack.  */

#define CUMULATIVE_ARGS int

/* Initialize a variable CUM of type CUMULATIVE_ARGS
   for a call to a function whose data type is FNTYPE.
   For a library call, FNTYPE is 0.  */

#define INIT_CUMULATIVE_ARGS(CUM, FNTYPE, LIBNAME, INDIRECT, N_NAMED_ARGS) \
  (CUM) = 0

/* Define intermediate macro to compute the size (in registers) of an argument
   for the Alpha.  */

#define ALPHA_ARG_SIZE(MODE, TYPE, NAMED)				\
  ((MODE) == TFmode || (MODE) == TCmode ? 1				\
   : (((MODE) == BLKmode ? int_size_in_bytes (TYPE) : GET_MODE_SIZE (MODE)) \
      + (UNITS_PER_WORD - 1)) / UNITS_PER_WORD)

/* Make (or fake) .linkage entry for function call.
   IS_LOCAL is 0 if name is used in call, 1 if name is used in definition.  */

/* This macro defines the start of an assembly comment.  */

#define ASM_COMMENT_START " #"

/* This macro produces the initial definition of a function.  */

#define ASM_DECLARE_FUNCTION_NAME(FILE,NAME,DECL) \
  alpha_start_function(FILE,NAME,DECL);

/* This macro closes up a function definition for the assembler.  */

#define ASM_DECLARE_FUNCTION_SIZE(FILE,NAME,DECL) \
  alpha_end_function(FILE,NAME,DECL)

/* Output any profiling code before the prologue.  */

#define PROFILE_BEFORE_PROLOGUE 1

/* Never use profile counters.  */

#define NO_PROFILE_COUNTERS 1

/* Output assembler code to FILE to increment profiler label # LABELNO
   for profiling a function entry.  Under OSF/1, profiling is enabled
   by simply passing -pg to the assembler and linker.  */

#define FUNCTION_PROFILER(FILE, LABELNO)

/* EXIT_IGNORE_STACK should be nonzero if, when returning from a function,
   the stack pointer does not matter.  The value is tested only in
   functions that have frame pointers.
   No definition is equivalent to always zero.  */

#define EXIT_IGNORE_STACK 1

/* Define registers used by the epilogue and return instruction.  */

#define EPILOGUE_USES(REGNO)	((REGNO) == 26)

/* Length in units of the trampoline for entering a nested function.  */

#define TRAMPOLINE_SIZE    32

/* The alignment of a trampoline, in bits.  */

#define TRAMPOLINE_ALIGNMENT  64

/* A C expression whose value is RTL representing the value of the return
   address for the frame COUNT steps up from the current frame.
   FRAMEADDR is the frame pointer of the COUNT frame, or the frame pointer of
   the COUNT-1 frame if RETURN_ADDR_IN_PREVIOUS_FRAME is defined.  */

#define RETURN_ADDR_RTX  alpha_return_addr

/* Provide a definition of DWARF_FRAME_REGNUM here so that fallback unwinders
   can use DWARF_ALT_FRAME_RETURN_COLUMN defined below.  This is just the same
   as the default definition in dwarf2out.c.  */
#undef DWARF_FRAME_REGNUM
#define DWARF_FRAME_REGNUM(REG) DBX_REGISTER_NUMBER (REG)

/* Before the prologue, RA lives in $26.  */
#define INCOMING_RETURN_ADDR_RTX  gen_rtx_REG (Pmode, 26)
#define DWARF_FRAME_RETURN_COLUMN DWARF_FRAME_REGNUM (26)
#define DWARF_ALT_FRAME_RETURN_COLUMN DWARF_FRAME_REGNUM (64)
#define DWARF_ZERO_REG 31

/* Describe how we implement __builtin_eh_return.  */
#define EH_RETURN_DATA_REGNO(N)	((N) < 4 ? (N) + 16 : INVALID_REGNUM)
#define EH_RETURN_STACKADJ_RTX	gen_rtx_REG (Pmode, 28)
#define EH_RETURN_HANDLER_RTX \
  gen_rtx_MEM (Pmode, plus_constant (stack_pointer_rtx, \
				     crtl->outgoing_args_size))

/* Addressing modes, and classification of registers for them.  */

/* Macros to check register numbers against specific register classes.  */

/* These assume that REGNO is a hard or pseudo reg number.
   They give nonzero only if REGNO is a hard reg of the suitable class
   or a pseudo reg currently allocated to a suitable hard reg.
   Since they use reg_renumber, they are safe only once reg_renumber
   has been allocated, which happens in local-alloc.c.  */

#define REGNO_OK_FOR_INDEX_P(REGNO) 0
#define REGNO_OK_FOR_BASE_P(REGNO) \
((REGNO) < 32 || (unsigned) reg_renumber[REGNO] < 32  \
 || (REGNO) == 63 || reg_renumber[REGNO] == 63)

/* Maximum number of registers that can appear in a valid memory address.  */
#define MAX_REGS_PER_ADDRESS 1

/* Recognize any constant value that is a valid address.  For the Alpha,
   there are only constants none since we want to use LDA to load any
   symbolic addresses into registers.  */

#define CONSTANT_ADDRESS_P(X)   \
  (CONST_INT_P (X)		\
   && (unsigned HOST_WIDE_INT) (INTVAL (X) + 0x8000) < 0x10000)

/* Include all constant integers and constant doubles, but not
   floating-point, except for floating-point zero.  */

#define LEGITIMATE_CONSTANT_P  alpha_legitimate_constant_p

/* The macros REG_OK_FOR..._P assume that the arg is a REG rtx
   and check its validity for a certain class.
   We have two alternate definitions for each of them.
   The usual definition accepts all pseudo regs; the other rejects
   them unless they have been allocated suitable hard regs.
   The symbol REG_OK_STRICT causes the latter definition to be used.

   Most source files want to accept pseudo regs in the hope that
   they will get allocated to the class that the insn wants them to be in.
   Source files for reload pass need to be strict.
   After reload, it makes no difference, since pseudo regs have
   been eliminated by then.  */

/* Nonzero if X is a hard reg that can be used as an index
   or if it is a pseudo reg.  */
#define REG_OK_FOR_INDEX_P(X) 0

/* Nonzero if X is a hard reg that can be used as a base reg
   or if it is a pseudo reg.  */
#define NONSTRICT_REG_OK_FOR_BASE_P(X)  \
  (REGNO (X) < 32 || REGNO (X) == 63 || REGNO (X) >= FIRST_PSEUDO_REGISTER)

/* ??? Nonzero if X is the frame pointer, or some virtual register
   that may eliminate to the frame pointer.  These will be allowed to
   have offsets greater than 32K.  This is done because register
   elimination offsets will change the hi/lo split, and if we split
   before reload, we will require additional instructions.  */
#define NONSTRICT_REG_OK_FP_BASE_P(X)		\
  (REGNO (X) == 31 || REGNO (X) == 63		\
   || (REGNO (X) >= FIRST_PSEUDO_REGISTER	\
       && REGNO (X) < LAST_VIRTUAL_POINTER_REGISTER))

/* Nonzero if X is a hard reg that can be used as a base reg.  */
#define STRICT_REG_OK_FOR_BASE_P(X) REGNO_OK_FOR_BASE_P (REGNO (X))

#ifdef REG_OK_STRICT
#define REG_OK_FOR_BASE_P(X)	STRICT_REG_OK_FOR_BASE_P (X)
#else
#define REG_OK_FOR_BASE_P(X)	NONSTRICT_REG_OK_FOR_BASE_P (X)
#endif

/* Try a machine-dependent way of reloading an illegitimate address
   operand.  If we find one, push the reload and jump to WIN.  This
   macro is used in only one place: `find_reloads_address' in reload.c.  */

#define LEGITIMIZE_RELOAD_ADDRESS(X,MODE,OPNUM,TYPE,IND_L,WIN)		     \
do {									     \
  rtx new_x = alpha_legitimize_reload_address (X, MODE, OPNUM, TYPE, IND_L); \
  if (new_x)								     \
    {									     \
      X = new_x;							     \
      goto WIN;								     \
    }									     \
} while (0)

/* Go to LABEL if ADDR (a legitimate address expression)
   has an effect that depends on the machine mode it is used for.
   On the Alpha this is true only for the unaligned modes.   We can
   simplify this test since we know that the address must be valid.  */

#define GO_IF_MODE_DEPENDENT_ADDRESS(ADDR,LABEL)  \
{ if (GET_CODE (ADDR) == AND) goto LABEL; }

/* Specify the machine mode that this machine uses
   for the index in the tablejump instruction.  */
#define CASE_VECTOR_MODE SImode

/* Define as C expression which evaluates to nonzero if the tablejump
   instruction expects the table to contain offsets from the address of the
   table.

   Do not define this if the table should contain absolute addresses.
   On the Alpha, the table is really GP-relative, not relative to the PC
   of the table, but we pretend that it is PC-relative; this should be OK,
   but we should try to find some better way sometime.  */
#define CASE_VECTOR_PC_RELATIVE 1

/* Define this as 1 if `char' should by default be signed; else as 0.  */
#define DEFAULT_SIGNED_CHAR 1

/* Max number of bytes we can move to or from memory
   in one reasonably fast instruction.  */

#define MOVE_MAX 8

/* If a memory-to-memory move would take MOVE_RATIO or more simple
   move-instruction pairs, we will do a movmem or libcall instead.

   Without byte/word accesses, we want no more than four instructions;
   with, several single byte accesses are better.  */

#define MOVE_RATIO(speed)  (TARGET_BWX ? 7 : 2)

/* Largest number of bytes of an object that can be placed in a register.
   On the Alpha we have plenty of registers, so use TImode.  */
#define MAX_FIXED_MODE_SIZE	GET_MODE_BITSIZE (TImode)

/* Nonzero if access to memory by bytes is no faster than for words.
   Also nonzero if doing byte operations (specifically shifts) in registers
   is undesirable.

   On the Alpha, we want to not use the byte operation and instead use
   masking operations to access fields; these will save instructions.  */

#define SLOW_BYTE_ACCESS	1

/* Define if operations between registers always perform the operation
   on the full register even if a narrower mode is specified.  */
#define WORD_REGISTER_OPERATIONS

/* Define if loading in MODE, an integral mode narrower than BITS_PER_WORD
   will either zero-extend or sign-extend.  The value of this macro should
   be the code that says which one of the two operations is implicitly
   done, UNKNOWN if none.  */
#define LOAD_EXTEND_OP(MODE) ((MODE) == SImode ? SIGN_EXTEND : ZERO_EXTEND)

/* Define if loading short immediate values into registers sign extends.  */
#define SHORT_IMMEDIATES_SIGN_EXTEND

/* Value is 1 if truncating an integer of INPREC bits to OUTPREC bits
   is done just by pretending it is already truncated.  */
#define TRULY_NOOP_TRUNCATION(OUTPREC, INPREC) 1

/* The CIX ctlz and cttz instructions return 64 for zero.  */
#define CLZ_DEFINED_VALUE_AT_ZERO(MODE, VALUE)  ((VALUE) = 64, TARGET_CIX)
#define CTZ_DEFINED_VALUE_AT_ZERO(MODE, VALUE)  ((VALUE) = 64, TARGET_CIX)

/* Define the value returned by a floating-point comparison instruction.  */

#define FLOAT_STORE_FLAG_VALUE(MODE) \
  REAL_VALUE_ATOF ((TARGET_FLOAT_VAX ? "0.5" : "2.0"), (MODE))

/* Canonicalize a comparison from one we don't have to one we do have.  */

#define CANONICALIZE_COMPARISON(CODE,OP0,OP1) \
  do {									\
    if (((CODE) == GE || (CODE) == GT || (CODE) == GEU || (CODE) == GTU) \
	&& (REG_P (OP1) || (OP1) == const0_rtx))		\
      {									\
	rtx tem = (OP0);						\
	(OP0) = (OP1);							\
	(OP1) = tem;							\
	(CODE) = swap_condition (CODE);					\
      }									\
    if (((CODE) == LT || (CODE) == LTU)					\
	&& CONST_INT_P (OP1) && INTVAL (OP1) == 256)			\
      {									\
	(CODE) = (CODE) == LT ? LE : LEU;				\
	(OP1) = GEN_INT (255);						\
      }									\
  } while (0)

/* Specify the machine mode that pointers have.
   After generation of rtl, the compiler makes no further distinction
   between pointers and any other objects of this machine mode.  */
#define Pmode DImode

/* Mode of a function address in a call instruction (for indexing purposes).  */

#define FUNCTION_MODE Pmode

/* Define this if addresses of constant functions
   shouldn't be put through pseudo regs where they can be cse'd.
   Desirable on machines where ordinary constants are expensive
   but a CALL with constant address is cheap.

   We define this on the Alpha so that gen_call and gen_call_value
   get to see the SYMBOL_REF (for the hint field of the jsr).  It will
   then copy it into a register, thus actually letting the address be
   cse'ed.  */

#define NO_FUNCTION_CSE

/* Define this to be nonzero if shift instructions ignore all but the low-order
   few bits.  */
#define SHIFT_COUNT_TRUNCATED 1

/* Control the assembler format that we output.  */

/* Output to assembler file text saying following lines
   may contain character constants, extra white space, comments, etc.  */
#define ASM_APP_ON (TARGET_EXPLICIT_RELOCS ? "\t.set\tmacro\n" : "")

/* Output to assembler file text saying following lines
   no longer contain unusual constructs.  */
#define ASM_APP_OFF (TARGET_EXPLICIT_RELOCS ? "\t.set\tnomacro\n" : "")

#define TEXT_SECTION_ASM_OP "\t.text"

/* Output before read-only data.  */

#define READONLY_DATA_SECTION_ASM_OP "\t.rdata"

/* Output before writable data.  */

#define DATA_SECTION_ASM_OP "\t.data"

/* How to refer to registers in assembler output.
   This sequence is indexed by compiler's hard-register-number (see above).  */

#define REGISTER_NAMES						\
{"$0", "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8",		\
 "$9", "$10", "$11", "$12", "$13", "$14", "$15",		\
 "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",	\
 "$24", "$25", "$26", "$27", "$28", "$29", "$30", "AP",		\
 "$f0", "$f1", "$f2", "$f3", "$f4", "$f5", "$f6", "$f7", "$f8",	\
 "$f9", "$f10", "$f11", "$f12", "$f13", "$f14", "$f15",		\
 "$f16", "$f17", "$f18", "$f19", "$f20", "$f21", "$f22", "$f23",\
 "$f24", "$f25", "$f26", "$f27", "$f28", "$f29", "$f30", "FP"}

/* Strip name encoding when emitting labels.  */

#define ASM_OUTPUT_LABELREF(STREAM, NAME)	\
do {						\
  const char *name_ = NAME;			\
  if (*name_ == '@' || *name_ == '%')		\
    name_ += 2;					\
  if (*name_ == '*')				\
    name_++;					\
  else						\
    fputs (user_label_prefix, STREAM);		\
  fputs (name_, STREAM);			\
} while (0)

/* Globalizing directive for a label.  */
#define GLOBAL_ASM_OP "\t.globl "

/* The prefix to add to user-visible assembler symbols.  */

#define USER_LABEL_PREFIX ""

/* This is how to output a label for a jump table.  Arguments are the same as
   for (*targetm.asm_out.internal_label), except the insn for the jump table is
   passed.  */

#define ASM_OUTPUT_CASE_LABEL(FILE,PREFIX,NUM,TABLEINSN)	\
{ ASM_OUTPUT_ALIGN (FILE, 2); (*targetm.asm_out.internal_label) (FILE, PREFIX, NUM); }

/* This is how to store into the string LABEL
   the symbol_ref name of an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.
   This is suitable for output with `assemble_name'.  */

#define ASM_GENERATE_INTERNAL_LABEL(LABEL,PREFIX,NUM)	\
  sprintf ((LABEL), "*$%s%ld", (PREFIX), (long)(NUM))

/* We use the default ASCII-output routine, except that we don't write more
   than 50 characters since the assembler doesn't support very long lines.  */

#define ASM_OUTPUT_ASCII(MYFILE, MYSTRING, MYLENGTH) \
  do {									      \
    FILE *_hide_asm_out_file = (MYFILE);				      \
    const unsigned char *_hide_p = (const unsigned char *) (MYSTRING);	      \
    int _hide_thissize = (MYLENGTH);					      \
    int _size_so_far = 0;						      \
    {									      \
      FILE *asm_out_file = _hide_asm_out_file;				      \
      const unsigned char *p = _hide_p;					      \
      int thissize = _hide_thissize;					      \
      int i;								      \
      fprintf (asm_out_file, "\t.ascii \"");				      \
									      \
      for (i = 0; i < thissize; i++)					      \
	{								      \
	  register int c = p[i];					      \
									      \
	  if (_size_so_far ++ > 50 && i < thissize - 4)			      \
	    _size_so_far = 0, fprintf (asm_out_file, "\"\n\t.ascii \"");      \
									      \
	  if (c == '\"' || c == '\\')					      \
	    putc ('\\', asm_out_file);					      \
	  if (c >= ' ' && c < 0177)					      \
	    putc (c, asm_out_file);					      \
	  else								      \
	    {								      \
	      fprintf (asm_out_file, "\\%o", c);			      \
	      /* After an octal-escape, if a digit follows,		      \
		 terminate one string constant and start another.	      \
		 The VAX assembler fails to stop reading the escape	      \
		 after three digits, so this is the only way we		      \
		 can get it to parse the data properly.  */		      \
	      if (i < thissize - 1 && ISDIGIT (p[i + 1]))		      \
		_size_so_far = 0, fprintf (asm_out_file, "\"\n\t.ascii \"");  \
	  }								      \
	}								      \
      fprintf (asm_out_file, "\"\n");					      \
    }									      \
  }									      \
  while (0)

/* This is how to output an element of a case-vector that is relative.  */

#define ASM_OUTPUT_ADDR_DIFF_ELT(FILE, BODY, VALUE, REL) \
  fprintf (FILE, "\t.gprel32 $L%d\n", (VALUE))

/* This is how to output an assembler line
   that says to advance the location counter
   to a multiple of 2**LOG bytes.  */

#define ASM_OUTPUT_ALIGN(FILE,LOG)	\
  if ((LOG) != 0)			\
    fprintf (FILE, "\t.align %d\n", LOG);

/* This is how to advance the location counter by SIZE bytes.  */

#define ASM_OUTPUT_SKIP(FILE,SIZE)  \
  fprintf (FILE, "\t.space "HOST_WIDE_INT_PRINT_UNSIGNED"\n", (SIZE))

/* This says how to output an assembler line
   to define a global common symbol.  */

#define ASM_OUTPUT_COMMON(FILE, NAME, SIZE, ROUNDED)  \
( fputs ("\t.comm ", (FILE)),			\
  assemble_name ((FILE), (NAME)),		\
  fprintf ((FILE), ","HOST_WIDE_INT_PRINT_UNSIGNED"\n", (SIZE)))

/* This says how to output an assembler line
   to define a local common symbol.  */

#define ASM_OUTPUT_LOCAL(FILE, NAME, SIZE,ROUNDED)	\
( fputs ("\t.lcomm ", (FILE)),				\
  assemble_name ((FILE), (NAME)),			\
  fprintf ((FILE), ","HOST_WIDE_INT_PRINT_UNSIGNED"\n", (SIZE)))


/* Print operand X (an rtx) in assembler syntax to file FILE.
   CODE is a letter or dot (`z' in `%z0') or 0 if no letter was specified.
   For `%' followed by punctuation, CODE is the punctuation and X is null.  */

#define PRINT_OPERAND(FILE, X, CODE)  print_operand (FILE, X, CODE)

/* Determine which codes are valid without a following integer.  These must
   not be alphabetic.

   ~    Generates the name of the current function.

   /	Generates the instruction suffix.  The TRAP_SUFFIX and ROUND_SUFFIX
	attributes are examined to determine what is appropriate.

   ,    Generates single precision suffix for floating point
	instructions (s for IEEE, f for VAX)

   -	Generates double precision suffix for floating point
	instructions (t for IEEE, g for VAX)
   */

#define PRINT_OPERAND_PUNCT_VALID_P(CODE) \
  ((CODE) == '/' || (CODE) == ',' || (CODE) == '-' || (CODE) == '~' \
   || (CODE) == '#' || (CODE) == '*' || (CODE) == '&')

/* Print a memory address as an operand to reference that memory location.  */

#define PRINT_OPERAND_ADDRESS(FILE, ADDR) \
  print_operand_address((FILE), (ADDR))

/* Tell collect that the object format is ECOFF.  */
#define OBJECT_FORMAT_COFF
#define EXTENDED_COFF

/* If we use NM, pass -g to it so it only lists globals.  */
#define NM_FLAGS "-pg"

/* Definitions for debugging.  */

#define SDB_DEBUGGING_INFO 1		/* generate info for mips-tfile */
#define DBX_DEBUGGING_INFO 1		/* generate embedded stabs */
#define MIPS_DEBUGGING_INFO 1		/* MIPS specific debugging info */

#ifndef PREFERRED_DEBUGGING_TYPE	/* assume SDB_DEBUGGING_INFO */
#define PREFERRED_DEBUGGING_TYPE  SDB_DEBUG
#endif


/* Correct the offset of automatic variables and arguments.  Note that
   the Alpha debug format wants all automatic variables and arguments
   to be in terms of two different offsets from the virtual frame pointer,
   which is the stack pointer before any adjustment in the function.
   The offset for the argument pointer is fixed for the native compiler,
   it is either zero (for the no arguments case) or large enough to hold
   all argument registers.
   The offset for the auto pointer is the fourth argument to the .frame
   directive (local_offset).
   To stay compatible with the native tools we use the same offsets
   from the virtual frame pointer and adjust the debugger arg/auto offsets
   accordingly. These debugger offsets are set up in output_prolog.  */

extern long alpha_arg_offset;
extern long alpha_auto_offset;
#define DEBUGGER_AUTO_OFFSET(X) \
  ((GET_CODE (X) == PLUS ? INTVAL (XEXP (X, 1)) : 0) + alpha_auto_offset)
#define DEBUGGER_ARG_OFFSET(OFFSET, X) (OFFSET + alpha_arg_offset)

/* mips-tfile doesn't understand .stabd directives.  */
#define DBX_OUTPUT_SOURCE_LINE(STREAM, LINE, COUNTER) do {	\
  dbxout_begin_stabn_sline (LINE);				\
  dbxout_stab_value_internal_label ("LM", &COUNTER);		\
} while (0)

/* We want to use MIPS-style .loc directives for SDB line numbers.  */
extern int num_source_filenames;
#define SDB_OUTPUT_SOURCE_LINE(STREAM, LINE)	\
  fprintf (STREAM, "\t.loc\t%d %d\n", num_source_filenames, LINE)

#define ASM_OUTPUT_SOURCE_FILENAME(STREAM, NAME)			\
  alpha_output_filename (STREAM, NAME)

/* mips-tfile.c limits us to strings of one page.  We must underestimate this
   number, because the real length runs past this up to the next
   continuation point.  This is really a dbxout.c bug.  */
#define DBX_CONTIN_LENGTH 3000

/* By default, turn on GDB extensions.  */
#define DEFAULT_GDB_EXTENSIONS 1

/* Stabs-in-ECOFF can't handle dbxout_function_end().  */
#define NO_DBX_FUNCTION_END 1

/* If we are smuggling stabs through the ALPHA ECOFF object
   format, put a comment in front of the .stab<x> operation so
   that the ALPHA assembler does not choke.  The mips-tfile program
   will correctly put the stab into the object file.  */

#define ASM_STABS_OP	((TARGET_GAS) ? "\t.stabs\t" : " #.stabs\t")
#define ASM_STABN_OP	((TARGET_GAS) ? "\t.stabn\t" : " #.stabn\t")
#define ASM_STABD_OP	((TARGET_GAS) ? "\t.stabd\t" : " #.stabd\t")

/* Forward references to tags are allowed.  */
#define SDB_ALLOW_FORWARD_REFERENCES

/* Unknown tags are also allowed.  */
#define SDB_ALLOW_UNKNOWN_REFERENCES

#define PUT_SDB_DEF(a)					\
do {							\
  fprintf (asm_out_file, "\t%s.def\t",			\
	   (TARGET_GAS) ? "" : "#");			\
  ASM_OUTPUT_LABELREF (asm_out_file, a); 		\
  fputc (';', asm_out_file);				\
} while (0)

#define PUT_SDB_PLAIN_DEF(a)				\
do {							\
  fprintf (asm_out_file, "\t%s.def\t.%s;",		\
	   (TARGET_GAS) ? "" : "#", (a));		\
} while (0)

#define PUT_SDB_TYPE(a)					\
do {							\
  fprintf (asm_out_file, "\t.type\t0x%x;", (a));	\
} while (0)

/* For block start and end, we create labels, so that
   later we can figure out where the correct offset is.
   The normal .ent/.end serve well enough for functions,
   so those are just commented out.  */

extern int sdb_label_count;		/* block start/end next label # */

#define PUT_SDB_BLOCK_START(LINE)			\
do {							\
  fprintf (asm_out_file,				\
	   "$Lb%d:\n\t%s.begin\t$Lb%d\t%d\n",		\
	   sdb_label_count,				\
	   (TARGET_GAS) ? "" : "#",			\
	   sdb_label_count,				\
	   (LINE));					\
  sdb_label_count++;					\
} while (0)

#define PUT_SDB_BLOCK_END(LINE)				\
do {							\
  fprintf (asm_out_file,				\
	   "$Le%d:\n\t%s.bend\t$Le%d\t%d\n",		\
	   sdb_label_count,				\
	   (TARGET_GAS) ? "" : "#",			\
	   sdb_label_count,				\
	   (LINE));					\
  sdb_label_count++;					\
} while (0)

#define PUT_SDB_FUNCTION_START(LINE)

#define PUT_SDB_FUNCTION_END(LINE)

#define PUT_SDB_EPILOGUE_END(NAME) ((void)(NAME))

/* Macros for mips-tfile.c to encapsulate stabs in ECOFF, and for
   mips-tdump.c to print them out.

   These must match the corresponding definitions in gdb/mipsread.c.
   Unfortunately, gcc and gdb do not currently share any directories.  */

#define CODE_MASK 0x8F300
#define MIPS_IS_STAB(sym) (((sym)->index & 0xFFF00) == CODE_MASK)
#define MIPS_MARK_STAB(code) ((code)+CODE_MASK)
#define MIPS_UNMARK_STAB(code) ((code)-CODE_MASK)

/* Override some mips-tfile definitions.  */

#define SHASH_SIZE 511
#define THASH_SIZE 55

/* Align ecoff symbol tables to avoid OSF1/1.3 nm complaints.  */

#define ALIGN_SYMTABLE_OFFSET(OFFSET) (((OFFSET) + 7) & ~7)

/* The system headers under Alpha systems are generally C++-aware.  */
#define NO_IMPLICIT_EXTERN_C
