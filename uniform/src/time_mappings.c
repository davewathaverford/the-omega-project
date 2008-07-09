/* $Id: time_mappings.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <math.h>
#include <basic/Dynamic_Array.h>
#include <omega/Relations.h>
#include <uniform/parallelism.h>
#include <uniform/uniform_misc.h>
#include <uniform/time_mappings.h>
#include <uniform/depend_dir.h>
#include <uniform/search.h>
#include <uniform/depend_rel.h>
#include <uniform/space_mappings.h>
#include <uniform/space_constants.h>
#include <uniform/wak_codegen.h>
#include <uniform/uniform_args.h>

namespace omega {

int best_time[max_stmts];
int time_coef[maxLevels][max_stmts];
int dist_loop[max_stmts][max_perms];

int group[maxLevels][max_stmts];
int nr_groups[maxLevels];

static depend_graph temp_graph;

#define dist_is_parallel(i)        (stored_pipeline_cost[i][best_space[i]] <                                      convert_cost(pow(uniform_args.n,                                                                 stmt_info[i+1].nest)))

#define listi                                good_perms[i][best_space[i]]
#define listj                                good_perms[j][best_space[j]]

#define good_stored_order_i(posn, pi) stored_perm_order[i][posn][listi[pi]]
#define good_order_i(posn,pi,si)      stored_perm_order[i][posn][good_perms[i][si][pi]]

#define max_subscripts                   10
#define max_refs                         10


static int references[max_stmts];
static int subscripts[max_stmts][max_refs];
static int involves[max_stmts][max_refs][max_subscripts][maxLevels];


static void traverse_parse_tree(int stmt, AssignStmt *parent)
    {
    for (AccessIterator ai = AccessLeftMost(parent);
	 !AccessLast(ai);
	 AccessNext(ai))
	{
        int ref = references[stmt];
	Access *a = AccessCurr(ai);
        subscripts[stmt][ref] = 0;
	for (SubscriptIterator si = SubscriptLeftMost(a);
	     !SubscriptLast(si);
	     SubscriptNext(si))
	    {
	    Expr *e = SubscriptCurr(si);
    
            int sub = subscripts[stmt][ref];
            if (ExprIsAffine(e))
		{
		for (TermIterator ti = TermLeftMost(e);
		     !TermLast(ti);
		     TermNext(ti))
		    {
		    Term *t = TermCurr(ti);
		    Variable *v = TermVar(t);
		    if (VarIsIndex(v))
                        {
                        int index = IndexLoopNr(v);
                        involves[stmt][ref][sub][index-1] = TermCoef(t);
                        }
                    }
		}
            else
                for (int t=0; t<stmt_info[stmt+1].nest; t++)
		    {
                    involves[stmt][ref][sub][t] = 1;
		    }

            subscripts[stmt][ref]++;
            }
        references[stmt]++;
        }
    } /* traverse_parse_tree */


search_cost locality_cost(int i, int pi, int si)
    {
#if 0
    fprintf(uniform_debug, "locality_cost(stmt %d, perm %d, space %d)\n", i,pi,si);
    fprintf(uniform_debug, "perm: ");
    for (int d=0; d<stmt_info[i+1].nest; d++)
        fprintf(uniform_debug, "%d ", good_order_i(d, pi,si));
    fprintf(uniform_debug, "\n");
#endif
    search_cost cache_misses = 0;
    for (int j = 0; j < references[i]; j++)
        {
        int pinned = -1;
        int partial_pin = -1;
        for (int k =0; k <= subscripts[i][j]-1; k++)
            for (int l=0; l<stmt_info[i+1].nest; l++)
                if (involves[i][j][k][l])
                    {
                    for (int m=0; m<stmt_info[i+1].nest; m++)
                        {
                        if (good_order_i(m, pi,si)-1 == l)
                            {
                            if (k < subscripts[i][j]-1)
                                partial_pin = max(partial_pin, m);
                            pinned = max(pinned, m);
                            break;
                            }
                        }
                    }
        if (pinned == partial_pin)
            cache_misses += convert_cost(pow(uniform_args.n, pinned+1));
        else
            cache_misses += convert_cost(pow(uniform_args.n, pinned+1)/10);
        }
    return cache_misses;
    } /* locality_cost */



#define reduction_op(op)	((op) == op_add_assign || (op) == op_mul_assign || (op) == op_max_assign || (op) == op_min_assign)


/* Note: currently doesn't depend on pi */

search_cost false_sharing_cost(int i, int pi, int si)
    {
    int d;
#if 0
    fprintf(uniform_debug, "false_sharing(stmt %d, perm %d, space %d)\n", i,pi,si);
    fprintf(uniform_debug, "perm: ");
    for (d=0; d<stmt_info[i+1].nest; d++)
        fprintf(uniform_debug, "%d ", good_order_i(d, pi,si));
    fprintf(uniform_debug, "\n");
#endif

    int dl = -1;
    for (d=0; d<stmt_info[i+1].nest; d++)
        if (good_order_i(d, pi, si) == si)
            {
            dl = d;
            break;
            }

    int dist_loop_carries_deps = 0;
    for (dir_list *n=closure_dir[i][i]; n!=NULL; n=n->next)
        {
        dddirection ddi = ddextract(n, si);
        if (ddi == ddind) continue;

	int j;
        for (j=0; j<dl; j++)
            {
            if (is_plus(n,i,good_order_i(j,pi,si)))
                break;
            }

        if (j==dl)
            {
            dist_loop_carries_deps = 1;
            break;
            }
        }

    int lhs = 0;
    int k;
    for (k = 0; k < subscripts[i][lhs]; k++)
        if (involves[i][lhs][k][si-1])
            break;

    int false_sharing;
    if (si == 0)
        false_sharing = 0;
    else if (reduction_op(stmt_info[i+1].stmt->nodeop) && 
             !dist_loop_carries_deps &&
             k == subscripts[i][lhs])
        false_sharing = 0;
    else if (k < (Use_Cyclic ? (subscripts[i][lhs]-1) : (subscripts[i][lhs])))
        false_sharing = 0;
    else 
        false_sharing = 1;

    search_cost total = convert_cost(false_sharing * 10 *
                                     pow(uniform_args.n, stmt_info[i+1].nest));

#if 0
    fprintf(uniform_debug, "false_sharing = %d\n", false_sharing);
    fprintf(uniform_debug, "cost = %f\n", convert_double(total));
#endif

    return total;
    } /* false_sharing_cost */


static int visited[max_stmts];
static int next_dfs_stmt[max_stmts];
static int last_stmt;
static int component_nr;


static void dfs_forward(int parent)
    {
    int child;
    rel_edge *e;
    
    visited[parent] = 1;
    
    for (e = temp_graph[parent+1].forw_link;
         e != NULL;
         e = e->next_forw_link) 
        {
        child = e->child_index;
        if (!visited[child])
            dfs_forward(child);
        }
    
    next_dfs_stmt[parent] = last_stmt;
    last_stmt = parent;
    } /* dfs_forward */




static void dfs_backward(int level, int parent)
    {
    int child;
    rel_edge *e;

    visited[parent] = 1;
    
    T1[2*level+1][parent] = Relation::True(stmt_info[parent+1].nest, 1);
    EQ_Handle eq = T1[2*level+1][parent].and_with_EQ();
    eq.update_coef(output_var(1), -1);
    eq.update_const(component_nr);
    time_coef[2*level+1][parent] = component_nr;
    
    if (component_nr == 0)
        {
        group[level + 1][parent] = group[level][parent];
        }
    else
        {
        group[level + 1][parent] = nr_groups[level + 1]-1;
        }
    
    for (e = temp_graph[parent+1].back_link;
         e != NULL;
         e = e->next_back_link) 
        {
        child = e->parent_index;
        if (!visited[child])
            dfs_backward(level, child);
        }
    } /* dfs_backward */




static void determine_constant_level(int level, int group_nr)
    {
    int stmt;
    
    last_stmt = -1;
    
    for (stmt = 0; stmt < nr_statements; stmt++)
        visited[stmt] = 0;
    
    for (stmt = nr_statements-1; stmt >= 0; stmt--)
        if (group[level][stmt] == group_nr && !visited[stmt])
            dfs_forward(stmt);
    
    for (stmt = 0; stmt < nr_statements; stmt++)
        visited[stmt] = 0;
    
    component_nr = 0;
    
    for (stmt = last_stmt; stmt >= 0; stmt = next_dfs_stmt[stmt]) 
        {
        if (group[level][stmt] == group_nr && !visited[stmt]) 
            {
            dfs_backward(level, stmt);
            component_nr++;
            nr_groups[level + 1]++;
            }
        }
    
    nr_groups[level + 1]--;
    } /* determine_constant_level */

 
static void revise_depends(int level)
    {
    Relation r;
    for (int i = 0; i < nr_statements; i++) 
        {
        if (temp_graph[i+1].self_depend.is_upper_bound_satisfiable()) 
            {
            r = Composition(Inverse(copy(T1[level][i])),
                            copy(T1[level][i]));
            temp_graph[i+1].self_depend =
                Intersection(temp_graph[i+1].self_depend, r);
            }
        rel_edge *e1, *next_edge;
        for (e1 = temp_graph[i+1].forw_link; e1 != NULL; e1 = next_edge) 
            {
            int from = e1->parent_index;
            int to = e1->child_index;

            next_edge = e1->next_forw_link;

            r = Composition(Inverse(copy(T1[level][to])),
                            copy(T1[level][from]));
            e1->rel = Intersection(e1->rel, r);

            if (!(e1->rel.is_upper_bound_satisfiable())) 
                remove_edge(e1, temp_graph);
            }
        }
    } /* revise_depends */



static Relation greater_than;

static int test_alignment(int i, int j, Relation &depend, int level)
    {
    if (group[level][i] != group[level][j])
	{
        return 1;
	}

    Relation R1 = Join(Join(copy(T1[2*level+2][i]), copy(greater_than)),
                       Inverse(copy(T1[2*level+2][j])));

    return ! Intersection(copy(R1),copy(depend)).is_upper_bound_satisfiable();
    } /* test_alignment */



static int alignment_required(int level)
    {
    for (int i = 0; i<nr_statements; i++)
        {
        if (!temp_graph[i+1].self_depend.is_null()) 
            if (! test_alignment(i,i,temp_graph[i+1].self_depend,level))
                return 1;
        for (rel_edge *e=temp_graph[i+1].forw_link; 
             e!=NULL; 
             e=e->next_forw_link)
            if (!test_alignment(e->parent_index,e->child_index,e->rel,level))
                return 1;
        }
    return 0;
    } /* alignment_required */

extern int is_const_dist[max_stmts][maxLevels];

static void determine_variable_level(int level)
    {
    for (int i=0; i<nr_statements; i++)
        {
        T1[2*level+2][i] = Relation::True(stmt_info[i+1].nest, 1);
        EQ_Handle eq = T1[2*level+2][i].and_with_EQ();
        eq.update_coef(output_var(1), -1);

        if (dist_loop[i][best_time[i]] == level)
            {
            eq.update_const(best_coef[i][0]);
            for (int j = 1; j<=uniform_global_list.size(); j++)
                eq.update_coef(T1[2*level+2][i].get_local(uniform_global_list[j]),
                               best_coef[i][j]);
            }

        if (level < stmt_info[i+1].nest)
            {
            int loop = good_stored_order_i(level, best_time[i]);
            eq.update_coef(input_var(loop), 1);
            }
        }

    if (alignment_required(level))
	UniformError("alignment required - not implemented yet\n");
    } /* determine_variable_level */



void analyze_locality()
    {
    for (int i=0; i<nr_statements; i++)
        {
        references[i] = 0;
        traverse_parse_tree(i, stmt_info[i+1].stmt);
        }
    } /* analyze_locality */



static void align()
    {
    int i;
    copy_depends_graph(all_depends, temp_graph);

    nr_groups[0] = 1;
    for (i=0; i<nr_statements; i++)
        group[0][i] = 0;

    {
    int i;
    for (i=0; i<global_max_nest; i++)
        {
        nr_groups[i+1] = nr_groups[i];

        for (int j=0; j<nr_groups[i]; j++)
            determine_constant_level(i, j);

        revise_depends(i*2+1);

        determine_variable_level(i);

        revise_depends(i*2+2);
        }

    nr_groups[i+1] = nr_groups[i];
    for (int j=0; j<nr_groups[i]; j++)
        determine_constant_level(i, j);

    revise_depends(i*2+1);
    }

    for (i = 0; i < nr_statements; i++) 
        {
        assert(! temp_graph[i+1].self_depend.is_upper_bound_satisfiable());
        assert(temp_graph[i+1].forw_link == NULL);
        assert(temp_graph[i+1].back_link == NULL);
        }

    combine_schedules(2*global_max_nest+1);
    } /* align */


void select_time_mappings()
    {
    int i;
    assert( 0 < Infinity && Infinity < MAX_COST);

    greater_than = Relation::True(1,1);
    GEQ_Handle geq = greater_than.and_with_GEQ();
    geq.update_coef(input_var(1), 1);
    geq.update_coef(output_var(1), -1);
    geq.update_const(-1);

#if defined (SPEED)
    edge_cost = new ma[nr_statements];
#else
    edge_cost.resize(nr_statements,nr_statements, 
                     max_candidates,max_candidates);
    candidates.resize(nr_statements);
    valid_candidate.resize(nr_statements, max_candidates);
#endif

    for (i=0; i<nr_statements; i++)
        {
        for (int pi=1; pi<=max(1, listi.size()); pi++)
            {
            dist_loop[i][pi] = -1;
            for (int j=0; j<stmt_info[i+1].nest; j++)
                {
                if (good_stored_order_i(j, pi) == best_space[i])
                    {
                    dist_loop[i][pi] = j;
                    break;
                    }
                }
            }

        candidates[i] = max(1, listi.size());
        if (candidates[i] >= max_candidates)
	    UniformError("too many candidate loop permutations to consider");
        valid_candidate[i][0] = 0;
        for (int j=1; j<=candidates[i]; j++)
            valid_candidate[i][j] = 1;
        }

    analyze_locality();

    for (i=0; i<nr_statements; i++)
        for (int pi=1; pi<=candidates[i]; pi++)
            {
            search_cost lcost = locality_cost(i, pi, best_space[i]);
            search_cost fcost = false_sharing_cost(i, pi, best_space[i]);

            edge_cost[i][i][pi][pi] = lcost + fcost;

#if 0
            fprintf(uniform_debug,
                    "stmt %d pi %d: lcost %9.0f fcost %9.0f\n",
                    i, pi, convert_double(lcost), convert_double(fcost));
#endif
            }

    for (i=0; i<nr_statements; i++)
        for (int j=0; j<nr_statements; j++)
	    delete_dir_list(closure_dir[i][j]);
    closure_dir.resize(0,0);

    for (i=0; i<nr_statements; i++)
        for (int j=i+1; j<nr_statements; j++)
            for (int pi=1; pi<=candidates[i]; pi++)
                for (int pj=1; pj<=candidates[j]; pj++)
                    edge_cost[i][j][pi][pj] = 
                        edge_cost[j][i][pj][pi] = 0;

    for (i=0; i<nr_statements; i++)
        for (int j=i+1; j<nr_statements; j++)
            for (int pi=1; pi<=listi.size(); pi++)
                for (int pj=1; pj<=listj.size(); pj++)
                    {
                    if (is_perm_compatible[i][j][listi[pi]][listj[pj]])
                        {
                        if (dist_is_parallel(i) && dist_is_parallel(j))
                            {
                            if (min(dist_loop[i][pi], dist_loop[j][pj]) < 
                                dist_level[i][j][listi[pi]][listj[pj]] &&
                                ((dist_loop[i][pi] != dist_loop[j][pj])   ||
                                 (stored_pipeline_cost[i][best_space[i]] != 
                                  stored_pipeline_cost[j][best_space[j]])))
                                {
                                edge_cost[i][j][pi][pj] = 
                                    edge_cost[j][i][pj][pi] = Bad;
                                }
                            }
                        else if ( dist_is_parallel(i) && !dist_is_parallel(j))
                            {
                            if (dist_loop[i][pi] < dist_level[i][j][listi[pi]][listj[pj]])
                                {
                                edge_cost[i][j][pi][pj] = 
                                    edge_cost[j][i][pj][pi] = Bad;
                                }
                            }
                        else if (!dist_is_parallel(i) && dist_is_parallel(j))
                            {
                            if (dist_loop[j][pj] < dist_level[i][j][listi[pi]][listj[pj]])
                                {
                                edge_cost[i][j][pi][pj] = 
                                    edge_cost[j][i][pj][pi] = Bad;
                                }
                            }
                        }
                    else
                        {
                        edge_cost[i][j][pi][pj] =
                            edge_cost[j][i][pj][pi] = Infinity;
                        }

#if 0
                    fprintf(uniform_debug, "edge_cost[%d][%d][%d][%d] = %f\n",
                            i, j, pi, pj, 
                            convert_double(edge_cost[i][j][pi][pj]));
#endif
                    }

#if defined (SPEED)
    delete is_perm_compatible;
    delete dist_level;
#else
    is_perm_compatible.resize(0,0,0,0);
    dist_level.resize(0,0,0,0);
#endif
           
#if 0
    fprintf(uniform_debug, "Select time mappings:\n");
#endif


    if (uniform_args.manual)
        {
        fprintf(stdout, "Select time mappings:\n");
        for (int i=0; i<nr_statements; i++)
            {
            fprintf(stdout, "stmt %d ", i);
            for (int j=1; j<=candidates[i]; j++)
                {
                fprintf(stdout, ",%d: ", j);
	        if (stmt_info[i+1].nest > 0)
                    print_perm(stdout, i, good_perms[i][best_space[i]][j]);
                }
            fprintf(stdout, "\n");
            }
	}

#if 0
    for (i=0; i<nr_statements; i++)
        {
        fprintf(uniform_debug, "stmt %d ", i);
        for (int j=1; j<=candidates[i]; j++)
            {
            fprintf(uniform_debug, ",%d: ", j);
	    if (stmt_info[i+1].nest > 0)
                print_perm(uniform_debug, i, good_perms[i][best_space[i]][j]);
            }
        fprintf(uniform_debug, "\n");
        }
#endif

    perform_time_search(0);

    if (best_cost >= Infinity)
	UniformError("incompatible permutations selected\n");

    if (best_cost >= Bad)
	fprintf(uniform_debug, "Warning: incompatible parallelism\n");

    for (i=0; i<nr_statements; i++)
        best_time[i] = best[i];

#if defined (SPEED)
    delete edge_cost;
#else
    edge_cost.resize(0, 0, 0, 0);
    candidates.resize(0);
    valid_candidate.resize(0, 0);
#endif

    align();

    for (i=0; i<nr_statements; i++)
        {
        fprintf(uniform_debug2, "T%d: ", i);
        time0[i].print_with_subs(uniform_debug2);
        }

    for (i=0; i<nr_statements; i++)
        {
        for (int j=0; j<=2*stmt_info[i+1].nest+1; j++)
            {
            is_loop_dist[i][j] = 0;
            is_const_dist[i][j] = 0;
            }

        if (best_space[i] > 0)
            {
            int dl = dist_loop[i][best_time[i]];

            for (int j=dl; j<=stmt_info[i+1].nest; j++)
                {
		int k;
                for (k=0; k<nr_statements; k++)
                    if (group[j+1][k]==group[j+1][i])
                        if (dist_loop[k][best_time[k]]!=dl)
                            break;
                if (k < nr_statements)
                    continue;
            
                if (j > dl)
                    is_loop_dist[i][2*j+1] = (Use_Cyclic?2:1);
                else
                    is_loop_dist[i][2*j+2] = (Use_Cyclic?2:1);

                break;
                }
            }
        else
            {
            for (int j=0; j<=stmt_info[i+1].nest; j++)
                {
		int k;
                for (k=0; k<nr_statements; k++)
                    if (group[j+1][k]==group[j+1][i])
                        {
                        if (best_space[k] > 0)
                            break;
			int jj;
                        for (jj = 1; 
                             jj<=uniform_global_list.size(); 
                             jj++)
                            if (best_coef[i][jj] != best_coef[k][jj])
                                break;
                        if (jj<=uniform_global_list.size())
                            break;
                        }
                if (k < nr_statements)
                    continue;
            
                is_const_dist[i][2*j+1] = (Use_Cyclic?2:1);
                break;
                }
            }
        }
    } /* select_time_mappings */
}
