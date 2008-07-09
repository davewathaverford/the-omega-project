/* $Id: */

#include <uniform/uniform_misc.h>
#include <uniform/parallelism.h>
#include <uniform/search.h>

namespace omega {

extern void select_time_mappings();
extern search_cost locality_cost(int p, int pi, int si);
extern search_cost false_sharing_cost(int p, int pi, int si);

extern void analyze_locality();

extern int best_time[max_stmts];

extern int time_coef[maxLevels][max_stmts];

extern int dist_loop[max_stmts][max_perms];

extern int group[maxLevels][max_stmts];
extern int nr_groups[maxLevels];

}

