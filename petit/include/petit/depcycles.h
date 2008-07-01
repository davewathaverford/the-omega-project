/* $Id: depcycles.h,v 1.1.1.1 2000/06/29 19:24:24 dwonnaco Exp $ */
#ifndef Already_Included_Depcycles
#define Already_Included_Depcycles

#include <basic/bool.h>


extern void FindFlowCycles(bool,bool);
extern void reset_flow_cycle_bits(bool);
extern void InvertDDdistances(void);

#endif
