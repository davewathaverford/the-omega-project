/* $Id: ddomega.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/*
   This file now contains only the main dependence test function.
   see refine.h, cover.h, and kill.h for the other functions that
   used to be declared here.

   The structures used in these functions are now also declared here.
 */


#ifndef Already_Included_DDOmega
#define Already_Included_DDOmega


#include <basic/bool.h>
#include <petit/range.h>
#include <petit/lang-interf.h>

namespace omega {


bool subscripts_ZIV_test(a_access access1, a_access access2);


/* 
   perform the omega test on the array accesses access1 and access2
   see dddriver.h for a description of "dd" arguments.
 */

void dd_omega_test(a_access access1, a_access access2,
		   ddnature oitype, ddnature iotype,
		   unsigned int nest1, unsigned int nest2, unsigned int bnest);



void GetLoopDirs(a_access, int, signed char *);

#if 0

extern var_id *current_set_of_vars;        /* used in getVarName fns */

/* 
   delta problem description contains information needed
   to associate variable accesses in the petit program with
   variables in the integer programming problem.
   The different ranges show which part of the array of
   variables in the IP problem correspond to different
   accesses in the petit program.
   */

typedef struct {
    range deltas;    /* deltas for common indices  */
    range access1s;  /* index variables for access 1 */
    range access2s;  /* index variables for access 2 */
    range nonloops;  /* symbolic constants */
    range steps1;    /* step constraints for a1 */
    range steps2;    /* step constraints for a1 */
    range scvars1;   /* scalar variables for a1 */
    range scvars2;   /* scalar variables for a2 */
    var_id vars[maxVars];
} delta_prob_desc;


/*
   build a delta_prob_desc for the dependence from access1 to access2,
   return 0 if it obviously can't have solutions because
   the subscripts obviously can't be equal.

   Note that *dpd should be allocated at least as long as *prob,
   since the _getVarNameArgs filed of *prob will point to it.
 */

int build_delta_prob_desc(delta_prob_desc *dpd, Problem *prob,
			  a_access access1, a_access access2,
			  int nest1, int nest2, int bnest,
			  signed char *loop_dirs);

/* the following are used by build_delta_prob_desc */


void delta_init(delta_prob_desc *dpd, Problem *p,
		int delta_color, unsigned int Nd, signed char *loop_dirs,
		unsigned int Na1, var_id a1_vars[],
		unsigned int Na2, var_id a2_vars[],
		unsigned int Nsc, var_id sc_vars[],
		unsigned int Ns1, var_id s1_vars[],
		unsigned int Ns2, var_id s2_vars[],
		unsigned int Nvr, var_id vr_vars[]);
void delta_inv(delta_prob_desc *dpd, Problem *p);
void delta_cleanup(delta_prob_desc *dpd);
#define delta_Nvars(dpd)     (r_last(&(dpd)->scvars2))

#endif

}

#endif
