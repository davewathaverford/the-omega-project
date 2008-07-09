/* dddriver.h,v 1.1.1.2 1992/07/10 02:39:46 davew Exp */
#ifndef Already_Included_DDDriver
#define Already_Included_DDDriver

#include <basic/bool.h>

namespace omega {

/* global DD algorithm to use */
extern unsigned int DDalgorithm;

#define DDalg_omega		1
#define DDalg_omega_preproc     2
#define DDalg_epsilon           3
#define DDalg_banerjee          4


/* routine to build data dependence graph. */
/* linkio must already have been called. */

extern void build_dd_graph( bool debug );

/* build_dd_graph uses the dependence test indicated by DDalgorithm
   to test for dependencies.  The test may either add dependence arcs
   to the nodes, or set dd, dddir, and dddiff to the combination of
   all dependencies:

   bool *dd		*dd is true iff there as a dependence
   dddirection *dddir	*dddir describes the direction vectors, as in tree.h
   Nvector dddiff	dddiff[1..bnest] are the dependence differences
			  the value "ddunknown" indicates unknown difference

   dd will be set to 0 if all dependencies have been added by the test
   (in other words, the 3 parameters above describe all un-added dependencies)
*/


/* zap_dd_graph removes all dd information.
   This routine should be called before build_dd_graph, unless
   the parse tree has not had any info added yet or you want to
   add to the existing dd information
 */

extern void zap_dd_graph();

}

#endif
