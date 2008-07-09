/* $Id: depend_filter.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Depend_Filter
#define Already_Included_Depend_Filter

#include <petit/tree.h>

namespace omega {

extern int flow_filter;
extern int output_filter;
extern int anti_filter;
extern int reduce_filter;
extern int memory_filter;
extern int value_filter;
extern int noncarried_filter;
extern int noncyclic_filter;
extern int scalar_filter;

extern void depend_filter_init(void);

extern int depend_filter(ddnode *dd);

}

#endif
