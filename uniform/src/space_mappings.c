/* $Id: space_mappings.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <math.h>
#include <uniform/space_mappings.h>
#include <uniform/space_constants.h>
#include <uniform/communication.h>
#include <uniform/uniform_args.h>
#include <uniform/parallelism.h>
#include <uniform/depend_dir.h>
#include <uniform/time_mappings.h>

namespace omega {


int Use_Cyclic;
int best_space[max_stmts];
Dynamic_Array2<extended_dir_list *> extended_original_dir("Stmts", "Stmts");
Dynamic_Array1<Relation> space("Stmts");
Dynamic_Array1<Relation> pos_space("Stmts");
Dynamic_Array1<int> neg_space("Stmts");


static int one_unbalanced = 0;



static int loop(int stmt, int s)
    {
    if (s > stmt_info[stmt+1].nest)
        return s - stmt_info[stmt+1].nest;
    else
        return s;
    } /* loop */



search_cost unbalanced_cost(int p, int cp)
    {
#if 0
    fprintf(uniform_debug, "unbalanced_cost (p %d, cp %d)\n", p, cp);
#endif
    if (Use_Cyclic || cp == 0 || !unbalanced(p, cp))
	{
	return 0;
	}
    else
	{
        one_unbalanced = 1;
	return convert_cost(pow(uniform_args.n, stmt_info[p+1].nest) / 2.0);
	}
    } /* unbalanced_cost */



static search_cost min_false_sharing_cost(int p, int cp)
    {
    search_cost min_cost = MAX_COST;
    for (int i=1; i<=good_perms[p][cp].size(); i++)
        {
        search_cost cost = false_sharing_cost(p, i, cp);
        assert(cost < MAX_COST);
        min_cost = min(min_cost, cost);
        }

    if (min_cost < MAX_COST)
        return min_cost;
    else
        return 0;
    } /* min_false_sharing_cost */



static int space_candidates(int stmt)
    {
    if (incompatible_stmt[stmt])
         return 2 * stmt_info[stmt+1].nest;
    else
         return stmt_info[stmt+1].nest;
    } /* space_candidates */



static bool can_distribute(int j, int cj)  
    {
    return (uniform_args.non_data_ok || 
            cj==0 || 
            stmt_info[j+1].distributed[loop(j,cj)]);
    } /* can_distribute */

static int space_valid_candidate(int stmt, int s)
    {
    if (! can_distribute(stmt, loop(stmt, s)))
        return 0;
    else
        return s <= stmt_info[stmt+1].nest || 
               !incompatible_node[stmt][loop(stmt, s)];
    } /* space_valid_candidate */


static int sequential(int i, int ci)
    {
    return ci == 0 || ci > stmt_info[i+1].nest;
    } /* sequential */



static void compute_edge_costs()
    {
    int i,j,ci,cj;

    for (i=0; i<nr_statements; i++) {
        for (ci=0; ci<=candidates[i]; ci++)
            {
            if (! valid_candidate[i][ci]) continue;

            search_cost ecost = compute_edge_cost(i, i, loop(i,ci), loop(i,ci));
            search_cost pcost = pipeline_cost2(i, loop(i,ci), sequential(i,ci));
            search_cost ucost = unbalanced_cost(i, loop(i, ci));

            search_cost fcost = min_false_sharing_cost(i, loop(i, ci));

            assert(ecost < MAX_COST);
            assert(pcost < MAX_COST);
            assert(ucost < MAX_COST);
            assert(fcost < MAX_COST);

            edge_cost[i][i][ci][ci] = ecost + pcost + ucost + fcost;

            if (uniform_args.trace_uniform)
                fprintf(uniform_debug, 
                    "stmt %d cand %d: "
		    "ecost %9.0f pcost %9.0f ucost %9.0f fcost %9.0f\n",
                    i, ci, 
                    convert_double(ecost),
                    convert_double(pcost),
                    convert_double(ucost),
		    convert_double(fcost));

            if (uniform_args.trace_uniform && edge_cost[i][i][ci][ci] > 0)
                fprintf(uniform_debug, "edge_cost[%d][%d][%d][%d] = %f\n", 
                        i, i, ci, ci, 
                        convert_double(edge_cost[i][i][ci][ci]));
            }
        }
        
    for (i=0; i<nr_statements; i++)
        for (j=i+1; j<nr_statements; j++)
            for (ci=0; ci<=candidates[i]; ci++)
                {
                if (!valid_candidate[i][ci]) continue;

                for (cj=0; cj<=candidates[j]; cj++)
                    {
                    if (!valid_candidate[j][cj]) continue;

                    edge_cost[j][i][cj][ci] =
                        edge_cost[i][j][ci][cj] =
                            compute_edge_cost(i, j, loop(i,ci), loop(j,cj)) +
                            compute_edge_cost(j, i, loop(j,cj), loop(i,ci));

                    if (!sequential(i, ci) && !sequential(j,cj) && 
                        !is_node_compatible[i][j][loop(i,ci)][loop(j,cj)])
                        edge_cost[j][i][cj][ci] =
                            edge_cost[i][j][ci][cj] = Infinity;

                    assert(ci > 0  || cj > 0 || edge_cost[i][j][ci][cj] == 0);

                    if (uniform_args.trace_uniform && edge_cost[i][j][ci][cj] > 0)
                        fprintf(uniform_debug, "edge_cost[%d][%d][%d][%d] = %f\n", 
                        i,j,ci,cj,convert_double(edge_cost[i][j][ci][cj]));

                    }
                }
#if defined(SPEED)
    delete is_node_compatible;
#else
    is_node_compatible.resize(0,0,0,0);
#endif
    } /* compute_edge_costs */



static void optimize_communication()
    {
    analyze_locality();

    compute_edge_costs();

    perform_space_search(0);
    }



void minimize_communication()
    {
    Dynamic_Array1<int> block_best(nr_statements);
    search_cost block_best_cost;

    determine_parallelism();

#if 0
    fprintf(uniform_debug, "Consider Block distribution:\n");
#endif

    Use_Cyclic = 0;
    analyze_parallelism();

#if defined (SPEED)
    edge_cost = new ma[nr_statements];
#else
    edge_cost.resize(nr_statements,nr_statements,
                     2*global_max_nest+1,2*global_max_nest+1);
    candidates.resize(nr_statements);
    valid_candidate.resize(nr_statements, 2*global_max_nest+1);
#endif

    {
    for (int i=0; i<nr_statements; i++)
        {
        candidates[i] = space_candidates(i);
	if(candidates[i] >= max_candidates)
            UniformError("too many candidate space mappings to consider");

        for (int j=0; j<=candidates[i]; j++)
            valid_candidate[i][j] = space_valid_candidate(i,j);
        }
    }

    assert(nr_statements < max_stmts);

    space.resize(nr_statements);
    pos_space.resize(nr_statements);
    neg_space.resize(nr_statements);

    initialize_extended_dirs(extended_original_dir);

    compute_reduction_volumes();
    add_extended_dirs(extended_original_dir);

    if (uniform_args.manual)
	{
	fprintf(stdout, "Select Space Mappings:\n");
	fprintf(stdout, "(blocked distribution):\n");
        for (int i=0; i<nr_statements; i++)
            {
	    fprintf(stdout, "stmt %d :", i);
            for (int j=0; j<=candidates[i]; j++)
		if (valid_candidate[i][j])
		    printf("%d ", j);
	    fprintf(stdout, "\n");
	    }
	}

    optimize_communication();

    if (one_unbalanced)
        {
        block_best_cost = best_cost;
	{   // extra braces apparently avoid Visual C++ bug
        for (int i=0; i<nr_statements; i++)
            block_best[i] = best[i];
	}

        Use_Cyclic = 1;
#if 0
        fprintf(uniform_debug, "Consider Cyclic distribution:\n");
#endif

        analyze_parallelism();

        if (uniform_args.manual)
	    {
	    fprintf(stdout, "Select Space Mappings\n");
	    fprintf(stdout, "(cyclic distribution):\n");
            for (int i=0; i<nr_statements; i++)
                {
	        fprintf(stdout, "stmt %d :", i);
                for (int j=0; j<=candidates[i]; j++)
		    if (valid_candidate[i][j])
		        printf("%d ", j);
	        fprintf(stdout, "\n");
	        }
	    }

        optimize_communication();

        if (block_best_cost > best_cost)
            {
            fprintf(uniform_debug2, "Unbalanced, use Cyclic Distribution\n");
#if 0
            fprintf(uniform_debug, "Unbalanced, use Cyclic Distribution\n");
#endif
            Use_Cyclic = 1;
            }
        else
            {
            fprintf(uniform_debug2, "Unbalanced, but use Block Distribution\n");
#if 0
            fprintf(uniform_debug, "Unbalanced, but use Block Distribution\n");
#endif
            Use_Cyclic = 0;
            analyze_parallelism();
            best_cost = block_best_cost;
            for (int i=0; i<nr_statements; i++)
                best[i] = block_best[i];
            }
        }
    else
        {
        fprintf(uniform_debug2, "Balanced, use Block Distribution\n");
#if 0
        fprintf(uniform_debug, "Balanced, use Block Distribution\n");
#endif
        Use_Cyclic = 0;
        }

    {
    for (int i=0; i<nr_statements; i++)
        {
        best_space[i] = best[i];
        }
    }

    select_constants();

    {
    for (int i=0; i<nr_statements; i++)
        {
        fprintf(uniform_debug2, "S%d: ", i);
        space[i].print_with_subs(uniform_debug2);
#if 0
        fprintf(uniform_debug, "S%d: ", i);
        space[i].print_with_subs(uniform_debug);
#endif
        }
    }

    for (int i=0; i<nr_statements; i++)
        for (int j=0; j<nr_statements; j++)
            {
            extended_dir_list *nnext;
            for (extended_dir_list *e=extended_original_dir[i][j]; 
                 e!=NULL; 
                 e=nnext)
                {
                nnext = e->next;
                delete e;
                }
            }

    extended_original_dir.resize(0, 0);

#if defined (SPEED)
    delete edge_cost;
#else
    edge_cost.resize(0,0,0,0);
    candidates.resize(0);
    valid_candidate.resize(0, 0);
#endif
    } /* minimize_communication */

}
