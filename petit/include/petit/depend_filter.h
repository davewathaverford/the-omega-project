/* $Id: depend_filter.h,v 1.1.1.1 2000/06/29 19:24:24 dwonnaco Exp $ */
#ifndef Already_Included_Depend_Filter
#define Already_Included_Depend_Filter

#include <petit/tree.h>

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

#endif
