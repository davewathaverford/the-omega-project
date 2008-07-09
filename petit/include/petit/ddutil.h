/* $Id: ddutil.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_DDUtil
#define Already_Included_DDUtil

#include <basic/bool.h>
#include <petit/tree.h>

namespace omega {

extern bool DDdebug;

#define maxnest 20
/* maxCommonNest controlled by # of available bits after those
   that are #defined in tree.h */

typedef int Nvector[maxnest+1];
typedef bool NBvector[maxnest+1];
typedef int NNvector[2*maxnest+1];
typedef int Narray[maxnest+1][maxnest+1];
typedef int NNarray[2*maxnest+1][2*maxnest+1];

/* various arrays are subscripted with donode->nodevalue
   where donode is an op_do, op_dovec, or op_dopar
   and its nodevalue has been "temporarily incremented"
   by dd_get_nests so that nodevalues are unique across
   the loops we happen to be investigating when we call
   any of these functions */

extern int SSfirst; /* index in DO array of loop indices used in subscripts */
extern int SSnloops; /* # of above */
extern Nvector SSnextloop;  /* index into DD arrays of next loop index */
/* To iterate over all valid indices for DO arrays:
	for( n = SSfirst; n > 0; n = SSnextloop[n] ){
	    work with DO*[n]
	}
*/

extern node *DOptr[maxnest+1];   /* pointers to op_index for each do */
extern Nvector DOinc;            /* compile-time constants for inc, */
extern Nvector DOlower;             /* lower bound */
extern Nvector DOupper;             /* and upper bound */
extern NBvector DOlowerknown;    /* are values above known? */
extern NBvector DOupperknown;
extern Narray DOlowercoef;       /* coefficient vectors for each do */
extern Narray DOuppercoef;

extern void dd_find_do_limits( unsigned int n );
/* sets DOlower, DOupper, DOlowerknown, DOupperknown */

extern void dd_find_do_coefs( unsigned int n );

extern unsigned int dd_linearity( node *ss );
/* sets SSnloops and SSfirst */

extern void dd_fill_coef( node *ss, Nvector coef );
/* fills Nvector with coeffiecients */

extern int dd_gcd( int a, int b );

extern void dd_kirch( int a, int b, int *g, int *x, int *y );

/* dump routines */
extern void dumpeq( unsigned int nest, Nvector ocoef, Nvector icoef );

/* dump a direction vector */
extern char *dir_vec( dddirection dddir, unsigned int nest );

/* dump a direction and difference vector */
extern char *dir_diff_vec( dddirection dddir, DDdifference dddiff, unsigned int nest );

#define ABS(x)	(x > 0 ? x : -x)

}

#endif
