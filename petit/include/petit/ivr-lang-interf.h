/* $Id: ivr-lang-interf.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/************************************************************
 *                                                          *
 * Language Interface for IVR and Forward Substitution (FS) *
 * System-dependent part. Tuned for Petit.                   *
 * Written by Vadim Maslov vadik@cs.umd.edu 01/03/93.       *
 *                                                          *
 ************************************************************/
#ifndef Already_Included_IvrLangInterf
#define Already_Included_IvrLangInterf

#include <basic/bool.h>



#ifdef TEST_GENERIC_LANG_INTERF
#include <petit/Exit.h>  // get Vadim's Exit.h for petit, not just Omega Library
#include <petit/lang-interf.generic>
#include <petit/ivr-lang-interf.generic>
#else

#include <petit/vutil.h>
#include <petit/make.h>
#include <petit/compare.h>
#include <petit/definitions.h>
#include <petit/lang-interf.h>
#include <petit/ops.h>

namespace omega {
/*
 * Here various properties of expression operations are described.
 */
#define expr_is_const(e)      (expr_op(e)==op_constant || expr_op(e)==op_real)
#define expr_is_int_const(e)  (expr_op(e)==op_constant)
#define expr_is_real_const(e) (expr_op(e)==op_real)
#define int_const_value(e)    ((e)->nodevalue)

#define expr_is_loop_index(e)    (expr_op(e)==op_index)

#define expr_is_scalar_fetch(e)  (expr_op(e)==op_fetch)
#define expr_is_scalar_store(e)  (expr_op(e)==op_store)
#define expr_is_scalar_update(e) (expr_op(e)==op_update)

#define expr_is_array_reference(e)  (is_array_ref(e))
#define expr_is_memory_reference(e) (is_mem_ref(e))

#define expr_is_phi(e)        (expr_op(e)==op_phi)

#define expr_is_assign(e)     (is_assign(e))

#define stmt_is_entry_node(e) ((e)==Entry)
#define stmt_is_exit_node(e)  ((e)==ExitNode)


/*
 * Also we need information about what your operations can do.
 * So below macros for handling expressions and operations are defiend.
 */
#define expr_op(e)       ((e)->nodeop)
#define expr_var_id(e)   (get_nodevalue_sym(get_nodevalue_node(e)))
#define expr_lineno(e)   ((e)->nodesequence)

/*
 * Relatives
 */
#define expr_parent(e)       ((e)->nodeparent)
#define bin_op_arg_left(e)   ((e)->nodechild)
#define bin_op_arg_right(e)  ((e)->nodechild->nodenext)

#define assgmt_parent(e)     ((e)->nodeparent)
#define assgmt_left_part(p)  ((p)->nodechild->nodenext)
#define assgmt_right_part(p) ((p)->nodechild)

#define phi_parent(e)        ((e)->ud.phi_parent)
#define where_defined(e)     ((e)->ud.def_node)

/*
 * Types
 */
#define expr_type_is_int(e)       ((e)->exptype==exprint)

/*
 * Moving expressions and statements around
 */
#define place_expr_instead(new, old) PlaceNodeInstead(new, old)
void place_new_stmt_after(pEXPR, pEXPR);

/*
 * Copying and making expressions and statements
 */
#define copy_expr_tree(e)         CopyTree(e,1)
#define make_expr_node(le,op,re)  ExprNode(le,op,re,1)
#define make_constant(value)      MakeConst(value, 1)
#define make_loop_index_ref(l) (AddSSAgraph(make_index((node*)(l)->nodechild)))
pSTMT   make_scalar_assgmt(pEXPR, EXPR_OPER, pEXPR); 
pEXPR   make_scalar_store(Var_Id);
pEXPR   make_scalar_fetch(Var_Id);
void    turn_update_to_store(pEXPR);
void    turn_store_to_update(pEXPR, EXPR_OPER, pEXPR);
void    place_shadow_expr(pEXPR,pEXPR);


/*
 * Removing and Freeing expressions and statements.
 * e is expression to free,
 * f is flag: TRUE:  remove from memory and SSA graph
 *            FALSE: remove only from SSA graph
 */
#define free_expr_tree(e,f)  FreeTree(e,-1+2*(f))
#define free_expr_node(e,f)  FreeNode(e,-1+2*(f))

#define remove_stmt(p)  {RmStmtNumber(p);  RemoveNode(p,1);}

/*
 * Comparing expressions
 */
#define compare_exprs(e1, e2) (compare_trees((e1), (e2), 0))


/*
 * Parameter loop info
 */
#define loop_val_start(p)  ((p)->nodechild->nodenext)
#define loop_val_finish(p) ((p)->nodechild->nodenext->nodenext)
#define loop_val_step(p)   ((p)->nodechild->nodenext->nodenext->nodenext)

/*
 * Print and debug stuff
 */
#define print_program(text, p) PrintPgm((text), (p), false)
#define expr_oper_name(op)     get_op_name(op)

/*
 * IVR&FS is done by calling function DoIvrFs.
 * What exactly is done, is controlled by boolean variables:
 *   ivr_on           -- do IVR
 *   ivr_ForwSub      -- do Forward Substitution
 *   ivr_SubstScalars -- if ivr_ForwSub, substitute integer scalars
 *                       with unknown values
 *   ivr_SubstReals   -- if ivr_ForwSub && ivr_SubstScalars, substitute
 *                       integer and real scalars with unknown values
 *   ivr_debug        -- print IVR debugging info
 *
 * Pointer to SSA graph node has pEXPR type, because (only)
 * expression operations can be vertexes of SSAG.
 *
 * Before calling DoIvrFs you should build a list of SSA graph vertexes.
 * Each vertex is added to the graph by calling IVR-supplied function
 *   AddSSAgraph(pEXPR node).
 */

/*
 * User-defined macro/function is_ssag_node(pSSAGnode p) tells whether
 *   node p can be in SSAG.
 */
#define is_ssag_node(p) \
  ((p)==Entry || (p)==ExitNode || \
   ((p)->nodeop>=op_constant && (p)->nodeop<=op_phi))

#define can_not_be_dead(e) \
  (!is_ssag_node(e) || \
   stmt_is_entry_node(e) || stmt_is_exit_node(e) || is_array_ref(e) || \
   is_do((e)->nodeparent) || (e)->nodeparent->nodeop==op_if)

/*
 * Edges of SSA graph are reported by functions supplied by user:
 *   SsaSource(p)        -- returns first SSAG node supplying data to node p
 *   SsaNextSource(p,p0) -- returns next SSAG node supplying data to node p
 *   SsaSink(p)          -- returns first SSAG node consuming data from node p
 *   SsaNextSink(p,p0)   -- returns next SSAG node consuming data from node p
 */
pEXPR ssa_source(pEXPR, void**);
pEXPR ssa_next_source(pEXPR, pEXPR, void**);
pEXPR ssa_sink(pEXPR, void**);
pEXPR ssa_next_sink(pEXPR, pEXPR, void**);


/*
 * SSA Graph node should have the following members:
 *   SCCNO sc_region;  -- SCC number 
 *   SCCNO sc_number;  -- for FindSCC 
 *   SCCNO sc_lowlink; -- for FindSCC 
 *   bool  sc_onStack; -- for FindSCC 
 *   bool  is_loopfun; -- flag: is it a closed form function of loop variable.
 *   pEXPR *ssag_node; -- pointer to the list of SSAG nodes.
 *
 * l-value of these members are supplied by user-defined macros, so
 * actual structure member names may be different.
 */
#define scc_region(e)    ((e)->sc_region)
#define scc_number(e)    ((e)->sc_number)
#define scc_lowlink(e)   ((e)->sc_lowlink)
#define scc_onstack(e)   ((e)->sc_onStack)
#define scc_isloopfun(e) ((e)->is_loopfun)
#define scc_pssag_list(e) ((e)->ssag_node)


/*
 * Traverse things
 */
void traverse_stmts(pSTMT, void (*)(pSTMT));
void traverse_expr1(pEXPR, void (*)(pEXPR));
void traverse_exprs(pSTMT, void (*)(pEXPR));
void traverse_expr1_shadow(pEXPR, void (*)(pEXPR));
void traverse_exprs_shadow(pSTMT, void (*)(pEXPR));


/*
 * SSA interface to dependence testing -- vadik
 */

/*
 * get_ssa_scalar_id(e) produces var_id for given expression.
 * The difference from expr_var_id(e) is that it produces
 * different IDs for different definitions of the same variable.
 * Doing this it may create additional symbol table entries or do some trick.
 */
Var_Id get_ssa_scalar_id(pEXPR); 

/*
 * tag_var_as_changeable_or_constant(var, use, leading0s)
 * tags variable as changeable variable or symbolic constant depending on
 * the context of variable use and definition.
 */
void tag_var_as_changeable_or_constant(Var_Id, pEXPR, int);

} // end of omega namespace

#endif


#endif
