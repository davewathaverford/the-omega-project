/* $Id: uniform_misc.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <basic/Dynamic_Array.h>
#include <omega/Relations.h>
#include <omega/Rel_map.h>
#include <petit/uniform-interf.h>
#include <uniform/uniform_args.h>
#include <uniform/uniform_misc.h>
#include <uniform/depend_rel.h>
#include <uniform/parallelism.h>
#include <uniform/communication.h>
#include <uniform/select.h>
#include <uniform/time_mappings.h>
#include <uniform/simple_codegen.h>
#include <petit/Exit.h>

namespace omega {


int nr_statements;
Tuple<sinfo> stmt_info;
depend_graph val_flow_depends;
depend_graph all_depends;
int global_max_nest;
int global_maxdepth;
Dynamic_Array1<Relation> time0("Stmt");
Dynamic_Array2<Relation> T1("Level", "Stmt");
Dynamic_Array2<Coef_Var_Decl *> coef_var("Stmt", "Nest");
Dynamic_Array2<int> was_negative("Stmt","Nest");
Dynamic_Array2<String> var_name_list("Stmt", "Nest");
FILE *uniform_debug, *uniform_debug2;



void UniformError(char *t) 
    {
    fprintf(stderr,"\nUniform Error: %s\n", t);
    Exit(1);
    } /* UniformError */



void find_names()
    {
    int i, j;
 
    var_name_list.resize(nr_statements, global_max_nest+1);
 
    for (i=0; i<nr_statements; i++)
        {
        j = stmt_info[i+1].nest;
        for (LoopIterator li = LoopInnerMost(stmt_info[i+1].stmt);
	     !LoopLast(li);
	     LoopNext(li))
            var_name_list[i][j--] = VarName(LoopIndex(LoopCurr(li)));
        }
    } /* find_names */




void remove_edge(rel_edge *e, depend_graph &d)
    {
    rel_edge *e1, *e2;
    int p1, p2;

    p1 = e->parent_index;
    p2 = e->child_index;

    if (d[p1+1].forw_link == e)
        d[p1+1].forw_link = e->next_forw_link;
    else
        {
        for (e1 = d[p1+1].forw_link; 
             e1->next_forw_link != e; 
             e1 = e1->next_forw_link);

        e1->next_forw_link = e->next_forw_link;
        }

    if (d[p2+1].back_link == e)
        d[p2+1].back_link = e->next_back_link;
    else
        {
        for (e2 = d[p2+1].back_link; 
             e2->next_back_link != e; 
             e2 = e2->next_back_link);

        e2->next_back_link = e->next_back_link;
        }

    delete e;
    e = NULL;
    } /* remove_edge */


depend_info::depend_info()
    {
    forw_link = NULL;
    } /* depend_info */


depend_info::~depend_info()
    {
    rel_edge *nnext;
    for (rel_edge *e = forw_link; e!=NULL; e = nnext)
        {
        nnext = e -> next_forw_link;
        delete e;
        }
    } /* depend_info */



void determine_steps(int i)
    {
    int level = stmt_info[i+1].nest;
    for (LoopIterator li = LoopInnerMost(stmt_info[i+1].stmt);
         !LoopLast(li);
         LoopNext(li))
	{
        was_negative[i][level] = ExprIsNegOne(LoopStep(LoopCurr(li)));
	level--;
	}
    } /* determine_steps */



void combine_schedule(int level, int p1)
    {
    time0[p1] = T1[1][p1];

    for (int j=2; j<=level; j++)
        {
        int i;
        Mapping map1(time0[p1].n_inp(), time0[p1].n_out());
        for (i=1; i<=time0[p1].n_inp(); i++)
            map1.set_map(Input_Var, i, Input_Var, i);
        for (i=1; i<=time0[p1].n_out(); i++)
            map1.set_map(Output_Var, i, Output_Var, i);

        Relation s2 = T1[j][p1];
        Mapping map2(s2.n_inp(), s2.n_out());
        for (i=1; i<=s2.n_inp(); i++)
            map2.set_map(Input_Var, i, Input_Var, i);
        map2.set_map(Output_Var, 1, Output_Var, time0[p1].n_out()+1);

        time0[p1] = MapAndCombineRel2(time0[p1], s2, map1, map2, Comb_And,
                                      time0[p1].n_inp(), 
                                      time0[p1].n_out()+1);
        time0[p1].simplify(2,2);
        }
    } /* combine_schedule */


void combine_schedules(int level)
    {
    time0.resize(nr_statements*2);
    for (int p1=0; p1<nr_statements; p1++)
	{
	combine_schedule(level, p1);
	for (int k=1; k<=stmt_info[p1+1].nest; k++)
	    time0[p1].name_input_var(k, var_name_list[p1][k]);
	}
    } /* combine_schedules */




void perform_space_search(int manual)
    {
    perform_search(manual);
    } /* perform_space_search */



void perform_time_search(int manual)
    {
    perform_search(manual);
    } /* perform_time_search */

}
