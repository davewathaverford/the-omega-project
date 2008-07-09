/* this is mostly experimental, not very useful right now. */


#include <omega/Relations.h>
#include <omega/pres_tree.h>
#include <petit/depend_filter.h>
#include <petit/tree.h>
#include <petit/browse.h>
#include <petit/print.h>
#include <petit/lang-interf.h>
#include <petit/debug.h>
#include <petit/Exit.h>
#include <petit/Zima.h>
#include <petit/omega2flags.h>
#include <petit/hpp.h>
#include <code_gen/spmd.h>

namespace omega {

#define inc_filter 0
#if inc_filter
static void save_dd_filter();
static void restore_dd_filter();
#endif

// from tatiana's code, I hacked it a little
// now returns list
// Don't really need to do this but it simplifies later code to have it.
static List<node *> find_assignment_stmts()
{
    List<node *> statements;
    node *n;
    for(n = br_go_to_another(Entry); n!=Entry; n = br_go_to_another(n))
	{
        if (is_assign(n) && n->nodechild->nodeop != op_phi)
	    {
		//fprintf(hpp_debug_file, "statement %d: ", statements.size()+1);
		//print_petit_out(n,0);
            statements.append(n);
	    }
        }
    return statements;
    } /* find_assignment_stmts */

Relation off_processor(Relation &processor_decl,
		       Tuple<Free_Var_Decl *> myprocs) {
    assert(processor_decl.is_set()); // && processor_decl.n_set() == 1);
    int pdim = processor_decl.n_set();
    Relation r(processor_decl.n_set());

    Relation myproc_bounds = processor_decl;
    EQ_Handle e = myproc_bounds.and_with_EQ();
    e.update_coef(myproc_bounds.get_local(myprocs[1]),1);
    e.update_coef(set_var(1),-1);
    myproc_bounds = Project_On_Sym(myproc_bounds);
    myproc_bounds = Extend_Set(myproc_bounds);

    F_Or *o = r.add_or();
    for (int d = 1; d <= pdim; d++){
	F_And *a= o->add_and();
	GEQ_Handle g = a->add_GEQ();
	g.update_const(-1);
	g.update_coef(set_var(d),-1);
	g.update_coef(r.get_local(myprocs[d]),1);
	a = o->add_and();
	g = a->add_GEQ();
	g.update_const(-1);
	g.update_coef(set_var(d),1);
	g.update_coef(r.get_local(myprocs[d]),-1);
    }

    r = Intersection(r,copy(processor_decl));
    r = Intersection(r, copy(myproc_bounds));
    r.finalize();
    r.simplify(2,2);
    return r;
}




inline bool stmt_lhs_is_array(node *n) {
    return n->nodechild->nodenext->nodeop == op_store_array;
}

inline node *rhs_of_stmt(node *n) {
    return n->nodechild;
}

inline node *lhs_of_stmt(node *n) {
    return n->nodechild->nodenext;
}

inline int dim_of_a_access(a_access a) {
    int dim=0;
    for(sub_iterator s = sub_i_for_access(a); !sub_i_done(s); sub_i_next(s)) dim++;
    return dim;
}


Relation proc_eq_myproc(Tuple<Free_Var_Decl*> &myprocs, int isize, int osize) {
    Relation zzz(isize,osize);
    F_And *a = zzz.add_and();
    for(int ov=1; ov<=osize; ov++){
	EQ_Handle e = a->add_EQ();
	e.update_coef(output_var(ov),1);
	e.update_coef(zzz.get_local(myprocs[ov]),-1);
	e.finalize();
    }
    zzz.finalize();
    return zzz;
}

#define MAX_PROC_DIM 1


Relation alignment(node *n) {
    symtabentry *s = get_nodevalue_sym(get_nodevalue_node(n));
    return *s->alignment;
}


void set_template_info(node *n) {
    symtabentry *s = get_nodevalue_sym(get_nodevalue_node(n));
    template_info *ti = lookup_template(s->template_name);
    List<int> *l = lookup_proc(ti->proc_name);
    assert(l->length() == 1);
    Num_Procs = (*l)[1];
    distribution_list &dl = *(ti->dist_list);
    assert(dl.length() == 1);
    assert(dl[1].type == block && dl[1].blocksize == 0 ||
	   dl[1].type == cyclic && dl[1].blocksize > 0);
    Block_Size = dl[1].blocksize;
}


Relation get_is(node *n) {
    a_access a = n;
    Relation R(node_depth(a));
    AccessIteration ai(a, &R, Input_Tuple);
    F_And *f = R.add_and();
    access_in_bounds(f, ai);
    R.finalize();
    return R;
}

/* Generate a space mapping from the subscript of a node
   It's not necessary to include the loop bounds in a space mapping.  */
Relation make_space(node * n) {

	    a_access a = n;
	    int dim = dim_of_a_access(a); 
	    

/* First, create R:iterations -> array locations */
	    Relation R(node_depth(a), dim);
	    AccessIteration ai(a, &R, Input_Tuple);
	    F_And *f = R.add_and();
            subscripts_equal_to_tuple(f,ai,&output_vars);

/*Modify R to be iterations -> template locations (i.e. virtual processors) */ 
	    R = Join(R, alignment(n));

#if 0
	    fprintf(hpp_debug_file, "\nmap iterations to locations: ");
	    R.print_with_subs(hpp_debug_file);
#endif

	    R.finalize();
	    return R;
}

/* To maintain loop structures, for each enclosing loop, add an index
   variable at the outermost level, equate it to the line number of the 
   specified loop.  Add a final level equal to the line number of this 
   statement to order the statements within this loop body. */
Relation make_time(node *n) {
    EQ_Handle e;
    node *p = n;
    const a_access &a = (a_access) n;
    int dep = node_depth(a);
    int outward = 0;
    Relation T(dep, dep+dep+1);
    F_And *f = T.add_and();
    for(int i=dep; i>=1; i--) {
        /* add the identity part */
	e = f->add_EQ();
	e.update_coef(T.input_var(i), -1);
	e.update_coef(T.output_var(i+dep), 1);
	e.finalize();
        /* add the loop-numbered part */
	e = f->add_EQ();
	while(outward < dep-i+1) {
	    p = p->nodeparent;
	    if (p->nodeop == op_do) outward++;
	}
	e.update_coef(output_var(i), -1);
	e.update_const(p->nodesequence);
	e.finalize();
    }
    e = f->add_EQ();
    e.update_coef(output_var(dep+dep+1), -1);
    e.update_const(n->nodesequence);
    T.finalize();
    return T;
}

void assert_all_same_template(node *n) {

static bool first = true;
static String name;

    symtabentry *s = get_nodevalue_sym(get_nodevalue_node(n));
    if(first) {
	name = s->template_name;
	first = false;
    } else {
	assert(s->template_name == name && "Only handling one template now");
    }
}

#if 0
void insert_communication(Tuple<Relation>& space_mappings,
			  Tuple<Relation>&  time_mappings,
			  Tuple<Relation>&  iteration_spaces,
			  Tuple<spmd_stmt_info *> &name_info_tuple,
			  node *n) {
#endif
void insert_communication(Tuple<Relation>& ,
			  Tuple<Relation>& ,
			  Tuple<Relation>& ,
			  Tuple<spmd_stmt_info *> &,
			  node *) {

    return;
}


Tuple<Free_Var_Decl *> setup_myprocs() {
    Tuple<Free_Var_Decl *> myprocs(0);
    for(int dmp = 1; dmp<= MAX_PROC_DIM; dmp++)
	myprocs.append(new Free_Var_Decl("myproc_"+itoS(dmp)));
    return myprocs;
}

void setup_lb_and_ub(Tuple<Free_Var_Decl*>& lowerBounds,
		     Tuple<Free_Var_Decl*>& upperBounds) {

    for (int i = 1; i<= MAX_PROC_DIM; i++)
	{
	    lowerBounds.append(new Free_Var_Decl("lb" + itoS(i)));
	    upperBounds.append(new Free_Var_Decl("ub" + itoS(i)));
	}
}



int hpp_generate(int) {
    Tuple<Relation> space_mappings(0), time_mappings(0), iteration_spaces(0);
    Tuple<spmd_stmt_info *> name_info(0);

    node *n;
    int i = 0;

    List<node *> statements = find_assignment_stmts();
    fprintf(hpp_debug_file, "%d statements found\n", statements.size());
    for(List_Iterator<node *> li = statements; li; li++) {
	n = *li;
	if (stmt_lhs_is_array(n)) {
	    i++;
	    node *lhs = lhs_of_stmt(n);
	    fprintf(hpp_debug_file, "Found array assignment, lhs: ");
	    assert_all_same_template(lhs);
//	    print_petit_out(lhs,0);
	    space_mappings.append(make_space(lhs));
	    time_mappings.append(make_time(lhs));
	    iteration_spaces.append(get_is(lhs));
	    print_template_stmt(n);
	    char *stmt_string = new char[printBuf.length() + 1];
	    strcpy(stmt_string, (const char *)printBuf);
	    name_info.append(
		new numbered_stmt_info(i, time_mappings[i], 
				       space_mappings[i], stmt_string));
	    insert_communication(space_mappings, time_mappings,
				 iteration_spaces, name_info, n);
	    set_template_info(lhs);  // totally redundant but easy(only 1 templ.)
	}
    }


    Tuple<Free_Var_Decl*> myprocs = setup_myprocs();

    Tuple<Free_Var_Decl*> lower_bounds(0), upper_bounds(0);
    setup_lb_and_ub(lower_bounds, upper_bounds); 

    gen_dash = 1;
    overheadEffort=-1;

    String code = 
	SPMD_GenerateCode("", space_mappings, time_mappings,
			   iteration_spaces, 
			   name_info,
			   lower_bounds,upper_bounds,myprocs,
			   space_mappings.size());
    fprintf(stdout,"%s", (const char*) code);
    return 0;
}



#if inc_filter
static int old_flow_filter;      
static int old_reduce_filter;    
static int old_memory_filter;    
static int old_value_filter;     
static int old_noncarried_filter;
static int old_noncyclic_filter; 
static int old_scalar_filter;    
static int old_output_filter;    
static int old_anti_filter;      

static void save_dd_filter() {
    old_flow_filter = flow_filter;
    old_reduce_filter = reduce_filter;
    old_memory_filter = memory_filter;
    old_value_filter = value_filter;
    old_noncarried_filter = noncarried_filter;
    old_noncyclic_filter = noncyclic_filter;
    old_scalar_filter = scalar_filter;
    old_output_filter = output_filter;
    old_anti_filter = anti_filter;

    flow_filter = 1;
    reduce_filter = 1;
    memory_filter = 1;
    value_filter = 0;
    noncarried_filter = 1;
    noncyclic_filter = 1;
    scalar_filter = 1;
    output_filter = 1;
    anti_filter = 1;
}

static void restore_dd_filter() {
    flow_filter        = old_flow_filter;
    reduce_filter      = old_reduce_filter;
    memory_filter      = old_memory_filter;
    value_filter       = old_value_filter;
    noncarried_filter  = old_noncarried_filter;
    noncyclic_filter   = old_noncyclic_filter;
    scalar_filter      = old_scalar_filter;
    output_filter      = old_output_filter;
    anti_filter        = old_anti_filter;
}

#endif

} // omega namespace

#include <petit/motif.h>

namespace omega {

void* build_hpp() {
#if ! defined BATCH_ONLY_PETIT
    struct Menu_struc M[] = 
    {{"Generate",	0,	hpp_generate,	0,	0},
     {"Xcape",    	0, 	0,		0,	0},
     {"Quit",		0,	0,		0,	0},
     {0,		0,	0,		0,	0}};

    return Build_Menu( M, do_display, 0 );
#else
    assert(0);
    return 0;
#endif
}

} // omega namespace
