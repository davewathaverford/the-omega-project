/* $Id: dddir.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/*
 * Dependence flags. Separated from direction vectors
 * Added new flags 
 * Separated direction vectors from flags 
 * Renamed lt,eq,gt direction vectors to fwd,ind,bck
 * Obsolete flags are deleted, nes added 
 */
#ifndef Already_Included_dddir
#define Already_Included_dddir

namespace omega {

typedef long unsigned int dd_flags;

#define ddmem             (dd_flags)0x08000000  /* memory-based dependence */
#define ddval             (dd_flags)0x04000000  /* value-based dependence */
#define ddnotPresumedVal  (dd_flags)0x02000000  /* is known not to be val. */
#define ddvalidForCovers  (dd_flags)0x01000000  /* can be used as value-based
                                                  dd for coverlist */

                                              
#define ddzappableWC      (dd_flags)0x00100000
#define ddzappable        (dd_flags)0x00800000
#define ddZapExact        (dd_flags)0x00000080
#define ddZapPartial      (dd_flags)0x00000040
#define ddZapUseless      (dd_flags)0x00000020
#define ddisRefKilled     (dd_flags)0x00400000	/* killed while refining */
#define ddisInCycle       (dd_flags)0x00200000	/* in cycle */

// WE don't need to distinguish these any more - just use ddMemIncompFlags
#if 0
#define ddIncompDest	  (dd_flags)0x00020000	/* nonlinear destination iteration space */
#define ddIncompSrc	  (dd_flags)0x00010000	/* nonlinear source iteration space */
#define ddIncompSubs      (dd_flags)0x00008000	/* other nonlinear black eqns  */
#endif

#define ddIncompKill      (dd_flags)0x00004000	/* nonlinear kill equations    */
#define ddBad4Monica      (dd_flags)0x00002000
#define ddisInFlowCycle   (dd_flags)0x00001000	/* in flow cycle */
#define ddMemIncompFlags ((dd_flags)0x00038000)
#define ddIncompFlags     (ddMemIncompFlags | ddIncompKill)

    /* bits showing what we know about the relation */
    /* BAD4Monica   = !(write iteration defined
                        in terms of EQ constraints on
			read & sym)     */


/* ?TS? - BAD
   The next 4 flags are used for filtering dependencies.
   This probably should be moved to special flag type 
 */

#define ddloopCarried     (dd_flags)0x00000100  /* is loop carried */
#define ddIsFlow          (dd_flags)0x00000200  /* flow dependence */
#define ddIsAnti          (dd_flags)0x00000400  /* anti dependence */
#define ddIsOutput        (dd_flags)0x00000800  /* output dependence */

// keep this macro not to make changes in old code (f.e. arrayExpand.c)
#define ddisDead(d)  ((d)->ddflags & ddnotPresumedVal) 
#define ddisValue(d)  ((d)->ddflags & ddval) 
#define ddisMemory(d)  ((d)->ddflags & ddmem) 


/*
 * Direction vectors. Separated from dependence flags.
 */
typedef long unsigned int dddirection;

/* directions: */
#define ddfwd		(dddirection)0x1
#define ddind		(dddirection)0x2
#define ddbck		(dddirection)0x4
#define ddall		(dddirection)0x7

#define dddirBits       (dddirection)0x77777777
#define dddirPositions  (dddirection)0x11111111

/* shift a direction 'd' to appropriate position for nest 'n' */
#define dddirnest(d,n)		((d)<<(((n)-1)*4))

/* test if direction vector 'dv' has direction 'd' set at nest 'n' */
#define dddirtest(dv,d,n)	((dv)&dddirnest(d,n))

/* return direction vector except for direction n */
#define ddallBut(dv,n)		((dv)&(dddirBits & ~dddirnest(ddall,n)))

/* set direction 'd' at vector 'dv' for nest 'n' */
#define dddirset(dv,d,n)	(dv|=dddirnest(d,n))

/* reset all directions at nest 'n' in 'dv' except for 'd' */
#define dddironly(dv,d,n)	(dv=(((dv)&~dddirnest(ddall,n))|((dv)&dddirnest(d,n))))

/* reset all directions at nest 'n' in 'dv', then set 'd' */
#define dddirsetonly(dv,d,n)	(dv=(((dv)&~dddirnest(ddall,n))|(dddirnest(d,n))))

/* reset direction 'd' at nest 'n' in 'dv' */
#define dddirreset(dv,d,n)	(dv&=(~dddirnest(d,n)))

/* extract direction vector element at nest 'n' from 'dv' */
#define ddextract1(dv,n)	(((dv)>>(((n)-1)*4))&ddall)

/* test direction 'd' in extracted direction vector element 'dv' */
#define ddtest1(dv,d)		((dv)&(d))

/* reset direction 'd' in extracted direction vector element 'dv' */
#define ddreset1(dv,d)		(dv&=(~(d)))

/* set direction 'd' in extracted direction vector element 'dv' */
#define ddset1(dv,d)		(dv|=(d))

/* filter all direction vector elements with direction 'd' set in 'dv' */
#define ddfilter(dv,d)		(((dv)&((d*dddirPositions)))/d)

/* set all filtered direction vector elements to direction 'd' */
#define ddsetfilter(dv,f,d)	(dv|=((f)*(d)))

#if 0
/* set all-equal bit in 'dv' */
#define ddeqset(dv)		(dv|=(ddallequal))

/* test if direction vector 'dv' all-equal bit set */
#define ddeqtest(dv)		((dv)&(ddallequal))

/* reset all-equal bit in 'dv' */
#define ddeqreset(dv)		(dv&=(~(ddallequal)))
#endif


/* unknown difference */
#define ddunknown	(dddirection)0x80000000

/* return the depth of the loop that carries dv, or the length+1 for loop ind.
   dv has only the 0 bit set at levels [1 .. dd_carried_by(dv, length(dv))]
 */
extern int dd_carried_by(dddirection dv, int length);
extern int leading_zeros(dddirection dv, int length);
extern void append_dd_flags(char *, dddirection dv, const char *iflags);
/* code for the above is currently in ddodriver.c */

}

#endif
