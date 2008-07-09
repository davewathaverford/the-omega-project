/* $Id: */
#ifndef Already_Included_Search
#define Already_Included_Search

#include <omega/Relations.h>
#include <basic/Dynamic_Array.h>
#include <uniform/uniform_misc.h>

#define max_nest	14
#define max_perms	24

#define search_cost double

#define MAX_COST	MAXDOUBLE
#define convert_double(c)	((double)c)
#define convert_cost(c)		((double)c)

#define max_candidates  24

namespace omega {

#if defined (SPEED)


typedef search_cost ma[max_stmts][maxnest][maxnest];
typedef search_cost mb[max_stmts][maxnest];
typedef char na[max_stmts][maxnest][maxnest];
typedef char nb[max_stmts][max_perms][max_perms];

extern ma *edge_cost;
extern int candidates[max_stmts];
extern int valid_candidate[max_stmts][max_candidates];

#else

extern Dynamic_Array4<search_cost> edge_cost;
extern Dynamic_Array1<int> candidates;
extern Dynamic_Array2<int> valid_candidate;

#endif


extern Dynamic_Array1<int> best;
extern search_cost best_cost;

extern void perform_search(int aut);
extern void process_edge_costs();

}

#endif
