//#undef Relation
//#undef Formula
#undef LOWER_BOUND
#undef UPPER_BOUND
#undef EXACT_BOUND
#undef UNSET_BOUND
#undef assert

#include <basic/leave_Iterator.h>

namespace omega {


typedef enum {AST_eq, AST_lt, AST_gt, AST_geq, AST_leq, AST_neq} Rel_Op;

} // end of namespace omega
