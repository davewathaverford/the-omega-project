/* $Id: uniform_misc.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Uniform_Misc
#define Already_Included_Uniform_Misc

#include <omega/Relations.h>
#include <basic/Dynamic_Array.h>
#include <basic/assert.h>
#include <petit/uniform-interf.h>
#include <uniform/depend_dir.h>

//#include <basic/String.h> AIS
//#include "Exit.h" AIS

namespace omega {

#define maxLevels	14
#define max_stmts 	74

#define Infinity        (pow(uniform_args.n, global_max_nest+1) * (double)nr_statements * (double)nr_statements * 1000000.0)

#define Bad        (pow(uniform_args.n, global_max_nest+1) * (double)nr_statements * (double)nr_statements * 1000.0)


typedef struct rel_arc
    {
    int child_index, parent_index;
    Relation rel;
    Relation local_diff[maxLevels]; 
    Relation constraints;
    struct rel_arc *next_forw_link, *next_back_link;
    } rel_edge;

class sinfo
    {
    public:
    AssignStmt *stmt;
    int nest;
    Relation bounds;
    int distributed[maxLevels];

    bool operator == (const sinfo &) {return false;}
    };

class depend_info
    {
    public:
    Relation self_depend;
    Relation self_local_diff[maxLevels];
    Relation constraints;
    Relation current_constraints;
    rel_edge *forw_link, *back_link;

    bool operator == (const depend_info &) {return false;}

    depend_info();
    ~depend_info();
    };

typedef Tuple<depend_info> depend_graph;

extern Tuple<sinfo> stmt_info;
extern depend_graph all_depends;
extern depend_graph val_flow_depends;

extern int nr_statements;
extern int global_max_nest;
extern int global_maxdepth;
extern Dynamic_Array1<Relation> time0;
extern Dynamic_Array2<Relation> T1;
extern Dynamic_Array2<Coef_Var_Decl *> coef_var;
extern Dynamic_Array2<int> negative;
extern Dynamic_Array2<int> was_negative;
extern Dynamic_Array2<String> var_name_list;

extern FILE *uniform_debug, *uniform_debug2;

extern void combine_schedules(int level);
extern void combine_schedule(int level, int p1);
extern void free_graph(depend_graph &d);
extern void remove_edge(rel_edge *e, depend_graph &d);
extern void exit_uniform();

extern void perform_space_search(int manual);
extern void perform_time_search(int manual);

extern void UniformError(char *t);

extern void process_uniform_args(char *arg, int &j);

extern void determine_steps(int i);

extern void find_names();

}

#endif
