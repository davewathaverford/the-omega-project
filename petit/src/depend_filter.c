/* $Id: depend_filter.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <petit/depend_filter.h>
#include <petit/ops.h>

namespace omega {

int flow_filter;
int output_filter;
int anti_filter;
int reduce_filter;
int memory_filter;
int value_filter;
int noncarried_filter;
int noncyclic_filter;
int scalar_filter;

void depend_filter_init(void)
{
    flow_filter = 1;
    reduce_filter = 1;
    memory_filter = 1;
    value_filter = 1;
    noncarried_filter = 1;
    noncyclic_filter = 1;
    scalar_filter = 1;
#if !defined Live_RT
    output_filter = 1;
    anti_filter = 1;
#else
    output_filter = 0;
    anti_filter = 0;
#endif
    } /* depend_filter_init */


int depend_filter( ddnode *dd)
    {
    unsigned int n;
    int carried = 0;

    for (n=1; n<=dd->ddnest; n++)
        carried = carried || ddextract1(dd->dddir, n) != ddind;

    return
           (dd->ddtype != ddflow   || flow_filter) &&
           (dd->ddtype != ddanti   || anti_filter) &&
           (dd->ddtype != ddoutput || output_filter) &&
           (dd->ddtype != ddreduce || reduce_filter) &&
           (((dd->ddflags  &  ddmem) &&  memory_filter) ||
           ((dd->ddflags  &  ddval) &&  value_filter)) &&
           (is_array_ref(dd->ddsucc) || scalar_filter) &&
           (carried                  || noncarried_filter) &&
           ((dd->ddflags  & ddisInFlowCycle)   || noncyclic_filter);
    } /* depend_filter */

} // end namespace omega
