/* $Id: wak_codegen.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#define NEED_PARALLEL
#define NEED_SYNC

#include <stdio.h>
#include <ctype.h>
#include <omega.h>
#include <omega/Relations.h>
#include <uniform/uniform_misc.h>
#include <uniform/depend_dir.h>
#include <uniform/space_mappings.h>
#include <uniform/space_constants.h>
#include <uniform/time_mappings.h>
#include <uniform/wak_codegen.h>
#include <uniform/simple_codegen.h>
namespace omega {

int is_loop_dist[max_stmts][maxLevels];
int is_const_dist[max_stmts][maxLevels];


static String outputSpaces(int indent_level) 
    {
    String spaces = "";
    for(int i=0; i<=indent_level; i++) 
        spaces += "    ";
    return spaces;
    }


static String replace_vars(const char *my_stmt, Relation global_time)
    {
    String S = "";
    int len = strlen(my_stmt);
    for (int i=0; i<len; i++)
        if (my_stmt[i] != '#')
            {
            char s[2];
            s[0] = my_stmt[i];
            s[1] = '\0';
            S += String(s);
            }
        else
            {
            int p = my_stmt[++i] - '0';
            S += global_time.print_outputs_with_subs_to_string(p);
            }
    return S;
    } /* replace_vars */


Relation wak_project_onto_levels(Relation R, int last_level, bool wildcards) 
    {
    assert(last_level >= 0 && R.is_set() && last_level <= R.n_set());
    if (last_level == R.n_set()) return R;

    int orig_vars = R.n_set();
    int num_projected = orig_vars - last_level;
    R = Extend_Set(R,num_projected);
    Mapping m1 = Mapping::Identity(R.n_set());

    for(int i=last_level+1; i <= orig_vars; i++) 
        {
        m1.set_map(Set_Var, i, Exists_Var, i);
        m1.set_map(Set_Var, i+num_projected, Set_Var, i);
        }

    MapRel1(R, m1, Comb_Id);
    R.finalize();
    if (!wildcards)
        R = Approximate(R,1);
    R.simplify(1,1);
    assert(R.is_set());
    return R;
    } /* wak_project_onto_levels */



String outputEasyBound(const GEQ_Handle &g, Variable_ID v, int lower_bound)
    {
    String s = "";
    bool first = true;

    int v_coef;
    if (v)
        v_coef = g.get_coef(v);
    else
        v_coef = -1;

    int v_sign = v_coef > 0 ? 1 : -1;
    v_coef *= v_sign;
    int sign_adj = -v_sign;

    for (int j=1; j<=g.relation()->n_set(); j++)
        g.relation()->name_set_var(j, "t" + itoS(j));
    g.relation()->setup_names();

    if (v_coef != 1)
	s += "(";

    for(Constr_Vars_Iter c2(g,false); c2; c2++) 
        {
        if ((*c2).var != v && (*c2).var->kind()!=Wildcard_Var)
            {
            int cf = (*c2).coef*sign_adj;
            if (cf > 1) 
                {
                if (!first) s += "+";
                s += itoS( cf );
                }
            else if (cf == 1) 
                {
                if (!first) s += "+";
                 }
            else if (cf == -1) 
                {
                s += "-";
                }
            else if (cf < -1) 
                {
                s += "-";
                s += itoS( -cf );
                }

            if (cf != 1 && cf != -1)
                s += "*";
            s += (*c2).var->name();
            first = false;
            }
        }

    if (g.get_const()) 
        {
        int cf = g.get_const()*sign_adj;
        if (first && cf > 0)
            s += itoS(cf);
        else if(cf > 0)
            s += "+" + itoS(cf);
        else
            s += "-" + itoS(-cf);
        }
    else
        if (first) 
            s += "0";

    if (v_coef != 1)
	{
	if (lower_bound)
	    {
	    s += "+";
	    s += itoS(v_coef-1);
	    }
	s += ")";
	s += "/";
	s += itoS(v_coef);
	}

    return s;
    } /* outputEasyBound */



static String lower_bound(Conjunct *c, Variable_ID v, Relation &known, 
                          int dist, int back)
    {
    String s = "";
    int lower_bounds = 0;
    int seenLB = 0;

    for (GEQ_Iterator g1(c); g1; g1++) 
        if ((*g1).get_coef(v) > 0) 
            lower_bounds++;

    assert(lower_bounds > 0);

    if (lower_bounds > 1 || dist == 1)
        s += "max(";

    for (GEQ_Iterator g(c); g; g++) 
        if ((*g).get_coef(v) > 0) 
            {
            if (seenLB) 
                s += ",";
            s += outputEasyBound(*g, v, 1);
            known.and_with_GEQ(*g);
            seenLB = true;
            }

#if defined NEED_PARALLEL
    if (dist == 1)
	{
        s += ",";
	if (back)
	    s += "-ub";
	else
            s += "lb";
	}
#endif

    if (lower_bounds > 1 || dist == 1)
        s += ")";

#if defined NEED_PARALLEL
    if (dist == 2)
        {
        if (s == String("0") || s == String("1"))
            s = "max(" + s + ",_my_id)";
        else
            s = "adjust(" + s + ")";
        }
#endif

    return s;
    } /* lower_bound */


static String upper_bound(Conjunct *c, Variable_ID v, Relation &known, 
                          int dist, int back)
    {
    String s = "";
    int upper_bounds = 0;
    int seenUB = 0;

    for (GEQ_Iterator g1(c); g1; g1++)
        if ((*g1).get_coef(v) < 0)
            upper_bounds++;

    assert(upper_bounds > 0);

    if (upper_bounds > 1 || dist == 1)
        s += "min(";

    for (GEQ_Iterator g(c); g; g++)
        if ((*g).get_coef(v) < 0)
            {
            if (seenUB)
                s += ",";
            s += outputEasyBound(*g, v, 0);
            known.and_with_GEQ(*g);
            seenUB = true;
            }

#if defined NEED_PARALLEL
    if (dist == 1)
	{
        s += ",";
	if (back)
	    s += "-lb";
	else
            s += "ub";
	}
#endif

    if (upper_bounds > 1 || dist == 1)
        s += ")";

    return s;
    } /* upper_bound */



static int which_group(int level, Tuple<int> active)
    {
    for (int i = 0; i<nr_statements; i++)
	if (active[i+1])
	    {
	    return group[level][i];
	    }
    assert(0);
    return 0;
    } /* which_group */

static Tuple<Variable *> var_list[maxLevels];

static String local_declarations()
    {
    String local_decls = "";
    for (int level=0; level<maxLevels; level++)
        for (int i=1; i<=var_list[level].size(); i++)
	    {
	    char *rest;
	    String local;
            char decl[80];
	    Variable *v = var_list[level][i];
    
	    strcpy(decl, (const char *) VarDecl(v));
    
	    int name_length = strlen(VarName(v));
	    String dims = "";
	    if (decl[0] == 'i') /* int var[...] */
	        {
	        rest = decl + 4 + name_length;
		if (VarDimension(v) > 0) dims = String(rest);
	        local = String("    int _") + String(VarName(v)) + 
		        itoS(level) + dims + String(";\n");
	        }
	    else /* double var[...] */
	        {
	        rest = decl + 7 + name_length;
		if (VarDimension(v) > 0) dims = String(rest);
	        local = String("    double _") + String(VarName(v)) + 
			itoS(level) + dims + String(";\n");
	        }
    
	    local_decls += local;
	    }
    return local_decls;
    } /* local_declarations */


static String reduce_initialize(int indent, 
				Tuple<Variable *> vlist, 
				Tuple<String> olist, 
				int level)
    {
    String s = "";
    for (int i=1; i<=vlist.size(); i++)
        {
        int rest;
        String local = outputSpaces(indent);
        Variable *v = vlist[i];
        char decl[80];
 
        strcpy(decl, (const char *) VarDecl(v));
 
        int name_length = strlen(VarName(v));
	String type;
        if (decl[0] == 'i') /* int var[...] */
	    {
	    type = "int";
            rest = 4 + name_length;
	    }
        else
            {
	    type = "double";
            rest = 7 + name_length;
	    }

        local += "init_";
        local += olist[i];
	local += "_";
	local += type;
	local += "(";
	if (VarDimension(v) == 0)
	    local += "&";
	local += "_";
	local += VarName(v);
	local += itoS(level);
	local += ", ";
	
	int first = 1;
	int len = strlen(decl);
	for (int p=rest; p<len; p++)
	    {
	    char ch = decl[p];
	    if (ch == '[') 
		{
		if (!first) local += "*";
		if (VarDimension(v) > 1)
		    local += "(";
		first = 0;
		}
	    else if (ch == ']') 
		{
		if (VarDimension(v) > 1)
		    local += ")";
		}
            else
		local += String(ch);
	    }
        if (first)
	    local += "1";
        local += ");\n";
	s += local;
        }
    return s;
    } /* reduce_initialize */


static String reduce_finalize(int indent, 
			      Tuple<Variable *> vlist, 
			      Tuple<String> olist, 
			      int level)
    {
    String s = outputSpaces(indent);
    s += "suif_reduction_lock(0);\n";
    for (int i=1; i<=vlist.size(); i++)
        {
        int rest;
        String local = outputSpaces(indent);
        Variable *v = vlist[i];
        char decl[80];
 
        strcpy(decl, (const char *) VarDecl(v));
 
        int name_length = strlen(VarName(v));
	String type;
        if (decl[0] == 'i') /* int var[...] */
	    {
	    type = "int";
            rest = 4 + name_length;
	    }
        else
            {
	    type = "double";
            rest = 7 + name_length;
	    }

        local += "reduce_";
	local += olist[i];
	local += "_";
	local += type;
	local += "(";
	if (VarDimension(v) == 0)
	    local += "&";
	local += VarName(v);
	local += ", ";
	if (VarDimension(v) == 0)
	    local += "&";
	local += "_";
	local += VarName(v);
	local += itoS(level);
	local += ", ";
	
	int first = 1;
	int len = strlen(decl);
	for (int p=rest; p<len; p++)
	    {
	    char ch = decl[p];
	    if (ch == '[') 
		{
		if (!first) local += "*";
		if (VarDimension(v) > 1)
		    local += "(";
		first = 0;
		}
	    else if (ch == ']') 
		{
		if (VarDimension(v) > 1)
		    local += ")";
		}
            else
		local += String(ch);
	    }
        if (first)
	    local += "1";
        local += ");\n";
	s += local;
        }
    s += outputSpaces(indent);
    s += "suif_reduction_unlock(0);\n";
    return s;
    } /* reduce_finalize */


static String output_constant_level(int level, int indent,Tuple<int> &active,
                                    Relation known);

static String output_variable_level(int level, int indent, Tuple<int> &active,
                                    Relation &known)
    {
#if 0
    fprintf(uniform_debug, "output_variable_level(level %d, indent %d, active ",
            level, indent);
    for (int jj=1; jj<=nr_statements; jj++)
        if (active[jj])
            fprintf(uniform_debug, "%d ", jj);
    fprintf(uniform_debug, ")\n");
#endif

    Relation new_known = known;
    String s = "";
    Tuple<Relation> projected_nIS(nr_statements);

    int min_pos = MAXINT, max_pos = -MAXINT;

    int dist = 0;
    int back = 0;
    int stmt = -1;
    Tuple<Variable *> loop_var_list(0);
    Tuple<String> loop_op_list(0);
    for (int j=1; j<=nr_statements; j++)
        if (active[j])
            {
            Relation new_IS = 
                Range(Restrict_Domain(copy(time0[j-1]), 
                                      copy(stmt_info[j].bounds)));
            projected_nIS[j] = wak_project_onto_levels(new_IS, level, 0);
            min_pos = min(min_pos, time_coef[level+1][j-1]);
            max_pos = max(max_pos, time_coef[level+1][j-1]);
#if defined NEED_PARALLEL
	    if (is_loop_dist[j-1][level])
		{
                dist = is_loop_dist[j-1][level];
                back = neg_space[j-1];
		stmt = j-1;
		if (parallel_reduction[j-1])
		    {
		    Access *lhs = AccessCurr(AccessLeftMost(stmt_info[j].stmt));
		    Variable *sym = AccessVar(lhs);
		    if (var_list[(level-2)/2].index(sym) == 0)
		        var_list[(level-2)/2].append(sym);
		    if (loop_var_list.index(sym) == 0)
			{
		        loop_var_list.append(sym);

	                if (AssignIsSum(stmt_info[j].stmt))
			    loop_op_list.append("sum");
	                if (AssignIsProduct(stmt_info[j].stmt))
			    loop_op_list.append("product");
	                if (AssignIsMax(stmt_info[j].stmt))
			    loop_op_list.append("max");
	                if (AssignIsMin(stmt_info[j].stmt))
			    loop_op_list.append("min");
			}
		    }
		}
#endif
            }

    Relation hull = Hull(projected_nIS, active,1);

    hull = Gist(hull, known);

    Conjunct *c = hull.query_DNF()->single_conjunct();

    Variable_ID v = set_var(level);

    int needLoop = 1;
    for(EQ_Iterator eq(c); eq.live();eq++) 
        {
        if ((*eq).get_coef(v)) 
            {
            for(Constr_Vars_Iter w(*eq, 1); w; w++)
                if ((*w).coef != 0) goto nextEQ;
            new_known.and_with_EQ(*eq);
            needLoop = 0;
            }
        nextEQ: ;
        }

    if (needLoop)
        {
        int x;
#if defined NEED_SYNC
	int g = which_group(level/2, active);
        for (x=1; x<=wait_required[level][g].size(); x++)
	    {
	    assert(wait_required[level][g][x] == -1);
	    int reversed = post_wait_rev[level][g][x];

   	    if (dist == 1 && !reversed)
	        {
                s += outputSpaces(indent);
	        s += "if (_my_id > 0)\n";
                s += outputSpaces(indent+1);
	        s += "{\n";
                s += outputSpaces(indent+1);
	        s += "suif_counter_wait(_my_id-1,0,_counter0);\n";
                s += outputSpaces(indent+1);
	        s += "_counter0++;\n";
                s += outputSpaces(indent+1);
	        s += "}\n";
	        }

   	    if (dist == 1 && reversed)
	        {
                s += outputSpaces(indent);
	        s += "if (_my_id < _my_nprocs-1)\n";
                s += outputSpaces(indent+1);
	        s += "{\n";
                s += outputSpaces(indent+1);
	        s += "suif_counter_wait(_my_id+1,1,_counter1);\n";
                s += outputSpaces(indent+1);
	        s += "_counter1++;\n";
                s += outputSpaces(indent+1);
	        s += "}\n";
	        }
	    }

	if (loop_var_list.size() > 0)
	    {
	    s += reduce_initialize(indent, loop_var_list, loop_op_list,
				   (level-1)/2);
	    }
#endif

        String variable_name = "t" + itoS(level);
        s += outputSpaces(indent);
        s += "for (";
        s += variable_name;
        s += " = ";
        s += lower_bound(c, v, new_known, dist, back);
        s += "; ";
        s += variable_name;
        s += " <= ";
        s += upper_bound(c, v, new_known, dist, back);
        s += "; ";
        s += variable_name;
#if defined NEED_PARALLEL
        if (dist == 2)
            s += " += _my_nprocs)\n";
        else
#endif
            s += "++)\n";

        s += outputSpaces(indent+1);
        s += "{\n";

#if defined NEED_SYNC
	g = which_group(level/2, active);
        for (x=1; dist != 1 && x<=wait_required[level][g].size(); x++)
	    {
	    assert(wait_required[level][g][x] == -1);

            if (post_wait_rev[level][g][x])
	        {
                s += outputSpaces(indent+1);
	        s += "if (_my_id < _my_nprocs-1)\n";
                s += outputSpaces(indent+2);
	        s += "{\n";
                s += outputSpaces(indent+2);
	        s += "suif_counter_wait(_my_id+1,1,_counter1);\n";
                s += outputSpaces(indent+2);
	        s += "_counter1++;\n";
                s += outputSpaces(indent+2);
	        s += "}\n";
	        }
            else
	        {
                s += outputSpaces(indent+1);
	        s += "if (_my_id > 0)\n";
                s += outputSpaces(indent+2);
	        s += "{\n";
                s += outputSpaces(indent+2);
	        s += "suif_counter_wait(_my_id-1,0,_counter0);\n";
                s += outputSpaces(indent+2);
	        s += "_counter0++;\n";
                s += outputSpaces(indent+2);
	        s += "}\n";
	        }
	    }
#endif

        s += output_constant_level(level+1, indent+1, active, new_known);

#if defined NEED_SYNC
	g = which_group(level/2, active);
	for (x=1; 
	     dist != 1 && x<=post_required[level][g].size(); 
	     x++)
	    {
	    assert(post_required[level][g][x] == -1);

            s += outputSpaces(indent+1);
            if (post_wait_rev[level][g][x])
	        s += "suif_counter_incr(_my_id,1);\n";
	    else
	        s += "suif_counter_incr(_my_id,0);\n";
	    }
#endif

        s += outputSpaces(indent+1);
        s += "}\n";

#if defined NEED_SYNC
	if (loop_var_list.size() > 0)
	    {
	    s += reduce_finalize(indent, loop_var_list, loop_op_list,
				 (level-1)/2);
	    }

	g = which_group(level/2, active);
	for (x=1; 
	     dist == 1 && x<=post_required[level][g].size(); 
	     x++)
	    {
	    assert(post_required[level][g][x] == -1);

            s += outputSpaces(indent);
            if (post_wait_rev[level][g][x])
	        s += "suif_counter_incr(_my_id, 1);\n";
	    else
	        s += "suif_counter_incr(_my_id, 0);\n";
            }
#endif
        }
    else
        s += output_constant_level(level+1, indent, active, new_known);
    
    return s;
    } /* output_variable_level */


#if defined NEED_SYNC
static void reduction_expansion(String &stmt, int p1)
    {
    String var = "";
    int p = 0;
    char ch = stmt[p++];
    while (isalnum(ch) || ch == '_' || ch == '$' || ch == '#')
	{
	var += String(ch);
        ch = stmt[p++];
	}

    int d_level = dist_loop[p1][best_time[p1]];

    String new_stmt = "_" + var + itoS(d_level);

    while (p <= stmt.length())
	{
	new_stmt += String(ch);
	ch = stmt[p++];
	}

    stmt = new_stmt;
    } /* reduction_expansion */
#endif


static String output_constant_level(int level, int indent, Tuple<int> &active,
                                    Relation known)
    {
#if 0
    fprintf(uniform_debug, "output_constant_level(level %d, indent %d, active ",
            level, indent);
    for (int jj=1; jj<=nr_statements; jj++)
        if (active[jj])
            fprintf(uniform_debug, "%d ", jj);
    fprintf(uniform_debug, ")\n");
#endif

    String s = "";
    Tuple<int> new_active(max_stmts);
    int active_found = 1;
    int this_group = which_group((level-1)/2, active);

    int posn;
    for (posn=0; active_found ; posn++)
        {
        Relation new_known = known;
        int if_indent  = 0;
        active_found = 0;
        for (int j=1; j<=nr_statements; j++)
            {
            new_active[j] = (active[j] && time_coef[level][j-1] == posn);
            if (new_active[j])
		{
                active_found = 1;
		}
            }

        if (active_found)
            {
            int x;
#if defined NEED_SYNC
	    if (barrier_required[level][this_group].index(posn) != 0)
		{
                s += outputSpaces(indent);
		s += "suif_global_barrier(0);\n";
		}

	    for (x=1; x<=wait_required[level][this_group].size(); x++)
		{
		if (wait_required[level][this_group][x] != posn) continue;

		if (post_wait_rev[level][this_group][x])
		    {
                    s += outputSpaces(indent);
	            s += "if (_my_id < _my_nprocs-1)\n";
                    s += outputSpaces(indent+1);
	            s += "{\n";
                    s += outputSpaces(indent+1);
	            s += "suif_counter_wait(_my_id+1,1,_counter1);\n";
                    s += outputSpaces(indent+1);
	            s += "_counter1++;\n";
                    s += outputSpaces(indent+1);
	            s += "}\n";
		    }
		else
		    {
                    s += outputSpaces(indent);
	            s += "if (_my_id > 0)\n";
                    s += outputSpaces(indent+1);
	            s += "{\n";
                    s += outputSpaces(indent+1);
	            s += "suif_counter_wait(_my_id-1,0,_counter0);\n";
                    s += outputSpaces(indent+1);
	            s += "_counter0++;\n";
                    s += outputSpaces(indent+1);
	            s += "}\n";
		    }
		}
#endif

            EQ_Handle eq = new_known.and_with_EQ();
            eq.update_coef(set_var(level), -1);
            eq.update_const(posn);

            Tuple<Relation> projected_nIS(nr_statements);

#if defined NEED_PARALLEL
            int dist_const = -1;
            int d_level = -1;
            int dist = -1;
            int stmt = -1;
	    int back = -1;
#endif

            for (int j=1; j<=nr_statements; j++)
                if (new_active[j])
                    {
                    Relation new_IS = 
                        Range(Restrict_Domain(copy(time0[j-1]), 
                                              copy(stmt_info[j].bounds)));
                    projected_nIS[j] = wak_project_onto_levels(new_IS, level-1, 0);

#if defined NEED_PARALLEL
                    dist = is_loop_dist[j-1][level];
                    d_level = dist_loop[j-1][best_time[j-1]] * 2 + 2;
                    back = neg_space[j-1];
                    dist_const = is_const_dist[j-1][level];
                    stmt = j;
#endif
                    }

            Relation new_hull = Hull(projected_nIS, new_active,1);

            Relation guard_rel = Gist(new_hull, copy(known));
            guard_rel.simplify();
            guard_rel = EQs_to_GEQs(guard_rel, true);
            Conjunct *c = guard_rel.query_DNF()->single_conjunct();
            int seen = 0;

            if (! guard_rel.is_obvious_tautology() 
#if defined NEED_PARALLEL
		|| dist > 0 || dist_const
#endif
		)
                {
                String guard = "";
                for (GEQ_Iterator g(c); g; g++)
		    {
                    for (int lvl = level-1; lvl>0; lvl = lvl - 2)
                        {
                        Variable_ID v = set_var(lvl);
                        String variable_name = "t" + itoS(lvl);

                        if ((*g).get_coef(v) != 0)
                            {
                            if (seen) guard += " && ";
                            seen = 1;
                            new_known.and_with_GEQ(*g);
                            }

                        if ((*g).get_coef(v) < 0)
                            {
                            guard += variable_name;
                            guard += " <= ";
                            guard += outputEasyBound(*g, v, 0);
			    break;
                            }

                        if ((*g).get_coef(v) > 0)
                            {
                            guard += outputEasyBound(*g, v, 1);
                            guard += " <= ";
                            guard += variable_name;
			    break;
                            }
                        }
	            if (! seen && level == time0[0].n_out())
			{
			new_known.and_with_GEQ(*g);
			guard += "0 <=";
		 	guard += outputEasyBound(*g, NULL, 1);
			seen = 1;
			}
		    }

#if defined NEED_PARALLEL
                if (dist > 0)
                    {
                    String variable_name = "t" + itoS(d_level);

                    if (dist == 1)
                        {
                        if (seen) guard += " && ";
                        seen = 1;
                        guard += "lb <= ";
			if (back)
			    guard += "-";
                        guard += variable_name;
                        guard += " && ";
			if (back)
			    guard += "-";
                        guard += variable_name;
                        guard += " <= ub";
                        }

                    if (dist == 2)
                        {
                        if (seen) guard += " && ";
                        seen = 1;
			if (back)
			    guard += "-";
                        guard += variable_name;
                        guard += " % _my_nprocs == _my_id";
                        }
                    }

                if (dist_const)
                    {
		    String con = "";
                    int first = 1;
                    for (int j = 1; j<=uniform_global_list.size(); j++)
                        {
                        int coef = best_coef[stmt-1][j];
                        if (coef != 0)
                            {
                            if (!first) con += " + ";
                            con += itoS(coef);
                            con += "*";
                            con += String((const char *) uniform_global_list[j]->base_name());
                            first = 0;
                            }
                        }
                    int coef = best_coef[stmt-1][0];
                    if (coef != 0 || first)
                        {
                        if (!first) con += " + ";
                        con += itoS(coef);
                        }
                    if (seen) guard += " && ";
                    seen = 1;

		    if (dist_const == 1)
                        guard += "lb <= " + con + " && " + con + " <= ub";
		    else
                        guard += con + "% _my_nprocs == _my_id";
                    }
#endif

                if (seen)
                    {
                    s += outputSpaces(indent);
                    s += "if (";
                    s += guard;
                    s += ")\n";
                    if_indent = 1;
                    s += outputSpaces(indent+if_indent);
                    s += "{\n";
                    }
                }
    
            if (level < time0[0].n_out())
                s+=output_variable_level(level+1,indent+if_indent,new_active,
                                         new_known);
            else
                {
                for (int j=1; j<=nr_statements; j++)
                    if (new_active[j])
                        {
                        String ASS = AssignStmtString(stmt_info[j].stmt); 
#if defined NEED_SYNC
			if (parallel_reduction[j-1])
			    reduction_expansion(ASS, j-1);
#endif
                        Relation Inv_mapping = Inverse(copy(time0[j-1]));
                        for (int k=1; k<=Inv_mapping.n_inp(); k++)
                            Inv_mapping.name_input_var(k, "t" + itoS(k));
                        s += outputSpaces(indent+if_indent);
                        s += replace_vars((const char *)ASS,Inv_mapping);
                        s += "\n";

                        Relation new_IS = 
                            Range(Restrict_Domain(copy(time0[j-1]), 
                                                  copy(stmt_info[j].bounds)));

                        Relation R1 = Gist(copy(new_IS), 
                                           copy(new_known),1);
			if (!R1.is_obvious_tautology())
			    {
			    fprintf(stderr, 
				    "statement %d: under constrainted\n",
				    j);
			    R1.print_with_subs(stderr);
		            assert(0);
			    }
				    
                        Relation R2 = Gist(copy(new_known),
                                           copy(new_IS),1);
			if (!R2.is_obvious_tautology())
			    {
			    fprintf(stderr, 
				    "statement %d: over constrainted\n",
				    j);
			    R2.print_with_subs(stderr);
		            assert(0);
			    }
                        }
                }

            if (seen)
                {
                s += outputSpaces(indent+if_indent);
                s += "}\n";
	        }

#if defined NEED_SYNC
	    for (x=1; x<=post_required[level][this_group].size(); x++)
		{
		if (post_required[level][this_group][x] != posn) continue;

                s += outputSpaces(indent);
		if (post_wait_rev[level][this_group][x])
		    s += "suif_counter_incr(_my_id, 1);\n";
		else
		    s += "suif_counter_incr(_my_id, 0);\n";
		}
#endif
            }
        }

#if defined NEED_SYNC
    if (barrier_required[level][this_group].index(posn) != 0)
	{
        s += outputSpaces(indent);
	s += "suif_global_barrier(0);\n";
	}
#endif

    return s;
    } /* output_constant_level */



String WakGenerateCode(String init)
    {
    Tuple<int> active(max_stmts);
    for (int k=0; k<maxLevels; k++)
	var_list[k].clear();

    for (int j=1; j<=time0[0].n_out(); j++)
        time0[0].name_output_var(j, "t" + itoS(j));

    for (int i=1; i<=nr_statements; i++)
        active[i] = 1;

    String s =
	output_constant_level(1,0,active,Relation::True(time0[0].n_out()));

    return local_declarations() + init + s;
    } /* WakGenerateCode */
}
