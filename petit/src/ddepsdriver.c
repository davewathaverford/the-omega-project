/* $Id: ddepsdriver.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/* murka - 11.12.93 */

#include <basic/assert.h>
#include <basic/Exit.h>
#include <petit/lang-interf.h>
#include <petit/ddnest.h>
#include <petit/notedd.h>
#include <petit/browse.h>
#include <petit/dddriver.h>
#include <petit/ddepsdriver.h>
#include <petit/ddepsilon.h>
#include <petit/timeTrials.h>
#include <petit/timer.h>
#include <petit/ddInf.h>
#include <petit/dd_misc.h>
#include <petit/depcycles.h>
#include <petit/petit_args.h>

namespace omega {

/***
 * dd_eps_test - test dependence for 2 accesses (whole thing)
 * ENTRY
 *  onode, inode - access nodes
 *  oitype,iotype - dependence types
 *  onest, inest - nest levels
 *  bnest        - common nest level
 * EXIT
 *   1 - preproc epsilon was OK
 *   0 - still need omega
 ***/

/* I am not concerned with Time trials and Enty/Exit nodes right now.
   This should be added later 
   I also don't handle negative step. 
 */

int dd_eps_test(a_access access1, a_access access2,
			ddnature oitype, ddnature iotype,
			unsigned int nest1, unsigned int nest2, unsigned int bnest)
{

dir_and_diff_info d_info;
int i;
situation  sit;

/* some assertions. May be I need all from dd_omega-test ? */

assert(access_sym(access2) == access_sym(access1)); /* same var */



d_info.nest=bnest;
d_info.direction=0;
d_info.dd_relation=NULL;
/* for now assume all steps to be positive */
for (i=1; i<=(int)bnest; i++)
  {
  d_info.loop_dir[i] = 1;
  d_info.differenceKnown[i]=0;
  d_info.difference[i]=0;
  dddirset(d_info.direction,ddall,i);
}

d_info.flags = ddmem;
d_info.restraint = (unsigned)-1;

  
sit.access1=access1; 
sit.access2=access2;
sit.oitype=oitype;
sit.iotype=iotype;
sit.nest1=nest1;
sit.nest2=nest2;
sit.commonNesting=bnest;

epsilon_test(access1,access2,&d_info,&sit);

return 0;
} /*dd_eps_test */

/****
 * eps_dd_intersect - intersect 2 accesses
 * ENTRY
 *  onode, inode - access nodes
 *  oitype,iotype - dependence types
 ****/

static void eps_dd_intersect(node * onode, node * inode,
           ddnature oitype, ddnature iotype)
{
#if defined newTimeTrials
    unsigned int count = 0;
    long epstime;
#endif


#if defined compareOmegaEpsilon
    resetAtomCount();
#endif

 /* nest level of loop around onode, inode, both nodes, max nest */
 unsigned int onest, inest, bnest, mnest;
         
dd_get_nests( onode, inode, &onest, &inest, &bnest );
dd_fix_nests( onode, inode, onest, inest, bnest );

/* mnest is the maximum nest with which we have to deal */
mnest = inest>onest?inest:onest;
    
if( bnest > maxCommonNest || mnest > maxnest ){
        fprintf( stderr, "*** too many nested loops ***\n" );
        Exit(2);
}
#if defined newTimeTrials
    timing1reset();
    storeResult = 1;
#endif

#if defined compareOmegaEpsilon
    if (petit_args.DDalgorithm == DDalg_epsilon)
      storeInf = 1;
#endif

dd_eps_test(onode,inode, oitype, iotype, onest, inest, bnest);

#if defined compareOmegaEpsilon

 /* if we're not checking the top level, don't print stats for it */
 if (bnest != 0 || !petit_args.omitTopLevel)
   {
   n_atom_output(DDalg_epsilon,  onode, inode);
   if (petit_args.DDalgorithm == DDalg_epsilon)
      class_inf_output(onode,inode);
 }
#endif

/* measure time */
#if defined(newTimeTrials) && ! defined(OMIT_GETRUSAGE)
  storeResult = 0;

#if defined compareOmegaEpsilon
  storeInf = 0;
#endif

 /* if we're not checking the top level, don't print stats for it */
 if (bnest == 0 && petit_args.omitTopLevel)
   return;



  start_clock();
  do {
	dd_eps_test(onode, inode, oitype, iotype, onest, inest, bnest);
	count++;
  } while (count<timeMaxReps &&
	     (count<timeMinReps || clock_diff()<timePeriod));
  epstime = clock_diff();
  eps_time_output(oitype, onode,inode, count,epstime);
#endif

} /* eps_dd_intersect */



/****
 * eps_dd_intersect_list - find all dependencies for a variable
 *                         using epsilon test
 * ENTRY
 *   list - beginning of the references for a variable
 ****/


/* for now do like in dd_intersect list.
   Later add update_op and Entry, Exit from intersect_write etc
 */

static void eps_dd_intersect_list(node * list) {
node *l1, *l2;
    
for(l2=list; l2!=NULL; l2=l2->nodelink) {
 if(is_phi_access(l2))  continue;
    /* stop second loop after l1 reaches l2 */
    for( l1=list; l1 != NULL; l1 = (node *) ( l1==l2 ? NULL : l1->nodelink ) ){
	  if(is_phi_access(l1)) continue;
	  
          if ((store_op(l1->nodeop) && store_op(l2->nodeop))
            ||(update_op(l1->nodeop) && store_op(l2->nodeop))
            ||(store_op(l1->nodeop) && update_op(l2->nodeop)))
                 eps_dd_intersect(l1,l2,ddoutput,ddoutput);
           
          if ((store_op(l1->nodeop) && fetch_op(l2->nodeop))
            ||(update_op(l1->nodeop) && fetch_op(l2->nodeop))
            ||(store_op(l1->nodeop) && update_op(l2->nodeop)))
                 eps_dd_intersect(l1,l2,ddflow,ddanti);
           
          if ((fetch_op(l1->nodeop) && store_op(l2->nodeop))
            ||(fetch_op(l1->nodeop) && update_op(l2->nodeop))
            ||(update_op(l1->nodeop) && store_op(l2->nodeop)))
                 eps_dd_intersect(l1,l2,ddanti,ddflow);
          if (update_op(l1->nodeop) && update_op(l2->nodeop))
           {
              if (accesses_are_same_update_type(l1,l2))
                 eps_dd_intersect(l1,l2,ddreduce,ddreduce);
              else {
                 eps_dd_intersect(l1,l2,ddflow,ddanti);
                 eps_dd_intersect(l1,l2,ddanti,ddflow);
                 eps_dd_intersect(l1,l2,ddoutput,ddoutput);
                 }
            }
	} /* for l1 */
    }  /* for l2 */
}/* eps_dd_intersect_list */


/*****
 * epsilon_build_dd_graph - build data dependence graph for 
 *   the epsilon test
 *   I don't like some things in build_dd_graph and I don't need all
 *   value-based stuff from build_smaller_dd_graph (for omega test).
 *   So this is the third function for the same stuff
 ****/

void epsilon_build_dd_graph(void)
{ 
    nametabentry *n;
    node *sd;
   
  assert(Entry->nodenext == NULL);

/* don't do initializeContainingLoops for now.
   First, I am not sure I need it, second it's static in ddodriver
 */

  /* intersect the IN/OUT lists */

  for( n = NameHead; n != NULL; n = n->namenext ){
   sd = n->namest.symdecl;
  /* variable name is:  n->namest.symname */
   if(sd != NULL && sd->nodelink != NULL && n->namest.symclass!=CLBUILTIN) {
       if (petit_args.omitScalars && n->namest.symdims == 0) continue;
       eps_dd_intersect_list( sd->nodelink );
       o_dd_merge(sd->nodelink,1,0); 
     }
   }
    reset_flow_cycle_bits(false);
    FindFlowCycles(1,0);
    reset_flow_cycle_bits(true);
    FindFlowCycles(1,1);

}/* Epsilon_build_dd_graph */

	
/****
 * get_difdep_number - get number of different dependences found 
 * EXIT 
 *  return - number of different dependeces found
 */
 
/* NOTE - this probably won't work. I need to be sure that dependeces
   between the same nodes are adjacent. Nothing garantees that
 */

int get_difdep_number(void)
{
node *n;
ddnode *dd;
unsigned int s;
node * ddpred, *ddsucc;

    s = 0;
    ddpred=ddsucc=NULL;
    for(n = Entry; n != ExitNode; n = br_go_to_another( n )) {
        switch( n->nodeop ){
	    /* print out dependence relations here */
	CASE_MEMREF:
	case op_entry:
	    for( dd = n->nodeddout; dd != NULL; dd = dd->ddnextsucc ){
             if (dd->ddpred!=ddpred || dd->ddsucc!=ddsucc)
               s++;
             ddpred=dd->ddpred;
             ddsucc=dd->ddsucc;
	   } /* for */
             break;
        default: break;
        }
    }
    
    return s;
}/* get_difdep_number */

} // end of omega namespace
