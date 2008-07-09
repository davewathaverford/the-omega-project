/* $Id: ivr.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/*********************************************************************
 *                                                                   *
 * Include file for induction variable recognition.                  *
 * System-independent part (see ivr-lang-interf.h for sys-dep part). *
 * Written by Vadim Maslov vadik@cs.umd.edu 08/15/92.                *
 *                                                                   *
 *********************************************************************/
#ifndef Already_Included_IVR
#define Already_Included_IVR


#include <petit/ivr-lang-interf.h>

namespace omega {
/*
 * Global data fot Tarjan algorithm
 */
#define MAXPTSTACK 600
extern pEXPR pt_stack[];  /* FindSCC: stack */
extern int   ps_i;	  /* FindSCC: stack position */
extern SCCNO vertex_no;   /* FindSCC: number of current vertex */
extern SCCNO sc_region;	  /* FindSCC: number of current SCC */

extern pEXPR (*GoDownFun)(pEXPR, void**); 
extern pEXPR (*GoRightFun)(pEXPR, pEXPR, void**);
extern void  (*AddToScc)(pEXPR); 
extern void  (*ProcessScc)(pEXPR);
extern char   *FindSccTxt; 

extern int   no_ssag_nodes; 


/*
 * Function prototypes
 */
void      InitSSAgraph(void);
pEXPR     AddSSAgraph(pEXPR);
void      RemoveSSAgraph(pEXPR);

void      DoIvrFs(pSTMT, int);
SCCNO     FindSCC(pEXPR);
void      NoOpListDelete(void);

void      ElimDeadAssgmts(void);

void      RecUpdate(pSTMT);

} // end omega namespace

#endif
