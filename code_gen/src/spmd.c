/* $Id: spmd.c,v 1.1.1.1 2004/09/13 21:07:47 mstrout Exp $ */
#include <basic/bool.h>
#include <code_gen/code_gen.h>
#include <code_gen/spmd.h>
#include <omega/Relations.h>
#include <omega/hull.h>
#include <omega/Rel_map.h>
#include <omega/pres_conj.h>
#include <math.h>
#ifndef WIN32
#include <sys/time.h>
#include <sys/resource.h>
#endif

using namespace omega;

namespace omega {

FILE *cyclic_debug_file = DebugFile;

int overheadEffort = -1;

int gen_dash = 1;
int Block_Size = 0;
int Num_Procs = 0;



String program_head(String Decls, int timeDepth) {
    String s = "";
    if (gen_dash)
	{
	s += "#include <math.h>\n\n";
	s += "#define min(x,y) ((x)<(y)?(x):(y))\n";
	s += "#define max(x,y) ((x)>(y)?(x):(y))\n\n";
	s += "extern void p4_doall(void (*)(int, void *), void *);\n";
	s += "extern int _my_nprocs;\n";
	s += "extern void p4_post(int i, ...);\n";
	s += "extern void p4_wait(int id, int i, ...);\n\n";
	s += "extern int _suif_nproc;\n";
	s += "int _suif_nproc;\n\n";
	s += "struct _BLDR_struct_000 { int _field_0; };\n";
	s += "static struct _BLDR_struct_000 _main_0_struct;\n\n";

	s += Decls;

	s += "static void _main_0_func(int _my_id, void *_my_struct_param)\n";
	s += "    {\n";

        s += "    int global_lb, global_ub;\n";
        s += "    int block_size;\n";
        s += "    int lb, ub;\n";
	}
    else
	{
        s += "#include \"sim-interf.h\"\n";
        if (Block_Size) 
	    s += "int fold(int vp) { return (vp-global_lb)/" 
	      + itoS(Block_Size) 
	      + " % num_procs;}\n";
	else 
	    s += "int fold(int vp) { return (vp-global_lb)/block_size;}\n";

        s += "void program() {\n"; // }
	}

    assert (timeDepth > 0);
    String index_vars = "    int t1";
    for(int ind = 2; ind <= timeDepth; ind++)
	index_vars += ", t" + itoS(ind);
    index_vars += ";\n";

   return s + index_vars;
}

static Relation 
select_output(Relation R, int level) {
    Mapping m1 = Mapping::Identity(R.n_inp(), R.n_out());
    for(int i = 1; i <= R.n_out(); i++)
	if (level != i)
	    m1.set_map_out(i,Exists_Var,i);
    m1.set_map_out(level,Output_Var,1);
    MapRel1(R, m1, Comb_Id);
    return R;
}


static int new_is_constant(RelTuple &T, int member, int level)
    { 
    T[member].uncompress();
    Relation S = select_output(T[member],level);
    T[member].compress();
    EQ_Iterator e = S.single_conjunct()->EQs(); 
    if (!e.live())
	return 0;
    for (Constr_Vars_Iter c(*e, 0); c ; c++)
	if ((*c).var != output_var(1) && (*e).get_coef((*c).var) != 0)
	    return 0;

    e.next();
    return !e.live();
    } /* new_is_constant */


static coef_t new_constant_val(RelTuple &T, int member, int level)
    {
    T[member].uncompress();
    Relation S = select_output(T[member],level);
    T[member].compress();
    EQ_Iterator e = S.single_conjunct()->EQs(); 
    assert(e.live());
    coef_t coef = (*e).get_coef(output_var(1));
    assert(coef == -1 || coef == 1);
    return (*e).get_const() * -coef;
    } /* new_constant_val */

static int nr_groups[40];
static int group[40][40];

// A version of calc. groups that allows you to pass off a tuple of current
// time mappings instead of using all those globals.

void new_calculate_groups(int level, RelTuple &T)
    {
    int nr_statements = T.size();
    nr_groups[level] = nr_groups[level-1];

    for (int g = 0; g < nr_groups[level-1]; g++)
	{
	int all_constant = 1;
	int p;
	for (p = 0; p<nr_statements; p++)
	    if (group[level-1][p] == g && !new_is_constant(T,p+1,level))
		{
		all_constant = 0;
		break;
		}

	for (p = 0; p<nr_statements; p++)
	    if (group[level-1][p] == g)
		{
                if (all_constant)
		    {
	            int found_group = 0;
		    int found_match = 0;
	            for (int q = 0; q<p; q++)
	                if (group[level-1][q] == g && p != q)
			    {
			    found_group = 1;
			    if (new_constant_val(T,p+1,level) == new_constant_val(T,q+1,level))
				{
				group[level][p] = group[level][q];
				found_match = 1;
				break;
				}
			    }

                    if (! found_group)
			group[level][p] = group[level-1][p];

                    if (found_group && ! found_match)
			group[level][p] = nr_groups[level]++;
		    }
                else
		    group[level][p] = group[level-1][p];
		}
	}
    } /* calculate_groups */




static Relation concatenate_outputs(Relation a, Relation b) {
    assert(a.n_inp() == b.n_inp());
    Mapping m1 = Mapping::Identity(a.n_inp(), a.n_out());
    Mapping m2 = Mapping::Identity(b.n_inp(), b.n_out());
    for(int i = 1; i <= b.n_out(); i++)
	m2.set_map(Output_Var, i, Output_Var, i+a.n_out());
    return MapAndCombineRel2(a,b,m1,m2,Comb_And);
}



#if 0
static void
move_input_and_output(int from_input, int from_output, int to, Relation &R) {
// Input or output tuple:
// [i_1,..i_to,..,i_from,..,i_n] becomes [i1,..,ifrom,..,ito,..,i_n]
//                                                       ^^^ Ito, get it?
//                                                       You know, as in Judge?

    assert(from_output > to && from_input > to);
    Mapping m1(R.n_inp(),R.n_out());
    int p;
    for(p=1; p < to; p++) m1.set_map_in(p,Input_Var,p);
    for(p=1; p < to; p++) m1.set_map_out(p,Output_Var,p);

    for(p=to; p < from_input; p++) m1.set_map_in(p,Input_Var,p+1);
    for(p=to; p < from_output; p++) m1.set_map_out(p,Input_Var,p+1);

    m1.set_map(Input_Var, from_input, Input_Var, to);
    m1.set_map(Output_Var, from_output, Output_Var, to);

    for(p=from_input+1; p <= R.n_inp(); p++) m1.set_map_in(p,Input_Var,p);
    for(p=from_output+1; p <= R.n_out(); p++) m1.set_map_out(p,Output_Var,p);

    MapRel1(R,m1,Comb_Id);
}
#endif

void
move_just_output(int from, int to, Relation &R) {
// Output tuple:
// [i_1,..i_to,..,i_from,..,i_n] becomes [i1,..,ifrom,..,ito,..,i_n]
//                                                       ^^^ Ito, get it?
//                                                       You know, as in Judge?

    assert(from > to);
    Mapping m1(R.n_inp(),R.n_out());
    int p;
    for(p=1; p <= R.n_inp(); p++) m1.set_map_in(p,Input_Var,p);

    for(p=1; p < to; p++) m1.set_map_out(p,Output_Var,p);
    for(p=to; p < from; p++) m1.set_map_out(p,Output_Var,p+1);
    m1.set_map(Output_Var, from, Output_Var, to);
    for(p=from+1; p <= R.n_out(); p++) m1.set_map_out(p,Output_Var,p);

    MapRel1(R,m1,Comb_Id);
}


void
move_just_input(int from, int to, Relation &R) {
// Input tuple:
// [i_1,..i_to,..,i_from,..,i_n] becomes [i1,..,ifrom,..,ito,..,i_n]
//                                                       ^^^ Ito, get it?
//                                                       You know, as in Judge?

    if (from == to) return;
    assert(from > to);
    Mapping m1(R.n_inp(),R.n_out());
    int p;
    for(p=1; p <= R.n_out(); p++) m1.set_map_out(p,Output_Var,p);

    for(p=1; p < to; p++) m1.set_map_in(p,Input_Var,p);
    for(p=to; p < from; p++) m1.set_map_in(p,Input_Var,p+1);
    m1.set_map(Input_Var, from, Input_Var, to);
    for(p=from+1; p <= R.n_inp(); p++) m1.set_map_in(p,Input_Var,p);

    MapRel1(R,m1,Comb_Id);
}



static bool level_ok(int level, Relation t, Relation s) {
//  FIX:  Have already extended the space mappings. Need to fix that.
    Relation project_outs = Identity(level);
    project_outs = Extend_Domain(project_outs,t.n_out() - level);
    t = Join(t,project_outs);
    Relation ts = concatenate_outputs(t,s);
    Relation rts = Range(ts), rt = Range(t);
    int rts_rank,rt_rank;
    rts_rank = rts.is_obvious_tautology()? rts.n_set() : rts.single_conjunct()->rank();
    rt_rank = rt.is_obvious_tautology() ? rt.n_set() : rt.single_conjunct()->rank();
    if(rts_rank == rt_rank)
	return true;
    else
	return false;
}




Tuple<int> compute_alphalevels(Tuple<Relation> &time, Tuple<Relation> &space){

    assert(time[1].n_inp() == space[1].n_inp());
    int levels = time[1].n_out();
    int statements = time.size();
    Tuple<int> alphalevels(statements);
    for(int i = 1; i <= statements; i++) alphalevels[i] = -1;
    
    for(int lev = 1 ; lev <= levels; lev++)
	for(int s = 1; s <= statements; s++) {
	    time[s].uncompress(); space[s].uncompress();
	    if(alphalevels[s] < 0 && level_ok(lev,time[s],space[s]))
		alphalevels[s] = lev;
	    time[s].compress(); space[s].compress();
	}
    if(code_gen_debug)
	for(int check = 1; check <= statements; check++) {
	    assert(alphalevels[check] > 0 && alphalevels[check] <= levels);
	    fprintf(cyclic_debug_file,"Statement %d alpha level = %d\n",check,
		    alphalevels[check]);
	}

    return alphalevels;
}


// "[i_1,...,i_n] -> [j_1,...,j_n]" becomes
//     "[i_1,...,i_n,i_{n+1}] -> [j_1,...,j_n,j_{n+1}] : i_{n+1} = j_{n+1}}"
static void
extend_and_equate(Relation &R) {
    R = Extend_Domain(R);
    R = Extend_Range(R);
    EQ_Handle e = R.and_with_EQ();
    e.update_coef(input_var(R.n_inp()), 1);
    e.update_coef(output_var(R.n_out()), -1);
}

// Add another level to each of these relations, so that there is space
// to put in the  level for enumerating cyclic blocks.  We put it last,
// which is always safe, and then we move it in later as an optimization.
static void
extend_mappings(Relation &time,     /* Relation&space, */
		Relation &iteration_space){
    time.uncompress();
//    space.uncompress();
//    assert(!space.is_set())
    iteration_space.uncompress();
    assert(!time.is_set() &&  iteration_space.is_set());

    extend_and_equate(time);
//    extend_and_equate(space);
    iteration_space = Extend_Set(iteration_space);
    time.compress();
//    space.compress();
    iteration_space.compress();
}


static Tuple<int>
move_cyclic_loop_outward(SetTuple &Space, RelTuple &Time,
			 SetTuple & /*Local*/,        // currently unused
			 Relation & /*processors*/ )  // currently unused
{

/* modifies time mapping to move the cyclic "helper" loop out as far 
   as possible
*/

// Test the conditions under which we can move the "alpha" level (the
// loop which enumerates the blocks in a cyclic distribution) outward,
// for effieciency.

    const int stmts = Space.size();
    const int levels = Time[1].n_out();

    Tuple<int> stmt_alpha = compute_alphalevels(Time,Space);
    Tuple<int> move_to(stmts);
    Tuple<int> group_alpha(stmts); // never more than this many groups
    Tuple<bool> alpha_finished(stmts); 
    for(int b = 1; b <= alpha_finished.size(); b++) alpha_finished[b] = false;

    nr_groups[0] = 1; // These are Wayne's globals, declared in schedule.h
    for (int j=0; j<stmts; j++)
	group[0][j] = 0;

    /* at each level, find out if the whole group can agree to put alpha here*/
    for (int level = 1; level <= levels; level++)
      {
      new_calculate_groups(level,Time);
      for (int cur_group = 1; cur_group <= nr_groups[level-1]; cur_group++)
	{
	bool all_same_level = true, all_less_than_current = true;
	group_alpha[cur_group] = -1;
        for (int stmt=1; stmt<=stmts; stmt++)
	    if (!alpha_finished[stmt] && group[level-1][stmt-1] ==cur_group-1) 
		{
		    if(group_alpha[cur_group] < 0) 
			group_alpha[cur_group] = stmt_alpha[stmt];
		    all_same_level= all_same_level && 
			            (group_alpha[cur_group]==stmt_alpha[stmt]);
		    all_less_than_current = all_less_than_current && 
                                            (stmt_alpha[stmt] < level);
		}
	if(all_same_level || all_less_than_current)
	    {
		// Found a place to put the alpha loop safely
		for (int stmt=1; stmt<=stmts; stmt++) 
		    if(!alpha_finished[stmt])
			if(group[level-1][stmt-1] == cur_group-1)
			{
			    int to;
			    if(all_less_than_current)
				to = level;  // place here
			    else
				to = group_alpha[cur_group];
//			    move_just_output(from,to,Time[stmt]);
			    move_to[stmt] = to;
			    alpha_finished[stmt] = true;
			    if(code_gen_debug)
				fprintf(cyclic_debug_file,
					"Statement %d alpha level actually placed at %d, as part  of level %d's group %d\n",
					stmt, to,level,cur_group);
			}
	    }
	}
      }  // FIX: Need to set up appropriate time mappings for alpha
#ifndef NDEBUG
    for(int check = 1; check <= stmts; check++)
	assert(alpha_finished[check]);
#endif


    return move_to;
}


static Relation Block(int processorDim,
		  Tuple<Free_Var_Decl*> &lowerBounds,
		  Tuple<Free_Var_Decl*> &upperBounds) {

    Relation blocking = Relation::True(processorDim);

    assert(Block_Size == 0);
    for(int j = 1; j <= processorDim; j++) {
	// ubj, lbj are already declared for 1<=j<=processorDim 
	// p >= lbj 
	GEQ_Handle g = blocking.and_with_GEQ();
	g.update_coef(set_var(j),1);
	g.update_coef(blocking.get_local(lowerBounds[j]),-1);
	// p <= ubj
	g = blocking.and_with_GEQ();
	g.update_coef(set_var(j),-1);
	g.update_coef(blocking.get_local(upperBounds[j]),1);
    }

    if(code_gen_debug > 2) {
	fprintf(DebugFile, "Processor blocking:\n");
	blocking.prefix_print(DebugFile);
    }

    blocking.finalize();
    return blocking;
}

Relation Cyclic(int processorDim,
		Tuple<Free_Var_Decl*> &lowerBounds,
		Tuple<Free_Var_Decl*> & /*upperBounds*/, // currently unused
		Tuple<Free_Var_Decl*> &my_procs) {

    assert(Num_Procs > 0 && Block_Size > 0 && processorDim > 0);

    assert(processorDim == 1 && "don't handle multidim cyclic dist yet");

    Relation cyclic(processorDim+1);
    F_And *and_node = cyclic.add_and();
    Variable_ID p = set_var(1), alpha = set_var(2);
    cyclic.name_set_var(2,"alpha");

    int block_stride = Block_Size * Num_Procs;
    /* lb_k + cP + myproc*c <= p_k <= lb_k + cP + myproc*c + c - 1 */
    GEQ_Handle g = and_node->add_GEQ(); 
    g.update_coef(cyclic.get_local(lowerBounds[1]),-1);
    g.update_coef(cyclic.get_local(my_procs[1]),-Block_Size);
    g.update_coef(alpha, -block_stride);
    g.update_coef(p,1);

    g = and_node->add_GEQ(); 
    g.update_coef(cyclic.get_local(lowerBounds[1]),1);
    g.update_coef(cyclic.get_local(my_procs[1]),Block_Size);
    g.update_coef(alpha, block_stride);
    g.update_coef(p,-1);
    g.update_const(Block_Size-1);

    if(code_gen_debug > 2) {
	fprintf(cyclic_debug_file, "Processor cyclic distribution:\n");
	cyclic.prefix_print(cyclic_debug_file);
    }

    cyclic.finalize();
    return cyclic;
}



inline
Relation Distribution(int processorDim,
		      Tuple<Free_Var_Decl*> &lowerBounds,
		      Tuple<Free_Var_Decl*> &upperBounds,
		      Tuple<Free_Var_Decl*> &my_procs) {

    if (Block_Size == 0)
	return Block(processorDim,lowerBounds,upperBounds);
    else
	return Cyclic(processorDim,lowerBounds,upperBounds,my_procs);
}


String SPMD_GenerateCode(String Decls, SetTuple &Space, RelTuple &Time,
			SetTuple &IterationSpaces, 
			Tuple<spmd_stmt_info *> &NameInfo,
			Tuple<Free_Var_Decl*> &lb, Tuple<Free_Var_Decl*> &ub,
			Tuple<Free_Var_Decl*> &my_procs,
			int nr_statements) {

    int i;
    int statements = Time.size();
    for(i = 1; i <= statements; i++) {
	fprintf(DebugFile, 
		"Space mapping, statement %d: %s ----------------\n", i,NameInfo[i]->debug_char_name());
        Space[i].uncompress();
	Space[i].print_with_subs(DebugFile);
        Space[i].compress();
    }


    assert(statements);
    assert(Space.size() > 0);
    int processorDim = Space[1].n_out();
    int targetDim = Time[1].n_out();

    SetTuple Local(statements);

    // Create a relation that contains symbolic variables for 
    // to bound each dimension of the processor space

    Tuple<Free_Var_Decl*> &lowerBounds = lb;
    Tuple<Free_Var_Decl*> &upperBounds = ub;
    assert(lowerBounds.size() == processorDim && 
	   upperBounds.size() == processorDim &&
	   my_procs.size() == processorDim);

    assert(processorDim == 1);

    SetTuple I(statements);
    Relation R;
    for(int s = 1; s<= nr_statements; s++) {
	Space[s].uncompress();
	IterationSpaces[s].uncompress();
        I[s] = Range(Restrict_Domain(copy(Space[s]), copy(IterationSpaces[s])));
        I[s].simplify(2,4);
	if (s == 1) R = I[s];
	else R = Union(R,I[s]);
	Space[s].compress();
	IterationSpaces[s].compress();
    }

    Relation virtualBounds = Hull(R);
    virtualBounds.simplify(2,4);

    virtualBounds = EQs_to_GEQs(virtualBounds,true);
    Conjunct *conj = virtualBounds.single_conjunct();
    Relation exclude_upper = Relation::True(virtualBounds);
    Relation exclude_lower = Relation::True(virtualBounds);

    int lb_const = 0;
    int ub_const = 0;
    coef_t lbc, ubc;
    int num_lower = 0, num_upper = 0;
    for (GEQ_Iterator k = conj->GEQs(); k.live(); k.next())
	{
	if ((*k).get_coef(set_var(1)) >= 0)
	    (void) exclude_upper.and_with_GEQ(*k);
	if ((*k).get_coef(set_var(1)) <= 0)
	    (void) exclude_lower.and_with_GEQ(*k);

	if ((*k).get_coef(set_var(1)) > 0)
	    {
	    num_lower++;
            Constr_Vars_Iter c(*k, 0);
            for (; c ; c++)
		if (c.curr_var() != set_var(1))
		    break;
            if (!c)
		{
		lbc = - (*k).get_const();
		lb_const = 1;
		}
	    }
	if ((*k).get_coef(set_var(1)) < 0)
	    {
	    num_upper++;
            Constr_Vars_Iter c(*k, 0);
            for (; c ; c++)
		if (c.curr_var() != set_var(1))
		    break;
            if (!c)
		{
		ubc = (*k).get_const();
		ub_const = 1;
		}
	    }
	}
    virtualBounds = Relation::Null();

    int space_size_const = 0;
    coef_t space_size;
    if (num_lower == 1 && num_upper == 1 && lb_const && ub_const)
	{
	space_size_const = 1;
	space_size = ubc - lbc + 1;
	}

    EQ_Handle el = exclude_upper.and_with_EQ();
    el.update_coef(exclude_upper.get_local(lowerBounds[1]), -1);
    el.update_coef(set_var(1), 1);

    EQ_Handle eu = exclude_lower.and_with_EQ();
    eu.update_coef(exclude_lower.get_local(upperBounds[1]), -1);
    eu.update_coef(set_var(1), 1);

    Relation Known = Relation::True(targetDim);
    Known = Intersection(Project_On_Sym(exclude_lower),
                     Project_On_Sym(exclude_upper));
    Known = Extend_Set(Known, targetDim);
    fprintf(DebugFile, "Known:\n");
    Known.print_with_subs(DebugFile);
    Known.compress();

    Relation Processors;
    Processors = Distribution(processorDim,lowerBounds,upperBounds,my_procs);

    if(Block_Size != 0) {  /* handle cyclic case */
	Known.uncompress();
	Known = Extend_Set(Known);
	Known.compress();
	Tuple<int> move_to = 
	    move_cyclic_loop_outward(Space,Time,Local,Processors);
	for(i = 1; i <= statements; i++) {
	    extend_mappings(Time[i],IterationSpaces[i]);
	    Space[i].uncompress();
	    extend_and_equate(Space[i]);
	    Space[i].compress();
	    Time[i].uncompress();
	    move_just_output(Time[i].n_out(), move_to[i], Time[i]);
	    NameInfo[i]->add_cyclic_level(move_to[i]);
	    Time[i].simplify();
	    Time[i].compress();
	}
    }

    for(i = 1; i <= statements; i++) {
        // Restrict the domain of the Space mappings to the 
        // original iteration spaces
	Space[i].uncompress();
	IterationSpaces[i].uncompress();
        Local[i] = Restrict_Domain(copy(Space[i]), copy(IterationSpaces[i]));
	Local[i]= Restrict_Range(Local[i],copy(Processors));
	Space[i].compress();
	IterationSpaces[i].compress();

        // Get the constraints on original iteration variables            
        // that result.  
        Local[i] = Domain(Local[i]);
        Local[i].simplify();
	Local[i].compress();
     
        if(code_gen_debug > 2) {
            fprintf(DebugFile, 
		    "Space mapping, statement %d: %s ----------------\n",
 	            i,NameInfo[i]->debug_char_name());
            Space[i].uncompress();
            Space[i].print_with_subs(DebugFile);
            Space[i].compress();
            fprintf(DebugFile, "Time mapping:\n");
	    Time[i].uncompress();
            Time[i].print_with_subs(DebugFile);
	    Time[i].compress();
            fprintf(DebugFile, "Original iteration space:\n");
	    IterationSpaces[i].uncompress();
            IterationSpaces[i].print_with_subs(DebugFile);
	    IterationSpaces[i].compress();
            fprintf(DebugFile, "Local iteration space:\n");
	    Local[i].uncompress();
            Local[i].print_with_subs(DebugFile);
	    Local[i].compress();
            fprintf(DebugFile, "Transformed local iteration space:\n");
	    Time[i].uncompress();
	    Local[i].uncompress();
            Range(Restrict_Domain(copy(Time[i]),
			          copy(Local[i]))).print_with_subs(DebugFile);
	    Local[i].compress();
	    Time[i].compress();
        }
    }
    Processors = Relation::Null();

    // Free the memory used by space mappings and iteration spaces
    for(i = 1; i <= statements; i++) {
        Space[i] = Relation::Null();
        IterationSpaces[i] = Relation::Null();
    }

    // Need to generate appropriate assignments to lower bound
    // and upper bounds variables based on the Space mappings.
    // Get the upper and lower bounds on the processor variables
    // this time, then impose either blocking or cyclic
    // constraints.
    
    // Here we assume Num_Procs is unknown for the blocked case.
    //   We could generate better code 
    // if it was known.

#define maxStmts 20

    int redund_low[maxStmts], redund_high[maxStmts];
    Relation low[maxStmts], high[maxStmts];

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
	redund_low[i] = 0;
	redund_high[i] = 0;
	I[i].compress();
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
                        coef_t L = -((*g).get_const());
                        coef_t m = (*g).get_coef(input_var(1));
                        coef_t sb  =  (int) (ceil(((float) L) /m));
                        start += itoS(sb);
                        }
                    else 
                        {
                        start += outputLBasString(*g, low[i], input_var(1), 1, 
                                                  EQ_Handle(), Relation());
                        } 
                    seenLB = true;
                    }
                }
            if (lower_bounds > 1) start += ")";
            seenMax = 1;
            }
    if (non_redund_low > 1) start += ")";

    for (i = 1; i<=nr_statements; i++)
        for (int j = 1; j<=nr_statements; j++)
	    if (i!=j && !redund_high[j] && Must_Be_Subset(copy(high[i]),copy(high[j])))
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
                        coef_t L = -((*g).get_const());
                        coef_t m = (*g).get_coef(input_var(1));
                        coef_t sb  =  L / m;
                        finish += itoS(sb);
                        }
                    else 
                        {
                        finish += outputUBasString(*g, high[i],input_var(1), 1, 
                                                   EQ_Handle());
                        } 
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

    String assignments = "\n  global_lb = " + start + ";\n  global_ub = "+finish + ";\n";
 
     if (Block_Size == 0) 
	 {
         if (gen_dash)
	    {
	    assignments += "\n  block_size = max(0,(global_ub-global_lb+1+(_my_nprocs-1))/_my_nprocs);\n";
	    assignments += "  lb = global_lb+_my_id*block_size;\n";
	    assignments += "  ub = min(global_ub,lb+block_size-1);\n\n";
	    }
         else
	    {
	    assignments += "\nblock_size = max(0,(global_ub-global_lb+1+(num_procs-1))/num_procs);\n";
	    assignments += "lb = global_lb+my_proc*block_size;\n";
	    assignments += "ub = min(global_ub,lb+block_size-1);\n\n";
	    }
	 }

    // Now that we have the local iteration space for each, we
    // can simply hand off to MMGenerateCode with the Time 
    // mappings as the mapping-per-statement

    String loopNest = MMGenerateCode(Time, Local, NameInfo, Known, 
				     overheadEffort);

    // {
    if (gen_dash) 
	{
	loopNest += "  p4_exit_log();\n";
	loopNest += "  return;\n";
	loopNest += "  }\n\n";
	loopNest += "extern int _suif_start()\n";
	loopNest += "  {\n";
	loopNest += "  p4_doall(_main_0_func, &_main_0_struct);\n";
	loopNest += "  return 0;\n";
	}

    loopNest += "  }\n";

    return program_head(Decls, targetDim) + assignments + loopNest;
}



String SPMD_GenerateCode(String Decls, SetTuple &Space, RelTuple &Time,
			SetTuple &IterationSpaces) {
    Tuple<Free_Var_Decl*> lowerBounds;
    Tuple<Free_Var_Decl*> upperBounds;
    Tuple<Free_Var_Decl*> my_procs;

    assert(Space.size() >0);
    int processorDim = Space[1].n_out();
    for(int j = 1; j <= processorDim; j++) {
        // declare ubj, lbj
	lowerBounds.append(new Free_Var_Decl("lb" + itoS(j)));
	upperBounds.append(new Free_Var_Decl("ub" + itoS(j)));
	my_procs.append(new Free_Var_Decl("my_proc" + itoS(j)));
    }
    Tuple<spmd_stmt_info *> NameInfo;
    for (int stmt = 1; stmt <= Time.size(); stmt++) {
	NameInfo.append(new numbered_stmt_info(stmt,Time[stmt],
					       Space[stmt],"s"));
    }
    return SPMD_GenerateCode(Decls, Space, Time, IterationSpaces, NameInfo,
			     lowerBounds,upperBounds,my_procs,Time.size());
}

} // end namespace omega
