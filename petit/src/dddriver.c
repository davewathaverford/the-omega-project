/* dddriver.c */

#include <stdio.h>
#include <stdlib.h>

#if !defined(OMIT_GETRUSAGE)
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <basic/bool.h>
#include <basic/assert.h>
#include <petit/Exit.h>
#include <petit/tree.h>
#include <petit/ops.h>
#include <petit/ddutil.h>
#include <petit/ddomega.h>
#include <petit/dddriver.h>
#include <petit/ddodriver.h>
#include <petit/ddepsdriver.h>
#include <petit/ddnest.h>
#include <petit/print.h>
#include <petit/browsedd.h>
#include <petit/lang-interf.h>
#include <petit/timeTrials.h>
#include <petit/timer.h>
#include <petit/debug.h>
#include <petit/petit_args.h>

namespace omega {

void build_dd_graph( bool debug_flag )
{
#if defined graphBuildTime
    int parsed_number;
    long time;
#endif


    if (!Entry) return;  /* defensive programming */

    DDdebug = debug_flag;
    if( DDdebug ) printf("DD debug output:\n");
    
    /* intersect the IN/OUT lists */


#if defined(graphBuildTime) && !defined(OMIT_GETRUSAGE)
    start_clock();
#endif

    if (petit_args.DDalgorithm == DDalg_omega || petit_args.DDalgorithm == DDalg_omega_preproc) {
	build_smaller_dd_graph();
    }
    else if (petit_args.DDalgorithm == DDalg_epsilon || petit_args.DDalgorithm == DDalg_banerjee) {
        epsilon_build_dd_graph();
    } else {
	assert(0);
      }

#if defined(graphBuildTime) && !defined(OMIT_GETRUSAGE)
   time=clock_diff();
   parsed_number=get_difdep_number();
   if (petit_args.DDalgorithm==DDalg_omega)
     fprintf(debug,"O %6.3f %d\n", time/1000., parsed_number);
   else if (petit_args.DDalgorithm==DDalg_epsilon)
     fprintf(debug,"E %6.3f %d\n", time/1000., parsed_number);
#endif
}/* build_dd_graph */


static void zap_dd_list(ddnode *ddn)
{  ddnode *next;
   while (ddn != 0)  {
	   next = ddn->ddnextsucc;
	   if (ddn->ddsucc->nodeddin == ddn)
	   	ddn->ddsucc->nodeddin = NULL;
	   if (ddn->ddpred->nodeddin == ddn)
	   	ddn->ddpred->nodeddin = NULL;
	   if (ddn->ddsucc->nodeddout == ddn)
	   	ddn->ddsucc->nodeddout = NULL;
	   if (ddn->ddpred->nodeddout == ddn)
	   	ddn->ddpred->nodeddout = NULL;
	   delete ddn;
	   ddn = next;
	   }
  }

static void zap_dds(node *n)
{
    zap_dd_list(n->nodeddout);
/*
    assert(n->nodeddout == NULL);
*/
    if (n->nodechild)
	zap_dds(n->nodechild);
    if (n->nodenext)
	zap_dds(n->nodenext);
}

/* reset cover_depth and terminator_depth */
static void zap_levels(node *n)
{
    n->cover_depth = n->terminator_depth = -1;
    if (n->nodechild)
	zap_levels(n->nodechild);
    if (n->nodenext)
	zap_levels(n->nodenext);
}

void zap_dd_graph(void)
{
    BRDD_Tagged = 0;
    /* Wayne doesn't want me to reset BRDD_Current here,
       "Just in case" */

    if (Entry) {
	zap_dds(Entry);
	zap_levels(Entry);
    }
}

} // end omega namespace
