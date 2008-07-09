/* $Id: depend_dir.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <basic/Dynamic_Array.h>
#include <omega.h>
#include <omega/pres_conj.h>
#include <uniform/uniform_args.h>
#include <uniform/depend_dir.h>
#include <uniform/uniform_misc.h>
#include <uniform/select.h>

namespace omega {

static int reduction_vol[max_stmts];
Tuple<Global_Var_ID> uniform_global_list(0);

ddudirection ddextract(dir_list *d, int n)
    {
    if (n <= d->nest)
        return dduextract1(d->dir, n);
    else if (n == d->nest+1)
        {
        for (int i=1; i<=d->nest; i++)
            if (dduextract1(d->dir, i) != dduind)
                return ddufwd + dduind + ddubck;
        return ddufwd + dduind;
        }
    else
        return ddufwd + dduind + ddubck;
    } /* ddextract */


static int ok;

static void wprint(char c)
    {
    fprintf(uniform_debug, "%c", c);
    ok = 1;
    } /* wprint */


void print_dir(dir_list *start)
    {
    fprintf(uniform_debug, "%p", start);
    char ch = '(';
    for(int n=1; n<=start->nest; ++n)
        {
        ok = 0;
        fprintf(uniform_debug, "%c", ch);
        ch = ',';
        ddudirection thisdd = dduextract1(start->dir, n);
        if(thisdd == ddufwd+dduind+ddubck)
            wprint('*');
        else
            {
            if (ddutest1(thisdd,dduind)) wprint('0');
            if (ddutest1(thisdd,ddufwd)) wprint('+');
            if (ddutest1(thisdd,ddubck)) wprint('-');
            }
        assert(ok);
        }
    if (start->nest == 0) fprintf(uniform_debug,"(");
    fprintf(uniform_debug,") ");
    } /* print_dir */


void print_dirs(dir_list *start)
    {
    while (start) 
        {
        print_dir(start);
        start = start->next;
        }
    fprintf(uniform_debug, "\n");
    } /* print_dirs */


Relation restrict_less(Relation &R, int level)
    {
    Relation RR = R;
    GEQ_Handle geq = RR.and_with_GEQ();

    geq.update_coef(input_var(level), 1);
    geq.update_coef(output_var(level), -1);
    geq.update_const(-1);

    return RR;
    } /* restrict_less */


Relation restrict_more(Relation &R, int level)
    {
    Relation RR = R;
    GEQ_Handle geq = RR.and_with_GEQ();

    geq.update_coef(input_var(level), -1);
    geq.update_coef(output_var(level), 1);
    geq.update_const(-1);

    return RR;
    } /* restrict_more */


Relation restrict_equal(Relation &R, int level)
    {
    Relation RR = R;
    EQ_Handle eq = RR.and_with_EQ();

    eq.update_coef(input_var(level), 1);
    eq.update_coef(output_var(level), -1);

    return RR;
    } /* restrict_equal */


dir_list *dir_list_union(dir_list *d1, dir_list *d2)
    {
    if (d1 == d2)
        return d1;

    if (d1 == NULL)
        return d2;

    if (d2 == NULL)
        return d1;

    dir_list *dd;
    for (dd = d1; dd->next != NULL; dd=dd->next);

    dd->next = d2;

    return d1;
    } /* dir_list_union */



static dir_list *split_levels(int stmt1, Relation &R, int level, 
                              dir_list *nn, int nest)
    {
    int lb, ub;
    bool guaranteed;
    ddudirection signs = 0;
    dir_list *big_list;

    if (level <= nn->nest)
        {
        big_list = NULL;
        R.query_difference(output_var(level), input_var(level), 
                           lb, ub, guaranteed);

        if (lb < 0) 
            {
            Relation restricted = restrict_less(R, level);
            if (restricted.is_upper_bound_satisfiable())
                {
                dir_list *n = new dir_list;
                n->nest = nn->nest;
                n->dir = nn->dir;
                n->next = NULL;
    
                signs = ddubck;
                ddudirsetonly(n->dir, signs, level);
                big_list = dir_list_union(
                    big_list, 
                    split_levels(stmt1, restricted, level+1, n, nest));
                delete n;
                }
            }
        if (ub > 0) 
            {
            Relation restricted = restrict_more(R, level);
            if (restricted.is_upper_bound_satisfiable())
                {
                dir_list *n = new dir_list;
                n->nest = nn->nest;
                n->dir = nn->dir;
                n->next = NULL;
    
                signs = ddufwd;
                ddudirsetonly(n->dir, signs, level);
                big_list = dir_list_union(
                    big_list, 
                    split_levels(stmt1, restricted, level+1, n, nest));
                delete n;
                }
            }
        if (lb <= 0 && 0 <= ub)
            {
            Relation restricted = restrict_equal(R, level);
            if (restricted.is_upper_bound_satisfiable())
                {
                dir_list *n = new dir_list;
                n->nest = nn->nest;
                n->dir = nn->dir;
                n->next = NULL;
    
                signs = dduind;
                ddudirsetonly(n->dir, signs, level);
                big_list = dir_list_union(
                    big_list, 
                    split_levels(stmt1, restricted, level+1, n, nest));
                delete n;
                }
            }
        }
    else
        {
        dir_list *n = new dir_list;
        n->nest = nn->nest;
        n->dir = nn->dir;
        n->next = NULL;

        big_list = n;
        }

    return big_list;
    } /* splits_levels */



dir_list *add_direction(int stmt1, unsigned int nest, 
                        NOT_CONST Relation &input_dep_rel) {
    Relation dep_rel = consume_and_regurgitate(input_dep_rel);

    dep_rel = Lower_Bound(dep_rel);

    dir_list *n = new dir_list;
    n->nest = min(dep_rel.n_inp(), dep_rel.n_out());
    n->dir = 0;
    n->next = NULL;

    dir_list *big_list = NULL;

    for (DNF_Iterator s(dep_rel.query_DNF()); s; s++)
        {
        Relation this_part(dep_rel, s.curr());
        big_list = dir_list_union(
                       big_list,
                       split_levels(stmt1, this_part, 1, n, nest));
        }
    delete n;
    return big_list;
    } /* add_direction */


dir_list *compose_dir(dir_list *in, dir_list *out, dir_list **last)
    {
    dir_list *start = NULL;
    for (dir_list *in1=in; in1!=NULL; in1=in1->next)
        for (dir_list *out1=out; out1!=NULL; out1=out1->next)
            {
            dir_list *n = new dir_list;
            n->nest = min(in1->nest, out1->nest);

            ddudirection direction = 0;
            for (int i = 1; i<= n->nest; i++)
                {
                ddudirection ddin = dduextract1(in1->dir, i);
                ddudirection ddout = dduextract1(out1->dir, i);

                ddudirection dd = 0;
                if (ddutest1(ddin, ddufwd) && ddutest1(ddout, ddufwd))
                    dd |= ddufwd;
                if (ddutest1(ddin, ddufwd) && ddutest1(ddout, dduind))
                    dd |= ddufwd;
                if (ddutest1(ddin, ddufwd) && ddutest1(ddout, ddubck))
                    dd |= ddufwd + dduind + ddubck;
                if (ddutest1(ddin, dduind) && ddutest1(ddout, ddufwd))
                    dd |= ddufwd;
                if (ddutest1(ddin, dduind) && ddutest1(ddout, dduind))
                    dd |= dduind;
                if (ddutest1(ddin, dduind) && ddutest1(ddout, ddubck))
                    dd |= ddubck;
                if (ddutest1(ddin, ddubck) && ddutest1(ddout, ddufwd))
                    dd |= ddufwd + dduind + ddubck;
                if (ddutest1(ddin, ddubck) && ddutest1(ddout, dduind))
                    dd |= ddubck;
                if (ddutest1(ddin, ddubck) && ddutest1(ddout, ddubck))
                    dd |= ddubck;

                ddudirsetonly(direction, dd, i);
                }

            n->dir = direction & ddudirBits;

            n->next = start;
            if (start == NULL) *last = n;
            start = n;
            }
#if 0
    fprintf(uniform_debug,"Composing ");
    print_dirs(in);
    fprintf(uniform_debug," and ");
    print_dirs(out);
    fprintf(uniform_debug," gives ");
    print_dirs(start);
    fprintf(uniform_debug,"\n");
#endif
    return start;
    } /* compose_dir */



int merged(dir_list *n, dir_list *m, dir_list *prevm, dir_list **last)
    {
    if (n->nest != m->nest)
        return 0;

    if ((n->dir | m->dir) == n->dir)
        {
        if (m == *last) *last = prevm;
        prevm->next = m->next;
        delete m;
        return 1;
        }

    if ((n->dir | m->dir) == m->dir)
        {
        n->dir = m->dir;
        if (m == *last) *last = prevm;
        prevm->next = m->next;
        delete m;
        return 1;
        }
        
    int i;
    for (i=1; i<=n->nest; i++)
        if (dduextract1(n->dir, i) != dduextract1(m->dir, i))
            break;

    for (int j=i+1; j<=n->nest; j++)
        if (dduextract1(n->dir, j) != dduextract1(m->dir, j))
            return 0;

    n->dir |= m->dir;
    if (m == *last) *last = prevm;
    prevm->next = m->next;
    delete m;
    return 1;
    }



void merge_dirs(dir_list *start, dir_list **last)
    {
#if 0
    fprintf(uniform_debug, "Before merge\n");
    print_dirs(start);
#endif

    int changed = 0;
    for (dir_list *n=start; n!=NULL; n=n->next)
        {
        dir_list *nextm, *prevm = n;
        for (dir_list *m=n->next; m!=NULL; m=nextm)
            {
            nextm = m->next;
            changed = changed || merged(n, m, prevm, last);
            prevm = m;
            }
        }

#if 0
    fprintf(uniform_debug, "After merge\n");
    print_dirs(start);
    fprintf(uniform_debug, "changed = %d\n", changed);
#endif

    if (changed) merge_dirs(start, last);
    } /* merge_dirs */



void delete_dir_list(dir_list *tmp_list)
    {
    dir_list *next_tmp;
    for (dir_list *tmp=tmp_list; tmp!=NULL; tmp=next_tmp)
        {
        next_tmp = tmp->next;
        delete tmp;
        }
    } /* delete_dir_list */



void close_dirs(Dynamic_Array2<dir_list *> &dir_array, 
                       int i,int k, int j)
    {
#if 0
    fprintf(uniform_debug, "%d -> %d -> %d\n", i, k, j);
    fprintf(uniform_debug, "old dir array:\n");
    print_dirs(dir_array[i][j]);
    fprintf(uniform_debug, "%d -> %d:\n",i, k);
    print_dirs(dir_array[i][k]);
    fprintf(uniform_debug, "%d -> %d:\n",k, k);
    print_dirs(dir_array[k][k]);
    fprintf(uniform_debug, "%d -> %d:\n",k, j);
    print_dirs(dir_array[k][j]);
#endif

    dir_list *new_dirs, *last = NULL;

    if (dir_array[k][k] != NULL)
        {
        dir_list *tmp_list;
        tmp_list = compose_dir(dir_array[i][k], dir_array[k][k], &last);
        merge_dirs(tmp_list, &last);

        new_dirs = compose_dir(tmp_list, dir_array[k][j], &last);
        merge_dirs(new_dirs, &last);

        delete_dir_list(tmp_list);

        last->next = dir_array[i][j];
        dir_array[i][j] = new_dirs;
        merge_dirs(dir_array[i][j], &last);
        }
    
    new_dirs = compose_dir(dir_array[i][k], dir_array[k][j], &last);
    merge_dirs(new_dirs, &last);

    last->next = dir_array[i][j];
    dir_array[i][j] = new_dirs;
    merge_dirs(dir_array[i][j], &last);
    } /* close_dirs */



void close_dir_array(Dynamic_Array2<dir_list *> &dir_array)
    {
#if 0
    int i;
    fprintf(uniform_debug,"Before Closure:\n");
    for (i=0; i<nr_statements; i++)
        for (int j=0; j<nr_statements; j++) if (dir_array[i][j])
            {
            fprintf(uniform_debug,"%d->%d: ", i, j);
            print_dirs(dir_array[i][j]);
            fprintf(uniform_debug,"\n");
            }
    fprintf(uniform_debug,"Start of Closure\n");
#endif

    for (int k=0; k<nr_statements; k++)
        for (int i=0; i<nr_statements; i++)
            if (dir_array[i][k]!=NULL && i != k)
                for (int j=0; j<nr_statements; j++)
                    if (dir_array[k][j]!=NULL && k != j)
                        if (1 || i > k || j > k || i == j)
                            close_dirs(dir_array, i, k, j);

#if 0
    fprintf(uniform_debug, "After Closure:\n");
    for (i=0; i<nr_statements; i++)
        for (int j=0; j<nr_statements; j++) if (dir_array[i][j])
            {
            fprintf(uniform_debug,"%d->%d: ", i, j);
            print_dirs(dir_array[i][j]);
            fprintf(uniform_debug,"\n");
            }
#endif
    } /* close_dir_array */



int is_plus(dir_list * n, int /*stmt*/, int j)
    {
    ddudirection dd = ddextract(n, j);
    return (dd == ddufwd);
    } /* is_plus */

int is_minus(dir_list * n, int /*stmt*/, int j)
    {
    ddudirection dd = ddextract(n, j);
    return (dd == ddubck);
    } /* is_minus */

int contains_plus(dir_list * n, int /*stmt*/, int j)
    {
    ddudirection dd = ddextract(n, j);
    return ddutest1(dd, ddufwd);
    } /* contains_plus */

int contains_minus(dir_list * n, int /*stmt*/, int j)
    {
    ddudirection dd = ddextract(n, j);
    return ddutest1(dd, ddubck);
    } /* contains_minus */


void add_dirs(Dynamic_Array2<dir_list *> &dir_array, bool reduce)
    {
    int i;
    for (i=0; i<nr_statements; i++)
        for (int j=0; j<nr_statements; j++)
            dir_array[i][j] = NULL;

    for (i = 1; i<= nr_statements; i++)
        {
        for (AccessIterator ai = AccessLeftMost(stmt_info[i].stmt);
             !AccessLast(ai);
             AccessNext(ai))
            {
            Access *n = AccessCurr(ai);
            for(DependIterator di = DependOutgoing(n);
                !DependLast(di);
                DependNext(di))
                {
                Depend *dd = DependCurr(di);
 
		if (reduce == DependIsReduction(dd))
                    {
                    if (AccessIsEntry(DependSrc(dd)) ||
		        AccessIsExit(DependDest(dd)))
		        continue;
                    int bnest = AccessCommonNest(DependSrc(dd), DependDest(dd));
                    int p1 = StmtNr(DependSrc(dd));
                    int p2 = StmtNr(DependDest(dd));
		    assert(p1 != -1 && p2 != -1);
                    dir_array[p1][p2] = dir_list_union(
                        dir_array[p1][p2],
                        add_direction(p1, bnest, copy(*DependRelation(dd))));
                    dir_list *last = NULL;
                    merge_dirs(dir_array[p1][p2], &last);
                    }
                }
            }
        }
    } /* add_dirs */



void print_extended_dir(extended_dir_list *start)
    {
    char ch1 = '{';
    for (int n1 = 0; n1<=start->nest1; n1++)
        {
        fprintf(uniform_debug, "%c", ch1);
        ch1 = ',';
        char ch = '(';
        for (int n2 = 0; n2<=start->nest2; n2++)
            {
            fprintf(uniform_debug, "%c", ch);
            ch = ',';
            if (start->zero[n1][n2])
                fprintf(uniform_debug, "=");
            else if (start->constant[n1][n2])
                {
                fprintf(uniform_debug, "%d", start->coefs[n1][n2][0]);
                for (int i=1; i<=uniform_global_list.size(); i++)
                    if (start->coefs[n1][n2][i] != 0)
                        fprintf(uniform_debug, " + %d %s", start->coefs[n1][n2][i],
                            (const char *) uniform_global_list[i]->base_name());
                }
            else
                fprintf(uniform_debug, "*");
            }
        if (start->nest2 == 0) fprintf(uniform_debug,"(");
        fprintf(uniform_debug,")");
        }
    if (start->nest1 == 0) fprintf(uniform_debug,"{");
    fprintf(uniform_debug,"} ");
    } /* print_extended_dir */



void print_extended_dirs(extended_dir_list *start)
    {
    while (start)
        {
        print_extended_dir(start);
        start = start->next;
        }
    fprintf(uniform_debug, "\n");
    } /* print_extended_dirs */



static void add_edge(int input_pos, int output_pos, EQ_Iterator e, int mult, 
                     int symbolics, extended_dir_list *n)
    {
    n->coefs[input_pos][output_pos][0] = (*e).get_const() * mult;

    n->constant[input_pos][output_pos] = 1;
    n->zero[input_pos][output_pos] = ((*e).get_const()==0 && symbolics==0);

    if (symbolics > 0)
        {
        for (Constr_Vars_Iter c(*e, 0); c ; c++)
            {
            Variable_ID vi = (*c).var;
            if (vi->kind() == Global_Var)
                {
                Global_Var_ID gvi = vi->get_global_var();
                int p = uniform_global_list.index(gvi);
                if (p == 0) uniform_global_list.append(gvi);
                p = uniform_global_list.index(gvi);
                n->coefs[input_pos][output_pos][p] = c.curr_coef() * mult;
                }
            }
        }
    } /* add_edge */



static void simple_query_diff(int stmt1, int stmt2, Relation &R, 
                              extended_dir_list *n)
    {
    for (int i=0; i<=stmt_info[stmt1+1].nest+stmt_info[stmt2+1].nest; i++)
        for (int j=0; j<=stmt_info[stmt1+1].nest+stmt_info[stmt2+1].nest; j++)
            {
            n->constant[i][j] = 0;
            n->zero[i][j] = 0;
            }

    n->constant[0][0] = 1;
    n->zero[0][0] = 1;
    n->coefs[0][0][0] = 0;

    for (EQ_Iterator e(R.single_conjunct()); e; e++)
        {
        int inputs_outputs = 0, syms = 0, other = 0;
        Variable_ID var1 = NULL, var2 = NULL;
        int posn1 = 0, posn2 = 0;
        int neg1 = 0, neg2 = 0;
        for (Constr_Vars_Iter c(*e, 0); c ; c++)
            {
            Variable_ID vi = (*c).var;
            if (vi->kind() == Input_Var || vi->kind() == Output_Var)
                {
                if (inputs_outputs == 0)
                    {
                    var1 = vi;
                    if (vi->kind() == Input_Var)
                        {
                        posn1 = vi->get_position();
                        neg1 = was_negative[stmt1][vi->get_position()];
                        }
                    else
                        {
                        posn1 = R.n_inp()+vi->get_position();
                        neg1 = was_negative[stmt2][vi->get_position()];
                        }
                    }
                else if (inputs_outputs == 1)
                    {
                    var2 = vi;
                    if (vi->kind() == Input_Var)
                        {
                        posn2 = vi->get_position();
                        neg2 = was_negative[stmt1][vi->get_position()];
                        }
                    else
                        {
                        posn2 = R.n_inp()+vi->get_position();
                        neg2 = was_negative[stmt2][vi->get_position()];
                        }
                    }
                inputs_outputs++;
                }
            else if (vi->kind() == Global_Var)
                syms++;
            else
                other++;
            }

        if (1 <= inputs_outputs && inputs_outputs <= 2 && other == 0)
            {
            int coef1 = 0, coef2 = 0;
            if (inputs_outputs == 1)
                {
                coef1 = (*e).get_coef(var1);
                if (abs(coef1) == 1)
                    {
                    add_edge(0, posn1, e, -coef1, syms, n);
                    add_edge(posn1, 0, e, coef1, syms, n);
                    }
                }
            else if (inputs_outputs == 2)
                {
                coef1 = (*e).get_coef(var1) *(neg1?-1:1);
                coef2 = (*e).get_coef(var2) *(neg2?-1:1);
                if (abs(coef1) == 1 && coef1 == - coef2)
                    {
                    add_edge(posn1, posn2, e, coef1, syms, n);
                    add_edge(posn2, posn1, e, -coef1, syms, n);
                    }
                }
            }
        }

  for (int k=0; k<=stmt_info[stmt1+1].nest+stmt_info[stmt2+1].nest; k++)
    for (int i=0; i<=stmt_info[stmt1+1].nest+stmt_info[stmt2+1].nest; i++)
        for (int j=0; j<=stmt_info[stmt1+1].nest+stmt_info[stmt2+1].nest; j++)
            if (n->constant[i][k] && n->constant[k][j])
                {
                if (!n->constant[i][j])
                    {
                    n->constant[i][j] = 1;
                    n->zero[i][j] = 1;

                    for (int kk=0; kk<maxVars; kk++)
                        {
                        n->coefs[i][j][kk]=
                            n->coefs[k][j][kk] + n->coefs[i][k][kk];
                        n->zero[i][j] = 
                            n->zero[i][j] && n->coefs[i][j][kk]==0;
                        }
                    }
                }
    } /* simple_query_diff */




void compute_reduction_volumes() 
    {
    for (int i=0; i<nr_statements; i++)
        {
        Access *lhs = AccessCurr(AccessLeftMost(stmt_info[i+1].stmt));
        if (AccessIsUpdate(lhs))
            {
            reduction_vol[i] = VarDimension(AccessVar(lhs));
	    }
        }
    }


// Handle reductions as follows:
//     value-based flows from constant initialization -> reductions: ndim = 0 
//     reductions: ndim = (#loops - dim(lhs)) + dim(rhs), pdim = 1
//     flow from reduction: ndim = (#loops - dim(lhs)) + dim(rhs)

void compute_dimension(Depend *dd, int stmt1, int stmt2, 
                      Conjunct *c, Relation &dep_rel, int& ndim, int& pdim) 
    {
    if(DependIsReduction(dd))
        {
        if (stmt1 != stmt2)
	    {
#if 0
	    fprintf(uniform_debug, "reduction different\n");
#endif
            pdim = 0;
            ndim = 0;
            return;
            } 
        else 
	    {
#if 0
	    fprintf(uniform_debug, "reduction same\n");
#endif
            ndim = reduction_vol[stmt1];
            pdim = 1;
            return;
            }
        }

    if (DependIsFlow(dd) && AccessIsUpdate(DependSrc(dd)))
	{
#if 0
	    fprintf(uniform_debug, "reduction use\n");
#endif
        pdim = 0;
        ndim = reduction_vol[stmt1];
        return;
        }
        
#if 0
	    fprintf(uniform_debug, "non reduction\n");
#endif
    pdim = 0;
    int ndim_all, ndim_domain;
    c->calculate_dimensions(dep_rel, ndim_all, ndim_domain);
    ndim = ndim_domain;
    return;
}


void add_extended_direction(Depend *dd,
                            int stmt1, int stmt2, extended_dir_list **start, 
                            int store, NOT_CONST Relation &input_dep_rel)
    {
    Relation dep_rel = consume_and_regurgitate(input_dep_rel);
    for (DNF_Iterator s(dep_rel.query_DNF()); s; s++)
        {
        int i;
        Relation this_part(dep_rel, s.curr());

        extended_dir_list *n = new extended_dir_list;
        n->nest1 = stmt_info[stmt1+1].nest;
        n->nest2 = stmt_info[stmt2+1].nest;
        for (i = 0; i<=n->nest1; i++)
            for (int j = 0; j<=n->nest2; j++)
                for (int k = 0; k<maxVars; k++)
                    n->coefs[i][j][k] = 0;

        if (store)
            {
            compute_dimension(dd, stmt1, stmt2, *s, dep_rel, n->dim, n->pdim);
            assert(n->pdim == 1 || n->pdim == 0);
            }

        extended_dir_list nn;

        nn.nest1 = stmt_info[stmt1+1].nest;
        nn.nest2 = stmt_info[stmt2+1].nest;
        for (i = 0; i<=nn.nest1+nn.nest2; i++)
            for (int j = 0; j<=nn.nest1+nn.nest2; j++)
                for (int k = 0; k<maxVars; k++)
                    nn.coefs[i][j][k] = 0;

        simple_query_diff(stmt1, stmt2, this_part, &nn); 

        for (int n1 = 0; n1<=stmt_info[stmt1+1].nest; n1++)
            {
            n->constant[n1][0] = nn.constant[0][n1];
            n->zero[n1][0] = nn.zero[0][n1];
            if (n->constant[n1][0])
                for (int k = 0; k < maxVars; k++)
                    n->coefs[n1][0][k] = -nn.coefs[0][n1][k];

            for (int n2 = 1; n2<=stmt_info[stmt2+1].nest; n2++)
                {
                n->constant[n1][n2] = 
                    nn.constant[n1][stmt_info[stmt1+1].nest+n2];
                n->zero[n1][n2] = nn.zero[n1][stmt_info[stmt1+1].nest+n2];
                if (n->constant[n1][n2] || n->zero[n1][n2])
                    for (int k = 0; k < maxVars; k++)
                        n->coefs[n1][n2][k] = 
                            nn.coefs[n1][stmt_info[stmt1+1].nest+n2][k];
                }
            }

        if (uniform_args.trace_uniform)
            {
            fprintf(uniform_debug,"%d->%d\ndim = %d, pdim = %d, extended dep dis = ",
                stmt1, stmt2, n->dim,n->pdim);
            this_part.print_with_subs(uniform_debug);
            print_extended_dir(n);
            fprintf(uniform_debug, "\n");
            }

        n->next = *start;
        *start = n;
        }
    } /* add_extended_direction */


void initialize_extended_dirs(Dynamic_Array2<extended_dir_list *> 
                              &extended_dir_array) {
    extended_dir_array.resize(nr_statements, nr_statements);
    for (int i=0; i<nr_statements; i++)
        for (int j=0; j<nr_statements; j++)
            extended_dir_array[i][j] = NULL;
}


void add_extended_dirs(Dynamic_Array2<extended_dir_list *> 
                       &extended_dir_array)
    {
    for (int i = 1; i<= nr_statements; i++)
        {
        for (AccessIterator ai = AccessLeftMost(stmt_info[i].stmt);
             !AccessLast(ai);
             AccessNext(ai))
            {
            Access *n = AccessCurr(ai);
            for(DependIterator di = DependOutgoing(n);
                !DependLast(di);
                DependNext(di))
                {
                Depend *dd = DependCurr(di);

                if (AccessIsEntry(DependSrc(dd)) ||
                    AccessIsExit(DependDest(dd))) 
                    continue;

                int p1 = StmtNr(DependSrc(dd));
                int p2 = StmtNr(DependDest(dd));
		assert(p1 != -1 && p2 != -1);

                if ((DependIsFlow(dd) && DependIsValue(dd)) ||
                    (DependIsReduction(dd)))
                    {
                    add_extended_direction(dd, p1, p2, 
                                           &extended_dir_array[p1][p2],
                                           1, copy(*DependRelation(dd)));
                    }
                }
            }
        }
    } /* add_extended_dirs */

}
