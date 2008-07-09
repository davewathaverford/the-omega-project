/* $Id: parallelism.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Parallelism
#define Already_Included_Parallelism

#include <basic/Dynamic_Array.h>
#include <uniform/depend_dir.h>
#include <uniform/search.h>

namespace omega {

typedef Tuple<int> IntTuple;

extern Dynamic_Array2<search_cost> stored_pipeline_cost;
extern Dynamic_Array1<int> incompatible_stmt;
extern Dynamic_Array2<int> incompatible_node;
extern Dynamic_Array2<int> used_order;
extern Dynamic_Array2<IntTuple> good_perms;
extern Dynamic_Array2<IntTuple> stored_perm_dir;
extern Dynamic_Array2<IntTuple> stored_perm_order;
extern Dynamic_Array2<dir_list *> closure_dir;


#if defined(SPEED)
extern na *is_node_compatible;
extern nb *is_perm_compatible;
extern nb *dist_level;
#else
extern Dynamic_Array4<int> is_node_compatible;
extern Dynamic_Array4<int> is_perm_compatible;
extern Dynamic_Array4<int> dist_level;
#endif


extern void determine_parallelism();
extern void analyze_parallelism();
extern void print_perm(FILE *f, int stmt, int perm);
extern int unbalanced(int i, int si);
extern search_cost pipeline_cost2(int p, int cp, int sequential);
extern search_cost pipeline_cost(int p, int /*cp*/, int x, int y,
			         int dist_level, int sequential);

}

#endif
