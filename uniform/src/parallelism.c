/* $Id: parallelism.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <math.h>
#include <basic/Dynamic_Array.h>
#include <omega/Relations.h>
#include <omega/closure.h>
#include <uniform/uniform_args.h>
#include <uniform/parallelism.h>
#include <uniform/depend_dir.h>
#include <uniform/depend_rel.h>
#include <uniform/uniform_misc.h>
#include <uniform/space_mappings.h>

namespace omega {


Dynamic_Array2<search_cost> stored_pipeline_cost("Stmt", "Nest");
Dynamic_Array1<int> incompatible_stmt("Stmts");  
Dynamic_Array2<int> incompatible_node("Stmts", "Nest");
Dynamic_Array2<IntTuple> good_perms("Stmts", "Nest");
Dynamic_Array2<IntTuple> stored_perm_dir("Stmts", "Nest");
Dynamic_Array2<IntTuple> stored_perm_order("Stmts", "Nest");
Dynamic_Array2<dir_list *> closure_dir("Stmts", "Stmts");

#if defined (SPEED)

na *is_node_compatible;
nb *is_perm_compatible;
nb *dist_level;

#else

Dynamic_Array4<int> is_node_compatible("Stmts", "Stmts", "Nest", "Nest");
Dynamic_Array4<int> is_perm_compatible("Stmts","Stmts","Perms","Perms");
Dynamic_Array4<int> dist_level("Stmts","Stmts","Perms","Perms");

#endif

static Dynamic_Array1<int> order("Nest");
static Dynamic_Array1<int> dir("Nest");
static Dynamic_Array2<Relation> trans("Stmts", "Stmts");

static depend_graph temp_graph1;

static int is_fwd(dir_list * n, int stmt, int j)
    {
    return dir[j] > 0 ? is_plus(n,stmt,order[j] )
                : is_minus(n,stmt,order[j]) ;
    }
        
static int contains_bck(dir_list * n, int stmt, int j)
    {
    return dir[j] < 0 ? contains_plus(n,stmt,order[j] )
            : contains_minus(n,stmt,order[j]) ;
    }
        
static int legal_permutation(int stmt, int level, dir_list *test)
    {
#if 0
    fprintf(uniform_debug, "test legality of permutation for stmt %d:\n",stmt);
    for (int i = 0; i<level; i++)
        fprintf(uniform_debug, "%d ", dir[i] * order[i]);
    fprintf(uniform_debug, "\n");
#endif

    for (dir_list *n=test; n!=NULL; n=n->next)
        {
        int j;

        for (j=0; j<level; j++)
            {
            if (contains_bck(n, stmt, j)) {
                return 0;
                }
            if (is_fwd(n, stmt, j)) break;
            }
        }
    return 1;
    } /* legal_permutation */



static int min_dist_level(int stmt, dir_list *test)
    {
    int max_level = 0;

    for (dir_list *n=test; n!=NULL; n=n->next)
        {
	int j;
        for (j=0; j<stmt_info[stmt+1].nest && !is_fwd(n,stmt,j); j++);

        max_level = max(max_level, j+1);
        }

    return max_level;
    } /* min_dist_level */



void print_perm(FILE *f, int stmt, int perm)
    {
    fprintf(f, "(");
    for (int i = 0; i<stmt_info[stmt+1].nest; i++)
        {
        if (i > 0)
            fprintf(f, ",");
        if (stored_perm_dir[stmt][i][perm] == -1)
            fprintf(f, "-");
        fprintf(f, "%s", 
                (const char *)
                var_name_list[stmt][stored_perm_order[stmt][i][perm]]);
        }
    fprintf(f, ") ");
    } /* print_perm */




search_cost pipeline_cost(int p, int /*cp*/, int x, int y, int dist_level,
			  int sequential)
    {
    search_cost cost; 
    search_cost barriers;
    search_cost D;

    search_cost L = uniform_args.comps_per_comm;
    search_cost B = (Use_Cyclic ? 1.0:(uniform_args.n / uniform_args.Num_Procs));
    int P = uniform_args.Num_Procs;
    int m = stmt_info[p+1].nest;

    if (y == dist_level)
	D = P * (L + B * convert_cost(pow(uniform_args.n, m-y)));
    else if (y > x)
	D = P * L;
    else
	D = L;

    int interrupted = (D + L > B * pow(uniform_args.n, m-x+1));
 
    if (x > 0)
	{
	if (interrupted)
            barriers = convert_cost(pow(uniform_args.n, x+1) / (B * P));
	else
            barriers = convert_cost(pow(uniform_args.n, x));
	}
    else
	{
	if (y > 0)
            barriers = 1;
	else
            barriers = 0;
	}

    cost = barriers * D;

#if 0
    if (uniform_args.trace_uniform)
        fprintf(uniform_debug, "Stmt %d Loop %d: "
		"barriers=%f, barrier cost=%f, total_cost=%f\n",
                p, cp, convert_double(barriers), 
		convert_double(D), convert_double(cost));
#endif

    int num_syncs;
    if (sequential)
        num_syncs = stmt_info[p+1].nest;
    else
        num_syncs = y;

    search_cost sync_cost;
    if (num_syncs > 0)
        sync_cost = convert_cost(pow(uniform_args.n, num_syncs)/10);
    else
        sync_cost = 0;

    cost += sync_cost;
    
#if 0
    if (uniform_args.trace_uniform)
        fprintf(uniform_debug,
                "Stmt %d requires n^%d  = %f syncs\n",
                p, y, convert_double(sync_cost));
#endif

    return cost;
    } /* pipeline_cost */



search_cost pipeline_cost2(int p, int cp, int sequential)
    {
    if (cp == 0)
	return convert_cost(pow(uniform_args.n, stmt_info[p+1].nest));
    else if (sequential)
        return pipeline_cost(p,cp,stmt_info[p+1].nest,
				  stmt_info[p+1].nest,
				  stmt_info[p+1].nest, 1);
    else
        return stored_pipeline_cost[p][cp];
    } /* pipeline_cost2 */


static void analyse_permutation(int stmt)
    {
    int i;

    if (stored_perm_dir[stmt][0].size() >= max_perms)
        UniformError("too many loop permutations to consider");

    for (i = 0; i<stmt_info[stmt+1].nest; i++)
        {
        stored_perm_dir[stmt][i].append(dir[i]);
        stored_perm_order[stmt][i].append(order[i]);
        }

    good_perms[stmt][0].append(stored_perm_dir[stmt][0].size());

#if 0
    fprintf(uniform_debug, "analyse permutation:\n");
    print_perm(uniform_debug, stmt, stored_perm_dir[stmt][0].size());
    fprintf(uniform_debug, "\n");
#endif

    for (i = 0; i<stmt_info[stmt+1].nest; i++)
        {
#if 0
        fprintf(uniform_debug, "consider loop %d\n", order[i]);
#endif
        int barriers =  0;
        int syncs =  0;
        for (dir_list *n=closure_dir[stmt][stmt]; n!=NULL; n=n->next)
            {
#if 0
            print_dir(n);
#endif
            ddudirection ddi = ddextract(n, order[i]);
            if (ddi == dduind) continue;

	    int j;
            for (j=0; j<i; j++)
                if (is_fwd(n,stmt,j)) break;

#if 0
	    fprintf(uniform_debug, "%d", j+1);
#endif

            syncs = max(syncs, j+1);
            if (contains_bck(n,stmt,i) || Use_Cyclic) 
		{
                barriers = max(barriers,j+1);
#if 0
	        fprintf(uniform_debug, "[%d]", barriers);
#endif
		}
            }
#if 0
        fprintf(uniform_debug, "barriers = %d, syncs = %d\n", barriers, syncs);
#endif
        search_cost cost = pipeline_cost(stmt,order[i],barriers,syncs,i+1,0);

#if 0
        print_perm(uniform_debug, stmt, stored_perm_dir[stmt][0].size());
	fprintf(uniform_debug, "stmt %d cand %d: "
		"barriers %2d syncs %2d dist %2d cost %f\n\n",
		stmt, order[i], barriers, syncs, i+1, convert_double(cost));
#endif

        if (stored_pipeline_cost[stmt][order[i]] > cost)
            {
            good_perms[stmt][order[i]].clear();
            good_perms[stmt][order[i]].append(stored_perm_dir[stmt][0].size());
            stored_pipeline_cost[stmt][order[i]] = cost;
            }
        else if (stored_pipeline_cost[stmt][order[i]] == cost)
            {
            good_perms[stmt][order[i]].append(stored_perm_dir[stmt][0].size());
            }
        }
    } /* analyse_permutation */


static void permute_loops(int stmt, int level)
    {
    for (int i = 1; i<=stmt_info[stmt+1].nest; i++)
        {
        /* for (int d=-1; d<=1; d+=2) */
        int d = 1;
            {
	    int j;
            for (j = 0; j<level; j++)
                if (order[j] == i)
                    break;
            if (j < level)
                continue;

            order[level] = i;
            dir[level] = d;
            if (legal_permutation(stmt, level, closure_dir[stmt][stmt]))
                {
                if (level+1 < stmt_info[stmt+1].nest)
                    permute_loops(stmt, level+1);
                else
                    analyse_permutation(stmt);
                }
            }
        }
    } /* permute_loops */



static void analyze_stmt(int stmt)
    {
    for (int k = 0; k<2*global_max_nest+1; k++)
        {
        stored_pipeline_cost[stmt][k] = MAX_COST;
        }

    order.resize(stmt_info[stmt+1].nest);
    dir.resize(stmt_info[stmt+1].nest);

#if 0
    fprintf(uniform_debug,"Original closed dependences for stmt %d:\n",stmt);
    print_dirs(closure_dir[stmt][stmt]);
#endif

    permute_loops(stmt, 0);

    order.resize(0);
    dir.resize(0);
    } /* analyze_stmt */


#if 0
static void print_stmt_summary(int stmt)
    {
    fprintf(uniform_debug, "For statement %d\n", stmt);
    print_dirs(closure_dir[stmt][stmt]); fprintf(uniform_debug, "\n");

    for (int k = 1; k<=stmt_info[stmt+1].nest; k++)
        {
        fprintf(uniform_debug,"loop %d: parallelism cost %f\n",
                k, stored_pipeline_cost[stmt][k]);
        fprintf(uniform_debug, "   ");
        for (int j=1; j<=good_perms[stmt][k].size(); j++)
            print_perm(uniform_debug, stmt, good_perms[stmt][k][j]);
        fprintf(uniform_debug, "\n");

        }
    fprintf(uniform_debug,"\n");
    } /* print_stmt_summary */
#endif


static void closure_rel(int i, int k, int j)
    {
    Relation tik;

    if (trans[k][k].is_upper_bound_satisfiable())
        {
#if 0
        fprintf(uniform_debug, "i %d, k %d, j %d\n", i, k, j);
        trans[k][k].print_with_subs(uniform_debug);
        stmt_info[k+1].bounds.print_with_subs(uniform_debug);
#endif

        Relation tkk = TransitiveClosure(copy(trans[k][k]), 1,
                                         copy(stmt_info[k+1].bounds));
        tkk.simplify(2,4);

#if 0
        fprintf(uniform_debug, "tkk\n");
	tkk.print_with_subs(uniform_debug);
#endif
        tik = Composition(tkk, copy(trans[i][k]));
        tik.simplify(2,4);
        tik = Union(copy(trans[i][k]), tik);
        tik.simplify(2,4);
        }
    else
        {
        tik = trans[i][k];
        }
    Relation fresh;
    fresh = Composition(copy(trans[k][j]), tik);
    fresh.simplify(2,4);
    trans[i][j] = Union(trans[i][j], fresh);
    trans[i][j].simplify(2,4);

#if 0
    fprintf(uniform_debug, "%d -> %d -> %d\n", i, k, j);
    trans[i][j].print_with_subs(uniform_debug);
#endif
    } /* closure_rel */



static void close_rels(Dynamic_Array2<dir_list *> &dir_array)
    {
    int i;
    for (i=0; i<nr_statements; i++)
        for (int j=0; j<nr_statements; j++)
            trans[i][j] = Relation::False(stmt_info[i+1].nest,
                                          stmt_info[j+1].nest);

    for (i=0; i<nr_statements; i++)
        {
        trans[i][i] = all_depends[i+1].self_depend;
        for (rel_edge *e1=all_depends[i+1].forw_link;
             e1!=NULL;
             e1=e1->next_forw_link)
            trans[e1->parent_index][e1->child_index] = e1->rel;
        }

    for (int k=0; k<nr_statements; k++)
        for (int i=0; i<nr_statements; i++)
            if (trans[i][k].is_upper_bound_satisfiable() && i != k)
                for (int j=0; j<nr_statements; j++)
                    if (trans[k][j].is_upper_bound_satisfiable() && k != j)
                        if (i > k || j > k || i == j)
                            closure_rel(i, k, j);

    for (i=0; i<nr_statements; i++)
        for (int j=0; j<nr_statements; j++)
            {
            int bnest = AssignCommonNest(stmt_info[i+1].stmt, 
					 stmt_info[j+1].stmt);
            dir_array[i][j] = add_direction(i,bnest,copy(trans[i][j]));
            dir_list *last = NULL;
            merge_dirs(dir_array[i][j], &last);
            }
    } /* close_rels */



static dir_list *execution_order_dir(int i, int j, int pj)
    {
    dir_list *start = NULL;
    for (int n1=0; n1<stmt_info[j+1].nest; n1++)
        {
        int n2;
        dir_list *n = new dir_list;
        n->nest = stmt_info[j+1].nest;

        ddudirection direction = 0;

        for (n2=0; n2<n1; n2++)
            ddudirsetonly(direction, dduind, stored_perm_order[j][n2][pj]);

        if (stored_perm_dir[j][n1][pj] == 1)
            ddudirsetonly(direction, ddufwd, stored_perm_order[j][n1][pj]);
        else
            ddudirsetonly(direction, ddubck, stored_perm_order[j][n1][pj]);

        for (n2=n1+1; n2<stmt_info[j+1].nest; n2++)
            ddudirsetonly(direction, ddufwd+dduind+ddubck, 
                         stored_perm_order[j][n2][pj]);

        n->dir = direction & ddudirBits;
        n->next = start;
        start = n;
        }

    dir_list *last = NULL;
    dir_list *x = compose_dir(closure_dir[i][j], start, &last);
    delete_dir_list(start);
    dir_list *y = compose_dir(x, closure_dir[j][i], &last);
    delete_dir_list(x);

    merge_dirs(y, &last);

    return y;
    } /* execution_order_dir */



static dir_list *execution_order_rel(int i, int j, int pj)
    {
    Relation tjj = Relation::False(stmt_info[j+1].nest,stmt_info[j+1].nest);

    for (int n1=0; n1<stmt_info[j+1].nest; n1++)
        {
        Relation R = Relation::True(stmt_info[j+1].nest,stmt_info[j+1].nest);

        for (int n2=0; n2<n1; n2++)
            {
            EQ_Handle eq1 = R.and_with_EQ();
            eq1.update_coef( input_var(stored_perm_order[j][n2][pj]),  1);
            eq1.update_coef(output_var(stored_perm_order[j][n2][pj]), -1);
            }

        if (stored_perm_dir[j][n1][pj] == 1)
            {
            GEQ_Handle geq1 = R.and_with_GEQ();
            geq1.update_coef( input_var(stored_perm_order[j][n1][pj]), -1);
            geq1.update_coef(output_var(stored_perm_order[j][n1][pj]),  1);
            geq1.update_const(-1);
            }
        else
            {
            GEQ_Handle geq1 = R.and_with_GEQ();
            geq1.update_coef( input_var(stored_perm_order[j][n1][pj]),  1);
            geq1.update_coef(output_var(stored_perm_order[j][n1][pj]), -1);
            geq1.update_const(-1);
            }
        tjj = Union(tjj, R);
        }

#if 0
    printf("i %d, j %d, pj %d\n", i, j, pj);
    tjj.print_with_subs(stdout);
    stmt_info[j+1].bounds.print_with_subs(stdout);
#endif

    Relation tjjj = TransitiveClosure(tjj, 1, copy(stmt_info[j+1].bounds));
    tjjj.simplify(2,4);

    Relation tijjj = Composition(tjjj, copy(trans[i][j]));
    tijjj.simplify(2,4);

    Relation tijjji = Composition(copy(trans[j][i]), tijjj);
    tijjji.simplify(2,4);

    dir_list *y = add_direction(i, stmt_info[i+1].nest, tijjji);
    dir_list *last = NULL;
    merge_dirs(y, &last);

    return y;
    } /* execution_order_rel */



static void perm_compatible(int i, int j, int pj)
    {
    dir_list *y;

    assert(i < j);

    if (uniform_args.gen_do_closure)
        y = execution_order_rel(i, j, pj);
    else
        y = execution_order_dir(i, j, pj);

    order.resize(stmt_info[i+1].nest);
    dir.resize(stmt_info[i+1].nest);

    for (int pi = 1; pi <= stored_perm_dir[i][0].size(); pi++)
        {
        for (int n1=0; n1<stmt_info[i+1].nest; n1++)
            {
            dir[n1] = stored_perm_dir[i][n1][pi];
            order[n1] = stored_perm_order[i][n1][pi];
            }
    
        assert(pi < max_perms && pj < max_perms);

        is_perm_compatible[i][j][pi][pj] =
            legal_permutation(i, stmt_info[i+1].nest, y);

        if (is_perm_compatible[i][j][pi][pj])
	    {
            dist_level[i][j][pi][pj] = min_dist_level(i, y);
	    }

#if 0
        print_dirs(y); 
        fprintf(uniform_debug, "%d %d", i, pi);
        print_perm(uniform_debug, i, pi);
        fprintf(uniform_debug, "%d %d", j, pj);
        print_perm(uniform_debug, j, pj);

        if (is_perm_compatible[i][j][pi][pj])
            fprintf(uniform_debug, "is compatible\n");
        else
            fprintf(uniform_debug, "\n");
#endif
        }

    delete_dir_list(y);

    order.resize(0);
    dir.resize(0);
    } /* perm_compatible */



static void node_compatible(int i, int j, int ci, int cj)
    {
#if 0
    fprintf(uniform_debug, "node_compatible(%d,%d,%d,%d)\n",i,j,ci,cj);
#endif

    assert(i < j);

    if (ci == 0 || cj== 0)  {
                is_node_compatible[i][j][ci][cj] = true;
                return;
                }

    for (int li=1; li<=good_perms[i][ci].size(); li++)
        for (int lj=1; lj<=good_perms[j][cj].size(); lj++)
            if (is_perm_compatible[i][j]
                    [good_perms[i][ci][li]] [good_perms[j][cj][lj]])
                {
                is_node_compatible[i][j][ci][cj] = true;
#if 0
                fprintf(uniform_debug, "compatible i=%d, j=%d, ci=%d, cj=%d, %d,%d\n",
                        i, j, ci, cj, li, lj);
                print_perm(uniform_debug, i, li);
                print_perm(uniform_debug, j, lj);
#endif
                return;
                }

#if 0
    fprintf(uniform_debug, "incompatible i=%d, j=%d, ci=%d, cj=%d\n",
            i, j, ci, cj);
#endif

    is_node_compatible[i][j][ci][cj] = false;

    incompatible_node[i][ci] = true;
    incompatible_stmt[i] = true;
    incompatible_node[j][cj] = true;
    incompatible_stmt[j] = true;

    } /* node_compatible */

void analyze_parallelism()
    {
    int i;
    stored_pipeline_cost.resize(nr_statements, 2*global_max_nest+1);

    for (i=0; i<nr_statements; i++)
        {
#if 0
        fprintf(uniform_debug, "STATEMENT %d\n", i);
#endif
        for (int j=0; j<2*global_max_nest+1; j++)
            {
            stored_perm_dir[i][j] = Tuple<int>(0);
            stored_perm_order[i][j] = Tuple<int>(0);
            good_perms[i][j] = Tuple<int>(0);
            }

        analyze_stmt(i);

#if 0
        fprintf(uniform_debug, "all permutations:\n");
        for (j=1; j<=stored_perm_dir[i][0].size(); j++)
            print_perm(uniform_debug, i, j);
        fprintf(uniform_debug, "\n");

        int x;
        for (x=1; x<=stmt_info[i+1].nest; x++)
                {
                fprintf(uniform_debug, "perms for loop %d:\n", x);
                for (int j=1; j<=good_perms[i][x].size(); j++)
                    print_perm(uniform_debug, i, good_perms[i][x][j]);
                fprintf(uniform_debug, "\n");
                }
#endif

#if 0
        print_stmt_summary(i);
#endif
        }

#if !defined(SPEED)
    is_node_compatible.resize(nr_statements,nr_statements,
			      2 * global_max_nest+1,2 * global_max_nest+1);
#else
    is_node_compatible = new na[nr_statements];
#endif

    incompatible_stmt.resize(nr_statements);
    incompatible_node.resize(nr_statements, 2 * global_max_nest+1);

    for (i=0; i<nr_statements; i++)
        {
        incompatible_stmt[i] = 0;
        for (int y=0; y<=stmt_info[i+1].nest; y++)
            incompatible_node[i][y] = 0;
        }

    for (i=0; i<nr_statements; i++)
        for (int j=i+1; j<nr_statements; j++)
            {
            for (int pi=0; pi<=max(1,stored_perm_dir[i][0].size()); pi++)
                for (int pj=0; pj<=max(1,stored_perm_dir[j][0].size());pj++)
                    {
                    is_perm_compatible[i][j][pi][pj] = 1;
                    dist_level[i][j][pi][pj] = 0;
                    }

            if (closure_dir[i][j]!=NULL && closure_dir[j][i]!=NULL)
                {
                for (int pj = 1; pj <= stored_perm_dir[j][0].size(); pj++)
                    perm_compatible(i, j, pj);

                for (int ci=1; ci<=stmt_info[i+1].nest; ci++)
                    for (int cj=1; cj<=stmt_info[j+1].nest; cj++)
                        node_compatible(i, j, ci, cj);
                }
            else
                for (int ci=1; ci<=stmt_info[i+1].nest; ci++)
                    for (int cj=1; cj<=stmt_info[j+1].nest; cj++)
                        {
                        is_node_compatible[i][j][ci][cj] = 1;
#if 0
                        fprintf(uniform_debug, 
                                "is_node_compatible[%d][%d][%d][%d] = 1\n", 
                                i, j, ci, cj);
#endif
                        }
#if 0
            for (pi = 1; pi <= stored_perm_dir[i][0].size(); pi++)
                for (int pj = 1; pj <= stored_perm_dir[j][0].size(); pj++)
                    {
                    fprintf(uniform_debug, "%d %d", i, pi);
                    print_perm(uniform_debug, i, pi);
                    fprintf(uniform_debug, "%d %d", j, pj);
                    print_perm(uniform_debug, j, pj);

                    fprintf(uniform_debug, "comp = %3d, depth = %3d\n",
                            is_perm_compatible[i][j][pi][pj],
                            dist_level[i][j][pi][pj]);
                    }
#endif
            }
    } /* analyze_parallelism */



void determine_parallelism()
   {
   stored_perm_dir.resize(nr_statements, 2 * global_max_nest+1);
   stored_perm_order.resize(nr_statements, 2 * global_max_nest+1);
   good_perms.resize(nr_statements, 2 * global_max_nest+1);
   trans.resize(nr_statements, nr_statements);

#if !defined (SPEED)
   is_perm_compatible.resize(nr_statements,nr_statements,max_perms,max_perms);
   dist_level.resize(nr_statements,nr_statements,max_perms,max_perms);
#else
   is_perm_compatible = new nb[nr_statements];
   dist_level = new nb[nr_statements];
#endif

   closure_dir.resize(nr_statements, nr_statements);

   if (uniform_args.gen_do_closure)
       close_rels(closure_dir);
   else
       {
       add_dirs(closure_dir, false);
       close_dir_array(closure_dir);
       }
   } /* determine_parallelism */



int unbalanced(int stmt, int si)
    {
    int nest = stmt_info[stmt+1].nest;
    Relation rel = stmt_info[stmt+1].bounds;
    for (GEQ_Iterator g(rel.single_conjunct()); g; g++)
        {
        if ((*g).get_coef(set_var(si)) == 0) continue;

        for (int j = 1; j<=nest; j++)
            if (si != j && (*g).get_coef(set_var(j)) != 0)
	        return 1;

	}
    return 0;
    } /* unbalanced */

}
