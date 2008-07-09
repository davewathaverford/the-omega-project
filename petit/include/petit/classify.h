/* $Id: classify.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/* classification information about dependence */

#ifndef Already_Included_Classify
#define Already_Included_Classify

namespace omega {

/* reasons why dependence test isn't fully applicable */
#define APPL_FULL             0 /* tests are fully applicable */
#define APPL_NONAFFINE_SUBSCR 0x1 /* non-affine expr in the subscript */
#define APPL_SYMBOLIC_SUBSCR  0x2 /* symbolic constant in the subscript */
#define APPL_NONAFFINE_BOUND  0x4 /* non-affine expr in the loop bound */
#define APPL_SYMBOLIC_BOUND   0x8 /* symbolic constant in the loop bound */
#define APPL_TRIANG_BOUND     0x10 /* triangular loop bounds */
#define APPL_NONUNIT_STEP     0x20 /* non-unit loop step */


/* how soon dependence is found by epsilon test */
#define EPSILON_ASSUME    0  /* solved while forming problem */
#define EPSILON_ZIV       1
#define EPSILON_SIMPLE    2  /* found by simple tests */
#define EPSILON_BANERJEE  3  /* found by Banerjee tests */
#define EPSILON_OTHER     4  /* some other reason - !!shouldn't be at all !!*/

}

#endif


