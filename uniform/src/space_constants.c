/* $Id: */

#include <stdio.h>
#include <basic/Dynamic_Array.h>
#include <uniform/space_constants.h>
#include <uniform/space_mappings.h>
#include <uniform/communication.h>

namespace omega {

static Dynamic_Array1<int> parent("Stmts");
static Dynamic_Array1<int> rank("Stmts");
static Dynamic_Array3<int> diffs("Stmts", "Stmts", "Globals");
static Dynamic_Array2<int> diff_exists("Stmts", "Stmts");
static Dynamic_Array1<int> done("Stmts");

Dynamic_Array2<int> best_coef("Stmts", "Globals");



static void make_set(int x)
    {
    parent[x] = x;
    rank[x] = 0;
    } /* make_set */


static void link(int x, int y)
    {
    if (rank[x] > rank[y])
        parent[y] = x;
    else
        parent[x] = y;
    if (rank[x] == rank[y])
        rank[y] = rank[y] + 1;
    } /* link */


static int find_set(int x)
    {
    if (x != parent[x])
        parent[x] = find_set(parent[x]);
    return parent[x];
    } /* find_set */



static int priority_order(extended_dir_list *n1, extended_dir_list *n2,
                          int zero1, int zero2)
    {
    return ((n1->dim > n2->dim) ||
            (n1->dim == n2->dim && zero1 >= zero2));
    return ((zero1 > zero2) ||
            (zero1 == zero2 && n1->dim >= n2->dim));

    } /* priority_order */



static void expand(int i)
    {
    done[i] = 1;
    for (int j = 0; j<nr_statements; j++)
        if (! done[j])
            {
            if (diff_exists[i][j])
                {
                for (int k = 0; k <= uniform_global_list.size(); k++)
                    best_coef[j][k] = best_coef[i][k] - diffs[i][j][k];

                expand(j);
                }

            if (diff_exists[j][i])
                {
                for (int k = 0; k <= uniform_global_list.size(); k++)
                    best_coef[j][k] = best_coef[i][k] + diffs[j][i][k];

                expand(j);
                }
            }
    } /* expand */



static int loop(int stmt, int s)
    {
    if (s > stmt_info[stmt+1].nest)
        return s - stmt_info[stmt+1].nest;
    else
        return s;
    } /* loop */



typedef struct
    {
    int p;
    int q;
    int zero;
    extended_dir_list *n;
    } const_edge;

#define bp(i)        loop(cel[i].p, best[cel[i].p])
#define bq(i)        loop(cel[i].q, best[cel[i].q])

#define max_depends        1024
#define max_globals        10


void select_constants()
    {
    int i, p;
    const_edge cel[max_depends];
    int list_length = 0;

    parent.resize(nr_statements);
    rank.resize(nr_statements);
    diffs.resize(nr_statements, nr_statements, max_globals);
    diff_exists.resize(nr_statements, nr_statements);
    best_coef.resize(nr_statements, max_globals);
    done.resize(nr_statements);

    for (p = 0; p < nr_statements; p++)
        for (int q = 0; q < nr_statements; q++)
            for (extended_dir_list *n=extended_original_dir[p][q]; 
                 n!=NULL; 
                 n=n->next)
                if (p != q && n->constant[loop(p,best[p])][loop(q,best[q])])
                    {
                    cel[list_length].p = p;
                    cel[list_length].q = q;
                    cel[list_length].zero = 
                        n->zero[loop(p,best[p])][loop(q,best[q])];
                    cel[list_length].n = n;
                    list_length++;
                    }
     
    for (i = 0; i < list_length; i++)
        for (int j = i+1; j < list_length; j++)
            if (! priority_order(cel[i].n,cel[j].n, cel[i].zero,cel[j].zero))
                {
                const_edge tmp = cel[i];
                cel[i] = cel[j];
                cel[j] = tmp;
                }

    for (i = 0; i < nr_statements; i++)
        {
        make_set(i);
        for (int j = 0; j < nr_statements; j++)
            diff_exists[i][j] = 0;
        }

    int nr_sets = nr_statements;
    for (i = 0; i < list_length && nr_sets > 1; i++)
        {
        int pi = find_set(cel[i].p);
        int qi = find_set(cel[i].q);

        int con = abs(cel[i].n->coefs[bp(i)][bq(i)][0]);
        if (pi != qi && (con < 10 || Use_Cyclic || cel[i].n->dim > 0))
            {
            if (!Use_Cyclic && con >= 10)
		{
		fprintf(stderr, "Warning: large constant offset used\n");
		fprintf(uniform_debug, "%d -> %d diff = %d, dim = %d\n",
                        pi, qi, 
			cel[i].n->coefs[bp(i)][bq(i)][0],
			cel[i].n->dim);
		}

            assert(! diff_exists[cel[i].p][cel[i].q]);
            assert(! diff_exists[cel[i].q][cel[i].p]);

            for (int j = 0; j <= uniform_global_list.size(); j++)
                diffs[cel[i].p][cel[i].q][j] = 
                    cel[i].n->coefs[bp(i)][bq(i)][j];

            diff_exists[cel[i].p][cel[i].q] = 1;
            link(pi, qi);
            nr_sets--;
            }
        }

    for (i = 0; i < nr_statements; i++)
        done[i] = 0;

    for (i = 0; i < nr_statements; i++)
        if (!done[i])
            {
            for (int j = 0; j<=uniform_global_list.size(); j++)
                best_coef[i][j] = 0;
            expand(i);
            }

    for (i = 0; i < nr_statements; i++)
        {
        space[i] = Relation::True(stmt_info[i+1].nest, 1);
        pos_space[i] = Relation::True(stmt_info[i+1].nest, 1);
        for (int k=1; k<=stmt_info[i+1].nest; k++)
            {
            space[i].name_input_var(k, var_name_list[i][k]);
            pos_space[i].name_input_var(k, var_name_list[i][k]);
            }

        EQ_Handle eq = space[i].and_with_EQ();
        EQ_Handle pos_eq = pos_space[i].and_with_EQ();
        eq.update_coef(output_var(1), -1);
        pos_eq.update_coef(output_var(1), -1);
        assert(best_space[i] == loop(i, best[i]));
        neg_space[i] = 0;
        if (best_space[i] > 0)
            {
            pos_eq.update_coef(input_var(loop(i,best_space[i])), 1);
            if (was_negative[i][best_space[i]])
                {
#if 0
		fprintf(uniform_debug, "was_nagative[%d][%d] == %d\n",
		        i, best_space[i], was_negative[i][best_space[i]]);
#endif
                eq.update_coef(input_var(loop(i,best_space[i])), -1);
                neg_space[i] = 1;
                }
            else
                {
                eq.update_coef(input_var(loop(i,best_space[i])), 1);
                }
            }
        eq.update_const(best_coef[i][0]);
        pos_eq.update_const(best_coef[i][0]);
        for (int j = 1; j<=uniform_global_list.size(); j++)
            {
            eq.update_coef(space[i].get_local(uniform_global_list[j]), 
                           best_coef[i][j]);
            pos_eq.update_coef(pos_space[i].get_local(uniform_global_list[j]), 
                               best_coef[i][j]);
            }
        space[i].simplify();
        pos_space[i].simplify();
        }

    int saved[10], all[10];

    for (i = 0; i < 10; i++)
        {
        saved[i] = 0;
        all[i] = 0;
        }

    list_length = 0;
    for (p = 0; p < nr_statements; p++)
        for (int q = 0; q < nr_statements; q++)
            for (extended_dir_list *n=extended_original_dir[p][q]; 
                 n!=NULL; 
                 n=n->next)
                {
                cel[list_length].p = p;
                cel[list_length].q = q;
                cel[list_length].n = n;
                all[n->dim]++;
                if (p == q)
                    {
                    if (!n->zero[loop(p,best[p])][loop(q,best[q])])
                        {
                        if (n->constant[loop(p,best[p])][loop(q,best[q])])
                           {
                           cel[list_length].zero = 1;
                           list_length++;
                           }
                        else
                           {
                           cel[list_length].zero = 0;
                           list_length++;
                           }
                        }
                    else
                        saved[n->dim]++;
                    }
                else
                    {
                    if (n->constant[loop(p,best[p])][loop(q,best[q])])
                        {
			int j;
                        for (j=0; j<=uniform_global_list.size(); j++)
                            {
                            if (best_coef[p][j] - best_coef[q][j] !=
                                n->coefs[loop(p,best[p])][loop(q,best[q])][j])
                                break;
                            }

                        if (j<=uniform_global_list.size())
                           {
                           cel[list_length].zero = 1;
                           list_length++;
                           }
                        else
                            saved[n->dim]++;
                        }
                    else
                        {
                        cel[list_length].zero = 0;
                        list_length++;
                        }
                    }
                }

    for (i = 0; i < list_length; i++)
        for (int j = i+1; j < list_length; j++)
            if (! priority_order(cel[i].n,cel[j].n, cel[i].zero,cel[j].zero))
                {
                const_edge tmp = cel[i];
                cel[i] = cel[j];
                cel[j] = tmp;
                }

#if 0
    if (uniform_args.trace_uniform)
        {
        fprintf(uniform_debug, "\n");

        for (int i = 9; i > 0; i--)
            if (all[i] > 0)
                fprintf(uniform_debug, 
                        "%2d of %2d n^%d dependences made intra-processor\n",
                        saved[i], all[i], i);

        fprintf(uniform_debug, "\ninter-processor dependences:\n");
        for (int i = 0; i < list_length; i++)
            {
            if (!cel[i].zero)
                fprintf(uniform_debug, "    normal    ");
            else
                fprintf(uniform_debug, "    neighbour ");
    
            search_cost cost;
            if (cel[i].zero)
                cost = convert_cost(uniform_args.comps_per_comm * 
                                    pow(uniform_args.n, cel[i].n->dim - 1) *
                                    uniform_args.Num_Procs);
            else
                cost = convert_cost(uniform_args.comps_per_comm * 
                                    pow(uniform_args.n, cel[i].n->dim));
            
            fprintf(uniform_debug,"n^%d communication from %2d to %2d cost %f :",
                    cel[i].n->dim, cel[i].p, cel[i].q, convert_double(cost));
            print_extended_dir(cel[i].n);
            fprintf(uniform_debug, "\n");
            }
        }
#endif
 
    for (i = 0; i < nr_statements; i++)
        if (best[i] > 0 && !stmt_info[i+1].distributed[loop(i,best[i])])
            fprintf(uniform_debug, "warning: not distributed S%-2d loop %d\n", 
                    i, loop(i, best[i]));

    parent.resize(0);
    rank.resize(0);
    diffs.resize(0, 0, 0);
    diff_exists.resize(0, 0);
    done.resize(0);
    } /* select_constants */
}
