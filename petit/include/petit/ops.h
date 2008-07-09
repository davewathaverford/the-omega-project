/* ops.h,v 1.1.1.2 1992/07/10 02:40:59 davew Exp */
#ifndef Already_Included_Ops
#define Already_Included_Ops

namespace omega {

/*
 * Expression operation properties -- used in ivr-lang-interf.h
 */
#define is_array_ref(e) ((e)->nodeop>=op_fetch_array && \
			 (e)->nodeop<=op_update_array)
#define CASE_ARRAY_REF  case op_fetch_array: \
                        case op_store_array: \
                        case op_update_array

#define is_scalar_ref(e) ((e)->nodeop>=op_fetch && (e)->nodeop<=op_update)
#define CASE_SCALAR_REF  case op_fetch: \
                         case op_store: \
                         case op_update

#define is_mem_ref(e)   ((e)->nodeop>=op_fetch && (e)->nodeop<=op_update_array)
#define CASE_MEMREF     case op_fetch:    case op_fetch_array: \
                        case op_store:    case op_store_array: \
                        case op_update:   case op_update_array

#define is_phi_access(e) \
     (((e)->nodeop==op_store && (e)->nodeprev->nodeop==op_phi) || \
      ((e)->nodeop==op_fetch && (e)->nodeparent->nodeop==op_phi))

#define is_constant(e)   ((e)->nodeop==op_constant)

#define is_assign(e)  ((e)->nodeop>=op_assign && (e)->nodeop<=op_min_assign)
#define CASE_ASSIGN   case op_assign: \
                      case op_add_assign:  case op_mul_assign: \
                      case op_max_assign:  case op_min_assign

#define is_expr(e)    ((e)->nodeop>=op_constant && (e)->nodeop<=op_negate)

#define is_do(e)      ((e)->nodeop>=op_do && (e)->nodeop<=op_doany)
#define CASE_DO       case op_do: \
                      case op_dovec: \
                      case op_dopar: \
                      case op_doany

/*
 * Obsolete stuff ( not for compare_op and logic_op - murka 02.01.95)
 */
#define array_op(OP) ((OP)>=op_fetch_array && (OP)<=op_update_array)
#define store_op(OP) (((OP) == op_store_array) || ((OP) == op_store))
#define fetch_op(OP) (((OP) == op_fetch_array) || ((OP) == op_fetch))
#define update_op(OP) (((OP) == op_update_array) || ((OP) == op_update))
#define compare_op(OP) ((OP) == op_gt || (OP) == op_ge || (OP) == op_lt || (OP) == op_le || (OP) == op_eq || (OP) == op_ne)
#define logic_op(OP)   ((OP) == op_and || (OP) == op_or || (OP) == op_not)

#define do_op(OP)    (((OP) == op_do) || (OP) == op_dovec || (OP) == op_dopar || (OP) == op_doany)
#define if_branch_op(OP)  (((OP) == op_then) || ((OP) == op_else))

/*
 * A macros for a type of the expression
 */

#define integer_expr(E) ((E)==exprint || (E) == exprrefint || (E) == exprintarray || (E) == exprintfun)

} // end of namespace omega

#endif
