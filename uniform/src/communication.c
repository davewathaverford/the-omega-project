/* $Id: communication.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <math.h>
#include <uniform/communication.h>
#include <uniform/uniform_args.h>
#include <uniform/parallelism.h>
#include <uniform/space_mappings.h>

namespace omega {

static search_cost comm_far(extended_dir_list *n)
    {
#if 0
    fprintf(uniform_debug, "far %d %d\n", n->dim, n->pdim);
#endif

    return convert_cost(uniform_args.comps_per_comm * 
			pow(uniform_args.n, n->dim) * 
			(n->pdim == 0 ? 1 : uniform_args.Num_Procs));
    } /* comm_far */


static search_cost comm_near(extended_dir_list *n)
    {
#if 0
    fprintf(uniform_debug, "near %d %d\n", n->dim, n->pdim);
#endif

    return convert_cost(uniform_args.comps_per_comm * 
                        pow(uniform_args.n, n->dim-1) * 
			(n->pdim == 0 ? 1 : uniform_args.Num_Procs) *
                       uniform_args.Num_Procs);
    } /* comm_near */



search_cost compute_edge_cost(int p, int q, int cp, int cq)
    {
    search_cost cost = 0;
#if 0
    fprintf(uniform_debug, "compute_edge_cost(p %d, q %d, cp %d, cq %d)\n", 
	    p, q, cp, cq);
#endif

    for (extended_dir_list *n=extended_original_dir[p][q]; n!=NULL; n=n->next)
        {
#if 0
        print_extended_dir(n);
        fprintf(uniform_debug, "\n");
#endif
        if (!n->zero[cp][cq])
            {
            if (n->constant[cp][cq] && !Use_Cyclic)
                cost = max(cost, comm_near(n));
            else
                cost = max(cost, comm_far(n));
            }
        }

    assert(cp > 0 || cq > 0 || cost == 0);
    assert(cost < MAX_COST);
    return cost;
    } /* compute_edge_cost */

}
