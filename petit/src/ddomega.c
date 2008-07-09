/* $Id: ddomega.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */


/* 
   Basic data dependence test using the omega test -
   dd_omega_test determines if two array accesses might access the same element

   Also, functions to manipulate the structures used to
   represent various sorts of problems that we pass to the
   omega test.

   Naming convention: Many of these functions and structures
   refer to "read iteration" or "write iteration" as if a
   test were being performed on flow dependence(s), even when
   the test works for other forms of dependence.
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <basic/bool.h>
#include <basic/assert.h>
#include <petit/Exit.h>
#include <petit/lang-interf.h>
#include <petit/Zima.h>
#include <petit/debug.h>
#include <petit/ddomega.h>
#include <petit/timeTrials.h>
#include <petit/affine.h>
#include <petit/petit_args.h>

namespace omega {

#define approximateTrivialDependences 0

void GetLoopDirs(a_access, int, signed char *);


/* adjust the problem to include equality of subscript expressions at
   nodes access1 and access2.  Index variables for access1 are looked
   up in range i1, and those for access2 in range i2.  The non-index
   variables are looked up in the range non_i.
   
   returns 0 if subscripts could not possibly be equal.
             In this case, we may not finish adding the equalities.
   returns 1 if the conditions for their equality have been completely
             described by the constraints added to p
   returns 2 if there was a non-affine expression, in which case the
             constraints added to p must be true for equality (but they
	     may also be true for some non-equal cases).
   */

bool subscripts_ZIV_test(a_access access1, a_access access2) {

    sub_iterator sub1, sub2;

    if(access1==Entry || access1==ExitNode) 
	return !access_is_private_var(access2);
    if(access2==Entry || access2==ExitNode) 
	return !access_is_private_var(access1);

    /* we assume children of the node are subscripts,
       so scalars MUST have no nodechildren 
    assert(array_op(access1->nodeop) || access1->nodechild == NULL);
    */
    
    sub1 = sub_i_for_access(access1);
    sub2 = sub_i_for_access(access2);
    
    while (!sub_i_done(sub1))
    {
	assert(!sub_i_done(sub2));  /* must have same # of subscripts */
	assert(sub_i_cur_affine(sub1) != NULL && sub_i_cur_affine(sub2) != NULL);
	
	if (sub_i_cur_is_affine(sub1) && sub_i_cur_is_affine(sub2)) {
            affine_expr *ae1,*ae2;
	    
	    assert((sub_i_cur_affine(sub1)->other_branch == NULL) &&
		   (sub_i_cur_affine(sub2)->other_branch == NULL));
	    
	    ae1 =  sub_i_cur_affine(sub1);
	    ae2 =  sub_i_cur_affine(sub2);

	    assert(ae1->terms[0].petit_var == NULL);
	    assert(ae2->terms[0].petit_var == NULL);


	    if (ae1->nterms == 1 && 
		    ae2->nterms == 1 && 
		    ae1->terms[0].coefficient != 
		    ae2->terms[0].coefficient) return 0;
	    }

	sub_i_next(sub1);
	sub_i_next(sub2);
    }
    
    assert(sub_i_done(sub2));  /* must have same # of subscripts */
    return 1;
}


static Relation memory_dependence_relation(a_access access1, a_access access2)
{
    Relation it(access_depth(access1), access_depth(access2));
    AccessIteration a(access1, &it, Input_Tuple);
    AccessIteration b(access2, &it, Output_Tuple);

    F_And *f = it.add_and();

    access_in_bounds(f, a);
    access_in_bounds(f, b);
    access_same_memory(f, a, b);
    it.finalize();

    return it;
}


static void store_dependence_with_empty_rel(
          ddnature oitype,  a_access access1, a_access access2, 
          dir_and_diff_info * d_info) {
  d_info->dd_relation = 
      new Relation(Relation::True(access_depth(access1), access_depth(access2)));
  d_info->dd_relation->simplify();
  store_dependence(oitype, access1, access2, d_info);
}

/* 
   perform the omega test on the array accesses access1 and access2
   see dddriver.h for a description of arguments.
 
 * Entry -> fetch/update dependencies.
 * write/update -> ExitNode dependencies.
 */
void dd_omega_test(a_access access1, a_access access2,
		   ddnature oitype, ddnature iotype,
		   unsigned int /*nest1*/, unsigned int /*nest2*/,  //unused
		   unsigned int bnest)
    {
    signed char loop_dirs[maxCommonNest+1];   /* signs of the steps */


#if defined newTimeTrials
    if (storeResult) omegaTests++;
#endif

    /*
      Construct things from entry and to exit.  These are now checked 
      to see if they are satisfiable, (i.e. the statement the reference
      appears in is never executed.) -- ejr
     */
    if((access1==Entry || access2==ExitNode)) {
	dir_and_diff_info d_info;
	if (access_is_private_var(access1) 
		|| access_is_private_var(access2)) return;
	d_info.nest = 0;
	d_info.direction = 0;
	d_info.flags = ddmem;
	d_info.restraint = 0;

	/*
	 * Build a dependence relation describing the dependences 
	 *  from access1 to access2 AND from access2 to access1.
	 * This relation must be split to get meaningful results.
	 */

        int inp_size= (access1==Entry) ?
             var_id_dims(get_nodevalue_sym(access_sym(access2)))
   	     :  access_depth(access1);

        int out_size= (access2==ExitNode) ?
             var_id_dims(get_nodevalue_sym(access_sym(access1)))
	     :  access_depth(access2);
 
	Relation *simple_rel = new Relation(inp_size, out_size);
      
	F_And *f = simple_rel->add_and();

        if (access1!=Entry) {
           assert (access2 == ExitNode);
  	   AccessIteration a(access1,simple_rel, Input_Tuple);
           access_in_bounds(f,a);
           subscripts_equal_to_tuple(f, a, &output_vars);
           } 


        if (access2!=ExitNode) {
           assert (access1 == Entry);
           AccessIteration b(access2,simple_rel, Output_Tuple);
  	   access_in_bounds(f, b);
           subscripts_equal_to_tuple(f, b, &input_vars);
	 }

	simple_rel->finalize();

        if (petit_args.dep_analysis_debug >= 1) 
	{
#if ! defined NOT_PETIT
        ddnode dd(0);
	dd.ddtype = oitype;
	dd.ddpred = access1;
	dd.ddsucc = access2;
	dd.ddnextpred = NULL;
	dd.ddnextpred = NULL;
	dd.ddlink = NULL;
	dd.ddnest = 0;
	dd.dddir = 0;
	dd.ddrestraint = 0; 
	dd.ddflags = 0;
	fprintf(debug, "\n\nconsidering dependences:\n");
	fprintf(debug, "%s\n", brdd_print(&dd));
	dd.ddtype = iotype;
	dd.ddpred = access2;
	dd.ddsucc = access1;
	fprintf(debug, "%s\n", brdd_print(&dd));
	if (petit_args.dep_analysis_debug >= 3)
	    simple_rel->print(debug);
#endif
	}

	simple_rel->simplify();
	if(simple_rel->is_upper_bound_satisfiable()) {
	    d_info.dd_relation = simple_rel;  
	    store_dependence(oitype, access1, access2, &d_info);
	}
        else {
          if (petit_args.dep_analysis_debug>=1)
           fprintf(debug,"Relation is unsatisfiable\n");
        }
         
	delete simple_rel;
	return;
    }
    
    assert(access_sym(access2) == access_sym(access1)); /* same var */
    assert(!(access_is_fetch(access1) && is_scalar_ref(access2)));
    assert(oitype != ddanti);

    /* if omitTopLevel, we are only interested in parallelizing loops,
       and thus don't care about data dependencies between things
       that don't share any common loops */



    if (petit_args.omitTopLevel && bnest == 0) {
	dir_and_diff_info d_info;

	d_info.nest = 0;
	d_info.direction = 0;
	d_info.flags = ddmem;
	d_info.restraint = 0;

	if (access_lexically_preceeds(access1, access2)) {
	  store_dependence_with_empty_rel(oitype, access1, access2, &d_info);
	} else if (access_lexically_preceeds(access2, access1)) {
	  store_dependence_with_empty_rel(iotype, access2, access1, &d_info);
	}
	/* else access1 == access2 or they're in different branches of an if */
	return;
    }

    /* Get loop step signs -- to handle negative step */
    GetLoopDirs(access1, bnest, &loop_dirs[0]);


    if (approximateTrivialDependences 
	&& (bnest == 0 || is_scalar_ref(access1))) {
      if (subscripts_ZIV_test(access1, access2)) {
	Relation mem_deps = memory_dependence_relation(access1, access2); 
	generateAllDDVectors(access1, access2, oitype, iotype,
			     bnest, &loop_dirs[0],ddmem,mem_deps);
	return;
      }
    }
    
    Relation mem_deps = memory_dependence_relation(access1, access2);

    if (petit_args.dep_analysis_debug >= 1) 
	{
#if ! defined NOT_PETIT
        ddnode dd(0);
	dd.ddtype = oitype;
	dd.ddpred = access1;
	dd.ddsucc = access2;
	dd.ddnextpred = NULL;
	dd.ddnextpred = NULL;
	dd.ddlink = NULL;
	dd.ddnest = 0;
	dd.dddir = 0;
	dd.ddrestraint = 0; 
	dd.ddflags = 0;
	fprintf(debug, "\n\nconsidering dependences:\n");
	fprintf(debug, "%s\n", brdd_print(&dd));
	dd.ddtype = iotype;
	dd.ddpred = access2;
	dd.ddsucc = access1;
	fprintf(debug, "%s\n", brdd_print(&dd));
	if (petit_args.dep_analysis_debug >= 3)
	    mem_deps.print(debug);
#endif
	}

    if (mem_deps.is_upper_bound_satisfiable())
	{
	  if (petit_args.dep_analysis_debug >= 4) {
	    int d = access_shared_depth(access1, access2);
	    Relation diff = Deltas(copy(mem_deps), d);
	    diff.simplify();
	    fprintf(debug, "--- Membased deltas ---\n");
	    diff.prefix_print(debug);
	  }
	  else if (petit_args.dep_analysis_debug >= 1)
	      fprintf(debug, "memory aliasing relationship is satisifiable\n");
	/* call omega test to add directions */
	calculateDDVectors(&mem_deps, access1, access2, oitype, iotype,
			   bnest, &loop_dirs[0],ddmem);
	}
    else if (petit_args.dep_analysis_debug >= 1)
	fprintf(debug, "Relation is unsatisfiable\n");

#if defined newTimeTrials
    if (storeResult) realOmegaTests++;
#endif

    }/* dd_omega_test */


// bnest currently unused
void GetLoopDirs(a_access access, int /*bnest*/, signed char *loop_dirs) {
     context_iterator c = cont_i_for_access(access);
     while ( !cont_i_done(c) ) {
       if ( cont_i_cur_is_loop(c) ) {
	 int d = cont_i_cur_depth(c);
	 loop_context limits;
	 int incr = 1;
	 bool known = 1;

	 assert(d>=0 && d<=maxCommonNest);
	 limits = cont_i_cur_loop(c);
	 if (loop_has_step(limits)) {
	   /* there is an increment expression */
	   loop_step(limits, &incr, &known);
	 }
	 loop_dirs[d] = incr > 0 ? 1 : -1;
       }
       cont_i_next(c);
     }
}

} // end of omega namespace
