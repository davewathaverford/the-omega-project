/* $Id: depcycles.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Depcycles
#define Already_Included_Depcycles

#include <basic/bool.h>

namespace omega {

extern void FindFlowCycles(bool,bool);
extern void reset_flow_cycle_bits(bool);
extern void InvertDDdistances(void);

}

#endif
