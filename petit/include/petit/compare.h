/* compare.h,v 1.1.1.2 1992/07/10 02:39:39 davew Exp */

#ifndef Already_Included_Array_Compare
#define Already_Included_Array_Compare

#include <basic/bool.h>

namespace omega {

extern bool compare_equal( node *a, node *b );
extern bool compare_trees( node *a, node *b , int);

}

#endif
