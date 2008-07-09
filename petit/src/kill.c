/* $Id: kill.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <basic/bool.h>
#include <basic/assert.h>
#include <omega/Relations.h>
#include <omega/pres_tree.h>
#include <petit/lang-interf.h>
#include <petit/Zima.h>
#include <petit/debug.h>
#include <petit/kill.h>
#include <petit/timeTrials.h>
#include <petit/omega2flags.h>
#include <petit/petit_args.h>

namespace omega {

int dep_analysis_skip_inexact_kills = -1;

// The following can be re-defined to control how killing is done.  E.g.,
// -Ddont_use_cover_analysis=1 -Dkill_with_some_obvious_constraints=getenv("PETIT_KILL_OBVIOUS")
// Look at the code to see what they do
#if ! defined dont_use_cover_analysis
#define dont_use_cover_analysis 0
#endif
#if ! defined kill_with_some_obvious_constraints
#define kill_with_some_obvious_constraints 0
#endif
#if ! defined keep_relations_compressed
#define keep_relations_compressed 0
#endif
#if ! defined dont_verify_kills
#define dont_verify_kills 1
#endif

/* The following table shows the possible results
	of combining two direction vectors:

	    !    +    0    0+   -    +-   0-   *
	!   !    !    !    !    !    !    !    !
	+   !    +    +    +    *    *    *    *
	0   !    +    0    0+   -    +-   0-   *
	0+  !    +    0+   0+   *    *    *    *
	-   !    *    -    *    -    *    -    *
	+-  !    *    +-   *    *    *    *    *
	0-  !    *    0-   *    -    *    0-   *
	*   !    *    *    *    *    *    *    *
*/


static int ddCombine[8][8] = 
      { {0, 0, 0, 0, 0, 0, 0, 0},
	{0, 1, 1, 1, 7, 7, 7, 7},
	{0, 1, 2, 3, 4, 5, 6, 7},
	{0, 1, 3, 3, 7, 7, 7, 7},
	{0, 7, 4, 7, 4, 7, 4, 7},
	{0, 7, 5, 7, 7, 7, 7, 7},
	{0, 7, 6, 7, 4, 7, 6, 7},
	{0, 7, 7, 7, 7, 7, 7, 7} };


/* quick test to see if dd might be (partially) killed by
   the combination of f and o.
 */

bool
kill_possible(dd_current dd, dd_current f, dd_current o,
	      int commonDepth, int allow_partial)
{
    int j,m,t;
    
    for(j=1; j <= commonDepth; j++) {
	if (dd_current_diff_known(dd,j) &&
	    dd_current_diff_known(f,j) &&
	    dd_current_diff_known(o,j) )
	{
	    if (dd_current_diff(f)[j] + dd_current_diff(o)[j] 
			!= dd_current_diff(dd)[j]) {
	        if (petit_args.dep_analysis_debug >= 4) 
		  fprintf(debug,
		   "distances at depth %d not compatable: %d + %d != %d\n",
			j,
			dd_current_diff(o)[j],dd_current_diff(f)[j],
			dd_current_diff(dd)[j]);
		break;
		}
	}
	else {
	    m = ddCombine[ddextract1(dd_current_dir(f),j)]
		[ddextract1(dd_current_dir(o),j)];
	    t = ddextract1(dd_current_dir(dd),j);
	    if (allow_partial) {
		/* test that m intersect t != empty */
		if (!(m & t)) break;
	    } else {
		/* test that m Must_Be_Subset t */
		if ((m & t) != t) break; 
	    }
	    
	    if (dd_current_diff_known(dd,j) && dd_current_diff(dd)[j] == 1 &&
		!(ddextract1(dd_current_dir(f),j)&(ddind|ddbck))
		&& !(ddextract1(dd_current_dir(o),j)&(ddind|ddbck)))
	    {
		break;
	    }
	}
    }
    if (j <= commonDepth)  /* break occured */
	{
	if (petit_args.dep_analysis_debug >= 4) 
	    fprintf(debug,"distances at depth %d not compatable\n",j);
	return false;
	}
    else
	{
	return true;
	}
}    

/* another kill_possible, that calculates common_depth,
   and always allows partial
 */

bool kill_possible(dd_current dd, dd_current f, dd_current o)
{
  int common_nest=min(std::min(dd_current_nest(dd), 
                               dd_current_nest(f)), dd_current_nest(o));

  return kill_possible (dd, f, o, common_nest,1);
}


/***** 
 * BUILD_INITIAL_EXPOSED
 *  
 *  build set of iterations for a particular
 *  access. Initially exposed iterations are all of them.
 *****/

// Returns a set.
static Relation build_initial_exposed(a_access access1)
{
  Relation it(access_depth(access1));
  AccessIteration a(access1, &it, Set_Tuple);

  F_And *f = it.add_and();

  access_in_bounds(f,a);
  it.finalize();
  return it;

} /* end build_initial_exposed */
  



/*******
 *  DO_SIMPLE_KILL
 *
 *  do standard kill test for a flow dependence (A->C) and a 
 *  potential  killing flow dependence (B->C).
 *  i.e. go through all output dependencies (A->B), make a simple check,
 *  and if it is possible that (A->B)o(B->C) = (A->C) substract
 *  (A->B)o(B->C) from (A->C) (Relations of couse, not ddvectors).
 *  INPUT:
 *     value_deps     - relation for A->C
 *     flow           - A->C
 *     killer_flow    - B->C
 *  OUTPUT:
 *     value_deps     - value_deps modified in the proper way
 *  RETURN VALUE:   - val_deps had changed or not
 * 
 *******/

typedef unsigned char change;
#define unchanged      0x0
#define really_changed 0x1


/******
info:
is the relation known to be an inexact rel
Have we left something out of it
has the relation been changed

1 bit - has "underlying" relation changed
1 bit - is it known to be marked as inexact (initially inexact or we marked it)
1 bit - became inexact when we left something out
***/


static change do_simple_kill(Relation &value_deps,
			     dd_current flow, dd_current killer_flow,
				bool &known_to_be_inexact)
    {
    dd_in_iterator killer_output;
    a_access A = dd_current_src(flow);
    a_access B = dd_current_src(killer_flow);
    a_access C = dd_current_dest(flow);
    change relation_has_changed = unchanged;
  
    if (dep_analysis_skip_inexact_kills < 0)
	{
	if (petit_args.dep_analysis_quick) dep_analysis_skip_inexact_kills = 1;
	else {
		char * e = getenv("DEP_ANALYSIS_SKIP_INEXACT_KILLS");
		dep_analysis_skip_inexact_kills = e ? atoi(e) : 0;
		}
	}

    assert(dd_current_is_flow(flow));
    assert(dd_current_is_flow(killer_flow));


    if (petit_args.dep_analysis_debug >= 2) 
	{
	fprintf(debug, "Doing kill via:\n");
	fprintf(debug, "  %s\n", dd_current_as_string(killer_flow));
	}

    killer_output = dd_i_i_for_access(B);
    while (!dd_i_i_done(killer_output))
	{
#if ! defined DAVEW_THESIS_REDUCTIONS
	if (dd_i_i_cur_src(killer_output) == A
	    && (dd_i_i_cur_is_output(killer_output)))
#else
	if (dd_i_i_cur_src(killer_output) == A
	    && (dd_i_i_cur_is_output(killer_output)
 		|| dd_i_i_cur_is_reduction(killer_output)) )
#endif
	    {
               
	    if (petit_args.dep_analysis_debug >= 2)
		{
		fprintf(debug, "Considering possibility of kill with:\n  %s\n", 
		         dd_current_as_string(dd_i_i_current(killer_output)));
		fprintf(debug, "  %s\n", dd_current_as_string(killer_flow));
		}

	    if (kill_possible(flow, killer_flow,
			      dd_i_i_current(killer_output))) 
		{
		Relation kf, ko;

		if (kill_with_some_obvious_constraints)
		    {
		    // To ensure we only add "unknown" if we have to,
		    // re-generate the constraints that we need from
		    // the program source, rather than taking them
		    // from a relation that may also contain "unknown"

		    int iarity = access_depth(A);
		    int karity = access_depth(B);
		    int oarity = access_depth(C);
		    
		    if (A != Entry)
			{
			ko = Relation(iarity, karity);
			AccessIteration ai(A, &ko, Input_Tuple);
			AccessIteration bj1(B, &ko, Output_Tuple);
			F_And *fko = ko.add_and();
			access_same_memory(fko, ai, bj1);
			connected_by_diff(fko, ai, bj1, killer_output);
			ko.finalize();
			}
		    else
			{
			ko = Relation(dd_current_relation(flow)->n_inp(),
				      karity);
			AccessIteration bj1(B, &ko, Output_Tuple);
			F_And *fko = ko.add_and();
			subscripts_equal_to_tuple(fko, bj1, &input_vars);
			ko.finalize();
			}
		    
		    kf = Relation(karity, oarity); 
		    AccessIteration bj2(B, &kf, Input_Tuple);
		    AccessIteration ck(C, &kf, Output_Tuple);
		    F_And *fkf = kf.add_and();

		    context_iterator already_done;
		    int lzo = leading_zeros(dd_current_dir(killer_output),
					    dd_current_nest(killer_output));
		    int lzf = leading_zeros(dd_current_dir(killer_flow),
					    dd_current_nest(killer_flow));
		    if (lzo > lzf)
			{
			already_done = access_shared_context_at_depth(B, A, lzo);
			}
		    else if (lzf > lzo)
			{
			already_done = access_shared_context_at_depth(B, C, lzf);
			}
		    else
			{
			already_done = access_inner_shared_context_at_depth(B, A, C, lzo);
			}

		    access_in_bounds(fkf, bj2, already_done);
		    access_same_memory(fkf, bj2, ck);
		    connected_by_diff(fkf, bj2, ck, killer_flow);
		    kf.finalize();
		    int canHandle =  leading_zeros(dd_current_dir(flow),
                                               dd_current_nest(flow));
		    if (kf.max_ufs_arity() > canHandle)
			kf = Intersection(kf,Relation::Unknown(kf));
		    if (ko.max_ufs_arity() > canHandle)
			ko = Intersection(ko,Relation::Unknown(ko));
		    }
		else
		    {
		    if (!keep_relations_compressed)
			kf = *(dd_current_relation(killer_flow));
		    else
			{
			assert(dd_current_relation(killer_flow)->is_compressed());
			dd_current_relation(killer_flow)->uncompress();
			kf = *(dd_current_relation(killer_flow));
			dd_current_relation(killer_flow)->compress();
			}

		    if (!keep_relations_compressed)
			{
			if (dd_current_relation(killer_output)->is_compressed())
			    dd_current_relation(killer_output)->uncompress();
			ko = *(dd_current_relation(killer_output));
			}
		    else
			{
			assert(dd_current_relation(killer_output)->is_compressed());
			dd_current_relation(killer_output)->uncompress();
			ko = *(dd_current_relation(killer_output));
			dd_current_relation(killer_output)->compress();
			}
		    }

		int exact_comp = prepare_relations_for_composition(kf,ko);
		if (keep_relations_compressed)
		    dd_current_relation(flow)->uncompress();
		bool already_inexact = !dd_current_relation(flow)->is_exact();
		if (keep_relations_compressed)
		    dd_current_relation(flow)->compress();

		if ((dep_analysis_skip_inexact_kills || already_inexact)
		     && !exact_comp)
		    {
		    if (petit_args.dep_analysis_debug >= 2)
			fprintf(debug,
				"This would be inexact and is being ignored.\n");
/*
		    if (!(known_to_be_inexact)) {
		      known_to_be_inexact = true;
		      value_deps =
			Intersection(value_deps, Complement(Relation::Unknown(value_deps)));
		      }
                      */
		    }
		else
		    {
		    if (petit_args.dep_analysis_debug >= 2) 
			{
			fprintf(debug, "\nKill is possible\n.");
			fprintf(debug, "Subtracting composition of:\n");
			fprintf(debug, "Killer flow:\n");
			kf.print_with_subs(debug);
			fprintf(debug, "\nKiller output:\n");
			ko.print_with_subs(debug);
			fprintf(debug, "\n");
			}
	   
		    Relation comp_rel; 
		    int kf_L0s = kf.query_guaranteed_leading_0s();
		    int ko_L0s = ko.query_guaranteed_leading_0s();

		    if (A != Entry && ko_L0s >= kf_L0s &&
			!kill_with_some_obvious_constraints
			&& 0 /* This seems to be broken */ )
			{
			    int lvl = kf_L0s+1;
			    int preserve = dd_current_nest(killer_flow);
			    if (kf_L0s < ko_L0s &&
				kf_L0s < (int)dd_current_nest(killer_output)) {
				lvl = 9999;
				preserve = 0;
				}
				
			    if (petit_args.dep_analysis_debug >= 2) 
				{
				fprintf(debug, "\nUsing After computation, lvl = %d:\n",lvl);
				fprintf(debug, " ko: %d leading 0z, %d common nest\n",
					ko_L0s,
					dd_current_nest(killer_output));
				fprintf(debug, " kf: %d leading 0z, %d common nest\n",
					kf_L0s,
					dd_current_nest(killer_flow));
				}
			
			    assert(lvl == 9999 || lvl <= (int)dd_current_nest(killer_output));
			    comp_rel=After(ko, lvl, preserve,
					   lvl == 9999 ? 0 :
                                    (dd_current_loop_dir(killer_output))[lvl]);
			    if (value_deps.n_out() > preserve)
			      comp_rel = Extend_Range(comp_rel,
				value_deps.n_out() - preserve);
			    assert(value_deps.n_out() == comp_rel.n_out());
			}
			    
		    else comp_rel=Composition(kf, ko);
#if defined STUDY_KILL_USE
		    number_of_kills_for_use_count++;
		    dd_current(killer_output)->kill_use_count++;
#endif
		    if (petit_args.dep_analysis_debug >= 2) 
			{
			fprintf(debug, "\n Their composition is:\n");
			comp_rel.prefix_print(debug);
			}

#if ! defined NDEBUG
		    Relation old_value_deps = value_deps;
#endif

		    value_deps=Difference(value_deps, comp_rel);

		    relation_has_changed |= really_changed;

		    }
		    if (petit_args.dep_analysis_debug >= 3) 
			{
			fprintf(debug, "\n After substracting composition value_deps is:\n");
			value_deps.prefix_print(debug);
			fprintf(debug, "\n Which simplifies to:\n");
			Relation vd = value_deps;
			vd.simplify();
			vd.print_with_subs(debug);
			}

		} /* end if kill_possible */
	    }
	dd_i_i_next(killer_output);
	} /* while */
    return relation_has_changed;  
    } /* end do_simple_kill */


/*****
 * UPDATE_EXPOSED
 *   update exposed reads set: substruct from it range of dependencies
 *   in the group
 * INPUT
 *    exposed_read - set of exposed reads
 *    cl           - cover list
 *    first        - first element in the group
 * OUTPUT
 *    updated exposed_read
 *****/

static void update_exposed(Relation &exposed_read, coverlist *cl, int first)
{
 int i;
 for (i=first; i<cl->size; i++)
   {
#if ! defined DAVEW_THESIS_REDUCTIONS
     if (!(cl->list[i].from_update) && 
         (dd_current_is_presumed_val(cl->list[i].dep) ||
          dd_current_is_valid_for_covers(cl->list[i].dep)))
#else
     if ((dd_current_is_presumed_val(cl->list[i].dep) ||
          dd_current_is_valid_for_covers(cl->list[i].dep)))
#endif
     { 
        Relation range;
        if (keep_relations_compressed)
	    {
	    assert(dd_current_relation(cl->list[i].dep)->is_compressed());
	    dd_current_relation(cl->list[i].dep)->uncompress();
	    range = Range(copy(*dd_current_relation(cl->list[i].dep)));
	    dd_current_relation(cl->list[i].dep)->compress();	
	    }
	else
	    range = Range(copy(*dd_current_relation(cl->list[i].dep)));

        range.simplify();
        if (petit_args.dep_analysis_debug >= 2)
	    {
	    if (petit_args.dep_analysis_debug == 2)
		fprintf(debug, "Finding range of dependence %s\n",
			dd_current_as_string(cl->list[i].dep));
	    else
		{
		fprintf(debug, "For dependence %s\n     range is:\n",
			dd_current_as_string(cl->list[i].dep));
		range.prefix_print(debug);
		}
          }
        exposed_read=Difference(exposed_read,range);    

        if (petit_args.dep_analysis_debug >= 2)
	    {
	    fprintf(debug,"\nSubtracting this range from the exposed read\n");
	    if (petit_args.dep_analysis_debug >= 3)
		{
		fprintf(debug, "Updated exposed read set now is:\n");
		exposed_read.prefix_print(debug);
		}
	    }
     }
     
     if (cl->list[i].end_of_group)
       break;
   }			   
    exposed_read.simplify();
    if (petit_args.dep_analysis_debug >= 3) {
	    fprintf(debug,"\nSimplified read set is:\n");
	    exposed_read.prefix_print(debug);
		}
} /* end update_exposed */


    
void verify_kills(a_access r)
    {
    if (!petit_args.dep_analysis_quick && !dont_verify_kills)
	  {
	  dd_in_iterator f1,f2;
	  f1 = dd_i_i_for_access(r);
	  while (!dd_i_i_done(f1)) {
	      if (dd_i_i_cur_is_flow(f1)
	     && !access_is_update(dd_current_src(dd_o_i_current(f1)))
	     && dd_current_is_val(dd_o_i_current(f1))
             && !(dd_current_flags(dd_o_i_current(f1)) & ddIncompFlags) ) {
		  // do self test for f1
		  if (dd_current_relation(dd_o_i_current(f1))->is_compressed())
		    dd_current_relation(dd_o_i_current(f1))->uncompress();
		  Relation r1 = *dd_current_relation(dd_o_i_current(f1));
		  if (keep_relations_compressed)
		      dd_current_relation(dd_o_i_current(f1))->compress();
		  Relation check = Composition(Inverse(copy(r1)),copy(r1));
		  check.simplify();
		  if(!check.is_upper_bound_satisfiable()){
			fprintf(debug,"Kill check didn't include identity??\n");
			fprintf(debug,"%s\n",
				dd_current_as_string(dd_o_i_current(f1)));
			r1.print_with_subs(debug);
			fprintf(debug,"check is:");
			check.print_with_subs(debug);
			}
		  else {
		      Relation I = Identity(check.n_inp());
		      if (!Must_Be_Subset(copy(check),I)) {
		        if ((!check.is_exact())) {
			  fprintf(debug,"Kill accuracy check unusable for:\n");
			  fprintf(debug,"%s\n",
				  dd_current_as_string(dd_o_i_current(f1)));
			  r1.print_with_subs(debug);
			  fprintf(debug,"check is:");
			  check.print_with_subs(debug);
			}
		      else {
			fprintf(debug,"Kill accuracy check failed for:\n");
			fprintf(debug,"%s\n",
				dd_current_as_string(dd_o_i_current(f1)));
			r1.print_with_subs(debug);
			fprintf(debug,"check is:");
			  check.print_with_subs(debug);
			check.make_level_carried_to(check.n_inp());
			check.simplify(0,4);
			fprintf(debug,"after additional simplication, check is:");
			  check.print_with_subs(debug);
			assert(0 && "Kill verification failed");
			  }
		      }
		      }
		  r1 = Range(r1);
		  r1.simplify();
		  // check against all other flow deps
		  f2 = f1;

		  dd_i_i_next(f2);
		  while (!dd_i_i_done(f2)) {
		      if (dd_i_i_cur_is_flow(f2)
	     && dd_current_is_val(dd_o_i_current(f2))
	     && !access_is_update(dd_current_src(dd_o_i_current(f2)))
             && !(dd_current_flags(dd_o_i_current(f2)) & ddIncompFlags) ) {
		          // check f1 vs. f2
		  if (dd_current_relation(dd_o_i_current(f2))->is_compressed())
		    dd_current_relation(dd_o_i_current(f2))->uncompress();
		  	  Relation r2 = *dd_current_relation(dd_o_i_current(f2));
		  if (keep_relations_compressed)
		      dd_current_relation(dd_o_i_current(f2))->compress();
			  r2 = Range(r2);
			  r2 = Intersection(copy(r1),r2);
			  r2.simplify();
			  if (r2.is_upper_bound_satisfiable() && !petit_args.dep_analysis_quick) {
			    int valid = (r2.is_exact()) ;
			    if (!valid)
			    fprintf(debug,"Kill accuracy check unusable\n");
			    else fprintf(debug,"Kill accuracy check failed\n");
			    fprintf(debug," for: %s\n",
				    dd_current_as_string(dd_o_i_current(f1)));
			    fprintf(debug,"and: %s\n",
				    dd_current_as_string(dd_o_i_current(f2)));
			    r2.print_with_subs(debug);
			    if (valid)
			   assert(0 && "Kill verification failed");
			    }
		          }
	              dd_i_i_next(f2);
		      }
		  } 
	      dd_i_i_next(f1);
	      }
	  }
    }



/*****
 * do partial covers kill analysis
 ****/

void do_partial_covers_kill(a_access r)
    {
    int i;
    coverlist *cl=build_coverlist (r);

    if (dont_use_cover_analysis)
	{
	for (i = 0; i < cl->size-1; i++)
	    cl->list[i].end_of_group = false;
	if (cl->size-1 >= 0)
	    assert(cl->list[cl->size-1].end_of_group);
	}
  int lastDependenceWeCareAbout = cl->size-1;
  if (petit_args.dep_analysis_quick) {
    for(i = cl->size-1; i>=0; i--)
      if (dd_current_flags(cl->list[i].dep) & ddisInCycle) break;
    if (i < 0) return;
    while (!cl->list[i].end_of_group) i++;
    lastDependenceWeCareAbout = i;
    };

  int groupStart[1000];
  int numGroups = 0;
  groupStart[0] = 0;
  for (i = 0; i <= lastDependenceWeCareAbout; i++) if (cl->list[i].end_of_group)
  	groupStart[++numGroups] = i+1;
  	
  if (petit_args.dep_analysis_debug >= 2)
     fprintf(debug,"\n******Kill analysis for %d: %s********\n", 
     access_lineno(r), access_as_string(r)); 

    assert(cl->size >= 0);  

    if (cl->size == 0)
	{
	free((char *)cl);
	return;
	}
    assert(lastDependenceWeCareAbout >= 0);      
    assert(numGroups > 0);

    Relation  exposed_read;
    if (numGroups > 1) exposed_read = build_initial_exposed(r);
    change relation_has_changed;

    int flow, killer_flow ;
    dd_current flow_dep;

    if (petit_args.dep_analysis_debug >= 2)
	{
	fprintf(debug, "\n*****\nStart cover analysis for read:\n");
	exposed_read.prefix_print(debug); 
	}

#if ! defined DAVEW_THESIS_REDUCTIONS
    int first_from_update=cl->size;
#endif
    for (int g = 0; g < numGroups; g++) {
	if (!keep_relations_compressed)
	    for (flow=groupStart[g]; flow < groupStart[g+1]; flow++)
		dd_current_relation(cl->list[flow].dep)->uncompress();
	for (flow=groupStart[g]; flow < groupStart[g+1]; flow++) 
	  if (!petit_args.dep_analysis_quick || (dd_current_flags(cl->list[flow].dep) & ddisInCycle))
	{
        
#if ! defined DAVEW_THESIS_REDUCTIONS
        if (cl->list[flow].from_update && flow<first_from_update)
           first_from_update=flow; 
#endif

	flow_dep=cl->list[flow].dep;

	/* copy memory-based relation */
	if (keep_relations_compressed)
	    {
	    assert(dd_current_relation(cl->list[flow].dep)->is_compressed());
	    dd_current_relation(cl->list[flow].dep)->uncompress();
	    }
	Relation value_deps(*dd_current_relation(cl->list[flow].dep));
	if (keep_relations_compressed)
	    {
	    dd_current_relation(cl->list[flow].dep)->compress();
	    }
	relation_has_changed=unchanged;
	bool is_dead = false;
	bool known_to_be_inexact = false;
	bool did_an_individual_kill = false;


	if (petit_args.dep_analysis_debug >= 2)
	    {  
	    fprintf(debug, "\nCalculating value-based dependence: [\n");
	    fprintf(debug, "%s\n", dd_current_as_string(cl->list[flow].dep));
	    value_deps.print_with_subs(debug);
	    if (petit_args.dep_analysis_debug >= 3)
		{
		fprintf(debug,"Relation for this flow dependence:\n");
		value_deps.prefix_print(debug);
		}
	    } 

	assert(!dont_use_cover_analysis || g == 0);
        
        // copied from line 203
        if (dep_analysis_skip_inexact_kills < 0)
	{
	        if (petit_args.dep_analysis_quick) 
                    dep_analysis_skip_inexact_kills = 1;
	        else {
		        char * e = getenv("DEP_ANALYSIS_SKIP_INEXACT_KILLS");
		        dep_analysis_skip_inexact_kills = e ? atoi(e) : 0;
		}
	}


	/* restrict its range to the reads currently exposed */
	if (g > 0)
	    {
	    known_to_be_inexact = (!exposed_read.is_exact());
	    // assert(!exposed_read.has_disjunction_with_unknown());

            // do not do the restrict range if the exposed reads
            // are known to be inexact and we are supposed to 
            // skip inexact kills
            if (known_to_be_inexact && dep_analysis_skip_inexact_kills) {
                if (petit_args.dep_analysis_debug >= 2)
                        fprintf(debug,
                            "This would be inexact and is being ignored.\n");
                known_to_be_inexact = false;  // because we aren't using it
                                              // to restrict range
            } else {
	        value_deps=Restrict_Range(value_deps, copy(exposed_read));
	        relation_has_changed=really_changed;
            }

	    value_deps.simplify(0,1);
	    is_dead = !value_deps.is_upper_bound_satisfiable();

	    if (petit_args.dep_analysis_debug >= 2)
		{
		fprintf(debug, "\nRestrict range of relation to exposed reads\n");
		if (petit_args.dep_analysis_debug >= 2)
		    {
		    fprintf(debug, "Resulting relation is:\n");
		    value_deps.prefix_print(debug);
		    }
		}
	    } 

    if (!is_dead) {
      int flow_level  = dd_carried_by(dd_current_dir(cl->list[flow].dep),dd_current_nest(cl->list[flow].dep));

      int conjs=0;
      for (DNF_Iterator di(value_deps.query_DNF()); di.live(); di.next
())
	      conjs++;

      if (petit_args.max_conjuncts  > 0 && conjs > petit_args.max_conjuncts ) {
	value_deps = Intersection(value_deps,Relation::Unknown(value_deps));
        }
      else {
	/* do simple stupid kill analysis for possible killers,
	   stop at the end of the group */


#if ! defined DAVEW_THESIS_REDUCTIONS
	    for (killer_flow=min(groupStart[g],first_from_update); killer_flow<groupStart[g+1]; killer_flow++)
		if ((killer_flow>=groupStart[g] || cl->list[killer_flow].from_update) &&
                    (dd_current_is_presumed_val(cl->list[killer_flow].dep) ||
		    dd_current_is_valid_for_covers(cl->list[killer_flow].dep))) {
#else
	    for (killer_flow=groupStart[g]; killer_flow<groupStart[g+1]; killer_flow++)
		if (dd_current_is_presumed_val(cl->list[killer_flow].dep) ||
		    dd_current_is_valid_for_covers(cl->list[killer_flow].dep)) {
#endif
	         int killer_flow_level  = dd_carried_by(dd_current_dir(cl->list[killer_flow].dep),dd_current_nest(cl->list[killer_flow].dep));
		if (petit_args.dep_analysis_quick 
		&& killer_flow_level <= flow_level 
		&& killer_flow_level <= (int)dd_current_nest(cl->list[killer_flow].dep)
		&& (dd_current_flags(cl->list[killer_flow].dep) & ddisInCycle)) {
		  if (petit_args.dep_analysis_debug >= 2)
		    {  
		    fprintf(debug, "\nIgnoring potential kill by \n");
		    fprintf(debug, "  %s\n", dd_current_as_string(cl->list[flow].dep));
		}}
		else {
		    change c = do_simple_kill(value_deps, flow_dep, 
				       cl->list[killer_flow].dep,
				       known_to_be_inexact);
		    if (c) {
			relation_has_changed  |= c;
			did_an_individual_kill = true;
			}
		    }
		}
	}
	}
	if (relation_has_changed != unchanged && !petit_args.dep_analysis_quick && !did_an_individual_kill && !is_dead && value_deps.is_upper_bound_satisfiable() && Is_Obvious_Subset(copy(*dd_current_relation(cl->list[flow].dep)),copy(value_deps)))  {
		if (petit_args.dep_analysis_debug) {
		        fprintf(debug, "\nIgnoring covers which had no effect: \n");
	                fprintf(debug, "%s\n", dd_current_as_string(cl->list[flow].dep));
	                fprintf(debug, "memory: \n");
			(dd_current_relation(cl->list[flow].dep))->prefix_print(debug);
	                fprintf(debug, "value: \n");
			value_deps.prefix_print(debug);
	                fprintf(debug, "\n");
			}
		relation_has_changed = unchanged;
		known_to_be_inexact = false;
		}

	if (!(relation_has_changed & really_changed))
	    {
	    if (petit_args.dep_analysis_debug >= 2) 
		fprintf(debug, "] No kills performed\n");

	    if (known_to_be_inexact)
		/* really just need to copy it and
		   set inexact bit in dd, make relations inexact */
		calculateDDVectors(
		    &value_deps, dd_current_src(flow_dep),
		    dd_current_dest(flow_dep), dd_current_type(flow_dep),
		    dd_current_type(flow_dep), dd_current_nest(flow_dep),
		    dd_current_loop_dir(flow_dep), ddval | ddIncompKill);  
	    else
		/* update this dependence to be value-based */
		dd_current_flags(flow_dep) |= ddval;
	    }
	else
	    { 
	    if (is_dead || !value_deps.is_upper_bound_satisfiable())
		{
		/* set that this dependence is killed */

		dd_current_flags(flow_dep) |= ddnotPresumedVal;

		if (petit_args.dep_analysis_debug >= 2)
		    fprintf(debug, "] Dependence is killed. \n");
		}   
	    else
		{
		dd_flags incompkill = 0;

		if (petit_args.dep_analysis_debug >= 2)
		    {
		    fprintf(debug, "] Dependence is value-based. \n");
		    if (petit_args.dep_analysis_debug >= 3)
			value_deps.prefix_print(debug);
		    }

		/* get and store new dependence, mark this dependence as
		   not presumed val but valid for cover analysis */
              
		dd_current_flags(flow_dep) |= 
		    (ddnotPresumedVal | ddvalidForCovers);

		if (!value_deps.is_exact())
		    incompkill = ddIncompKill;

		calculateDDVectors(
		    &value_deps, dd_current_src(flow_dep),
		    dd_current_dest(flow_dep), dd_current_type(flow_dep),
		    dd_current_type(flow_dep), dd_current_nest(flow_dep),
		    dd_current_loop_dir(flow_dep), ddval | incompkill);  
		}  
	    }}


	if((g+1) < numGroups)
	    {

	    /* collect ranges for dependencies in this group and substract
	       them from the exposed range */
         
	    if (petit_args.dep_analysis_debug >= 2)
		{
		fprintf(debug, "\nUpdating exposed reads for this group...\n");
		}
	    update_exposed(exposed_read,cl,groupStart[g]);


	    /* check if we already covered all iterations for the read.
	       While doing so simplified is done, so it's done once for each
	       group except last one.
	       */

	    if (!exposed_read.is_upper_bound_satisfiable())
		{ 
		if (petit_args.dep_analysis_debug >= 2)
		    {
		    fprintf(debug, "All reads are covered. Mark the rest to be killed \n");       
		    } 

		/* mark all the rest of dd as killed */
	        assert(0 < g+1);
	        assert(g+1 < numGroups);
	        for (g = g+1; g < numGroups; g++) 
		    for (flow=groupStart[g]; flow < groupStart[g+1]; flow++) {
		      dd_current_flags(cl->list[flow].dep)|=ddnotPresumedVal;
			 if (petit_args.dep_analysis_debug >= 3)
				    fprintf(debug, "%s\n", dd_current_as_string(cl->list[flow].dep));
			}

		goto all_done;
		}  

          
	    } /* end if end of not the last group */
   
	} /* end for group .... */

all_done:
    free((char *)cl);

    } /* end do_partial_covers_kill */

} // end of omega namespace
