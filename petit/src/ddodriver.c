/* $Id: ddodriver.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */


/*
   Driver routine for dependence testing with the omega test.

   A different driver is used for several reasons:
    - we use coverage information to decide whether or not to check for
      a flow dependence
    - we don't want the funky values for the loop nesting
 */

#if ! defined printKillRelations
#define printKillRelations 0
#endif

// this should be in-sync with kill.c
#if ! defined keep_relations_compressed
#define keep_relations_compressed 0
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <basic/assert.h>
#include <basic/bool.h>
#include <omega/Relations.h>
#include <petit/Zima.h>
#include <petit/Exit.h>
#include <petit/dddir.h>
#include <petit/lang-interf.h>
#include <petit/debug.h>
#include <petit/affine.h>  /* for an assertion about node_is_affine */
#include <petit/dddriver.h>
#include <petit/ddodriver.h>
#include <petit/omega2flags.h>
#include <petit/ddomega.h>
#include <petit/coverlist.h>
#include <petit/kill.h>
#include <petit/add-assert.h>
#include <petit/ddnest.h>
#include <petit/ops.h>
#include <petit/vutil.h>
#include <petit/timeTrials.h>
#include <petit/timer.h>
#include <petit/ddInf.h>
#include <petit/notedd.h>
#include <petit/dd_misc.h>
#include <petit/ddepsilon.h>
#include <petit/depcycles.h>
#include <petit/petit_args.h>

#if !defined(OMIT_QUANTIFY_CALLS)
#include <quantify.h>
#endif

namespace omega {

#if ! defined EXTRAVAGANT
int doing_all_accurate_kills;
int doing_all_refines;
#endif
static int memory_tests, kill_tests, zap_tests;

/*
 * Returns TRUE if given reference node is within a declaration.
 * Return FALSE otherwise.
 */


int InDecl(node *n) {
  assert(is_mem_ref(n));

  while(n!=Entry && n->nodeop<op_last) {
    switch(n->nodeop) {
    CASE_DO:
    CASE_ASSIGN:
    case op_if:
    case op_assert:
      return(0);

    case op_declare:
      return(1);
  
    default:
      n = n->nodeparent;
      assert(n->nodeparent);
      break;
    }
  }

  fprintf(stderr, "\nInDecl: memory reference node met in a strange place\n");
  return(0);
}


int dd_carried_by(dddirection dv, int length)
{
    int depth;
    
    for (depth=1; (depth <= length && ddextract1(dv, depth) == ddind); depth++) 
	;
    
    /* dd dirs [1..depth-1] are 0 */
    return depth;
}

int leading_zeros(dddirection dv, int length)
{
    int depth;
    
    for (depth=1; (depth <= length && ddextract1(dv, depth) == ddind); depth++) 
	;
    
    /* dd dirs [1..depth-1] are 0 */
    return depth-1;
}

int leading_zeros(dd_current dd)
    {
    dddirection dv = dd_current_dir(dd);
    int length = dd_current_nest(dd);
    int depth;
    
    for (depth=1; (depth <= length && ddextract1(dv, depth) == ddind); depth++) 
	;
    
    /* dd dirs [1..depth-1] are 0 */
    return depth-1;
}


/* New version of append_dd_flags that uses new flags */

void append_dd_flags(char *line, dd_flags dv, const char *iflags)
    {
    char s[20];

    static int show_o = -1;
    if (show_o < 0)
	show_o = (getenv("SHOW_ALL_DD_FLAGS") ?
		  abs(atoi(getenv("SHOW_ALL_DD_FLAGS"))) : 1);

#if defined Live_RT
    if(!(dv & (ddBad4Monica | ddIncompFlags | ddzappableWC)))
	return;
#endif


    if (dv)
	{

	*s=0;


	if (dv & ddmem)        strcat(s, "M");
	if (dv & ddval)        strcat(s, "V");

//        if ((dv & ddnotPresumedVal))  strcat(s, "k");

 	if (dv & ddisInFlowCycle) {
		strcat(s, "O"); 
		assert (dv & ddisInCycle);
		}
 	else if ((dv & ddisInCycle) && show_o) strcat(s, "o"); 
	
	if (dv & ddzappable)
	    {
	    assert(dv & (ddZapExact | ddZapPartial | ddZapUseless));
	    if (dv & ddZapExact)   strcat(s, "Z");
	    if (dv & ddZapPartial) strcat(s, "z");
	    if (dv & ddZapUseless) strcat(s, "?");
	    }


	if (dv & ddBad4Monica)      strcat(s, "m");

        if (dv & ddIncompFlags)
	    {
	    strcat(s, iflags);
	    }

        if (*s)
	    {
	    strcat(line, " [ ");          
	    strcat(line, s);
	    strcat(line, "]");
	    }
	}
    }


static void o_dd_intersect(node *onode, node *inode,
			   ddnature oitype, ddnature iotype)
{
    /* nest level of loop around onode, inode, both nodes, max nest */
    unsigned int onest, inest, bnest, mnest;
#if defined(newTimeTrials) && ! defined(OMIT_GETRUSAGE)
    unsigned int count = 0, count2 = 0;
    long omega1time, omega2time;
#endif

#if defined compareOmegaEpsilon
    resetAtomCount();
#endif

    dd_get_nests( onode, inode, &onest, &inest, &bnest );
    dd_fix_nests( onode, inode, onest, inest, bnest );
    
    /* mnest is the maximum nest with which we have to deal */
    mnest = inest>onest?inest:onest;
    
    if( bnest > maxCommonNest || mnest > maxnest ){
        fprintf( stderr, "*** too many nested loops ***\n" );
        Exit(2);
    }

#if defined(newTimeTrials) && ! defined(OMIT_GETRUSAGE)
    timing1reset();
    storeResult = 1;
    start_clock();
#endif

   bool do_preproc= ((petit_args.DDalgorithm == DDalg_omega_preproc) && 
       onode != ExitNode && onode != Entry  &&
       inode != ExitNode && inode != Entry); 

   bool dd_is_disproved=false;
   if (do_preproc)
     dd_is_disproved=epsilon_preproc(onode,inode);
   if (!dd_is_disproved)
       {
       dd_omega_test(onode, inode, oitype, iotype, onest, inest, bnest);
       memory_tests++;
       }

#if defined(newTimeTrials) && ! defined(OMIT_GETRUSAGE)
    omega1time=clock_diff();
    count=1;
    count2=1;
    omega2time=0;
    timing1output(oitype, onode,inode, count,omega1time, count2,omega2time);
#endif

#if defined compareOmegaEpsilon
    /* if we're not checking the top level, don't print stats for it */
    if (bnest != 0 || !(petit_args.omitTopLevel || petit_args.dep_analysis_quick))
       n_atom_output(DDalg_omega, onode,inode);
#endif

}/* o_dd_intersect */

/*
 * Find all write/write (output) dependencies.
 * Find all write/update(as write) dependencies 
 * Output dependences from Entry added 
 */
static void o_dd_intersect_writes( node *list )
{
    node *l1, *l2;
    
    for(l2=list; l2!=NULL; l2=l2->nodelink) {
        if(is_phi_access(l2))  continue;
	/* stop second loop after l1 reaches l2 */
	for(l1=Entry; l1!=NULL;  ) {
	  if(!is_phi_access(l1)) {
	    if( (((l1==Entry && petit_args.doEEdeps) || store_op(l1->nodeop)) 
		    && (store_op(l2->nodeop) || update_op(l2->nodeop)) )
		||
	       (update_op(l1->nodeop) && store_op(l2->nodeop)))
	    {
		if ( !(petit_args.omitTopLevel || petit_args.dep_analysis_quick) || 
		 access_shared_depth(l1,l2) > 0)
		o_dd_intersect( l1, l2, ddoutput, ddoutput );
	    }
	  }
	  if(l1==Entry) {
	    l1 = list;
	  } else if(l1==l2) {
	    l1 = NULL;
	  } else {
	    l1 = l1->nodelink;
	  }
	}
    }
}/* o_dd_intersect_writes */


/*
 * Find all update/update dependencies.
 */
static void o_dd_intersect_update_update( node *list )
{
    int cur_depth;
    node *u2, *u1;
    
    for (cur_depth = maxnest; cur_depth >= 0; cur_depth--)
    {
	for(u2=list; u2!=NULL; u2=u2->nodelink)
	{
	    if(is_phi_access(u2))  continue;
	    if (update_op(u2->nodeop) && access_depth(u2)>=cur_depth)
	    {
		/* stop second loop after w1 reaches u2 */
		for (u1=list; u1!=NULL; u1 = (node *) (u1==u2 ? NULL : u1->nodelink))
		{
		    if(is_phi_access(u1))  continue;
		    if (update_op(u1->nodeop) &&
			access_shared_depth(u1, u2)==cur_depth)
		    {
			if(accesses_are_same_update_type(u1,u2))
			{
			    /* update -> update : reduction/reduction dep-s */
			    o_dd_intersect(u1, u2, ddreduce, ddreduce);
			} else {
			    /* update+(fetch,store) -> update*(fetch,dtore):
			       flow/anti, anti/flow and output dependencies */
			    o_dd_intersect(u1, u2, ddflow, ddanti);
			    o_dd_intersect(u2, u1, ddflow, ddanti);
			    o_dd_intersect(u1, u2, ddoutput, ddoutput);
			}
		    }
		}
	    }
	}
    }
}



/*
 * find all write/read (flow and anti) dependencies.
 *
 * prototype traversal: correct, but inefficient.
 * profiling suggests that just over 1% of data dependence
 * testing time is taken by calls to access_shared_depth here.
 *
 * Find all write/update(as read) dependences 
 * Find all read/update dependences        
 * Find all Entry->memref and memref->ExitNode dependences 
 */
static void o_dd_intersect_reads( node *list )
{
    node *read, *write;
    int cur_depth;
    
    for(read=ExitNode; read!=NULL; read= read==ExitNode? list: read->nodelink)
    {
        if(is_phi_access(read))  continue;
	if((read==ExitNode && petit_args.doEEdeps) ||
	   (read!=ExitNode && !InDecl(read) &&
	    (fetch_op(read->nodeop) || update_op(read->nodeop))))
	{
	    optype rdop = read->nodeop;
	    for(cur_depth=node_depth(read); cur_depth >= ((petit_args.omitTopLevel || petit_args.dep_analysis_quick)?1:0); cur_depth--)
	    {
		for (write = Entry;
		     write != NULL;
		     write = write==Entry? list: write->nodelink)
		{
		    optype wrop = write->nodeop;
		    if(is_phi_access(write))  continue;
		    assert(is_mem_ref(write) || wrop==op_entry);
		    if ( ( (petit_args.doEEdeps && read==ExitNode 
				&& (store_op(wrop)||update_op(wrop)) && UsedAtEnd(write)) 
				||
			(petit_args.doEEdeps && write==Entry 
				&& (fetch_op(rdop)||update_op(rdop)) && DefinedAtBegin(read))
				||
			(store_op(write->nodeop)&& fetch_op(read->nodeop)) 
				||
			(store_op(write->nodeop)&& update_op(read->nodeop)) 
				||
			(update_op(write->nodeop)&& fetch_op(read->nodeop))) 
			&& access_shared_depth(read,write)==cur_depth)
		    {
			o_dd_intersect(write, read, ddflow, ddanti);
		    }
		}
	    }
	}
    }
} /* o_dd_intersect_reads */

#if defined NDEBUG
#define check_all_inexact_bits()
#else
static void check_all_inexact_bits()
    {
    nametabentry *n;
    node *sd;

    for (n=NameHead; n!=NULL; n = n->namenext)
	{
	sd = n->namest.symdecl;
	if (sd !=NULL && sd->nodelink !=NULL && n->namest.symclass!=CLBUILTIN)
	    {
	    node *list=sd->nodelink;
	    for (node * r = list; r!=NULL; r = r->nodelink)
		{
		if (! is_phi_access(r))
		    {
		    dd_in_iterator dep;
		    for (dep=dd_i_i_for_access(r); !dd_i_i_done(dep);
						    dd_i_i_next(dep))
			{
			dd_current_relation(dd_i_i_current(dep))->uncompress();
			if (!dd_current_relation(dd_i_i_current(dep))->is_exact())
			    assert(dd_current_flags(dd_i_i_current(dep)) & ddIncompFlags);
			dd_current_relation(dd_i_i_current(dep))->compress();
			}
		    }
		}
	    }
	}

    // do deps involving exit
    dd_in_iterator dep;
    for (dep=dd_i_i_for_access(ExitNode); !dd_i_i_done(dep);
					   dd_i_i_next(dep))
	{
	dd_current_relation(dd_i_i_current(dep))->uncompress();
	if (!dd_current_relation(dd_i_i_current(dep))->is_exact())
	    {
	    if (!(dd_current_flags(dd_i_i_current(dep)) & ddIncompFlags))
		{
		fprintf(debug,
			"Uh-oh, an inexact relation for dependence\n%s\n",
			dd_current_as_string(dep));
		dd_current_relation(dd_i_i_current(dep))->prefix_print();
		}
	    assert(dd_current_flags(dd_i_i_current(dep)) & ddIncompFlags);
	    }
	dd_current_relation(dd_i_i_current(dep))->compress();
	}
    }
#endif




/****
 * Do kill analysis for an exit dependence from the write F_exit:
 *  for all output dependencies O: W->W2 substruct domain of O from
 *  F_exit
 ****/

static void o_dd_do_exit_kill(a_access W)
{
 dd_out_iterator flow_i; 
 assert(!is_phi_access(W));
 assert(access_is_store(W) || access_is_update(W));


 if (petit_args.dep_analysis_debug >= 1)
   {
     fprintf(debug, "\nKilling dependencies from %s to exit\n", 
        access_as_string(W));
   }

 // copied this code from kill.c
 int dep_analysis_skip_inexact_kills = -1;
 if (dep_analysis_skip_inexact_kills < 0)
     {
        if (petit_args.dep_analysis_quick) dep_analysis_skip_inexact_kills = 1;
        else {
                char * e = getenv("DEP_ANALYSIS_SKIP_INEXACT_KILLS");
                dep_analysis_skip_inexact_kills = e ? atoi(e) : 0;
        }
      }



 // find the dependence to exit
 for (flow_i=dd_o_i_for_access(W); ! dd_o_i_done(flow_i); 
      flow_i=dd_o_i_next(flow_i))
   {
     if (dd_current_is_flow(dd_o_i_current(flow_i))
          && dd_o_i_cur_dest(flow_i)== ExitNode)
        break;
   }

 if (!flow_i)
   return;

 dd_current flow=dd_o_i_current(flow_i);
 assert(dd_current_dest(flow) == ExitNode);
 
 if (petit_args.dep_analysis_debug >= 1)
   {
    fprintf(debug, "\nKilling dependence %s\n", dd_current_as_string(flow));
  }

 bool have_to_change=false;
 Relation downward_exposed_writes;

// go through all output dependencies

 dd_out_iterator output_i;
 for (output_i=dd_o_i_for_access(W); ! dd_o_i_done(output_i);
      output_i=dd_o_i_next(output_i))
    if (dd_current_is_output(dd_o_i_current(output_i)))
    {

      if (!have_to_change)
      {
       have_to_change=true;
       dd_current_relation(flow)->uncompress();
       downward_exposed_writes=Domain(copy(*dd_current_relation(flow)));
       if (petit_args.dep_analysis_debug >= 3)
	 {
         fprintf(debug,"\nOriginal relation is:\n");
         downward_exposed_writes.print_with_subs(debug);
       }
       dd_current_relation(flow)->compress();
      }

      if (dd_current_relation(dd_o_i_current(output_i))->is_compressed())
        dd_current_relation(dd_o_i_current(output_i))->uncompress();       

      if (petit_args.dep_analysis_debug >= 2)
	{
         fprintf(debug,"\nSubtract output dependence %s\n", 
         dd_current_as_string(dd_o_i_current(output_i)));
	 if (petit_args.dep_analysis_debug >= 3)
	     {
	     fprintf(debug,"\nOutput dependence relation is:\n");
	     dd_current_relation(dd_o_i_current(output_i))->print_with_subs(debug);
	     }
       }

      Relation output = *(dd_current_relation(dd_o_i_current(output_i)));
      Relation output_domain= Domain(output);

      if (keep_relations_compressed)
	  dd_current_relation(dd_o_i_current(output_i))->compress();

      output_domain.simplify(1,1);

      if (petit_args.dep_analysis_debug >= 3)
	{
         fprintf(debug,"\n It's domain is:\n");
         output_domain.print_with_subs(debug);
       }

      if (dep_analysis_skip_inexact_kills && output_domain.is_inexact()) {
        if (petit_args.dep_analysis_debug >= 2)
          fprintf(debug, "This would be inexact and is being ignored.\n");
      } else {
        downward_exposed_writes=Difference(downward_exposed_writes,output_domain);
      }

      if (petit_args.dep_analysis_debug >= 3)
	{
         fprintf(debug,"The unsimplified difference is :\n");
         downward_exposed_writes.prefix_print(debug);
       }

      downward_exposed_writes.simplify(1,1);
  
      if (petit_args.dep_analysis_debug >= 3)
        {
         fprintf(debug,"\n The simplified difference is:\n");
         downward_exposed_writes.prefix_print(debug);
       }
               
    } 


    if (petit_args.dep_analysis_debug >= 2)
    {
     if (have_to_change)
     {
      fprintf(debug,"The dependence has changed\n.");
      if (petit_args.dep_analysis_debug >= 3)
	  {
	  fprintf(debug, "The new relations is:\n");
	  downward_exposed_writes.print_with_subs(debug);
	  }
     }
     else 
      fprintf(debug, "The dependence has not changed\n");
  
    }  

      
// update the dependece
     
    if (have_to_change)
    {
     if (!downward_exposed_writes.is_upper_bound_satisfiable())
     {
       dd_current_flags(flow) |= ddnotPresumedVal;
     }
     else
     { // create new dependence        
       dd_current_flags(flow) |= ddnotPresumedVal;
       dir_and_diff_info d_info;
       d_info.nest = 0;
       d_info.direction = 0;
       d_info.flags = ddval;
       d_info.restraint = 0;
       d_info.dd_relation= new Relation(Cross_Product(downward_exposed_writes,Relation::True(1)));
       d_info.dd_relation->simplify();
       store_dependence(ddflow, dd_current_src(flow), dd_current_dest(flow),
                         &d_info);
	delete d_info.dd_relation;
     }
   }
   else
       dd_current_flags(flow) |= ddval;
} /* end o_dd_do_exit_kill*/
  


/* 
  Do killing analysis using partial covers

*/

static void o_dd_do_accurate_kill(node *C)
    {
    assert(!is_phi_access(C));
    assert(!access_is_store(C));

//  Flow dependence relations are uncompressed in cover analysis
//  when we need them    
    do_partial_covers_kill(C);
    compress_dd_flow(C); 
	
   } /* end o_dd_do_accurate_kill */
    

// Check that all dependence relations are compressed.
// This function doesn't check dependencies to Exit
  

#if defined NDEBUG
#define check_all_is_compressed(X)
#define check_exit_is_compressed()
#else
static void check_all_is_compressed(int stop)
{
 nametabentry *n;
 node *sd;


 for (n=NameHead; n!=NULL; n = n->namenext)
 {
  sd = n->namest.symdecl;
  if (sd !=NULL && sd->nodelink !=NULL && n->namest.symclass!=CLBUILTIN)
    {
     node *list=sd->nodelink;
     for (node * r = list; r!=NULL; r = r->nodelink)
       {
         if (! is_phi_access(r))
	   {
            dd_in_iterator dep;
            for (dep=dd_i_i_for_access(r); !dd_i_i_done(dep);
                   dd_i_i_next(dep))
	      {
                bool Bad;
                 Bad=!
                  (dd_current_relation(dd_i_i_current(dep))->is_compressed());
                if (Bad)
		  {
                   printf("Uncompressed dependence at point %d:\n",stop);
                   printf("%s\n",brdd_print(dep));
                  }
                assert(!Bad && "this dependence should have been compressed");
	      }
	  } /* end if ! phi */
       } /* end for */
   } /* end if */
 }/* end for */
} /* end check_all_is_comressed */


static void check_exit_is_compressed()
{
            dd_in_iterator dep;
            for (dep=dd_i_i_for_access(ExitNode); !dd_i_i_done(dep);
                   dd_i_i_next(dep))
                assert (
                  dd_current_relation(dd_i_i_current(dep))->is_compressed());
} /* end shck_all_is_comressed */
#endif


#if defined STUDY_KILL_USE

int number_of_kills_for_use_count;

#define HIST_MAX 1024

static void reset_output_stats(node * list)
{
 node * l;
 dd_in_iterator dd;

 number_of_kills_for_use_count = 0;

 for (l=list; l!=NULL; l=l->nodelink)
   {
    for (dd=dd_i_i_for_access(l); !dd_i_i_done(dd); dd_i_i_next(dd))
     {
      if (dd_i_i_cur_is_output(dd))
        {
	     dd_i_i_current(dd)->kill_use_count = 0;
        }
    }
  }
}

static void do_output_stats(node * list)
{
 node * l;
 dd_in_iterator dd;
 int number_of_outputs;
 int usage_histogram[HIST_MAX];
 int i, use_max;

 number_of_outputs = use_max = 0;
 for (i = 0; i < HIST_MAX; i++)
   usage_histogram[i] = 0;

 for (l=list; l!=NULL; l=l->nodelink)
   {
    for (dd=dd_i_i_for_access(l); !dd_i_i_done(dd); dd_i_i_next(dd))
     {
      if (dd_i_i_cur_is_output(dd))
	  {
	  const int count = dd_i_i_current(dd)->kill_use_count;
	  assert(0 <= count && count < HIST_MAX);
	  number_of_outputs++;
	  set_max(use_max, count);
	  usage_histogram[count]++;
	  }
    }
  }

  fprintf(debug, "# kills using outputs: %d\n",
	  number_of_kills_for_use_count);
  fprintf(debug, "# outputs:             %d\n",
	  number_of_outputs);
  for (i=0; i <= use_max; i++)
      fprintf(debug, "# outputs in %2d kills: %d\n",
	      i, usage_histogram[i]);
}
#endif


/* Do all kill analysis for a given variable.
   The "list" argument is the beginning of a list of nodes,
   connected by "nodelink", that all refer to this variable */

static void o_dd_do_accurate_kills(node *list)
    {
    node *r;

#if defined STUDY_KILL_USE
    reset_output_stats(list);
#endif

    for(r = list;  r != NULL;  r = r->nodelink)
        {
	if ((access_is_fetch(r) || access_is_update(r)) && !is_phi_access(r))
	    {

#if defined(newTimeTrials) && ! defined(OMIT_GETRUSAGE)
            unsigned int count = 0;
            long killtime;
 
            timingKreset();
            storeResult=1;
            start_clock();
#endif

	    o_dd_do_accurate_kill(r);
	    kill_tests++;

#if defined(newTimeTrials) && ! defined(OMIT_GETRUSAGE)
            killtime=clock_diff();
            count=1;
            timingKoutput(Entry, r, 0, count, killtime);
#endif
#ifndef NDEBUG
    	    verify_kills(r); 
    	    compress_dd_flow(r); 
#endif

         } /* end if */
#if ! defined DAVEW_THESIS_REDUCTIONS
      if (!petit_args.dep_analysis_quick && petit_args.doEEdeps &&
	  !(petit_args.omitTopLevel || petit_args.dep_analysis_quick) && 
          (access_is_store(r) || access_is_update(r)) && !is_phi_access(r))
#else
      else if (!petit_args.dep_analysis_quick && petit_args.doEEdeps &&
	       !(petit_args.omitTopLevel || petit_args.dep_analysis_quick) &&
	       access_is_store(r) && ! is_phi_access(r))
#endif
	    {
		o_dd_do_exit_kill(r);
		kill_tests++;
	    }
         
 	} /* end for */

#if defined STUDY_KILL_USE
    fprintf(debug, "\nKill statistics for kills involving %s(",
	    get_nodevalue_sym(access_sym(list))->symname);
    for (unsigned int dimento = 1;
	 dimento<=get_nodevalue_sym(access_sym(list))->symdims;
	 dimento++)
	fprintf(debug, "*%s",
		dimento==get_nodevalue_sym(access_sym(list))->symdims?"":",");
    fprintf(debug, ")\n");
    do_output_stats(list);
#endif
    compress_dd_output(list);
    compress_dd_reduction(list);
    }


static void o_dd_zappable(node *list)
    {
    node *l;
    ddnode *f;
    
    for(l = list; l != NULL; l = l->nodelink)
	{
        if(is_phi_access(l))  continue;
	for (f = l->nodeddin; f != NULL; f = f->ddnextpred) {
	    /* don't bother zapping dead dds */
	    int tryme = f->ddpred != Entry && f->ddsucc != ExitNode;
	    if (petit_args.omitTopLevel && f->ddnest == 0) tryme = 0;
	    if (petit_args.dep_analysis_quick) {
		switch (f->ddtype) {
		  case ddflow: if (!(f->ddflags & ddisInCycle)) tryme = 0;
			       if (!ddisValue(f) && ddisDead(f)) tryme = 0;
				break;
		  case ddanti: 
		  case ddoutput: 
		  case ddreduce: 
				tryme = 0;
				break;
		  default:
			   assert(0 && "Can't get here");
		}
	}
 

	    if ( tryme ) { 
#if defined(newTimeTrials) && ! defined(OMIT_GETRUSAGE)
		long zapTime;
		storeResult = 1;
		start_clock();
#endif
		f->ddflags |= possible_to_eliminate(f);
	        if (petit_args.dep_analysis_debug >= 1) 
    		  fprintf(debug, "\nAfter Zap test:\n %s\n", 
			dd_current_as_string(f));
		zap_tests++;
#if defined(newTimeTrials) && ! defined(OMIT_GETRUSAGE)
		zapTime = clock_diff();
		
		timingZoutput(f->ddpred, f->ddsucc, 1, zapTime);
#endif
		}
	    }
	}
    } /* o_dd_zappable */


static void o_dd_remove(ddnode *dd)
    {
    if (dd->ddpred->nodeddout == dd)
	dd->ddpred->nodeddout = dd->ddnextsucc;
    else
	{
	ddnode *pred;
	for (pred=dd->ddpred->nodeddout; 
	     pred->ddnextsucc != dd;
	     pred=pred->ddnextsucc);
        pred->ddnextsucc = dd->ddnextsucc;
	}

    if (dd->ddsucc->nodeddin == dd)
	dd->ddsucc->nodeddin = dd->ddnextpred;
    else
	{
	ddnode *succ;
	for (succ=dd->ddsucc->nodeddin; 
	     succ->ddnextpred != dd;
	     succ=succ->ddnextpred);
        succ->ddnextpred = dd->ddnextpred;
	}

    delete dd;
    } /* o_dd_remove */


/* merge dependencies between 2 nodes */

void o_dd_merge_between_nodes(
      node *From, node * To,  int depth, bool merge_levels)
{
    ddnode *f1, *f2, *next_f2;
    int j, k;

    for(j=depth; j >= 1; j--) 
       for (f1 = To->nodeddin; f1 != NULL; f1 = f1->ddnextpred) {
           if (f1->ddpred != From) continue;
           assert(j <= (int)f1->ddnest);
	   int f1_level=1;
	   while (!dddirtest(f1->dddir, ddfwd, f1_level) &&
				f1_level <= depth) 
                 f1_level++;
	   for (f2 = f1->ddnextpred; f2 != NULL; f2 = next_f2) 
	     {
	      next_f2 = f2->ddnextpred;
              int f2_level=1; 
              if (f2->ddpred != From) continue;
              if (f1->ddtype != f2->ddtype) continue;
              assert (f1->ddsucc == To && f1->ddpred == From);

	                	            
              if ((dd_current_is_val(f1) && !dd_current_is_val(f2))
                           || (!dd_current_is_val(f1) && dd_current_is_val(f2))
                           )
                  continue;
              if ((dd_current_is_mem(f1) && !dd_current_is_mem(f2))
                           || (!dd_current_is_mem(f1) && dd_current_is_mem(f2))
                           )
                  continue;

	       if (ddallBut(f1->dddir,j) != ddallBut(f2->dddir,j)) 
		  continue;
	       while (!dddirtest(f2->dddir, ddfwd, f2_level) &&
				f2_level <= depth) f2_level++;
	       if (!merge_levels && f1_level != f2_level) 
				continue;

	        for(k=depth; k >= 1; k--) 
		{
		  if (k == j && 
		     ((f1->dddiff[k]==0&&(unsigned)f2->dddiff[k]==ddunknown)||
				 ((unsigned)f1->dddiff[k]==ddunknown&&f2->dddiff[k]==0)))
			    continue;
		   if (f1->dddiff[k] == f2->dddiff[k]) continue;
		      break;
		 }
	         if (k == 0) 
		 {
		  f1->dddir |= f2->dddir;
		  f1->ddrestraint |= f2->ddrestraint;
		  f1->ddflags |= f2->ddflags;
	          for(k=f1->ddnest; k >= 1; k--) 
		  if (f1->dddiff[k] != f2->dddiff[k]) 
				  f1->dddiff[k] = ddunknown;
                            o_dd_remove(f2);
		 }
	     } /* end for f2 */
          } /* end for f1 */
} /* o_dd_merge_between_nodes */


void map_over_dd(node* From, node * To, dd_map_fn fn, void * args)
{
  ddnode * f1;
        
  for (f1 = To->nodeddin; f1 != NULL; f1 = f1->ddnextpred) {
    if (f1->ddpred != From) continue;
    fn(f1, args);
  }
}
  


void o_dd_merge(node *list, bool merge_levels, bool do_relations)
{
    node *l;
    ddnode *f1, *f2, *next_f2;
    int j, k;

    for( l=list; l != NULL; l = l->nodelink )
	{
        if(is_phi_access(l))  continue;
	for(j=node_depth(l); j >= 1; j--) 
	    for (f1 = l->nodeddin; f1 != NULL; f1 = f1->ddnextpred) 
                if  (j <= (int)f1->ddnest)  {
		    int f1_level;
		    f1_level = 1;
		    while (!dddirtest(f1->dddir, ddfwd, f1_level) &&
				f1_level <= (int)f1->ddnest) f1_level++;
	            for (f2 = f1->ddnextpred; f2 != NULL; f2 = next_f2) 
			{
			int f2_level;
                        next_f2 = f2->ddnextpred;
	                if (f1->ddpred != f2->ddpred) continue;
	                if (f1->ddsucc != f2->ddsucc) continue;
	                if (f1->ddtype != f2->ddtype) continue;
                        if ((dd_current_is_val(f1) && !dd_current_is_val(f2))
                           || (!dd_current_is_val(f1) && dd_current_is_val(f2))
                           )
                           continue;
                        if ((dd_current_is_mem(f1) && !dd_current_is_mem(f2))
                           || (!dd_current_is_mem(f1) && dd_current_is_mem(f2))
                           )
                           continue;

	                if (ddallBut(f1->dddir,j) != ddallBut(f2->dddir,j)) 
			    continue;
		        f2_level = 1;
		        while (!dddirtest(f2->dddir, ddfwd, f2_level) &&
				f2_level <= (int)f2->ddnest) f2_level++;
			if (!merge_levels && f1_level != f2_level) 
				continue;

	                for(k=f1->ddnest; k >= 1; k--) 
			    {
		            if (k == j && 
				((f1->dddiff[k]==0&&(unsigned)f2->dddiff[k]==ddunknown)||
				 ((unsigned)f1->dddiff[k]==ddunknown&&(unsigned)f2->dddiff[k]==0)))
			        continue;
		            if (f1->dddiff[k] == f2->dddiff[k]) continue;
			    break;
		            }
	                if (k == 0) 
			    {
			    if (petit_args.dep_analysis_debug >= 2) {
			      fprintf(debug,"Merging: %s\n",brdd_print(f1));
			      fprintf(debug,"and %s\n",brdd_print(f2));
			      }
		            f1->dddir |= f2->dddir;
			    f1->ddrestraint |= f2->ddrestraint;
		            f1->ddflags |= f2->ddflags;
	                    for(k=f1->ddnest; k >= 1; k--) 
		              if (f1->dddiff[k] != f2->dddiff[k]) 
				  f1->dddiff[k] = ddunknown;
                            if (do_relations) {
                                Relation *r;  
				f1->dd_relation->uncompress();
				f2->dd_relation->uncompress();
                                r = new Relation(Union(*(f1->dd_relation),
						       *(f2->dd_relation)));
                                delete f1->dd_relation;
				r->compress();
                                f1->dd_relation=r;
                               } 
                            o_dd_remove(f2);
		            }
	                }
               }
	}
} /* o_dd_merge */



static 
void dd_split(ddnode *dd) {
  int i;
  for(i=1; i<=(int)dd->ddnest; i++) {
    int dv = ddextract1(dd->dddir, i);
    if(dv == ddind) {
    } else 
    if(dv == ddfwd) {
      break;
    } else if(dv == (ddfwd|ddind)) {
      /* split 0+ into leading 0 and leading + */
      ddnode *plus = dd;
      ddnode *zero = new ddnode(dd);

      dddirreset(plus->dddir, ddind, i);
      dddirreset(plus->ddrestraint, ddind, i);
      dddirreset(zero->dddir, ddfwd, i);
      dddirreset(zero->ddrestraint, ddfwd, i);
      zero->dddiff[i] = 0;
      zero->ddflags=plus->ddflags;

      /* link in zero node */
      plus->ddnextsucc = zero;
      plus->ddnextpred = zero;

      /* continue splitting */
      dd_split(plus);
      dd_split(zero);
      break;
    } else {
      Error("dd_split: bad direction vector");
    }
  }
}

void o_dd_split_between_nodes(node* From, node* To)
{
  ddnode *dd, *ddnext;

  for (dd = To->nodeddin; dd != NULL; dd = ddnext) {
	ddnext = dd->ddnextpred;
        if (dd->ddpred!=From)
          continue;
        if (!ddisDead(dd))    dd_split(dd);
} /* dd_split_between_nodes */


} /* end o_dd_split_levels_between_nodes */

/*
   Hacks to make petit-specific code run quickly:
   Reset the "inloop" field of every node before each time we
   call build_smaller_dd_graph.  This lets us determine how
   code is nested quickly, without having to worry about how
   this field should be updated when transforming code.
 */

void initializeContainingLoops(node *loop, node *n)
{
    node *nn;

    for( nn=n; nn != NULL; nn = nn->nodenext )
    {
	if( nn->nodeop == op_dolimit || if_branch_op(nn->nodeop) )
	    initializeContainingLoops(nn, nn->nodechild);
	else
	    initializeContainingLoops(loop, nn->nodechild);

	assert(nn->nodecontext == NULL);
	nn->nodecontext = loop;
    }
}


/* compress or uncompress: operation_type = 1 - compress, 0 - uncompress */

void do_compress_operation_on_reduction(node * list, int operation_type)
{
 node * l;
 dd_in_iterator dd;

 assert(operation_type == 1 || operation_type == 0);

 for (l=list; l!=NULL; l=l->nodelink)
   {
    for (dd=dd_i_i_for_access(l); !dd_i_i_done(dd); dd_i_i_next(dd))
     {
      if (dd_i_i_cur_is_reduction(dd))
        {
         if (operation_type == 1)
           dd_current_relation(dd_i_i_current(dd))->compress();
         else 
           dd_current_relation(dd_i_i_current(dd))->uncompress();   
        }
    }
  }
} /* end do_compress_operation_on_reduction */


/* compress or uncompress: operation_type = 1 - compress, 0 - uncompress */

void do_compress_operation_on_output(node * list, int operation_type)
{
 node * l;
 dd_in_iterator dd;

 assert(operation_type == 1 || operation_type == 0);

 for (l=list; l!=NULL; l=l->nodelink)
   {
    for (dd=dd_i_i_for_access(l); !dd_i_i_done(dd); dd_i_i_next(dd))
     {
      if (dd_i_i_cur_is_output(dd))
        {
         if (operation_type == 1)
           dd_current_relation(dd_i_i_current(dd))->compress();
         else 
           dd_current_relation(dd_i_i_current(dd))->uncompress();   
        }
    }
  }
} /* end do_compress_operation_on_output */


void do_compress_operation_on_flow(node * read, int operation_type)
{
  dd_in_iterator  dd;
 
  assert(operation_type == 1 || operation_type == 0);
  assert(!is_phi_access(read));
  assert(!access_is_store(read));

  for (dd=dd_i_i_for_access(read); !dd_i_i_done(dd); dd_i_i_next(dd))
     {
      if (dd_i_i_cur_is_flow(dd))
        {
         if (operation_type == 1)
           dd_current_relation(dd_i_i_current(dd))->compress();
         else 
           dd_current_relation(dd_i_i_current(dd))->uncompress();   
        }
     }
} /* end do_compress_operation_on_flow */

static void zapContainingLoops(node *n)
{
    node *nn;
    for( nn=n; nn != NULL; nn = nn->nodenext )
    {
	nn->nodecontext = NULL;
	zapContainingLoops( nn->nodechild );
    }
}

/* go through all reduction dependencies and change them
   to output/anti/flow dependencies if there is a flow
   dependence from the source of the reduction of the 
   shorter distance than the reduction one
 */

#if ! defined DAVEW_THESIS_REDUCTIONS
static void fixReductions(a_access  list) {
  a_access r;
  for (r=list;r!=NULL; r=r->nodelink) 
  {
    if (is_phi_access(r)) continue;
    for (dd_out_iterator ddoutred=dd_o_i_for_access(r);!dd_o_i_done(ddoutred);)

     {
     dd_current red=dd_o_i_current(ddoutred);
     dd_o_i_next(ddoutred);
     if (dd_current_is_reduction(red)) 
        {        
        for (dd_out_iterator ddoutflow=dd_o_i_for_access(r);
              !dd_o_i_done(ddoutflow); dd_o_i_next(ddoutflow)) 
          {
          if (dd_o_i_cur_is_flow(ddoutflow)) 
            {
            if (!definite_lt_t(red,dd_o_i_current(ddoutflow)))
              {
                ddnode * f=new ddnode(red);
                ddnode * a=new ddnode(red);
                red->ddtype=ddoutput;
                f->ddtype=ddflow;
                a->ddtype=ddanti;
                write_ddnode_to_graph(f);
                write_ddnode_to_graph(a);
                break;
              }
	  } // if flow
	} //for flow
      } // if reduction
   } //for reduction
  } // for nodes
} // fixReductions
#endif

} // end omega namespace

#if !defined(OMIT_GETRUSAGE)
#include <sys/time.h>
#include <sys/resource.h>

#include <petit/missing.h>

namespace omega {
static long user_millisecs(struct rusage *first, struct rusage *second)
{
    return 
      (second->ru_utime.tv_sec -first->ru_utime.tv_sec)*1000 +
      ((second->ru_utime.tv_usec-first->ru_utime.tv_usec)+500)/1000;
}
}
#endif
          
namespace omega {
void build_smaller_dd_graph(void)
    {
    nametabentry *n;
    node *sd;

    memory_tests = kill_tests = zap_tests = 0;

    assert(Entry->nodenext == NULL);
    initializeContainingLoops(Entry,Entry->nodechild);

    /* intersect the IN/OUT lists */

    if(!petit_args.quiet) printf("+ Omega 1 analysis\n");
    if (petit_args.dep_analysis_debug >= 1)
	fprintf(debug,
		"\n\n>>> Starting memory-based dependence analysis <<< [ \n");

    for( n = NameHead; n != NULL; n = n->namenext )
	{
	sd = n->namest.symdecl;
	/* variable name is:  n->namest.symname */
	if(sd != NULL && sd->nodelink != NULL && n->namest.symclass!=CLBUILTIN) 
	    {

	    if (petit_args.omitScalars || n->namest.symdims > 0) continue;
	    /* find all output dependencies: write - write, */
	    /* write - update(write), update(write) - write */
	    o_dd_intersect_writes( sd->nodelink );
	    /* intersect update/update for r, r */
	    o_dd_intersect_update_update( sd->nodelink );
	    /* then find flow and anti dependencies */
	    /* intersect write/update for f(c), a(t) */
	    /* intersect update/read for f(c), a(t) */
	    o_dd_intersect_reads(sd->nodelink);
#if ! defined DAVEW_THESIS_REDUCTIONS
            fixReductions(sd->nodelink);
#endif
	    }
	}


    memory_tests = 0;
#if !defined(OMIT_GETRUSAGE)
    struct rusage start_mem_time, end_mem_time, start_val_time, end_val_time,
		start_zap_time, end_zap_time;
    getrusage(RUSAGE_SELF, &start_mem_time);
#endif
#if !defined(OMIT_QUANTIFY_CALLS)
    quantify_start_recording_data();
#endif

    for( n = NameHead; n != NULL; n = n->namenext )
	{
	sd = n->namest.symdecl;
	/* variable name is:  n->namest.symname */
	if(sd != NULL && sd->nodelink != NULL && n->namest.symclass!=CLBUILTIN) 
	    {

	    if (n->namest.symdims == 0) continue;
	    /* find all output dependencies: write - write, */
	    /* write - update(write), update(write) - write */
	    o_dd_intersect_writes( sd->nodelink );
	    /* then find flow and anti dependencies */
	    /* intersect write/update for f(c), a(t) */
	    /* intersect update/read for f(c), a(t) */
	    o_dd_intersect_reads(sd->nodelink);
	    /* intersect update/update for r, r */
	    o_dd_intersect_update_update( sd->nodelink );
#if ! defined DAVEW_THESIS_REDUCTIONS
            fixReductions(sd->nodelink);
#endif
	    }
	}

#if !defined(OMIT_QUANTIFY_CALLS)
    quantify_stop_recording_data();
#endif
#if !defined(OMIT_GETRUSAGE)
    getrusage(RUSAGE_SELF, &end_mem_time);
#endif


    check_all_is_compressed(1);
    check_exit_is_compressed();
    check_all_inexact_bits();

    if (petit_args.dep_analysis_debug >= 1)
	fprintf(debug,
		"\n\n] >>> Finished memory-based dependence analysis <<<\n");
    reset_flow_cycle_bits(false);
    FindFlowCycles(petit_args.skipping_omega4,0);
    reset_flow_cycle_bits(true);
    FindFlowCycles(petit_args.skipping_omega4,1);


    if (!petit_args.skipping_omega4)
	{
	if (!petit_args.quiet) printf("+ Omega 4 analysis\n");
	if (petit_args.dep_analysis_debug >= 1)
	    fprintf(debug,
		    "\n\n>>> Starting value-based dependence analysis <<< [ \n");

	for( n = NameHead; n != NULL; n = n->namenext )
	    {
	    sd = n->namest.symdecl;
	    /* variable name is:  n->namest.symname */
	    if (sd != NULL && sd->nodelink != NULL &&
	        n->namest.symclass!=CLBUILTIN) 
		{
		if (petit_args.omitScalars || n->namest.symdims > 0)
		    continue;
		o_dd_do_accurate_kills(sd->nodelink);
		o_dd_merge(sd->nodelink, 0,1);
 		}
	    }

        reset_flow_cycle_bits(false);
        FindFlowCycles(0,0);
	reset_flow_cycle_bits(true);
	FindFlowCycles(0,1);

        kill_tests = 0;

#if !defined(OMIT_GETRUSAGE)
    getrusage(RUSAGE_SELF, &start_val_time);
#endif
#if !defined(OMIT_QUANTIFY_CALLS)
        quantify_start_recording_data();
#endif
	for( n = NameHead; n != NULL; n = n->namenext )
	    {
	    sd = n->namest.symdecl;
	    /* variable name is:  n->namest.symname */
	    if (sd != NULL && sd->nodelink != NULL &&
	        n->namest.symclass!=CLBUILTIN) 
		{
		if (n->namest.symdims == 0)
		    continue;
		o_dd_do_accurate_kills(sd->nodelink);
		o_dd_merge(sd->nodelink, 0,1);
 		}
	    }

#if !defined(OMIT_QUANTIFY_CALLS)
        quantify_stop_recording_data();
#endif
#if !defined(OMIT_GETRUSAGE)
    getrusage(RUSAGE_SELF, &end_val_time);
#endif
	
	if (petit_args.dep_analysis_debug >= 1)
	    fprintf(debug,
		    "\n\n] >>> Finished value-based dependence analysis <<<\n");

        reset_flow_cycle_bits(false);
        FindFlowCycles(1,0);
	reset_flow_cycle_bits(true);
	FindFlowCycles(1,1);
	}
    else 
	{ /* no Omega 4 */
	o_dd_merge( ExitNode,1 ,0);
	}

    check_all_is_compressed(4);
    check_exit_is_compressed();
    check_all_inexact_bits();

    
    if (petit_args.zappable_interesting_distance > 0)
	{
	if (!petit_args.quiet) printf("+ Conditional dependence analysis\n");
	if (petit_args.dep_analysis_debug >= 1)
	    fprintf(debug,
		    "\n\n>>> Starting search for conditional deps <<< [\n");

    zap_tests = 0;
#if !defined(OMIT_GETRUSAGE)
    getrusage(RUSAGE_SELF, &start_zap_time);
#endif
#if !defined(OMIT_QUANTIFY_CALLS)
        quantify_start_recording_data();
#endif

	for( n = NameHead; n != NULL; n = n->namenext )
	    {
	    sd = n->namest.symdecl;
	    /* variable name is:  n->namest.symname */
	    if (sd != NULL && sd->nodelink != NULL &&
		n->namest.symclass!=CLBUILTIN)
		{
		if (n->namest.symdims > 0)
		    o_dd_zappable( sd->nodelink );
		}
	    }
	o_dd_merge( ExitNode,1 ,0);
	o_dd_zappable( ExitNode );

#if !defined(OMIT_GETRUSAGE)
    getrusage(RUSAGE_SELF, &end_zap_time);
#endif
#if !defined(OMIT_QUANTIFY_CALLS)
        quantify_stop_recording_data();
#endif
	
	if (petit_args.dep_analysis_debug >= 1)
	    fprintf(debug,
		    "\n\n>>> Finished search for conditional deps <<<\n");
	}

    zapContainingLoops(Entry);

    check_all_is_compressed(5);
    check_exit_is_compressed();
    check_all_inexact_bits();

#if !defined(OMIT_GETRUSAGE)
    /* Get and print dependence analysis time */
    long memTime = user_millisecs(&start_mem_time,&end_mem_time);
    long valTime = user_millisecs(&start_val_time,&end_val_time);
    long zapTime = user_millisecs(&start_zap_time,&end_zap_time);
    long totalTime = memTime;
    if (kill_tests) {
	totalTime += valTime;
	assert(valTime >= 0);
	}
    if (zap_tests) {
	totalTime += zapTime;
	assert(zapTime >= 0);
	}

    static int print_all_timing_info = -1;
    if (print_all_timing_info < 0)
	print_all_timing_info = (getenv("PRINT_ALL_TIMING_INFO") ?
				 abs(atoi(getenv("PRINT_ALL_TIMING_INFO"))):0);

    if (print_all_timing_info)
	{
	printf("Dependence analysis total time is %6.3f seconds\n",
	       (double)(totalTime)/1000.);

	if (memory_tests)
	    {
	    printf("  Memory based: %6.3f s (avg of %6.3f ms for %d pairs)\n",
		   (double)memTime/1000.,
		   (double)memTime/1./memory_tests,memory_tests);
	    fprintf(debug,"  Memory based: %6.3f s (avg of %6.3f ms for %d pairs)\n",
		   (double)memTime/1000.,
		   (double)memTime/1./memory_tests,memory_tests);
	    }
	if (kill_tests)
	    {
	    printf("  Value  based: %6.3f s (avg of %6.3f ms for %d reads)\n",
		   (double)valTime/1000., (double)valTime/1./kill_tests,
				kill_tests);
	    fprintf(debug,"  Value  based: %6.3f s (avg of %6.3f ms for %d reads)\n",
		   (double)valTime/1000., (double)valTime/1./kill_tests,
				kill_tests);
	    }
	if (zap_tests)
	    {
	    printf("  Zappability : %6.3f s (avg of %6.3f ms for %d deps)\n",
		   (double)zapTime/1000., (double)zapTime/1./zap_tests,
			zap_tests);
	    fprintf(debug,"  Zappability : %6.3f s (avg of %6.3f ms for %d deps)\n",
		   (double)zapTime/1000., (double)zapTime/1./zap_tests,
			zap_tests);
	    }
	}
#endif

//  Don't need to invert dd distances any more - they are got right
//  frome the first time - i.e. from the relation that have to contain
//  correct information in any way.
//    InvertDDdistances();

    }/* build_smaller_dd_graph */



void calculate_depnum()
    {
    nametabentry *n;
    node *sd;

    int npairs=0, nmem=0, nval=0;

    for (n = NameHead; n != NULL; n = n->namenext)
        {
        sd = n->namest.symdecl;
        if (sd != NULL && sd->nodelink != NULL && 
	    n->namest.symclass!=CLBUILTIN && n->namest.symdims != 0)
            {
            //sd is array declaration node
            o_dd_merge(sd->nodelink, false, true);
            for (a_access write=sd->nodelink;write!=NULL; write=write->nodelink)
                {
                if (is_phi_access(write)) 
                    continue;
          
                if (!(store_op(write->nodeop) || update_op(write->nodeop)))
                    continue;

                for (a_access read=sd->nodelink;read!=NULL; read=read->nodelink)
                    {
                    if (is_phi_access(read))
                        continue;
                    if (!(fetch_op(read->nodeop) || update_op(read->nodeop)))
                        continue;

                    // now investigate dependence from write to read
            
                    npairs++;
                    if (access_shared_depth(write,read) == 0)
                        continue;
             
                    bool found_val=false;
                    bool found_mem=false;
                    for (dd_out_iterator dd=dd_o_i_for_access(write);
                         !dd_o_i_done(dd); dd_o_i_next(dd))
	                {
                        if (dd_o_i_cur_dest(dd) != read) 
			    continue;
                        if (!dd->dddir) //not loop carried
                            continue; 

                        // FOUND DEPENDENCE

                        assert(dd->ddtype == ddflow || dd->ddtype == ddreduce);

                        if  (dd_current_is_mem(dd))
		            {
                            assert(!found_mem);
                            nmem++;
                            found_mem=true;
		            }
                        if (dd_current_is_val(dd))
                            { 
			    nval++; 
                            assert(!found_val);
                            found_val=true;
                            }
	                } // for dd
	            } // for read
	        } //for write
            } //if good name
        } //for name

    printf("%4d %3d %3d \n", npairs, nmem, nval);
    } /* calculate_depnum */

} // end of namespace omega
