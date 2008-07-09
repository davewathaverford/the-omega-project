/* $Id: dd_misc.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_DD_Misc 
#define Already_Included_DD_Misc

#include <basic/bool.h>

namespace omega {

void o_dd_merge_between_nodes(a_access From, a_access To, int depth, 
                              bool merge_levels);

void o_dd_merge(a_access list, bool merge_levels, bool do_relations);

void o_dd_split_between_nodes(a_access From, a_access To);

typedef void (*dd_map_fn)(dd_current ddn, void *args);
extern  void map_over_dd(a_access From, a_access To, dd_map_fn fn,
                         void * args);


void do_compress_operation_on_reduction(a_access list, int operation_type);
void do_compress_operation_on_output(a_access list, int operation_type);
void do_compress_operation_on_flow (a_access list, int operation_type);

#define compress_dd_output(A)   do_compress_operation_on_output(A,1)
#define uncompress_dd_output(A) do_compress_operation_on_output(A,0)
#define compress_dd_reduction(A)   do_compress_operation_on_reduction(A,1)
#define uncompress_dd_reduction(A) do_compress_operation_on_reduction(A,0)
#define compress_dd_flow(A)     do_compress_operation_on_flow(A,1)
#define uncompress_dd_flow(A)   do_compress_operation_on_flow(A,0)

}

#endif 
