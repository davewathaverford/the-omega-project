/* $Id: ddInf.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/* dependence information to compare Omega and Epsilon tests */

#if defined compareOmegaEpsilon
 
#include <basic/bool.h>
#include <basic/assert.h>
#include <petit/lang-interf.h>
#include <petit/print.h>
#include <petit/debug.h>
#include <petit/dddriver.h>
#include <petit/ddInf.h>
#include <petit/classify.h>

namespace omega {

bool storeInf=0;

static int nAtomicV; /* number of atomic DD vectors */ 
static int appl;     /* applicability type */
static int eps;      /* how found by epsilon */

/*****
 * resetAtomCount - reset count of atomic DD vectors 
 **/

void resetAtomCount()
{
 nAtomicV=0;
 appl=APPL_FULL;
 eps=EPSILON_OTHER;
} 

/****
 * addAtomCount - note dependence into atomic DD vectors counting 
 * ENTRY:
 *   d_info - DD information
 ****/

void addAtomCount(dir_and_diff_info * d_info)
{
 int nestCount,s=0,m=1;
 dddirection dir;

 for (nestCount=1; nestCount<=d_info->nest; nestCount++)
   {
    dir=ddextract1(d_info->direction,nestCount);
    assert(!d_info->differenceKnown[nestCount]
           || (d_info->difference[nestCount]>0  && dir==ddfwd)
           || (d_info->difference[nestCount]==0 && dir==ddind)
           || (d_info->difference[nestCount]<0  && dir==ddbck) );
    s=0;
    if (ddtest1(dir,ddfwd))
      s++;
    if (ddtest1(dir,ddind))
      s++;
    if (ddtest1(dir,ddbck))
      s++;
    m*=s;
   } /* end for */
 nAtomicV+=m; 
} /* end addAtomCount */



/****
 * n_atom_output - output information about number of atomic DD vectors
 * ENTRY:
 *   alg - algorithm type
 *   a1,a2 - input/output nodes
 ****/


void n_atom_output ( unsigned int alg,  node * a1, node * a2)
{
 char c;

 switch (alg)
   {
   case DDalg_omega : c='N'; break;
   case DDalg_omega_preproc: c='J'; break;
   case DDalg_banerjee: c='L'; break;
   case DDalg_epsilon: c='M'; break
   }
   
 fprintf(debug,
         "%c %p %p %d\n", c, a1, a2, nAtomicV);  
} /* end n_atom_output */


/****
 * setApplReason - set a reson why test isn't applicable
 * ENTRY
 *   reason - 
 **/

void setApplReason(int reason) {
appl=appl | reason;
}


/****
 * setEpsInf - set information about epsilon test
 * ENTRY
 *   kind - kind of operations performed
 **/

void setEpsInf(int kind) {
eps=kind;
}

/***
 * getEpsInf - get information about epsilon test
 * EXIT
 *   information stored so far
 ***/

int getEpsInf()  {
return eps;
}



/****
 * class_inf_output - output information about dependence
 * ENTRY
 *   a1,a2 - input/output nodes  
 ****/

void class_inf_output(node * a1, node * a2) {
fprintf (debug,
         "I %p %p %d %d\n", a1,a2, appl, eps);
}
 
} // end omega namespace

#endif


