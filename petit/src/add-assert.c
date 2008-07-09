/* $Id: add-assert.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <basic/assert.h>
#include <basic/Exit.h>
#include <omega/Relations.h>
#include <omega/pres_tree.h>
#include <string.h>
#include <stdlib.h>
#include <petit/lang-interf.h>
#include <petit/Zima.h>
#include <petit/add-assert.h>
#include <petit/debug.h>
#include <petit/message.h>
#include <petit/timeTrials.h>
#include <petit/petit_args.h>

namespace omega {

#define afraid_of(rel) \
	(petit_args.panicked_by_nonaffinity ? \
	 ((rel.max_ufs_arity() > 0) || !rel.is_exact()) : \
	 0)

/*
   set *r = gist proj(dep,sym) given proj(loops_execute,sym), where
   loops_execute = all relevant loops & conditions execute at least once,
                   loop carrying dependence executes at least min_diff

   loops_execute must be satisfiable, or something is wrong.
   *r must not be false, or something is wrong

   return impossible if gist is empty (tautology?)
   return possible   otherwise

   NOTE In old code, reduceWithSubs had to be 0
 */

static dd_flags
build_zap_problem(dd_current dd, Relation *r)
    {
    if (petit_args.dep_analysis_debug >= 1) {
	fprintf(debug,"\n\nZap: Finding Gist of dependence:\n");
	fprintf(debug,"%s\n", dd_current_as_string(dd));
    }

#if defined newTimeTrials
    if (storeResult) zapTests++;
#endif

// FERD - uncompressing and re-compressing probably shouldn't
//        be necessary here, but it is
    bool was_compressed = dd_current_relation(dd)->is_compressed();
    if (was_compressed)
	dd_current_relation(dd)->uncompress();
    Relation dependence_exists = *dd_current_relation(dd);
    if (was_compressed)
	{
	dd_current_relation(dd)->compress();
	assert(dd_current_relation(dd)->is_compressed());
	}

    if (afraid_of(dependence_exists))
	{
	if (petit_args.dep_analysis_debug >= 1)
	    {
	    fprintf(debug,"  Zap: panicked by non-affinity in dependence\n");
	    }
	return ddzappable | ddZapUseless;
	}


  

    int de_conjs=0;
    for (DNF_Iterator di(dependence_exists.query_DNF()); di.live(); di.next())
      de_conjs++;

    if (petit_args.max_conjuncts  > 0 && de_conjs > petit_args.max_conjuncts ) {
	*r = dependence_exists;
	return ddzappable || ddZapUseless;
	}
	

    Relation loops_execute(access_depth(dd_current_src(dd)),
			   access_depth(dd_current_dest(dd)));
    AccessIteration src(dd_current_src(dd), &loops_execute, Input_Tuple);
    AccessIteration dest(dd_current_dest(dd), &loops_execute, Output_Tuple);

    F_And *le = loops_execute.add_and();
    
    assert((int)dd_current_nest(dd) <= access_depth(dd_current_src(dd)));
    assert((int)dd_current_nest(dd) <= access_depth(dd_current_dest(dd)));

    // loops before the carrier must have >= 1 iterations,
    // loop that carries dep must have at least
    // petit_args.zappable_interesting_distance iterations:

    int carried_by = dd_carried_by(dd_current_dir(dd),dd_current_nest(dd));

    int outer_independent_loops = carried_by-1;

    for (int d = 1; d<=outer_independent_loops; d++)
	{
	    EQ_Handle independent = le->add_EQ();
	    independent.update_coef(loops_execute.input_var(d), -1);
	    independent.update_coef(loops_execute.output_var(d), 1);
	    independent.finalize();
	}

    context_iterator c = cont_i_for_access(src.access());
    while (!cont_i_done(c) && !cont_i_cur_is_loop(c)) cont_i_next(c);
    if (!cont_i_done(c)) context_in_bounds(le, src, c);

    c = cont_i_for_access(dest.access());
    while (!cont_i_done(c) && !cont_i_cur_is_loop(c)) cont_i_next(c);
    if (!cont_i_done(c)) context_in_bounds(le, dest, c);

    assert(copy(loops_execute).is_upper_bound_satisfiable());

    if (carried_by <= (int)dd_current_nest(dd)) {
            int to_go=access_depth(dest.access())-carried_by;
            while (to_go>0) {
                 assert(!cont_i_done(c));
                 do {
                   cont_i_next(c);
                 }   
                 while (!cont_i_done(c) && !cont_i_cur_is_loop(c));
                 to_go--;
            }            
            assert (!cont_i_done(c) && cont_i_cur_is_loop(c));
            assert ((int)cont_i_cur_depth(c) == carried_by);  
          
	    int min_diff = petit_args.zappable_interesting_distance;
            int step = 1;
            if (loop_has_step(cont_i_cur_loop(c))) {
              bool known=true;
              loop_step(cont_i_cur_loop(c), &step, &known);
              assert(known);
            }

            int sign=(step>0?1:-1);

	    GEQ_Handle carried = le->add_GEQ();
	    carried.update_coef(loops_execute.input_var(carried_by), -sign);
	    carried.update_const(-min_diff*step*sign);
	    carried.update_coef(loops_execute.output_var(carried_by), sign);
	    carried.finalize();
	    }

    loops_execute.finalize();

    int le_conjs=0;
    { for (DNF_Iterator di(loops_execute.query_DNF()); di.live(); di.next())
      le_conjs++;

    }
    if (petit_args.max_conjuncts  > 0 
		&& le_conjs*de_conjs > petit_args.max_conjuncts ) {
	*r = dependence_exists;
	return ddzappable || ddZapUseless;
	}

    assert(!afraid_of(loops_execute));
    // we would have been afraid of the dependence itself

    if (petit_args.dep_analysis_debug >= 2)
	{
	fprintf(debug, "Zap: loops_execute =\n");
	loops_execute.prefix_print(debug);
	fprintf(debug, "Zap: dependence relation =\n");
	dependence_exists.prefix_print(debug);
	}

    if (!loops_execute.is_upper_bound_satisfiable())  // can't run 4 enough iterations
	{
	if (petit_args.dep_analysis_debug >= 1)
	    fprintf(debug, "Zap is NOT possible due to shortness of loop\n\n");
	return 0;
	}


    dependence_exists = Project_On_Sym(dependence_exists,
				       copy(loops_execute));

    int num_inp = omega::max(loops_execute.n_inp(),dependence_exists.n_inp());
    int num_out = omega::max(loops_execute.n_out(),dependence_exists.n_out());

    if (petit_args.dep_analysis_debug >= 2) {
	fprintf(debug,"dependence %d->%d,  interesting %d->%d,  result %d->%d\n",
		dependence_exists.n_inp(),
		dependence_exists.n_out(),
		loops_execute.n_inp(),
		loops_execute.n_out(),
		num_inp,
		num_out);
	}
    
    if (num_inp > dependence_exists.n_inp())
	dependence_exists = Extend_Domain(dependence_exists,
		num_inp-dependence_exists.n_inp());
    else if (num_inp > loops_execute.n_inp())
	loops_execute = Extend_Domain(loops_execute,
		num_inp-loops_execute.n_inp());
    if (num_out > dependence_exists.n_out())
	dependence_exists = Extend_Range(dependence_exists,
		num_out-dependence_exists.n_out());
    else if (num_out > loops_execute.n_out())
	loops_execute = Extend_Range(loops_execute,
		num_out-loops_execute.n_out());

#ifndef NDEBUG
    Relation de_copy = dependence_exists, le_copy = loops_execute;
#endif

    if (petit_args.dep_analysis_debug >= 2) {
	fprintf(debug,"dependence %d->%d,  interesting %d->%d,  result %d->%d\n",
		dependence_exists.n_inp(),
		dependence_exists.n_out(),
		loops_execute.n_inp(),
		loops_execute.n_out(),
		num_inp,
		num_out);
	fprintf(debug,"Trying to zap, context is:\n");
	loops_execute.prefix_print(debug);
	fprintf(debug,"Trying to zap, conditions are:\n");
	dependence_exists.prefix_print(debug);
	fprintf(debug,"Starting simplification:\n");
        dependence_exists.simplify();
	fprintf(debug,"Trying to zap, conditions are:\n");
	dependence_exists.prefix_print(debug);
	}

    *r = Gist(dependence_exists, loops_execute,1);

    if (petit_args.dep_analysis_debug >= 1)
	{
	Relation rr = *r;
	fprintf(debug, "Zap: gist is%s satisfiable, =\n",
		rr.is_upper_bound_satisfiable() ? "" : " NOT");
	rr.prefix_print(debug);
	}

    if (r->is_obvious_tautology())
	{
//	assert(Must_Be_Subset_On_Sym(le_copy, de_copy));
	if (petit_args.dep_analysis_debug >= 1)
	    fprintf(debug, "Zap is NOT possible\n\n");
	return 0;
	}
    else if (r->unknown_uses() & or_u)
	{
//	assert(Must_Be_Subset_On_Sym(le_copy, de_copy));
	if (petit_args.dep_analysis_debug >= 1)
	    fprintf(debug, "Zap is impossibly complicated\n\n");
	return ddzappable | ddZapUseless;
	}
    else if (r->unknown_uses() & and_u)
	{
//	assert(!Must_Be_Subset_On_Sym(le_copy, de_copy));
	if (petit_args.dep_analysis_debug >= 1)
	    fprintf(debug, "Zap is POSSIBLE but not exact\n\n");
	return ddzappable | ddZapPartial;
	}
    else
	{
	assert(r->is_exact());
	if (petit_args.dep_analysis_debug >= 1)
	    fprintf(debug, "Zap is POSSIBLE and EXACT\n\n");
	return ddzappable | ddZapExact;
	}
    }

bool try_to_eliminate(dd_current /*dd*/)
{
#if 0
    int old_reduceWithSubs;
    Problem p;
    zap_prob_desc zpd;
    bool result;
    elimination_possible ep;

    old_reduceWithSubs = reduceWithSubs;
    reduceWithSubs = 0;  /* to make combining problems easier */

    if ( (ep = build_zap_problem(dd, &p, &zpd)) == possible )
    {
	result = mccarthy(&p, zpd.vars);
    } else {
	switch (ep) {
	case too_hard:
	    Message_Add( "The Gist of this dependence is too complicated"); 
	    if (petit_args.dep_analysis_debug >= 1) {
		fprintf(debug, "Zap:  Gist contains wild cards - give up.\n");
	    }
	    break;
	case impossible:
	    Message_Add( "No interesting assertions zap the dependence"); 
	    if (petit_args.dep_analysis_debug >= 1) {
		fprintf(debug, "Zap:  Gist = \"True\" - don't bother asking if we should negate it.\n");
	    }
	    break;
	case possible:
	default:
	    assert(0 && "Can't get here.");
	}
	result = 0;
    }

    reduceWithSubs = old_reduceWithSubs;
    return result;
#endif
    return false;
}

Relation zap_conditions(dd_current dd)
    {
    Relation zap;
    build_zap_problem(dd, &zap);
    return zap;
    }


dd_flags possible_to_eliminate(dd_current dd)
    {
    Relation zap;
    return build_zap_problem(dd, &zap);
    }

} // end namespace omega
