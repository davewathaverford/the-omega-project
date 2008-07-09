/* $Id: Zima.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <basic/assert.h>
#include <omega/pres_tree.h>
#include <petit/lang-interf.h>
#include <petit/Zima.h>
#include <petit/debug.h>
#include <petit/affine.h>
#include <petit/dddir.h>
#include <petit/notedd.h>
#include <petit/dd_misc.h>
#include <petit/Exit.h>
#include <petit/petit_args.h>

namespace omega {

#define Unknown_Tuple 0

static void add_coefficients(Constraint_Handle &c, affine_expr *ae, int sign,
			     const Variable_ID_Tuple *indices,
			     Argument_Tuple of);

static void add_conditional(Formula *N, rel_expr re, bool add_negation,
                            const Variable_ID_Tuple *indices,
                            Argument_Tuple of);

static void add_loop (F_And * N, context_iterator c,
                      const Variable_ID_Tuple * indices, Argument_Tuple of);

static void add_global_assertions(F_And *N);
static void add_local_assertions(F_And *N, const AccessIteration& AI);

// can't be static because its a friend
void do_connected_by_diff(Formula *N, const AccessIteration &AI,
				 const AccessIteration &BJ, dd_current dd);

/***
 * precedes at infinately level, i.e. dependence isn't carried by any loop
 ***/

void precedes_inf(F_And *N, const AccessIteration &AI,
const AccessIteration &BJ)
    {
    int shared_depth= access_shared_depth(AI.access(), BJ.access());
    context_iterator c;


    if (!access_lexically_preceeds(AI.access(), BJ.access()))
	/* add false */
	{ EQ_Handle e=N->add_EQ();
        e.update_const(1);
	}

    else {
    c= cont_i_for_access(AI.access());
    int level=shared_depth;
    while (!cont_i_done(c))
	{
	if (cont_i_cur_is_loop(c) && (int)cont_i_cur_depth(c)<=shared_depth)
	    {
	    EQ_Handle unify=N->add_EQ();
	    Variable_ID var;

	    assert (loop_var_id(cont_i_cur_loop(c)) == AI.index_var_ids[level]);
	    assert (loop_var_id(cont_i_cur_loop(c)) == BJ.index_var_ids[level]);
//      assert (1<=level && level <= (*(AI.indices)).size());

	    var=(Variable_ID) (*(AI.indices))[level];
	    unify.update_coef(var, -1) ;
	    var=(Variable_ID) (*(BJ.indices))[level];
	    unify.update_coef(var,1);
	    unify.finalize();
	    level--;
	    }
	cont_i_next(c);
	}
    }
    } /* end preceds_inf */


/****
 * add constraints for A[I]<<B[I] at known level
 ****/

void precedes(F_And *N, const AccessIteration &AI,
const AccessIteration &BJ, int carried_level)
    {
    int shared_depth;
    context_iterator c;

    int step_sign=1;

    shared_depth= access_shared_depth(AI.access(), BJ.access());
    if (shared_depth == 0)
	{
	precedes(N, AI, BJ);
	return;
	}
    else if (carried_level == shared_depth + 1)
	{
	precedes_inf(N, AI, BJ);
	return;
	}

    assert (1 <= carried_level && carried_level<=shared_depth);

    /* go to the common loop at which dependence is carried */

    c = cont_i_for_access(AI.access());
    while (!cont_i_done(c) &&
	   ((int)cont_i_cur_depth(c)>carried_level ||
	    ((int)cont_i_cur_depth(c)==carried_level && !cont_i_cur_is_loop(c))
	       ))
	cont_i_next(c);

    // now c is the loop at which dependence is carried
    assert((int)cont_i_cur_depth(c) == carried_level);

    // build AI[c] < BJ[c]  ==  BJ[c]-AI[c]-1 >= 0

    GEQ_Handle carried=N->add_GEQ();

    if (loop_has_step(cont_i_cur_loop(c)))
	{
	bool known;
	loop_step(cont_i_cur_loop(c), &step_sign, &known);
	step_sign= (step_sign>=0)?1:-1;
	}

    Variable_ID var;
    assert (loop_var_id(cont_i_cur_loop(c)) == AI.index_var_ids[carried_level]);
    assert (loop_var_id(cont_i_cur_loop(c)) == BJ.index_var_ids[carried_level]);

//  assert (1<=carried_level && carried_level <= (*(AI.indices)).size());
    var=(Variable_ID) (*(AI.indices))[carried_level];
    carried.update_coef(var,-step_sign);

    var=(Variable_ID) (*(BJ.indices))[carried_level];
    carried.update_coef(var,step_sign);


    carried.update_const(-1);

    carried.finalize();


    /* add equality for iterators at less levels */
    cont_i_next(c);
    int level=carried_level;
    while (!cont_i_done(c))
	{
	if (cont_i_cur_is_loop(c))
	    {
            level--;
	    EQ_Handle unify=N->add_EQ();

            assert (loop_var_id(cont_i_cur_loop(c)) == AI.index_var_ids[level]);
            assert (loop_var_id(cont_i_cur_loop(c)) == BJ.index_var_ids[level]);

//          assert (1<=level && level <= (*(AI.indices)).size());

            var=(Variable_ID) (*(AI.indices))[level];
            unify.update_coef(var,-1);

            var=(Variable_ID) (*(BJ.indices))[level];
            unify.update_coef(var,1);

	    unify.finalize();

	    }
	cont_i_next(c);
	}
    } /* end precedes at known level */


/***
 * precedes at any level
 ***/

void precedes(F_And *N, const AccessIteration &AI,
const AccessIteration &BJ)
    {
    int shared_depth= access_shared_depth(AI.access(), BJ.access());


    if (shared_depth == 0)
	{
	if (!access_lexically_preceeds(AI.access(), BJ.access()))
	    /* add false */
	    {
	    EQ_Handle e=N->add_EQ();
	    e.update_const(1);
	    }
	}
    else
	/* shared depth isn't 0 */
	{
	if (shared_depth !=1)
	    {
	    F_Or * prec=N->add_or();
	    int i;
	    for (i=1; i<=shared_depth; i++)
		{
		F_And * prec_iter = prec->add_and();
		precedes(prec_iter, AI, BJ, i);
		}
	    }
	else
	    precedes(N,AI,BJ,1);
	}
    } /* end preceeds */



/*
  The following was stolen frow wak and hacked by davew.
  If it doesn't work, blame yourself or Evan.
 */

// static
void do_connected_by_diff(Formula *N, const AccessIteration &AI,
const AccessIteration &BJ, dd_current dd)
    {
    int i;

    for(i=1; i<=(int)dd_current_nest(dd); i++)
	if ((unsigned)dd_current_diff(dd)[i] == ddunknown &&
	    ddextract1(dd_current_dir(dd), i) == ddfwd+ddbck)
	    {
	    F_Or *disjunct = N->add_or();
	    dddirreset(dd_current_dir(dd), ddfwd, i);
	    do_connected_by_diff(disjunct, AI, BJ, dd);
	    dddirset(dd_current_dir(dd), ddfwd, i);
	    dddirreset(dd_current_dir(dd), ddbck, i);
	    do_connected_by_diff(disjunct, AI, BJ, dd);
	    return;
	    }

    F_And *conjunct = N->add_and();

    for(i=1; i<=(int)dd_current_nest(dd); i++)
	{
	if ((unsigned)dd_current_diff(dd)[i] != ddunknown)
	    {
	    EQ_Handle current_eq = conjunct->add_EQ();
	    current_eq.update_const(dd_current_diff(dd)[i]);
	    current_eq.update_coef((*AI.indices)[i], 1);
	    current_eq.update_coef((*BJ.indices)[i], -1);
	    current_eq.finalize();
	    }
	else if (i >= 1)
	    {
	    dddirection thisdd;
	    thisdd = ddextract1(dd_current_dir(dd), i);
	    if (dd_current_loop_dir(dd)[i] < 0)
		{
		dddirection corrected = 0;
		if (thisdd & ddfwd) corrected |= ddbck;
		if (thisdd & ddbck) corrected |= ddfwd;
		if (thisdd & ddind) corrected |= ddind;
		thisdd = corrected;
		}

	    if (thisdd == ddfwd)
	        {
	        GEQ_Handle current_geq = conjunct->add_GEQ();
		current_geq.update_const(-1);
		current_geq.update_coef((*AI.indices)[i], -1);
		current_geq.update_coef((*BJ.indices)[i], 1);
		current_geq.finalize();
	        }
	    if (thisdd == ddbck)
	        {
	        GEQ_Handle current_geq = conjunct->add_GEQ();
		current_geq.update_const(-1);
		current_geq.update_coef((*AI.indices)[i], 1);
		current_geq.update_coef((*BJ.indices)[i], -1);
		current_geq.finalize();
	        }
	    if (thisdd == ddind)
	        {
	        EQ_Handle current_eq = conjunct->add_EQ();
		current_eq.update_coef((*AI.indices)[i], 1);
		current_eq.update_coef((*BJ.indices)[i], -1);
		current_eq.finalize();
	        }
	    if (thisdd == ddind+ddfwd)
	        {
	        GEQ_Handle current_geq = conjunct->add_GEQ();
		current_geq.update_coef((*AI.indices)[i], -1);
		current_geq.update_coef((*BJ.indices)[i], 1);
		current_geq.finalize();
		}
	    if (thisdd == ddind+ddbck)
	        {
	        GEQ_Handle current_geq = conjunct->add_GEQ();
		current_geq.update_coef((*AI.indices)[i], 1);
		current_geq.update_coef((*BJ.indices)[i], -1);
		current_geq.finalize();
	        }
            }
	}
    }

void connected_by_diff(Formula *N, const AccessIteration &AI,
const AccessIteration &BJ, dd_current dd)
    {
    do_connected_by_diff(N, AI, BJ, dd);
    }


//
// For A[I] and B[J] to access the same memory location,
// their subscripts must be equal and, if the array is
// private within a loop, we must be in the same iteration
// of that loop.  For private variables inside an if, we
// must have taken the branch containing the variable for
// the access to occur at all.
//
void access_same_memory(F_And *N, const AccessIteration &AI,
			const AccessIteration &BJ)
    {

    sub_iterator sub1, sub2;

    if(AI.access()==Entry || AI.access()==ExitNode) {
	 if (access_is_private_var(BJ.access())) {
	   N->add_or();
	}
       return;
       }

   
    if(BJ.access()==Entry || BJ.access()==ExitNode) {
	 if (access_is_private_var(AI.access())) {
	   N->add_or();
	}
      return;
    }

    assert(access_sym(AI.access()) == access_sym(BJ.access()));

    sub1 = sub_i_for_access(AI.access());
    sub2 = sub_i_for_access(BJ.access());

    while (!sub_i_done(sub1))
	{
	assert(!sub_i_done(sub2));  /* must have same # of subscripts */
	assert(sub_i_cur_affine(sub1) != NULL && sub_i_cur_affine(sub2) != NULL);
	assert(!is_affine(sub_i_cur_affine(sub1)) ||
	       sub_i_cur_affine(sub1)->other_branch == NULL);  // no min/max
	assert(!is_affine(sub_i_cur_affine(sub2)) ||
	       sub_i_cur_affine(sub2)->other_branch == NULL);  // no min/max

	if (is_affine(sub_i_cur_affine(sub1)) &&
	    is_affine(sub_i_cur_affine(sub2)))
	    {
	    EQ_Handle e = N->add_EQ();

	    add_coefficients(e, sub_i_cur_affine(sub1), -1, AI.indices, AI.of);
	    add_coefficients(e, sub_i_cur_affine(sub2),  1, BJ.indices, BJ.of);
	    e.finalize();
	    }
	else
	    N->add_unknown();

	sub_i_next(sub1);
	sub_i_next(sub2);
	}
    assert(sub_i_done(sub2));


    if (access_is_private_var(AI.access()))
	{
	int private_level = access_private_var_level(AI.access());

	context_iterator ac;
	ac = cont_i_for_access(AI.access());
	while (!cont_i_done(ac) &&
	       ((int)cont_i_cur_depth(ac) > private_level ||
		!cont_i_cur_is_loop(ac)))
	    cont_i_next(ac);

#if ! defined NDEBUG
	context_iterator bc;
	bc = cont_i_for_access(BJ.access());
	while (!cont_i_done(bc) &&
	       ((int)cont_i_cur_depth(bc) > private_level ||
		!cont_i_cur_is_loop(bc)))
	    cont_i_next(bc);

	assert((int)cont_i_cur_depth(ac) == private_level);
	assert(cont_i_cur_is_loop(ac) && cont_i_cur_is_loop(bc));
	assert((int)cont_i_cur_depth(bc) == private_level);
#if ! defined NOT_PETIT
	assert(ac == bc);
#endif
#endif

	while (!cont_i_done(ac))
	    {
	    if (cont_i_cur_is_loop(ac))
		{
		EQ_Handle e = N->add_EQ();
		e.update_coef((*AI.indices)[cont_i_cur_depth(ac)], 1);
		e.update_coef((*BJ.indices)[cont_i_cur_depth(ac)], -1);
		e.finalize();
		}
	    else
		{
		assert (cont_i_cur_is_if(ac));
#if ! defined NOT_PETIT
		assert(access_is_in_then_or_else_of(AI.access(),
						    cont_i_cur_if(ac)));
#endif

		add_conditional(N, get_if_rel_expr(ac),
                                if_else_branch(cont_i_cur_if(ac)),
				AI.indices, AI.of);
		}
	    cont_i_next(ac);
	    } /* end while */
	}
    }


/* These  function are used to add the constraints on the
   dependencies from Entry or to Exit. 
 */

void subscripts_equal_to_tuple (F_And *N, const AccessIteration &AI,
                                const Variable_ID_Tuple *t)
{
 int dim=1;

 assert (AI.access() != Entry && AI.access() != ExitNode);
 sub_iterator sub=sub_i_for_access(AI.access());
    while (!sub_i_done(sub))
	{
	assert(sub_i_cur_affine(sub) != NULL);
	assert(!is_affine(sub_i_cur_affine(sub)) ||
	       sub_i_cur_affine(sub)->other_branch == NULL);  // no min/max

	if (is_affine(sub_i_cur_affine(sub)))
	    {
	    EQ_Handle e = N->add_EQ();

	    add_coefficients(e, sub_i_cur_affine(sub), -1, AI.indices, AI.of);
            e.update_coef((*t)[dim], 1);
	    e.finalize();
	    }
	else
	    N->add_unknown();

	sub_i_next(sub);
        dim++;
	}
}


          




void access_in_bounds(F_And *N, const AccessIteration &AI, context_iterator already_done, bool assertions)
    {
    context_iterator c;

    if(AI.access()==Entry || AI.access()==ExitNode)
	return;

    c = cont_i_for_access(AI.access());

    context_in_bounds(N,AI,c,already_done, assertions);
    }

void context_in_bounds(F_And *N, const AccessIteration &AI, 
	context_iterator c,
	context_iterator already_done,
	bool assertions)
    {

    while (!cont_i_done(c))
	{
	if (already_done == c)
	    {
	    return;
	    }
	assert(already_done == 0 || node_depth(c) >= node_depth(already_done));

	if (cont_i_cur_is_loop(c))
	    {
	    add_loop(N, c, AI.indices, AI.of);
	    }
	else /* current context is not a loop */
	    {
	    assert (cont_i_cur_is_if(c));
#if ! defined NOT_PETIT
	    assert(c->nodeparent->nodeop == op_if);
#endif
	    if (access_is_in_then_or_else_of(AI.access(),cont_i_cur_if(c)))
		{
		add_conditional(N, get_if_rel_expr(c),
                                if_else_branch(cont_i_cur_if(c)),
				AI.indices, AI.of);
		}
	    else
		{
#if ! defined NOT_PETIT
		assert(0 && "How can we possibly get HERE??");
#endif
		}
	    }

	cont_i_next(c);
	} /* end while */

#if ! defined NOT_PETIT
    if (already_done == 0 && assertions) {
	    add_local_assertions(N,AI);
	    add_global_assertions(N);
	    }
#endif
    }






static void add_loop (F_And * N, context_iterator c,
                      const Variable_ID_Tuple * indices, Argument_Tuple of)
 {
	Variable_ID index = (*indices)[cont_i_cur_depth(c)];
	int incr = 1;

	if (loop_has_step(cont_i_cur_loop(c)))
		{
		bool known = true;
		loop_step(cont_i_cur_loop(c), &incr, &known);
                if (!known)
                  Error("Cannot handle unknown loop step");

		if (incr!=1)
		    {
		    assert (known == 1);
		    Stride_Handle step=N->add_stride(incr);
		    step.update_coef(index,1);
		    add_coefficients(step, loop_start(cont_i_cur_loop(c)), -1,
				                 indices, of);
		    }
		}

	incr=(incr>=0)?1:-1;

	if (loop_start_is_affine(cont_i_cur_loop(c)))
    for (affine_expr *ae = loop_start(cont_i_cur_loop(c));
		     ae;
		     ae = ae->other_branch)
		  {
		   GEQ_Handle lower = N->add_GEQ();
		   add_coefficients(lower, ae, -incr, indices, of);
		   lower.update_coef(index, incr);
		   lower.finalize();
		  }
	else
		N->add_unknown();

	if (loop_end_is_affine(cont_i_cur_loop(c)))
		for (affine_expr *ae = loop_end(cont_i_cur_loop(c));
		     ae;
		     ae = ae->other_branch)
		    {
		    GEQ_Handle upper = N->add_GEQ();
		    add_coefficients(upper, ae, incr, indices, of);
		    upper.update_coef(index, -incr);
		    upper.finalize();
		    }
	 else
		 N->add_unknown();
 }




/*
 * for each term in ae, add sign*coefficient to the right variable in c
 */
static void add_coefficients(Constraint_Handle &c, affine_expr *ae, int sign,
const Variable_ID_Tuple *indices,
Argument_Tuple of)
    {
    int v;

    assert(is_affine(ae));
    assert(ae->terms[0].petit_var == NULL);
    c.update_const(sign * ae->terms[0].coefficient);

    for (v=1; v<ae->nterms; v++)
	{
	Variable_ID var;
	if (var_id_is_index(ae->terms[v].petit_var))
	    {
	    assert(indices);
	    var = (*indices)[var_id_loop_no(ae->terms[v].petit_var)];
	    }
	else
	    {
            Omega_Var *ov = get_omega_var(ae->terms[v].petit_var,
                                          ae->terms[v].arity);
	    if (ov->arity() > 0)
		{
		assert(of == Input_Tuple || of == Output_Tuple);
		var = c.get_local(ov, of);
		}
	    else
		{
		var = c.get_local(ov);
		}
	    }

	c.update_coef(var, sign * ae->terms[v].coefficient);
	}
    }

static void add_cond_term(F_And *NA, if_compare_operators kind,
                         affine_expr *left, affine_expr * right,
                         const Variable_ID_Tuple *indices,
                         Argument_Tuple of);

static void add_conditional(Formula *N, rel_expr re, bool add_negation,
                            const Variable_ID_Tuple *indices,
                            Argument_Tuple of)
{
  if (rel_expr_is_term(re)) {
    term t = rel_expr_term(re);
    F_And *N_and=N->add_and();

    if (rel_expr_term_is_ok(t)) {
        if_compare_operators op=term_compare_op(t);
        if (add_negation)
           op=negate_compare_operator(op);
        add_cond_term(N_and, op, term_compare_left(t),
                    term_compare_right(t), indices,of);
      }
    else
      N_and->add_unknown();
    }
  else {
       rel_operators rel_op = rel_expr_operator(re);
       assert (rel_operator_is_ok(rel_op));
       switch(rel_op) {
       case rel_and:
	   {
           if (add_negation)
              N=N->add_not();
           F_And * N_and = N->add_and();
           add_conditional(N_and, rel_expr_operand_1(re), false, indices, of);
           add_conditional(N_and, rel_expr_operand_2(re), false, indices, of);
           break;
	   }
       case rel_or:
	   {
           if (add_negation)
              N=N->add_not();
           F_Or * N_or = N->add_or();
           add_conditional(N_or, rel_expr_operand_1(re), false, indices, of);
           add_conditional(N_or, rel_expr_operand_2(re), false, indices, of);
           break;
	   }
       case rel_not:
	   {
           if (!add_negation)
             N = N->add_not();
           add_conditional(N, rel_expr_operand_1(re), false, indices, of);
           break;
	   }
       default:
	   {
           assert (0 && "Cannot get here");
	   }
       }
   }
}




/****
 * add_cond_term - add a term (comparison ) to the formula
 ***/

static void add_cond_term(F_And* NA, if_compare_operators kind,
affine_expr *left, affine_expr * right,
const Variable_ID_Tuple *indices,
Argument_Tuple of)
    {
    assert(left && right && is_affine(left) && is_affine(right));

    if (kind == equal) {
    EQ_Handle if_cond=NA->add_EQ();
    add_coefficients(if_cond,left,1,indices,of);
    add_coefficients(if_cond,right,-1,indices,of);
    if_cond.finalize();
    }

    else if (kind==not_equal) {
    F_Or * ne=NA->add_or();

    GEQ_Handle if_cond=(ne->add_and())->add_GEQ();
    add_coefficients(if_cond, left,1,indices,of);
    add_coefficients(if_cond, right,-1,indices,of);
    if_cond.update_const(-1);
    if_cond.finalize();

    if_cond=(ne->add_and())->add_GEQ();
    add_coefficients(if_cond, left,-1,indices,of);
    add_coefficients(if_cond, right,1,indices,of);
    if_cond.update_const(-1);
    if_cond.finalize();
    }

    else {
    GEQ_Handle if_cond = NA->add_GEQ();
    int sign = (kind==greater || kind== greater_eq)?1:-1;

    add_coefficients(if_cond, left, sign,indices,of);
    add_coefficients(if_cond, right, -sign,indices,of);
    if (kind==less || kind==greater)
	if_cond.update_const(-1);
    if_cond.finalize();
    }

    } /* end add_cond_term */


// ASSERTION FUNCTIONS

// Assertions are considered to be global or local.
// The assertion is global, if its vars are  program constants
// or variables with arity 0 and there are no loops or ifs surrounding
// it.


static  void vars_change(Var_Id v, int arity, void * args)
 {
  bool * res=(bool *) args;
  if (!(var_id_is_const(v) || (var_id_is_var(v) && arity == 0)))
    *res=true;
 }


static bool assert_is_global(pASSERT a)
{
  context_iterator c=cont_i_for_assert(a);

  if (!cont_i_done(c)) // there is a surrounding loop or if
    return false;
  else {
   bool is_local=false;
   assert_map_over_vars_affine(a, vars_change, &is_local);
   return !is_local;
 }
}

static void add_global_assertions(F_And *N)
    {
    for (assert_iterator c=assert_i_start(); !assert_i_done(c);
          c=assert_i_next(c))
        {
#if ! defined NOT_PETIT
	assert(c->nodeop == op_assert);
#endif
        if (assert_is_global(assert_i_cur(c))) 
          {
           add_conditional (N, get_assert_rel_expr(assert_i_cur(c)),
                            false, 0, (Argument_Tuple) Unknown_Tuple);
	 }
	}
    }

//A local assertion is a consecquent of all loop and if conditions
//surrounding it that are not shared with the access. All non-shared
//loop indices should be universaly quantified.  The universal
//quantification should happen just once for each index var, even
//if there are several assertions that use it. The sturcture
//context_descr_for_assert is used to keep track of the surrounding
//context.


void add_one_local_assertion(pASSERT ASR, const AccessIteration &AI,
                             Formula *N,
                             List<context_descr_for_assert *> L) 
{

	 int i;
	 int shared_depth=access_assert_shared_depth(AI.access(),ASR);
	 int assert_depth=access_depth(ASR);

	 Variable_ID_Tuple indices(assert_depth);
	 for (i=1; i<=shared_depth; i++)
	 	 indices[i]=(*AI.indices)[i];


	 //try to find a surrounding context in the list
	 context_iterator ci;
	 bool found=false;
	 int n_loop=0; //number of the loop contexts to be added in the list
	 int n_context=0; //number of the contexts (loop and if) to be added in the list
	 context_descr_for_assert* cd=NULL; //dscrp of the innermost found context

     
	 ci=cont_i_for_assert(ASR); 
         context_iterator access_ci=cont_i_for_access(AI.access());
         while (!cont_i_done(ci)&& !found && ci!=access_ci)
	 	{
	 		found=false;
	 		for (List_Iterator<context_descr_for_assert *> li(L); li; li++)
	 		 {
	 			 if (li.curr()->ci == ci)
	 				 {
	 					found=true;
	 					cd=li.curr();
	 					break;
	 				 }
	 		 }
	 		if (!found)
	 		{
	 		 if (cont_i_cur_is_loop(ci))
	 				n_loop++;
	 		 n_context++;
	 		}
                        cont_i_next(ci);
	 	}

	 // add loop variables of the loops that are already in the list to
	 // the indices.
	 if (found)
	  {
	 	 context_descr_for_assert *p;
	 	 for (i=assert_depth-n_loop,p=cd; i>=shared_depth; i--,cd=cd->prev_cd)
	 		 {
	 			assert(p!=NULL);
	 			indices[i]=p->v;
	 		 }

	  }


	 //for each context that is not in the list starting from the outer ones,
	 //add its conditions as a l.h.s. of the implication to the formula,
	 //universally quantify its index if it is loop, and add it to the list

	 for (i=n_context; i>=1; i--) {
                ci=cont_i_for_assert(ASR);
	 	for (int j=1; j<i; j++)
                    cont_i_next(ci);

	        assert(!cont_i_done(ci));
	 	context_descr_for_assert * cur_cd=new context_descr_for_assert;
	 	cur_cd->ci=ci;
	 	cur_cd->prev_cd=cd;



	 	if (cont_i_cur_is_if(ci))
	 		{
	 		 F_Or * N_Or=(cd?cd->N->add_or(): N->add_or());
	 		 cur_cd->N=N_Or;
	 		 add_conditional(N_Or, get_if_rel_expr(cont_i_cur_if(ci)),
	                               !if_else_branch(cont_i_cur_if(ci)),
	 			       &indices, AI.of);
	 		 cur_cd->v=NULL;
	 		}
	 	else
	 	  {
	 		 assert(cont_i_cur_is_loop(ci));
	 		 F_Forall * N_Forall=(cd?cd->N->add_forall() : N->add_forall());
	 		 Variable_ID cur_index = N_Forall->declare(
	 				              var_id_name(loop_var_id(cont_i_cur_loop(ci))));

	 		 indices[assert_depth-i+1]=cur_index;
	 		 cur_cd->v=cur_index;

	 		 F_Or * N_Or=N_Forall->add_or();
	 		 cur_cd->N=N_Or;
	 		 add_loop(N_Or->add_not()->add_and(), ci, &indices, AI.of);
	 		}

	  cd=cur_cd;
   }


   //add assertion itself

   add_conditional (cd?cd->N:N, get_assert_rel_expr(ASR),
                    false, &indices,AI.of);
}


static void add_local_assertions(F_And *N, const AccessIteration& AI)
    {

		List<context_descr_for_assert *> L;
    for (assert_iterator c=assert_i_start(); !assert_i_done(c);
          c=assert_i_next(c))
        {
#if ! defined NOT_PETIT
	 assert(c->nodeop == op_assert);
#endif
         if (!assert_is_global(assert_i_cur(c)))
	     add_one_local_assertion(assert_i_cur(c), AI, N,L);
	}
    }




static void load_index_var_ids(AccessIteration *AI, List<Var_Id> *t);


void AccessIteration::load_all_vars()
    {
    if (acc!=Entry && acc!=ExitNode)
	{
	load_index_var_ids(this, &index_var_ids);
	}
    }

AccessIteration::AccessIteration() : acc(NULL), rel(NULL), indices(NULL),
        of((Argument_Tuple)Unknown_Tuple)
   {
   }



AccessIteration::AccessIteration(a_access a, Relation *r,
                 const Variable_ID_Tuple *t) : acc(a), rel(r), indices(t)
    {
      if (t == &input_vars) {
         of = Input_Tuple;
         assert(r->n_inp() == depth());
       }
      else if (t == &output_vars) {
         of = Output_Tuple;
         assert(r->n_out() == depth());
       }
      else
         of = (Argument_Tuple)Unknown_Tuple;

      load_all_vars();
      name();
    }


AccessIteration::AccessIteration(a_access a, Relation *r,
                 Argument_Tuple _of) : acc(a), rel(r), of(_of)
    {
      assert (_of!=Unknown_Tuple &&
              "AccessIteration cannot be created for unknown tuple\n");
      switch (_of) {
      case   Input_Tuple:
          indices=&input_vars;
          if (r->is_set()) {
            assert(r->n_set() == depth());
	  }
          else {
            assert (r->n_inp() == depth ());
	  }
          break;
      case   Output_Tuple:
          indices=&output_vars;
          assert (r->n_out() == depth());
          break;
	default:
          assert (0 &&
             "AccessIteration cannot be created for unknown tuple\n");
       }

      load_all_vars();
      name();
    }



void AccessIteration::name()
{
context_iterator c;
enum {input, output,set, sym} local_name_type=sym;

bool first=true;

c=cont_i_for_access(access());
while ( !cont_i_done(c) ) {
if (cont_i_cur_is_loop(c)) {

if (first)
    {
    first=false;
    if (rel->is_set())
	local_name_type=set;
    else if ((*indices)[1]->kind()==Input_Var)
	local_name_type=input;
    else if ((*indices)[1]->kind()==Output_Var)
	local_name_type=output;
    else
	local_name_type=sym;
    }


switch(local_name_type)
    {
case input:
    rel->name_input_var(cont_i_cur_depth(c),
		      var_id_name(loop_var_id(cont_i_cur_loop(c))));
    break;
case output:
    rel->name_output_var(cont_i_cur_depth(c),
		       var_id_name(loop_var_id(cont_i_cur_loop(c))));
    break;
case set:
    rel->name_set_var(cont_i_cur_depth(c),
		     var_id_name(loop_var_id(cont_i_cur_loop(c))));
    break;
case sym:
default:
    (*indices)[cont_i_cur_depth(c)]->name_variable(
	var_id_name(loop_var_id(cont_i_cur_loop(c))));
    break;
    }
}
cont_i_next(c);
}
												 } /* end bind_and_name */


int AccessIteration::depth()
    {
    return access_depth(acc);
    }


static void
load_index_var_ids(AccessIteration *AI, List<Var_Id> *vs)
    {
    context_iterator c;
#if ! defined NDEBUG
    int n = AI->depth();
#endif

    c = cont_i_for_access(AI->access());
    while ( !cont_i_done(c) )
	{
	if ( cont_i_cur_is_loop(c)  ){
	vs->prepend(loop_var_id(cont_i_cur_loop(c)));
#if ! defined NDEBUG
	n--;
#endif
	}
	cont_i_next(c);
	}
#if ! defined NDEBUG
    assert(n == 0);
#endif
    }


/*
 * Functions for splitting dependence relations into direction vectors
 */


/* info used during construction */
struct unknowns{
    int unknownDirection[maxCommonNest+1];
    int unknownDirections;
};

/*
   process the omega test problem into dependence vectors
   if   dependencies are not coupled, just read them out.
   else break into cases for each possible dependence in (+,0,-)
        in each dimension by doing a recursive call
   each time a dependence vector is found, call keep_dependence to store it
 */

static void
findDirectionVector(Relation &dep_rel, situation *sit,
dir_and_diff_info *d_info, unknowns * /*u_info*/)
    {
    int i;
    bool selfDep = sit->access1 == sit->access2;

    assert(d_info->direction == (unsigned) -1 &&
	   d_info->restraint == (unsigned) -1);
    for (i = 1; i<=(int)d_info->nest; i++)
	d_info->differenceKnown[i] = 0;

    if (d_info->nest) dep_rel.make_level_carried_to(d_info->nest);
    for (int level = 1; level<=(int)d_info->nest; level++)
	{
#if ! defined NDEBUG
	for (i=1; i<level; i++)
	    assert(ddextract1(d_info->direction, i) == ddind);
#endif

	if (petit_args.dep_analysis_debug >= 1)
	    {
	    fprintf(debug,
		    "considering dependences carried at depth %d%s\n",
		    level, petit_args.dep_analysis_debug > 1 ? " in:" : "");
	    if (petit_args.dep_analysis_debug >= 3)
		dep_rel.prefix_print(debug);
	    }

        int loop_dir=d_info->loop_dir[level]; 
	Relation fwd = dep_rel.extract_dnf_by_carried_level(level, loop_dir);
	Relation bwd;
	if(!selfDep)
	    bwd = Inverse(dep_rel.extract_dnf_by_carried_level(level, -loop_dir));
                        

	// we need to put in a restraint vector if > 1 possibility
	int n_possible = ((dep_rel.query_DNF()->length() > 0) +
			  (fwd.query_DNF()->length() > 0) +
			  (bwd.query_DNF()->length() > 0));

	if (!n_possible)
	    break;

	for (int s=selfDep?1:-1; s<=1; s+=2)
	    {
	    Relation &this_part = (s > 0 ? fwd : bwd);

	    if (this_part.query_DNF()->length() > 0)
		{
		if (petit_args.dep_analysis_debug >= 1)
		    {
		    fprintf(debug,
			    "Dependences carried by level %d, sign %d:\n",
			    level, s);
		    if (petit_args.dep_analysis_debug >= 2)
			this_part.prefix_print(debug);
		    }

		if (n_possible > 1)
		    dddirsetonly(d_info->restraint, ddfwd, level);

		for (int l = level; l<=(int)d_info->nest; l++)
		    {
		    int lb, ub;
		    bool guaranteed;
		    dddirection signs = 0;
		    this_part.query_difference(output_var(l), input_var(l),
					       lb, ub, guaranteed);

		    if (l == level)
			{
			signs = ddfwd;
			}
		    else
			{
			if (lb < 0) signs |= 
                            (d_info->loop_dir[l]>0)?ddbck:ddfwd;
			if (ub > 0) signs |= 
                            (d_info->loop_dir[l]>0)?ddfwd:ddbck;
			if (lb <=0 && ub >= 0) signs |= ddind;
			}

#if ! defined NDEBUG
		    if (l == level && 
                         ((lb <= 0 && d_info->loop_dir[i]>0) 
                        ||(ub >=0  && d_info->loop_dir[i]<0)))
			{
			// querydifference must be approximate:
			// this_part && out[l]-in[l] <= 0 must be false
			Relation confirm_that = this_part;
			GEQ_Handle g = confirm_that.and_with_GEQ();
			g.update_coef(output_var(l), 
                                      -d_info->loop_dir[level]);
			g.update_coef(input_var(l),  
                                       d_info->loop_dir[level]);
			//confirm_that.prefix_print(debug);
			//confirm_that.simplify();
			//confirm_that.prefix_print(debug);
			if(confirm_that.is_upper_bound_satisfiable()) {
				fprintf(debug,"Oops.. level carried info incorrect\n");
				fprintf(debug,"dependence should be carried at lvl %d\n",level);
				fprintf(debug,"We have a report of bounds of %d <= Out_%d-In_%d <= %d\n",lb,l,l,ub);
				if (guaranteed) fprintf(debug,"query_difference quanrantees this\n");
				fprintf(debug,"Relation is:\n");
				this_part.prefix_print(debug);
				}
			assert(!confirm_that.is_upper_bound_satisfiable());
			}
#endif

		    dddirsetonly(d_info->direction, signs, l);
		    if (lb == ub)
			{
			d_info->differenceKnown[l] = 1;
			d_info->difference[l] = lb;
			}
		    else
			d_info->differenceKnown[l] = 0;
		    }

		d_info->dd_relation = &this_part;
		if (s>0)
		    store_dependence(sit->oitype, sit->access1,
				     sit->access2, d_info);
		else
		    store_dependence(sit->iotype, sit->access2,
				     sit->access1, d_info);
		}
	    }
	dddirsetonly(d_info->direction, ddind, level);
	d_info->differenceKnown[level] = 1;
	d_info->difference[level] = 0;
	if (n_possible > 1)
	    dddirsetonly(d_info->restraint, ddind, level);
	}

    if (dep_rel.query_DNF()->length() > 0)
	{
	if (access_lexically_preceeds(sit->access1, sit->access2))
	    {
	    if (petit_args.dep_analysis_debug >= 3)
		{
		fprintf(debug,"Loop independent 1-->2 dependences are:\n");
		dep_rel.prefix_print(debug);
		}

	    assert(!access_lexically_preceeds(sit->access2, sit->access1));
	    d_info->dd_relation = &dep_rel;
	    store_dependence(sit->oitype, sit->access1,
			     sit->access2, d_info);
	    }
	else if (access_lexically_preceeds(sit->access2, sit->access1))
	    {
	    dep_rel = Inverse(dep_rel);

	    if (petit_args.dep_analysis_debug >= 3)
		{
		fprintf(debug,"Loop independent 2-->1 dependences are:\n");
		dep_rel.prefix_print(debug);
		}

	    d_info->dd_relation = &dep_rel;
	    store_dependence(sit->iotype, sit->access2,
			     sit->access1, d_info);
	    }
	}

    } /* findDirectionVector */



/* add constraints for direction  */

static void add_direction_constraint(
        F_And * N, Variable_ID v1, Variable_ID v2, dddirection dir)
{
  if (dir == ddfwd+ddbck)
    {
     F_Or * disjunct = N->add_or();
     add_direction_constraint(disjunct->add_and(), v1,v2, ddfwd);
     add_direction_constraint(disjunct->add_and(), v1,v2, ddbck);
    }
  else
    {
      if (dir == ddfwd)
      {
                GEQ_Handle current_geq = N->add_GEQ();
                current_geq.update_const(-1);
                current_geq.update_coef(v1, -1);
                current_geq.update_coef(v2, 1);
                current_geq.finalize();
      }
      else if (dir == ddbck)
      {
	        GEQ_Handle current_geq = N->add_GEQ();
		current_geq.update_const(-1);
		current_geq.update_coef(v1, 1);
		current_geq.update_coef(v2, -1);
		current_geq.finalize();
      }
      else if (dir == ddind)
      {
	        EQ_Handle current_eq = N->add_EQ();
		current_eq.update_coef(v1, 1);
		current_eq.update_coef(v2, -1);
		current_eq.finalize();
      }
      else if (dir == ddind+ddfwd)
      {
	        GEQ_Handle current_geq = N->add_GEQ();
		current_geq.update_coef(v1, -1);
		current_geq.update_coef(v2, 1);
		current_geq.finalize();
      }
      else if (dir == ddind+ddbck)
      {
	        GEQ_Handle current_geq = N->add_GEQ();
		current_geq.update_coef(v1, 1);
		current_geq.update_coef(v2, -1);
		current_geq.finalize();
      }

    } /* end else */
} /* end add_direction_constraint */




/* add relation to the dd_node. Add direction vector(difference)
   constraints to the relation
 */

static void add_relation(dd_current dd, void * /*args*/) {
  assert(dd_current_relation(dd)!=NULL);

  if ((int)dd_current_restr(dd)!=-1) { /* there is information to add to relation */
    /* get relation with restrain information */
    if (petit_args.dep_analysis_debug >= 2) {
      fprintf(debug, "\n>>> restraining relation for dependence \n%s\n",
	      dd_current_as_string(dd));
    }

    dd_current_relation(dd)->uncompress();
    Relation *prel = dd_current_relation(dd);

    if (petit_args.dep_analysis_debug >= 3) {
      fprintf(debug,">>> original relation:\n");
      prel->prefix_print(debug);
    }

    F_And *restr_and = prel->and_with_and();
    for (int i=1; i<=(int)dd_current_nest(dd); i++) {
      dddirection thisdd = ddextract1(dd_current_restr(dd), i);
      if (thisdd != ddall) {
	Variable_ID v1 = prel->input_var(i);
	Variable_ID v2 = prel->output_var(i);
        dddirection onedd = 0;
        if ((dd_current_loop_dir(dd))[i] <0)
          {
           if (ddtest1(thisdd, ddind))
              ddset1(onedd, ddind);
           if (ddtest1(thisdd, ddfwd))
              ddset1(onedd, ddbck);
           if (ddtest1(thisdd, ddbck))
              ddset1(onedd,ddfwd);
          }
        else
          onedd=thisdd;   
	add_direction_constraint(restr_and, v1, v2, onedd);
      }
    }

    prel->simplify();

    if (petit_args.dep_analysis_debug >= 3) {
      fprintf(debug,">>> restrained relation:\n");
      prel->prefix_print(debug);
    }

    dd_current_relation(dd)->compress();
    dd_current_restr(dd) = (unsigned) -1;
  } /* if */
} /* end add_relation */


static void fix_relations(a_access a1, a_access a2, int bnest) {
       o_dd_split_between_nodes(a1, a2);
       o_dd_merge_between_nodes(a1, a2, bnest, 0);
       if (a1!=a2)
	 {
          o_dd_split_between_nodes(a2, a1);
          o_dd_merge_between_nodes(a2, a1, bnest, 0);
	}

    map_over_dd(a1, a2, add_relation,NULL);

    if (a1!=a2)
        map_over_dd(a2, a1, add_relation,NULL);


} /* end fix_relations */


/*
   calculateDDVectors just calls findDirectionVector now.

   The arrays dddir[] and dddiff[] need to have at least
   maxnest+1 spaces allocated

   flags - are flags to be set for calculated dependencies

*/

void calculateDDVectors(Relation *dep_rel, a_access access1, a_access access2,
			ddnature oitype, ddnature iotype,
			unsigned int bnest, signed char *loop_dirs, dd_flags flags
                       )
    {
    int i;
    situation sit;
    unknowns u_info;
    dir_and_diff_info d_info;

    d_info.nest = bnest;


    for(i=1; i<=(int)bnest; i++)
        d_info.loop_dir[i] = loop_dirs[i];

    u_info.unknownDirections = bnest;
    for(i=0;i<u_info.unknownDirections;i++)
	u_info.unknownDirection[i] = i+1;


    sit.access1 = access1;
    sit.access2 = access2;
    sit.oitype = oitype;
    sit.iotype = iotype;
    sit.nest1 = dep_rel->n_inp();
    sit.nest2 = dep_rel->n_out();
    sit.commonNesting = bnest;

    for (i=1; i<=(int)bnest; i++)
       dddirset( d_info.direction,ddall,i);

    d_info.flags = flags;
    d_info.restraint = (unsigned) -1;

    // prevent uninitialized memory read when we pass direction to
    // brdd_print, which then ignores it if nest == 0
    d_info.direction = (unsigned) -1;

    if (petit_args.dep_analysis_debug >= 2) {
      fprintf(debug,"*** Calculate DD vector ***\n");
    }

    if (dep_rel->is_upper_bound_satisfiable()) {
	    /* d_info built by findDirectionVector */
	    findDirectionVector(*dep_rel, &sit, &d_info, &u_info);
	    }
    }



void generateAllDDVectors(a_access access1,a_access access2,
			  ddnature oitype, ddnature iotype,
			  unsigned int bnest, signed char *loop_dirs, dd_flags flags,
                          Relation & dd_rel)
    {

    int i;
    situation sit;
    dir_and_diff_info d_info;

    d_info.nest = bnest;


    for(i=1; i<=(int)bnest; i++) 
	{
        d_info.loop_dir[i] = loop_dirs[i];
	d_info.differenceKnown[i] = 0;
	}


    sit.access1 = access1;
    sit.access2 = access2;
    sit.oitype = oitype;
    sit.iotype = iotype;
    sit.nest1 = node_depth(access1);
    sit.nest2 = node_depth(access2);
    sit.commonNesting = bnest;
    
    d_info.direction = (unsigned) -1;
    d_info.flags=flags; 

    d_info.restraint = (unsigned) -1;

    d_info.dd_relation=&dd_rel; 

    /* d_info built by findDirectionVector */
    noteDependence(&sit, &d_info);
   
    fix_relations(access1, access2, bnest);    
    }

} // end of namespace omega
