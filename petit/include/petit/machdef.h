/* machdef.h,v 1.1.1.2 1992/07/10 02:40:44 davew Exp */
#ifndef Already_Included_MachDef
#define Already_Included_MachDef

/* This includes the various parameters to describe the target machine */

namespace omega {
#define ALLIANT

/* ------------------------------------------------------------------------ */

#ifdef ALLIANT /* Alliant FX/8 */

#define INTEGER_SIZE    4       /* size of integer type */
#define INTEGER_ALIGN   4       /* required alignment of integers */
#define REAL_SIZE       8       /* size of real type */
#define REAL_ALIGN      8       /* required alignment of reals */

#define VECTOR_LENGTH   32      /* length of vector registers */

#define AP_REG	  0       /* argument pointer register number */
#define SP_REG	  7       /* stack pointer */
#define FNL_REG	 0       /* result of function returning long */
#define FND_REG	 0       /* result of function returning double */
#define FNV_REG	 0       /* result of function returning vector */
#define VEC_LEN_REG     4       /* vector length data register */
#define VEC_INC_REG     5       /* vector increment data register */
#define VEC_MASK_REG    6       /* vector mask data register */

#endif

/* ------------------------------------------------------------------------ */
}

#endif
