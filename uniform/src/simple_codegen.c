/* $Id: simple_codegen.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#define NEED_PARALLEL
#define NEED_SYNC

#include <stdio.h>
#include <math.h>
#include <omega.h>
#include <omega/Relations.h>
#include <petit/uniform-interf.h>
#include <uniform/uniform_misc.h>
#include <uniform/space_mappings.h>
#include <uniform/time_mappings.h>
#include <uniform/parallelism.h>
#include <uniform/simple_codegen.h>
#include <uniform/wak_codegen.h>
#include <uniform/depend_dir.h>

namespace omega {

int parallel_reduction[max_stmts];

Tuple<int> barrier_required[max_nest][max_stmts];
Tuple<int> post_required[max_nest][max_stmts];
Tuple<int> wait_required[max_nest][max_stmts];
Tuple<int> post_wait_rev[max_nest][max_stmts];

static int nr_subgroups[max_stmts][maxLevels];
static int must_loop[max_stmts][maxLevels];

String wak_program_head(String Decls, int timeDepth) {
    String s = "";
#if defined NEED_PARALLEL
    s += "#include \"runtime.h\"\n\n";
#endif
    s += "#include <stdio.h>\n";
    s += "#include <math.h>\n";
    s += "#define min(x,y) ((x)<(y)?(x):(y))\n";
    s += "#define max(x,y) ((x)>(y)?(x):(y))\n\n";

#if defined NEED_PARALLEL
    s += "int _suif_nproc = 0;\n";
    s += "extern void (**volatile _suif_aligned_task_f)(int);\n\n";
#endif

    s += Decls;

#if defined NEED_PARALLEL
    s += "static void doall_body(int _my_id)\n";
#else
    s += "main()\n";
#endif
    s += "    {\n";

#if defined NEED_PARALLEL
    s += "    int global_lb, global_ub;\n";
    s += "    int block_size;\n";
    s += "    int lb, ub;\n";
    s += "    int _counter0 = 1;\n";
    s += "    int _counter1 = 1;\n";
    s += "    int _my_nprocs = suif_num_total_ids();\n";
#endif

    assert (timeDepth > 0);
    String index_vars = "    int t1";
    for(int ind = 2; ind <= timeDepth; ind+=2)
        index_vars += ", t" + itoS(ind);
    index_vars += ";\n";

   return s + index_vars;
}



static String set_lb_ub()
    {
    int i;
    Tuple<Relation> I(nr_statements);
    for(int s = 1; s<= nr_statements; s++) 
        {
        I[s] = Range(Restrict_Domain(copy(space[s-1]), 
                     copy(stmt_info[s].bounds)));
        I[s].simplify(2,4);
        }

    int redund_low[max_stmts], redund_high[max_stmts];
    Relation low[max_stmts], high[max_stmts];

    for (i = 1; i<=nr_statements; i++)
        {
        low[i] = Relation::True(1);
        high[i] = Relation::True(1);

        I[i] = EQs_to_GEQs(I[i],true);
        Conjunct *c = I[i].single_conjunct();
    
        for (GEQ_Iterator k = c->GEQs(); k.live(); k.next())
            {
            if ((*k).get_coef(input_var(1)) > 0)
                (void) low[i].and_with_GEQ(*k);

            if ((*k).get_coef(input_var(1)) < 0)
                (void) high[i].and_with_GEQ(*k);
            }

        low[i].setup_names();
        high[i].setup_names();

        redund_low[i] = 0;
        redund_high[i] = 0;
        }


    for (i = 1; i<=nr_statements; i++)
        for (int j = 1; j<=nr_statements; j++)
            if (i!=j && !redund_low[j] && Must_Be_Subset(copy(low[i]),copy(low[j])))
                {
                redund_low[i] = 1;
                break;
                }
    int non_redund_low = 0;
    for (i = 1; i<=nr_statements; i++)
        if (!redund_low[i])
            non_redund_low++;
    String start = "";
    if (non_redund_low > 1)
        start += "min(";
    int seenMax = 0;
    for (i = 1; i<=nr_statements; i++)
        if (!redund_low[i])
            {
            if (seenMax) start += ",";
            int lower_bounds=0;
            for (GEQ_Iterator g1(low[i].single_conjunct()); g1; g1++) 
                if ((*g1).get_coef(input_var(1)) > 0) 
                    lower_bounds++;

            int seenLB = 0;
            if (lower_bounds > 1) start += "max(";
            for (GEQ_Iterator g(low[i].single_conjunct()); g; g++) 
                {
                if ((*g).get_coef(input_var(1)) > 0) 
                    {
                    if (seenLB) start += ",";
                    if ((*g).is_const(input_var(1))) 
                        {
                        int L = -((*g).get_const());
                        int m = (*g).get_coef(input_var(1));
                        int sb  =  (int) (ceil(((float) L) /m));
                        start += itoS(sb);
                        }
                    else 
                        start += outputEasyBound(*g, input_var(1), 1);
                    seenLB = true;
                    }
                }
            if (lower_bounds > 1) start += ")";
            seenMax = 1;
            }
    if (non_redund_low > 1) start += ")";

    for (i = 1; i<=nr_statements; i++)
        for (int j = 1; j<=nr_statements; j++)
            if (i!=j&& !redund_high[j] && Must_Be_Subset(copy(high[i]),copy(high[j])))
                {
                redund_high[i] = 1;
                break;
                }
    int non_redund_high = 0;
    for (i = 1; i<=nr_statements; i++)
        if (!redund_high[i])
            non_redund_high++;
    String finish = "";
    if (non_redund_high > 1)
        finish += "max(";
    int seenMin = 0;
    for (i = 1; i<=nr_statements; i++)
        if (!redund_high[i])
            {
            if (seenMin) finish += ",";
            int upper_bounds=0;
            for (GEQ_Iterator g1(high[i].single_conjunct()); g1; g1++) 
                if ((*g1).get_coef(input_var(1)) < 0) 
                    upper_bounds++;

            int seenUB = 0;
            if (upper_bounds > 1) finish += "min(";
            for (GEQ_Iterator g(high[i].single_conjunct()); g; g++) 
                {
                if ((*g).get_coef(input_var(1)) < 0) 
                    {
                    if (seenUB) finish += ",";
                    if ((*g).is_const(input_var(1))) 
                        {
                        int L = -((*g).get_const());
                        int m = (*g).get_coef(input_var(1));
                        int sb  =  L / m;
                        finish += itoS(sb);
                        }
                    else 
                        finish += outputEasyBound(*g, input_var(1), 0);
                    seenUB = true;
                    }
                }
            if (upper_bounds > 1) finish += ")";
            seenMin = 1;
            }
    if (non_redund_high > 1) finish += ")";

    for (i = 1; i<=nr_statements; i++)
        {
        high[i] = Relation::Null();
        low[i] = Relation::Null();
        I[i] = Relation::Null();
        }

    String assignments = 
        "    global_lb = " + start + ";\n    global_ub = "+finish + ";\n";
 
    assignments += 
        "    block_size = max(0,(global_ub-global_lb+1+(_my_nprocs-1))"
        "/_my_nprocs);\n";
    assignments += "    lb = global_lb+_my_id*block_size;\n";
    assignments += "    ub = min(global_ub,lb+block_size-1);\n";

    return assignments;
    } /* set_lb_ub */



static int max_level_carried(Relation &T)
    {
    Relation R = T;
    for (int level = 1; level <= T.n_inp()-1; level++)
        {
        EQ_Handle eq = R.and_with_EQ();
        eq.update_coef(input_var(level), 1);
        eq.update_coef(output_var(level), -1);

        if (! R.is_upper_bound_satisfiable())
            return level;
        }

    return T.n_inp();
    } /* max_level_carried */



static Tuple<int> forw_p1[maxLevels];
static Tuple<int> forw_p2[maxLevels];
static Tuple<int> forw_rev[maxLevels];
static Tuple<int> back_p1[maxLevels];
static Tuple<int> back_p2[maxLevels];

static Relation less_than;
static Relation greater_than;

static void insert_sync(int p1, int p2, Relation &rel)
    {
    Relation S_less_than;
    Relation S_greater_than;
    int reversed;

    if (neg_space[p1] && neg_space[p2])
        {
        S_less_than = 
            Join(Join(copy(space[p1]), copy(greater_than)), 
                 Inverse(copy(space[p2])));
        S_greater_than = 
            Join(Join(copy(space[p1]), copy(less_than)), 
                 Inverse(copy(space[p2])));
        reversed = 1;
        }
    else
        {
        S_less_than = 
            Join(Join(copy(space[p1]), copy(less_than)), 
                 Inverse(copy(space[p2])));
        S_greater_than =
            Join(Join(copy(space[p1]),copy(greater_than)),
                 Inverse(copy(space[p2])));
        reversed = 0;
        }

    Relation R_less_than = 
        Intersection(copy(rel), copy(S_less_than));

    if (R_less_than.is_upper_bound_satisfiable())
        {
        Relation T = Join(Join(Inverse(copy(time0[p1])), 
                               copy(R_less_than)),
                          copy(time0[p2]));

        int level = max_level_carried(T);

        if (is_loop_dist[p1][level-1] || is_loop_dist[p2][level-1])
            assert(0 && 
                 "backward dependence at carried loop, not implemented yet!");

#if 0
        fprintf(uniform_debug, "backward %d -> %d level %d\n", p1, p2, level);
#endif

        back_p1[level].append(p1);
        back_p2[level].append(p2);
        }

    Relation R_greater_than = 
        Intersection(copy(rel), copy(S_greater_than));

    if (R_greater_than.is_upper_bound_satisfiable())
        {
        Relation T = Join(Join(Inverse(copy(time0[p1])), 
                               copy(R_greater_than)),
                          copy(time0[p2]));

        int level = max_level_carried(T);

#if 0
        fprintf(uniform_debug, "forward %d -> %d level %d\n", p1, p2, level);
#endif

        forw_p1[level].append(p1);
        forw_p2[level].append(p2);
        forw_rev[level].append(reversed);

        if (Use_Cyclic)
            {
#if 0
            fprintf(uniform_debug, "cyclic %d -> %d level %d\n", p1, p2, level);
#endif

            back_p1[level].append(p1);
            back_p2[level].append(p2);
            }
        }
    } /* insert_sync */



static int subgroup(int group1, int level1, int group2, int level2)
    {
    for (int i=0; i<nr_statements; i++)
        if (group[(level1-1)/2][i] == group1 && 
            group[(level2-1)/2][i] == group2)
            return 1;
    return 0;
    } /* subgroup */



static int wak_nr_subgroups(int grp, int level)
    {
    int grps = 0;
    for (int g = 0; g<nr_groups[(level+1)/2]; g++)
        if (subgroup(grp, level, g, level+2))
            grps++;
    return grps;
    } /* wak_nr_subgroups */



static int ordered(int t1, int t2, int first)
    {
    return (first <= t1 && t1 < t2) ||
           (t1 < t2 && t2 < first) ||
           (t2 < first && first <= t1);
    } /* ordered */



static int max_order(int t1, int t2, int first)
    {
    if (ordered(t1, t2, first))
        return t2;
    else 
        return t1;
    } /* max_order */



static int min_order(int t1, int t2, int first)
    {
    if (ordered(t1, t2, first))
        return t1;
    else 
        return t2;
    } /* min_order */


static Tuple<int> p1_list(0);
static Tuple<int> p2_list(0);
static int bar_required[max_stmts];

static int place_first_barrier(int first, int grp, int level)
    {
#if 0
    fprintf(uniform_debug, "place_barrier_first(first %d, grp %d, level %d)\n",
            first, grp, level);
#endif

    assert(level%2 == 1);
    for (int p = 0; p <=nr_subgroups[grp][level]; p++)
        bar_required[p] = 0;

    bar_required[first] = 1;
    int nr_required = 1;
#if 0
    fprintf(uniform_debug, "place first barrier before %d\n", first);
#endif

    for (int j1 = 1; j1<= p1_list.size(); j1++)
        {
        for (int j2 = j1+1; j2<= p1_list.size(); j2++)
            {
            int t1 = time_coef[level][p1_list[j1]];
            int t2 = time_coef[level][p1_list[j2]];
            if (! ordered(t1,t2,first))
                {
                int tmp1 = p1_list[j1];
                p1_list[j1] = p1_list[j2];
                p1_list[j2] = tmp1;
                int tmp2 = p2_list[j1];
                p2_list[j1] = p2_list[j2];
                p2_list[j2] = tmp2;
                }
            }
        }

    int lower = first;
    int upper = first-1;

    int outstanding = 0;
    for (int j = 1; j <= p1_list.size(); j++)
        {
        int t1 = time_coef[level][p1_list[j]];
        int t2 = time_coef[level][p2_list[j]];

#if 0
        fprintf(uniform_debug, "dependence from %d (t1 %d) to %d (t2 %d)\n",
                p1_list[j], t1, p2_list[j], t2);
#endif

        if (ordered(t2,t1,first))
            {
#if 0
            fprintf(uniform_debug, "broken by first barrier\n");
#endif
            continue;
            }
        else
            outstanding = 1;

start:  if (ordered(t1,upper,first) && ordered(lower,t2,first))
            {
            lower = max_order(t1, lower, first);
            upper = min_order(t2, upper, first);
#if 0
            fprintf(uniform_debug, "new range %d to %d\n", lower, upper);
#endif
            }
        else
            {
            bar_required[upper] = 1;
            nr_required++;
#if 0
            fprintf(uniform_debug, "place another barrier before %d\n", upper);
#endif
            lower = max_order(t1, upper, first);
            upper = t2;
            goto start;
            }
        }

    if (outstanding)
        {
        bar_required[upper] = 1;
        nr_required++;
#if 0
        fprintf(uniform_debug, "place last barrier before %d\n", upper);
#endif
        }

#if 0
    fprintf(uniform_debug, "nr_required = %d\n", nr_required);
#endif

    return nr_required;
    } /* place_first_barrier */


static int block_dist(int grp, int level)
    {
    for (int j=0; j<nr_statements; j++)
        if (group[level/2][j] == grp)
            return (is_loop_dist[j][level] == 1);
    assert(0);
    return 0;
    } /* block_dist */



static int must_exec(int j, int level)
    {
    assert(level % 2 == 0);

    Relation R = 
        Range(Restrict_Domain(copy(time0[j]), copy(stmt_info[j+1].bounds)));

    Relation R1 = wak_project_onto_levels(R, level, 0);

    R1 = EQs_to_GEQs(R1, true);
    Relation Y = Relation::True(R1);

    for (GEQ_Iterator g1(R1.single_conjunct()); g1; g1++)
        if ((*g1).get_coef(set_var(level)) != 0)
            Y.and_with_GEQ(*g1);

    Relation R2 = Project(Y, level, Set_Var);

    int must = R2.is_obvious_tautology();

    return must;
    } /* must_exec */



static int wak_must_loop(int grp, int level)
    {
    for (int j=0; j<nr_statements; j++)
        if (group[level/2][j] == grp)
            if (must_exec(j, level))
                return 1;
    return 0;
    } /* wak_must_loop */


static int test_redundant(int grp, int level, int p1, int p2, int reversed, 
                          int bar)
    {
#if 0
    fprintf(uniform_debug, 
            "\ntest_redundant(grp %d,level %d,p1 %d,p2 %d,rev %d,bar %d)\n",
            grp, level, p1, p2, reversed, bar);
#endif

    if (level % 2 == 1)
        {
        int j;
        int t1, t2;
        int g1, g2;

        if (p1 >= 0)
            {
            t1 = time_coef[level][p1];
            g1 = group[(level+1)/2][p1];
            }
        else
            t1 = g1 = -MAXINT;

        if (p2 >= 0)
            {
            t2 = time_coef[level][p2];
            g2 = group[(level+1)/2][p2];
            }
        else
            t2 = g2 = MAXINT;

#if 0
        fprintf(uniform_debug, "t1 %d, t2 %d\n", t1, t2);
        fprintf(uniform_debug, "g1 %d, g2 %d\n", g1, g2);
#endif

        for (j=1; j<=barrier_required[level][grp].size(); j++)
            {
            int b = barrier_required[level][grp][j];

            if (t1 < b && b <= t2)
                {
#if 0
                fprintf(uniform_debug, "redundant due to barrier(%d,%d)[%d]\n", 
                        level, grp, b);
#endif
                return 1;
                }
            }

        for (j=1; !bar && j<=post_required[level][grp].size(); j++)
            {
            int src = post_required[level][grp][j];
            int dest = wait_required[level][grp][j];
            if (post_wait_rev[level][grp][j] != reversed)
                continue;

            if (t1 <= src && dest <= t2)
                {
#if 0
                fprintf(uniform_debug, "redundant due to post&wait(%d,%d)[%d->%d]\n", 
                        level, grp, src, dest);
#endif
                return 1;
                }
            }
    
        if (level+1 <= time0[0].n_out())
            {
            if (0 <= g1 && g1 <= nr_groups[(level+1)/2])
                if (test_redundant(g1, level+1, p1, -1, reversed, bar))
                    return 1;

            for (int g=max(0,g1+1); g<=min(nr_groups[(level+1)/2],g2-1); g++)
                if (subgroup(grp, level, g, level+1))
                    if (test_redundant(g, level+1, -1, -1, reversed, bar))
                        return 1;

            if (0 <= g2 && g2 <= nr_groups[(level+1)/2])
                if (test_redundant(g2, level+1, -1, p2, reversed, bar))
                    return 1;
            }
        }
    else
        {
        if (p1 != -1 || p2 != -1 || 
            block_dist(grp, level) == 1 || 
            must_loop[grp][level])
            {
            int found = 0;
            for (int jj=1; jj<= post_wait_rev[level][grp].size(); jj++)
                if (post_wait_rev[level][grp][jj] == reversed)
                    {
                    found = 1;
                    break;
                    }
            if (!bar && found)
                {
#if 0
                fprintf(uniform_debug, "redundant due to post&wait(%d,%d)\n", 
                        level, grp);
#endif
                return 1;
                }
    
#if 0
            fprintf(uniform_debug, "test even %d ->\n", p1);
#endif
            if (test_redundant(grp, level+1, p1, -1, reversed, bar))
                return 1;
            
#if 0
            fprintf(uniform_debug, "test even -> %d\n", p2);
#endif
            if (test_redundant(grp, level+1, -1, p2, reversed, bar))
                return 1;
            }
        }

    return 0;
    } /* test_redundant */



static void place_barriers(int grp, int level)
    {
#if 0
    fprintf(uniform_debug, "place_barriers(group %d, level %d)\n", grp, level);
#endif

    p1_list = Tuple<int>(0);
    p2_list = Tuple<int>(0);

    int required = 0;

    for (int j1 = 1; j1<= back_p1[level].size(); j1++)
        {
        int p1 = back_p1[level][j1];
        int p2 = back_p2[level][j1];
        if (group[(level-1)/2][p1] == grp)
            {
            if (test_redundant(grp, level, p1, p2, -1, 1))
                continue;

#if 0
            fprintf(uniform_debug, "add to list %d --> %d\n", p1, p2);
#endif
              required = 1;

            p1_list.append(p1);
            p2_list.append(p2);
            }
        }


    if (level > 1)
        {
        for (int j1 = 1; j1<= back_p1[level-1].size(); j1++)
            {
            int p1 = back_p1[level-1][j1];
            int p2 = back_p2[level-1][j1];
            if (group[(level-1)/2][p1] == grp)
                {
    
                int t1 = time_coef[level][p1];
                int t2 = time_coef[level][p2];

                if (t1 > t2)
                    {
                    if (test_redundant(grp, level-1, p1, p2, -1, 1))
                        continue;
#if 0
                    fprintf(uniform_debug, "add to list %d ---> %d\n", p1, p2);
#endif
                    required = 1;

                    p1_list.append(p1);
                    p2_list.append(p2);
                    }
                else
                    {
                    if (test_redundant(grp, level-1, -1, -1, -1, 1))
                        continue;
#if 0
                    fprintf(uniform_debug, "dont bother adding %d -/-> %d\n", p1, p2);
#endif
                    required = 1;
                    }
                }
            }
        }

#if 0
    fprintf(uniform_debug, "required = %d\n", required);
#endif

    if (required)
        {
        int best_bars = MAXINT;
        int best_first = -1;
        for (int first = 0; first <= nr_subgroups[grp][level]; first++)
            {
            int bars = place_first_barrier(first, grp, level);
            if (bars < best_bars)
                {
                best_first = first;
                best_bars = bars;
                }
            }

        (void) place_first_barrier(best_first, grp, level);

        assert(level%2 == 1);
        for (int p = 0; p <=nr_subgroups[grp][level]; p++)
            if (bar_required[p])
                {
#if 0
                fprintf(uniform_debug, 
                        "\nbarrier_required(level %d, grp %d, posn %d)\n",
                        level, grp, p);
#endif
                barrier_required[level][grp].append(p);
                }
        }
    } /* place_barriers */



static void place_post_and_waits(int grp, int level)
    {
#if 0
        fprintf(uniform_debug, "post_and_waits#1(grp %d, level %d)\n", grp, level);
#endif
    for (int j = 1; j<= forw_p1[level].size(); j++)
        {
        int p1 = forw_p1[level][j];
        int p2 = forw_p2[level][j];
        int reversed = forw_rev[level][j];

        if (group[(level-1)/2][p1] == grp)
            {
            int t1 = time_coef[level][p1];
            int t2 = time_coef[level][p2];

#if 0
            fprintf(uniform_debug, "post&wait %d(%d) -> %d(%d)\n", p1, t1, p2, t2);
#endif

            int redund = 0;
            for (int j1 = 1; !redund && j1<= forw_p1[level].size(); j1++)
                {
                if (j1 == j || forw_rev[level][j1] != reversed) continue;
    
                int pp1 = forw_p1[level][j1];
                int pp2 = forw_p2[level][j1];
    
                if (group[(level-1)/2][pp1] != grp) continue;

                int tt1 = time_coef[level][pp1];
                int tt2 = time_coef[level][pp2];
    
                if (t1 <= tt1 && tt2 < t2 || t1 < tt1 && tt2 <= t2)
                    {
                    redund = 1;
#if 0
                    fprintf(uniform_debug, "redundant  ");
                    fprintf(uniform_debug, "%d(%d) -> %d(%d)\n", pp1, tt1, pp2, tt2);
#endif
                    }
                }

            if (redund || test_redundant(grp, level, p1, p2, reversed, 0)) 
                continue;

            post_required[level][grp].append(t1);
            wait_required[level][grp].append(t2);
            post_wait_rev[level][grp].append(reversed);
#if 0
            fprintf(uniform_debug, "\npost required level %d group %d posn %d\n", 
                    level, grp, t1);
            fprintf(uniform_debug, "wait required level %d group %d posn %d\n", 
                    level, grp, t2);
            fprintf(uniform_debug, "reversed = %d\n", reversed);
#endif
            }
        }

    if (level > 1)
        {
#if 0
        fprintf(uniform_debug, "post_and_waits#2(grp %d, level %d)\n", grp, level);
#endif
        for (int j = 1; j<= forw_p1[level-1].size(); j++)
            {
            int p1 = forw_p1[level-1][j];
            int p2 = forw_p2[level-1][j];
            int reversed = forw_rev[level-1][j];

            if (group[(level-1)/2][p1] == grp)
                {
#if 0
                fprintf(uniform_debug, "post&wait %d -> %d\n", p1, p2);
#endif

                if (test_redundant(grp, level-1, p1, p2, reversed, 0)) 
                    continue;

                if (test_redundant(grp, level-1, p1, p1, -1, 1))
                    {
#if 0
                fprintf(uniform_debug, 
                        "\nspecial barrier_required(level %d, grp %d, posn %d)\n",
                        level, grp, 0);
#endif
                    barrier_required[level][grp].append(0);
                    }
                else
                    {
                    post_required[level-1][grp].append(-1);
                    wait_required[level-1][grp].append(-1);
                    post_wait_rev[level-1][grp].append(reversed);

#if 0
                    fprintf(uniform_debug, "\npost required level %d group %d\n", 
                            level-1, grp);
                    fprintf(uniform_debug, "wait required level %d group %d\n", 
                            level-1, grp);
                    fprintf(uniform_debug, "reversed = %d\n", reversed);
#endif
                    }
                break;
                }
            }
        }
    } /* place_post_and_waits */



static void post_order_place_sync(int grp, int level)
    {
    if (level < time0[0].n_out())
        for (int g = 0; g<nr_groups[(level+1)/2]; g++)
            {
            if (subgroup(grp, level, g, level+2))
                post_order_place_sync(g, level+2);
            }

    place_barriers(grp, level);

    place_post_and_waits(grp, level);
    } /* post_order_place_sync */



static void determine_synchronization()
    {
    for (int i = 1; i<=time0[0].n_out(); i++)
        {
        forw_p1[i] = Tuple<int>(0);
        forw_p2[i] = Tuple<int>(0);
        back_p1[i] = Tuple<int>(0);
        back_p2[i] = Tuple<int>(0);

        for (int g = 0; g<nr_groups[(i+1)/2]; g++)
            {
            barrier_required[i][g].clear();
            post_required[i][g].clear();
            wait_required[i][g].clear();
            }        
        }

    for (int p = 0; p<nr_statements; p++)
        {
        Relation &R = all_depends[p+1].self_depend;
        for (DNF_Iterator s(R.query_DNF()); s; s++)
            {
            Relation this_part(R, s.curr());
            insert_sync(p, p, this_part);
            }
        for (rel_edge *e2=all_depends[p+1].forw_link;
             e2!=NULL;
             e2=e2->next_forw_link)
            {
            Relation &R2 = e2->rel;
            for (DNF_Iterator s(R2.query_DNF()); s; s++)
                {
                Relation this_part(R2, s.curr());
                insert_sync(e2->parent_index,e2->child_index, this_part);
                }
            }
        }

    post_order_place_sync(0, 1);

#if 0
    for (i = 1; i<=time0[0].n_out(); i++)
        for (int g = 0; g<nr_groups[(i+1)/2]; g++)
            {
            int j;

            if (barrier_required[i][g].size() > 0)
                fprintf(uniform_debug, "level %d, group %d barriers:\n", i, g);
            for (j=1; j<=barrier_required[i][g].size(); j++)
                fprintf(uniform_debug, "%2d ", barrier_required[i][g][j]);
            if (barrier_required[i][g].size() > 0)
                fprintf(uniform_debug, "\n");

            if (post_required[i][g].size() > 0)
                fprintf(uniform_debug, "level %d, group %d post and waits:\n", i, g);
            assert (post_required[i][g].size() == wait_required[i][g].size());
            for (j=1; j<=post_required[i][g].size(); j++)
                fprintf(uniform_debug, "%2d->%2d ", 
                        post_required[i][g][j], wait_required[i][g][j]);
            if (post_required[i][g].size() > 0)
                fprintf(uniform_debug, "\n");
            }
#endif
    } /* determine_synchronization */



static void determine_reductions()
    {
    Dynamic_Array2<dir_list *> close_dir("Stmts", "Stmts");
    close_dir.resize(nr_statements, nr_statements);
    add_dirs(close_dir, true);

    for (int p = 0; p<nr_statements; p++)
        {
        int dist = dist_loop[p][best_time[p]];
        int grp = group[dist][(dist+1)/2];
        int parallel = 1;

        if (barrier_required[dist+1][grp].size() > 0 ||
            post_required[dist][grp].size() > 0)
            parallel = 0;

        int reduction = 0;
        for (dir_list *s = close_dir[p][p]; s!=NULL; s=s->next)
            {
            if (best_space[p] > 0 && ddextract(s, best_space[p]) != dduind)
                reduction = 1;
            }

        parallel_reduction[p] = (parallel && reduction);
        }

    dir_list *nnext;
    for (int i = 0; i<nr_statements; i++)
        for (int j = 0; j<nr_statements; j++)
            for (dir_list *n=close_dir[i][j]; n!=NULL; n = nnext)
                {
                nnext = n->next;
                delete n;
                }
    close_dir.resize(0, 0);
    } /* determine_reductions */



static void set_subgroups()
    {
    for (int level = 1; level <= time0[0].n_out(); level += 2)
        for (int grp = 0; grp<nr_groups[(level+1)/2]; grp++)
            nr_subgroups[grp][level] = wak_nr_subgroups(grp, level);
    } /* set_subgroups */



static void set_must_loop()
    {
    for (int level = 2; level <= time0[0].n_out(); level += 2)
        for (int grp = 0; grp<nr_groups[(level+1)/2]; grp++)
            must_loop[grp][level] = wak_must_loop(grp, level);
    } /* set_must_loop */


static void set_lt_gt()
    {
    less_than = Relation::True(1,1);
    greater_than = Relation::True(1,1);

    GEQ_Handle geq1 = less_than.and_with_GEQ();
    geq1.update_coef(input_var(1),   1);
    geq1.update_coef(output_var(1), -1);
    geq1.update_const(-1);

    GEQ_Handle geq2 = greater_than.and_with_GEQ();
    geq2.update_coef(input_var(1), -1);
    geq2.update_coef(output_var(1), 1);
    geq2.update_const(-1);
    } /* set_lt_gt */



static int inter_processor(Depend *dd)
    {
    int p1 = StmtNr(DependSrc(dd));     
    int p2 = StmtNr(DependDest(dd));     

    Relation S_less_than = 
        Join(Join(copy(space[p1]), copy(less_than)), 
             Inverse(copy(space[p2])));

    Relation R_less_than = 
        Intersection(copy(*DependRelation(dd)), copy(S_less_than));

    if (R_less_than.is_upper_bound_satisfiable())
	return 1;

    Relation S_greater_than =
        Join(Join(copy(space[p1]),copy(greater_than)),
             Inverse(copy(space[p2])));


    Relation R_greater_than = 
        Intersection(copy(*DependRelation(dd)), copy(S_greater_than));

    if (R_greater_than.is_upper_bound_satisfiable())
	return 1;

    return 0;
    } /* inter_processor */


static Variable *global_list[100];
static Variable *private_list[100];
static int global_count = 0;
static int private_count = 0;

static void wak_privatize()
    {
    for (VarIterator vi = VarFirst(); !VarLast(vi); VarNext(vi))
	{
        for (AccessIterator ai = AccessFirst(VarCurr(vi));
             !AccessLast(ai);
             AccessNext(ai))
	    {
	    DependIterator di;
	    for (di = DependOutgoing(AccessCurr(ai));
                 !DependLast(di);
                 DependNext(di))
		{
                Depend *dd = DependCurr(di);

		if (DependIsFlow(dd) &&
		    DependIsFlow(dd) && 
		    AccessIsExit(DependDest(dd)))
		    goto cantPriv;
		if (DependIsValue(dd) &&
		    DependIsFlow(dd) && 
		    inter_processor(dd))
		    goto cantPriv;
		}
	    for (di = DependIncoming(AccessCurr(ai));
                 !DependLast(di);
                 DependNext(di))
		{
                Depend *dd = DependCurr(di);

		if (DependIsValue(dd) && 
		    DependIsFlow(dd) && 
		    AccessIsEntry(DependSrc(dd)))
		    goto cantPriv;
		}
	    }

	private_list[private_count++] = VarCurr(vi);
	continue;

    cantPriv:
	global_list[global_count++] = VarCurr(vi);
	continue;
        }
    } /* wak_privatize */



void simple_codegen()
    {
    int i;
    set_subgroups();

    set_must_loop();

    set_lt_gt();

    determine_synchronization();

    determine_reductions();

    wak_privatize();

    String global_decls = "";
    for (i=0; i<global_count; i++)
	global_decls += VarDecl(global_list[i]) + ";\n";

    String s = wak_program_head(global_decls, time0[0].n_out());

    String private_decls = "";
    for (i=0; i<private_count; i++)
	private_decls += "    " + VarDecl(private_list[i]) + ";\n";

    String init = private_decls;

#if defined NEED_PARALLEL
    
    init += "\n";
    if (Use_Cyclic)
        init += "#define adjust(start)        "
             "max((start), ((start)/_my_nprocs)*_my_nprocs + _my_id)\n";
    else
        init += set_lb_ub();
    init += "\n";
#endif

    String body = WakGenerateCode(init);
    s += body;

    s += "    }\n\n";

#if defined NEED_PARALLEL
    s += "extern int _suif_start(int argc, char **argv, char **envp)\n";
    s += "    {\n";
    s += "    double time;\n";
    s += "    suif_counter_init_range(1);\n\n";
    s += "    *_suif_aligned_task_f = doall_body;\n";
    s += "    time = suif_clock();\n";
    s += "    suif_doall(doall_body);\n";
    s += "    time = suif_clock() - time;\n";
    s += "    printf(\"Executed program %s on %d processors in %lf seconds\\n\",\n";
    s += "           argv[0], suif_num_total_ids(), time);\n\n";
    s += "    return 0;\n";
    s += "    }\n";
#endif

    fprintf(uniform_debug2, "%s", (const char *)s);
    } /* simple_codegen */
}
