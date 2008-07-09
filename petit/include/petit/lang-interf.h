/* $Id: lang-interf.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Lang_Interf
#define Already_Included_Lang_Interf 1

#include <basic/bool.h>


#ifdef TEST_GENERIC_LANG_INTERF
namespace omega {
// make sure nothing with the petit-specific types "node" or "ddnode" gets thru
#define node {
#define ddnode {
#include <petit/lang-interf.generic>
}
#else

#include <omega/Relations.h>    // need definition of "Relation"
#include <petit/tree.h>
#include <petit/ops.h>
#include <petit/ddutil.h>
#include <petit/browsedd.h>
#include <petit/print.h>
#include <petit/evaluate.h>
#include <petit/ddodriver.h>


namespace omega {

struct affine_expr;

#if ! defined NDEBUG
/* similar to assert.h */
#define eassert(EXPR)	( (!(EXPR)) ? (void)fprintf(stderr,"\n\nAssertion \"%s\" failed: file \"%s\", line %d\n", #EXPR, __FILE__, __LINE__), Exit(-2), 0 : 1 )
#else
#define eassert(EXPR)   1
#endif

/*
 * Since we do IVR and FS, we need to be able to add statements & expressions.
 * We must therefore know the types for statements & expressions, so
 * you should define types:
 *  pSTMT -- pointer to the program statement node
 *           (in IVR, we are most interested in loops),
 *  pEXPR -- pointer to the expression operation node,
 *  EXPR_OPER -- expression operation,
 *  INT_CONST -- value of integer constant in expression.
 */
typedef node  *pSTMT;
typedef node  *pEXPR;
typedef node  *pASSERT;
typedef optype EXPR_OPER;
typedef long int INT_CONST;


/* identify an array access
   two accesses must be equal in a self-dependence test */
typedef node *a_access;

extern int node_depth(node *n);

#define access_as_string(A)        (print_to_buf(A,0,0),(const char *)printBuf)
#define access_sym(A)              (get_nodevalue_node(A))
#define access_lineno(A)           ((A)->nodesequence)
#define access_depth(A)            (node_depth(A))
#define access_cover_depth(A)      ((A)->cover_depth)
#define access_terminator_depth(A) ((A)->terminator_depth)

extern int access_shared_depth(a_access a1, a_access a2);

/* does A access a private var - if so, at what level is it private? */
#define access_is_private_var(A)       (A != Entry && A != ExitNode && get_nodevalue_sym(get_nodevalue_node(A))->symclass == CLPRIVATE)
#define access_private_var_level(A) (loop_containing(get_nodevalue_node(A))->nodevalue)

#define access_is_fetch(A)  fetch_op((A)->nodeop)
#define access_is_store(A)  store_op((A)->nodeop)
#define access_is_update(A) update_op((A)->nodeop)
#define accesses_are_same_update_type(A1, A2) \
    ((access_is_update(A1) && access_is_update(A2)) && \
     ((A1)->nodeparent->nodeop == (A2)->nodeparent->nodeop))

typedef node *sub_iterator;
#define sub_i_for_access(A)        ((A)->nodechild)
#define sub_i_next(SUBI)           ((SUBI) = (SUBI)->nodenext)
#define sub_i_done(SUBI)           ((SUBI)==0)
#define sub_i_cur_is_affine(SUBI)  (node_is_affine(SUBI))
#define sub_i_cur_affine(SUBI)     ((affine_expr *) (SUBI)->nodeaffine)
#define sub_i_map_over_cur_vars(SUBI,F,ARGS)  (map_over_vars(SUBI,F,ARGS));

typedef node *context_iterator;

extern context_iterator access_shared_context_at_depth(a_access a1, a_access a2, int d);
extern context_iterator access_inner_shared_context_at_depth(a_access a1, a_access a2, a_access a3, int d);



extern context_iterator cont_i_for_access(a_access a);
/* nodecontext is defined when we need to be doing lots of these fast;
   otherwise do a search: */
#define cont_i_next(C)        { if  ((C)->nodecontext) \
				     (C) = (C)->nodecontext; \
			        else do (C) = ((C)->nodeparent);  \
				     while (!cont_i_done(C) &&       \
 					    !cont_i_cur_is_loop(C) && \
 					    !cont_i_cur_is_if(C));    \
 			      }
#define cont_i_done(C)        ((C)->nodeop == op_entry)
#define cont_i_cur_is_loop(C)  ((C)->nodeop == op_dolimit)
#define cont_i_cur_is_if(C)    (if_branch_op((C)->nodeop))
#define cont_i_cur_if(C)      (C)
#define cont_i_cur_loop(C)    (C)
// NEEDED?  #define cont_i_cur(C)         (C)

#if ! defined NDEBUG
#define cont_i_cur_depth(C)   ((cont_i_cur_is_loop(C)) ? \
			       (((void)eassert((C)->nodeparent->nodevalue > 0)), \
				(C)->nodeparent->nodevalue) : \
			       (node_depth(C)))
#else
#define cont_i_cur_depth(C)   ((cont_i_cur_is_loop(C)) ? \
			       ((C)->nodeparent->nodevalue) : \
			       (node_depth(C)))
#endif
extern int access_is_in_then_or_else_of(a_access A, context_iterator C);
#define cont_i_cur_lineno(C)  ((C)->nodesequence)

typedef node *loop_context;

#define loop_var_id(LOOP)        (get_nodevalue_sym(LOOP))
// NEEDED? #define loop_step_var_id(LOOP)    (&(get_nodevalue_sym(LOOP)->nte->namenext->namest))
#define loop_start(LOOP)         ((affine_expr *)(LOOP)->nodenext->nodeaffine)
#define loop_end(LOOP)           ((affine_expr *)(LOOP)->nodenext->nodenext->nodeaffine)
#define loop_start_is_affine(LOOP) (node_is_affine((LOOP)->nodenext))
#define loop_end_is_affine(LOOP)   (node_is_affine((LOOP)->nodenext->nodenext))
#define loop_has_step(LOOP)    ((LOOP)->nodenext->nodenext->nodenext != NULL)
#define loop_step(LOOP,S,KNOWN) (evaluate((LOOP)->nodenext->nodenext->nodenext,S,KNOWN))
#define loop_map_over_start_vars(LOOP,F,ARGS) (map_over_vars((LOOP)->nodenext,F,ARGS))
#define loop_map_over_end_vars(LOOP,F,ARGS)   (map_over_vars((LOOP)->nodenext->nodenext,F,ARGS))

typedef node *if_context;

#define if_map_over_vars(IF,F,ARGS)  (map_over_vars((IF)->nodeparent->nodechild,F,ARGS))
#define if_else_branch(IC)	((IC)->nodeop == op_else)

typedef enum { rel_and = 0, rel_or = 1, rel_not = 2} rel_operators;

typedef node * term;
typedef node * rel_expr;

typedef enum { greater = 0, greater_eq = 1, equal = 2, not_equal =3,
               less = 4, less_eq = 5}
   if_compare_operators;

#define if_compare_operator_is_ok(CO)  ((CO)>=greater && (CO)<=less_eq)
#define rel_operator_is_ok(RO)      ((RO)<=rel_not)

#define negate_compare_operator(CO) (((CO) == greater)    ? less_eq :\
				    (((CO) == greater_eq) ? less :\
				    (((CO) == equal)      ? not_equal :\
				    (((CO) == not_equal)     ? equal :\
				    (((CO) == less)       ? greater_eq :\
						            greater)))))

extern rel_expr      get_if_rel_expr   (if_context);
extern bool          rel_expr_is_term  (rel_expr);
extern rel_operators rel_expr_operator (rel_expr);

#define    rel_expr_operand_1(RE)   ((RE)->nodechild)
#define    rel_expr_operand_2(RE)   ((RE)->nodechild->nodenext)
#define    rel_expr_term(RE)        ((void)eassert(rel_expr_is_term(RE)), (RE))

extern bool                 rel_expr_term_is_ok(term);
extern if_compare_operators term_compare_op(term);
extern affine_expr *        term_compare_left(term);
extern affine_expr *        term_compare_right(term);




#define get_assert_rel_expr(PA)  (get_if_rel_expr((PA)))
#define assert_map_over_vars_affine(PA,F,ARGS)  (map_over_vars_affine((PA)->nodechild,(F),(ARGS)))

extern context_iterator cont_i_for_assert(pASSERT);

/* operations for traversal of all program assertions */

typedef node * assert_iterator;
#define assert_i_start()   (Assertions)
#define assert_i_next(AI)  ((AI)->nodelink)
#define assert_i_done(AI)  (!(AI))
#define assert_i_cur(AI)   (AI)
#define assert_depth(A)    (node_depth(A))
#define access_assert_shared_depth(AC,AS) (access_shared_depth((AC),(AS)))


/* symtabentry pointers are used to identify variables in affine exprs */
struct symtabentry;

typedef struct symtabentry *Var_Id;

#define var_id_is_index(AE_VAR)   ((AE_VAR)->symdecl!=NULL && \
				  (AE_VAR)->symdecl->nodeop == op_dolimit)
#define var_id_is_const(AE_VAR)   (var_id_def_node(AE_VAR) != 0 && \
				   access_depth(var_id_def_node(AE_VAR))==0)
#define var_id_is_var(AE_VAR)     (!var_id_is_index(AE_VAR) && \
				   (AE_VAR)->symtype != symloopstep &&        \
				   (AE_VAR)->symtype != symfunction &&        \
				   eassert(var_id_def_node(AE_VAR) != 0),     \
				   access_depth(var_id_def_node(AE_VAR))>0)

#define var_id_loop(AE_VAR)     ((AE_VAR)->symdecl->nodeparent)
#define var_id_loop_no(AE_VAR)  ((AE_VAR)->symdecl->nodeparent->nodevalue)
#define var_id_tag(AE_VAR)      ((AE_VAR)->symtag)
#define var_id_name(AE_VAR)     ((AE_VAR)->symname)
#define var_id_def_node(AE_VAR) ((AE_VAR)->def_node)
#define var_id_dims(AE_VAR)     ((AE_VAR)->symdims)

#define loop_no(loop)            ((loop)->nodevalue)
/* return loop containing n or program root */
pSTMT loop_containing(pSTMT);


/* map_over_vars traverses through all the vars that occur in the
   expr.
   map_over_vars_affine traverses through all the vars that occur in
   affine expressions that are met in the expr.
   Unless, we have non-affine expr (i.e. with !is_affine(node_affine)
   these functions should go through the same set of variables

   We need to access vars from affine exprs when we want to know
   the arity in "map_fn".
 */

typedef void (*map_fn)(Var_Id v, void *args);
extern void map_over_vars(pEXPR expression, map_fn f, void *args);

typedef void (*ar_map_fn)(Var_Id v, int arity, void *args);
extern void map_over_vars_affine(pEXPR expression, ar_map_fn f, void * args);


/* all info about dependence, as used by findDirectionVector,
   and read by noteDependence */

// WARNING: these structures are memcpy'd in ddepsilon.c; don't add any 
// class objects here without removing those (copy constructors would not
// be called)
typedef struct dd_info {
    unsigned int         nest;
    dd_flags     flags;
    dddirection  direction;
    dddirection  restraint;
    bool         differenceKnown[maxCommonNest+1];
    int          difference[maxCommonNest+1];
    signed char  loop_dir[maxCommonNest+1];
 /*  1: step is positive,
    -1: step is negative */
    Relation *   dd_relation; 
} dir_and_diff_info;

#if ! defined NDEBUG
void ddi_check(const dir_and_diff_info *ddi);
#else
#define ddi_check(XXX)
#endif

#define d_info_do_eq(D_INFO, J) \
    if (ddextract1((D_INFO)->direction,(J)) == ddind)  \
    { \
	(D_INFO)->differenceKnown[(J)] = 1; \
	(D_INFO)->difference[(J)] = 0; \
    }

#if ! defined NDEBUG
#define d_info_inv(D_INFO) \
    { \
	int i; \
	for (i=1; i<=(D_INFO)->nest; i++) { \
	    if (ddextract1((D_INFO)->direction,i) == ddind) { \
		assert((D_INFO)->differenceKnown[i] && \
		       (D_INFO)->difference[i] == 0); \
	    } \
	} \
    }
#else
#define d_info_inv(X)
#endif

typedef ddnode *dd_in_iterator;  /* iterate thru dds in to an access */
typedef ddnode *dd_out_iterator; /* iterate thru dds out from an access */

typedef ddnode *dd_current;      /* point to the dd the iterator is on */

#define dd_current_nest(DDC)         ((DDC)->ddnest)
#define dd_current_diff(DDC)         ((DDC)->dddiff)
#define dd_current_diff_known(DDC,j) ((unsigned)(DDC)->dddiff[j] != ddunknown)
#define dd_current_dir(DDC)          ((DDC)->dddir)
#define dd_current_flags(DDC)        ((DDC)->ddflags)
#define dd_current_restr(DDC)        ((DDC)->ddrestraint)
#define dd_current_src(DDC)          ((DDC)->ddpred)
#define dd_current_dest(DDC)         ((DDC)->ddsucc)
#define dd_current_is_flow(DDC)       ((DDC)->ddtype == ddflow)
#define dd_current_is_anti(DDC)       ((DDC)->ddtype == ddanti)
#define dd_current_is_output(DDC)     ((DDC)->ddtype == ddoutput)
#define dd_current_is_reduction(DDC)  ((DDC)->ddtype == ddreduce)
#define dd_current_is_mem(DDC)        ((DDC)->ddflags & ddmem)
#define dd_current_is_val(DDC)        ((DDC)->ddflags & ddval)
#define dd_current_is_presumed_val(DDC) (!((DDC)->ddflags & ddnotPresumedVal))
#define dd_current_is_valid_for_covers(DDC) ((DDC)->ddflags & ddvalidForCovers)
#define dd_current_as_string(DDC)    brdd_print(DDC)
#define dd_current_type(DDC)         ((DDC)->ddtype)
#define dd_current_loop_dir(DDC)     ((DDC)->loop_dir)
#define dd_current_relation(DDC)     ((DDC)->dd_relation)


#define dd_i_i_for_access(ACC)  ((ACC)->nodeddin)
#define dd_i_i_done(DD_I_I)     ((DD_I_I)==0)
#define dd_i_i_next(DD_I_I)     ((DD_I_I)=(DD_I_I)->ddnextpred)
#define dd_i_i_current(DD_I_I)          (DD_I_I)
#define dd_i_i_cur_is(DD_I_I, DEP)      ((DD_I_I) == (DEP))


#define dd_o_i_for_access(ACC)  ((ACC)->nodeddout)
#define dd_o_i_done(DD_O_I)     ((DD_O_I)==0)
#define dd_o_i_next(DD_O_I)     ((DD_O_I)=(DD_O_I)->ddnextsucc)
#define dd_o_i_current(DD_O_I)          (DD_O_I)
#define dd_o_i_cur_is(DD_O_I, DEP)      ((DD_O_I) == (DEP))


/* the following are just abbreviations and don't need to be re-defined */
#define dd_i_i_cur_src(DD_I_I)          dd_current_src(dd_i_i_current(DD_I_I))
#define dd_i_i_cur_dest(DD_I_I)         dd_current_dest(dd_i_i_current(DD_I_I))
#define dd_i_i_cur_is_flow(DD_I_I)       dd_current_is_flow(dd_i_i_current(DD_I_I))
#define dd_i_i_cur_is_output(DD_I_I)     dd_current_is_output(dd_i_i_current(DD_I_I))
#define dd_i_i_cur_is_reduction(DD_I_I)  dd_current_is_reduction(dd_i_i_current(DD_I_I))
#define dd_i_i_cur_covers(DD_I_I)      dd_current_covers(dd_i_i_current(DD_I_I))

#define dd_o_i_cur_src(DD_O_I)          dd_current_src(dd_o_i_current(DD_O_I))
#define dd_o_i_cur_dest(DD_O_I)         dd_current_dest(dd_o_i_current(DD_O_I))
#define dd_o_i_cur_is_flow(DD_O_I)      dd_current_is_flow(dd_o_i_current(DD_O_I))
#define dd_o_i_cur_is_output(DD_O_I)     dd_current_is_output(dd_o_i_current(DD_O_I))
#define dd_o_i_cur_is_reduction(DD_O_I)  dd_current_is_reduction(dd_o_i_current(DD_O_I))
#define dd_o_i_cur_terminates(DD_O_I)  dd_current_terminates(dd_o_i_current(DD_O_I))


void store_dependence(ddnature nature, a_access from_access,
			 a_access to_access, dir_and_diff_info *d_info);
void write_ddnode_to_graph(ddnode *ddn);

#if 0
void store_dependence_with_rel(ddnature nature, a_access from_access,
		      a_access to_access, dir_and_diff_info *d_info,
                      const Relation&  r);
#endif

/* can we execute A1 and then A2 without going to another iteration */
bool access_lexically_preceeds(a_access A1, a_access A2);


/* convert dd nodes into stuff our functions can handle */
void ddnode_to_dir_and_diff(dd_current, dir_and_diff_info *);

/* copy info from a dir_and_diff_info into an existing dd node */
void dir_and_diff_into_ddnode(const dir_and_diff_info *ddi, dd_current);

/* take inequality number GEQ, and turn it into an assertion */
#define add_GEQ_assertion(P, VARS, GEQ) \
			 (add_assertion(petit_GEQ_expr_on_symconsts((P),(VARS),(GEQ))))

#if 0
extern node *petit_GEQ_expr_on_symconsts(Problem *p, Var_Id vars[], int geq);
extern void add_assertion(node *expr);
#endif

} // end of omega namespace

#endif


#endif
