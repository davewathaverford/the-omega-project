/* evaluate.h,v 1.1.1.2 1992/07/10 02:40:14 davew Exp */
#ifndef Already_Included_Evaluate
#define Already_Included_Evaluate

#include <basic/bool.h>

namespace omega {

extern void evaluate( node *n, int *val, bool *known );

/* evaluate simple expression into variable/constant part */
extern void evaluate_simple( node *n, int *val, node **rest );

}

#endif
