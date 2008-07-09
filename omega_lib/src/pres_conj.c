/* $Id: pres_conj.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <omega/omega_core/oc.h>
#include <omega/pres_conj.h>
#include <omega/pres_cmpr.h>
#include <omega/Relation.h>
#include <omega/omega_i.h>
#include <basic/Bag.h>

namespace omega {


int NR_CONJUNCTS, MAX_CONJUNCTS;

/*
 * Make a new wildcard variable, return WC number.
 * Should be static to this file, but must be a friend of conjunct.
 */
int new_WC(Conjunct *nc, Problem *) {
  Variable_ID wc = nc->declare();
  int wc_no = nc->map_to_column(wc);
  return(wc_no);
}


void Conjunct::promise_that_ub_solutions_exist(Relation &R) {
#ifndef NDEBUG
   Relation verify=Relation(R, this);
   assert(verify.is_upper_bound_satisfiable());
#endif 
   verified = true;
   }


int Conjunct::max_ufs_arity_of_set() 
   {
    int ma = 0, a;
    for (Variable_ID_Iterator v(mappedVars); v; v++)
        if ((*v)->kind() == Global_Var && (*v)->function_of() == Set_Tuple 
			&& query_variable_used(*v))
        {
        a = (*v)->get_global_var()->arity();
        if (a > ma)
            ma = a;
        }
    return ma;
    }
int Conjunct::max_ufs_arity_of_in() 
   {
    int ma = 0, a;
    for (Variable_ID_Iterator v(mappedVars); v; v++)
        if ((*v)->kind() == Global_Var && (*v)->function_of() == Input_Tuple
			&& query_variable_used(*v))
        {
        a = (*v)->get_global_var()->arity();
        if (a > ma)
            ma = a;
        }
    return ma;
    }
int Conjunct::max_ufs_arity_of_out() 
   {
    int ma = 0, a;
    for (Variable_ID_Iterator v(mappedVars); v; v++)
        if ((*v)->kind() == Global_Var &&  (*v)->function_of() == Output_Tuple
			&& query_variable_used(*v))
        {
        a = (*v)->get_global_var()->arity();
        if (a > ma)
            ma = a;
        }
    return ma;
    }

bool Conjunct::is_unknown() const
    { 
    assert(problem || comp_problem);
    return !exact && ((problem && problem->nEQs==0 && problem->nGEQs==0) ||
		      (comp_problem && comp_problem->no_constraints()));
    }



/*
 * Remove black constraints from the problem.
 * Make all the remaining red constraints black.
 */
void Conjunct::rm_color_constrs() {
  int geqs = 0, eqs = 0, i;

  possible_leading_0s = -1;
  guaranteed_leading_0s = -1;
  leading_dir = 0;

  for(i=0; i<problem->nGEQs; i++) {
    if(problem->GEQs[i].color) {
      if(geqs!=i)
	eqnncpy(&problem->GEQs[geqs], &problem->GEQs[i], problem->nVars);
      problem->GEQs[geqs].color = black;
      geqs++;
    }
  }
  problem->nGEQs = geqs;
  
  for(i=0; i<problem->nEQs; i++) {
    if(problem->EQs[i].color) {
      if(eqs!=i)
	eqnncpy(&problem->EQs[eqs], &problem->EQs[i], problem->nVars);
      problem->EQs[eqs].color = black;
      eqs++;
    }
  }
  problem->nEQs = eqs;
}



//
// Conjunct constructors.
//
Conjunct::Conjunct() : 
F_Declaration(NULL, NULL),
mappedVars(0),
n_open_constraints(0),
cols_ordered(false),
simplified(false),
verified(false),
guaranteed_leading_0s(-1),
possible_leading_0s(-1),
leading_dir(0),
exact(true),
r_constrs(0)
{
  NR_CONJUNCTS++; if (NR_CONJUNCTS>MAX_CONJUNCTS) MAX_CONJUNCTS = NR_CONJUNCTS;
  problem = new Problem;
  comp_problem = NULL;
  problem->get_var_name     = get_var_name;
  problem->getVarNameArgs = (void *) this ;
}

Conjunct::Conjunct(Formula *f, Rel_Body *r) : 
F_Declaration(f,r),
mappedVars(0),
n_open_constraints(0),
cols_ordered(false),
simplified(false),
verified(false),
guaranteed_leading_0s(-1),
possible_leading_0s(-1),
leading_dir(0),
exact(true),
r_constrs(0)
{ 
  NR_CONJUNCTS++; if (NR_CONJUNCTS>MAX_CONJUNCTS) MAX_CONJUNCTS = NR_CONJUNCTS;
  problem = new Problem;
  comp_problem = NULL;
  problem->get_var_name     = get_var_name;
  problem->getVarNameArgs = (void *) this ;
}

void internal_copy_conjunct(Conjunct* to, Conjunct* fr);

//
// Copy Conjunct.
//
Conjunct* Conjunct::copy_conj_diff_relation(Formula *parent,
					    Rel_Body *rel_body) {
  Conjunct *new_conj;
  if(problem && comp_problem==NULL) {
    new_conj = new Conjunct(parent, rel_body);
    internal_copy_conjunct(new_conj, this);
  } else if (problem==NULL && comp_problem) {
    /* copy compressed conjunct */
    assert(0 && "copy compressed conjunct");
    new_conj = 0;
  } else {
    assert(0 && "problem == NULL");
    new_conj = 0;
  }
  return new_conj;
}

void internal_copy_conjunct(Conjunct* to, Conjunct* fr) {
  copy_conj_header(to, fr);

  // 
  // We repeat part of what is done by copy_conj_header(to, fr) by
  // calling Problem::operator=(const Problem &).  
  // copy_conj_header should go away, but there is some code that still needs 
  // it in negate_conj.  
  //
  to->r_constrs = fr->r_constrs;
  to->simplified = fr->simplified;
  to->verified = fr->verified;
  to->guaranteed_leading_0s = fr->guaranteed_leading_0s;
  to->possible_leading_0s = fr->possible_leading_0s;
  to->leading_dir = fr->leading_dir;
  // the following duplicates some work of the "copy_conj_header" brain damage
  *to->problem = *fr->problem; 
  to->problem->getVarNameArgs = (void *)to;   // important
}

//
// Copy Conjunct variable declarations
// and problem parameters but not problem itself.
//
void copy_conj_header(Conjunct* to, Conjunct* fr) {
  free_var_decls(to->myLocals);  to->myLocals.clear();

  copy_var_decls(to->myLocals, fr->myLocals);
  to->mappedVars = fr->mappedVars;
  to->remap();
  reset_remap_field(fr->myLocals);

  to->cols_ordered = fr->cols_ordered;
  to->r_constrs = fr->r_constrs;
  to->simplified = fr->simplified;
  to->verified = fr->verified;
  to->guaranteed_leading_0s = fr->guaranteed_leading_0s;
  to->possible_leading_0s = fr->possible_leading_0s;
  to->leading_dir = fr->leading_dir;
  to->n_open_constraints = fr->n_open_constraints;
  to->exact=fr->exact;

  Problem *fp = fr->problem;
  Problem *tp = to->problem;
  tp->nVars    = fp->nVars;
  tp->safeVars = fp->safeVars;
  tp->variablesInitialized = fp->variablesInitialized;
  tp->variablesFreed = fp->variablesFreed;
  for(int i=1; i<=maxVars; i++) {  // only need nVars of var
    tp->forwardingAddress[i] = fp->forwardingAddress[i];
    tp->var[i] = fp->var[i];
  }
  to->problem->get_var_name     = get_var_name;
  to->problem->getVarNameArgs = (void *)to ;
}


void Conjunct::reverse_leading_dir_info()
    {
    leading_dir *= -1;
    }

void Conjunct::enforce_leading_info(int guaranteed, int possible, int dir) {
    skip_finalization_check++;
    guaranteed_leading_0s = guaranteed;


    int d = min(relation()->n_inp(),relation()->n_out());

    assert(0 <= guaranteed);
    assert(guaranteed <= possible);
    assert(possible <= d);

    for(int i = 1; i <= guaranteed; i++) {
	    EQ_Handle e = add_EQ();
            e.update_coef_during_simplify(input_var(i), -1);
            e.update_coef_during_simplify(output_var(i), 1);
            e.finalize();
	    }


    if (guaranteed == possible && guaranteed >= 0  && possible+1 <= d && dir) {
	    GEQ_Handle g = add_GEQ();
	    if (dir > 0) {
		    g.update_coef_during_simplify(input_var(possible+1), -1);
		    g.update_coef_during_simplify(output_var(possible+1), 1);
		    }
	    else {
		    g.update_coef_during_simplify(input_var(possible+1), 1);
		    g.update_coef_during_simplify(output_var(possible+1), -1);
		    }
	    g.update_const_during_simplify(-1);
            g.finalize();
	    possible_leading_0s = possible;
	    leading_dir = dir;
	    }
	else {
	    possible_leading_0s = d;
	    leading_dir = 0;
	    }

    skip_finalization_check--;
#if ! defined NDEBUG
    assert_leading_info();
#endif
    }
	
void Conjunct::invalidate_leading_info(int changed)
    {
    if (changed == -1) {
	guaranteed_leading_0s = possible_leading_0s = -1;
	leading_dir = 0;
	}
    else {
            int d = min(relation()->n_inp(), relation()->n_out());
	    assert(1 <= changed && changed <= d);
	    if (possible_leading_0s == changed -1) {
		possible_leading_0s  = d;
		}
	    guaranteed_leading_0s = min(guaranteed_leading_0s,changed-1);
	}
#if ! defined NDEBUG
    assert_leading_info();
#endif
    }

int Conjunct::leading_dir_valid_and_known()
    {
    if (relation()->is_set())
	{
	return 0;
	}
    // if we know leading dir, we can rule out extra possible 0's
    assert(leading_dir == 0 ||
	   possible_leading_0s == guaranteed_leading_0s);

    return (possible_leading_0s == guaranteed_leading_0s &&
	    possible_leading_0s >= 0 &&
	    possible_leading_0s < min(relation()->n_inp(),relation()->n_out())
	    && leading_dir);
    }

#if ! defined NDEBUG
void Conjunct::assert_leading_info()
    {
    if (relation()->is_set())
	{
	return;
	}

    int d = min(relation()->n_inp(), relation()->n_out());

    if ( guaranteed_leading_0s == -1
         && guaranteed_leading_0s == possible_leading_0s)
	assert(leading_dir == 0);
 
    if(leading_dir != 0 && 
	   possible_leading_0s != guaranteed_leading_0s) {
	use_ugly_names++;
	prefix_print(DebugFile);
	use_ugly_names--;
	}
	
    assert(leading_dir == 0 ||
	   possible_leading_0s == guaranteed_leading_0s);

    assert(possible_leading_0s <= d && guaranteed_leading_0s <= d);

    assert(possible_leading_0s == -1 ||
	   guaranteed_leading_0s <= possible_leading_0s);

    // check that there must be "guaranteed_leading_0s" 0s
    int carried_level;
    for (carried_level = 1;
	 carried_level <= guaranteed_leading_0s;
	 carried_level++)
	{
	Variable_ID in = input_var(carried_level),
	    out = output_var(carried_level);
	coef_t lb, ub;
	bool guar;
	query_difference(out, in, lb, ub, guar);
	if (lb != 0 && ub != 0)
	    {
	    // probably "query_difference" is just approximate
	    // add the negation of leading_dir and assert that
	    // the result is unsatisfiable;
	    // add in > out (in-out-1>=0) and assert unsatisfiable.

	    Conjunct *test = copy_conj_same_relation();
	    test->problem->turnRedBlack();
	    skip_finalization_check++;
	    
	    GEQ_Handle g = test->add_GEQ();
	    g.update_coef_during_simplify(in, -1);
	    g.update_coef_during_simplify(out, 1);
	    g.update_const_during_simplify(-1);
	    g.finalize();
	    assert(!simplify_conj(test, true, 0, 0));
	    // test was deleted by simplify_conj, as it was FALSE

	    test = copy_conj_same_relation();
	    test->problem->turnRedBlack();
	    g = test->add_GEQ();
	    g.update_coef_during_simplify(in, 1);
	    g.update_coef_during_simplify(out, -1);
	    g.update_const_during_simplify(-1);
	    g.finalize();
	    assert(!simplify_conj(test, true, 0, 0));
	    // test was deleted by simplify_conj, as it was FALSE

	    skip_finalization_check--;
	    }
	}

    carried_level = possible_leading_0s+1;

    // check that there can't be another
    if (guaranteed_leading_0s == possible_leading_0s
	&& possible_leading_0s >= 0 &&
	carried_level <= min(relation()->n_inp(), relation()->n_out()))
	{
	Variable_ID in = input_var(carried_level),
	    out = output_var(carried_level);
	coef_t lb, ub;
	bool guar;
	query_difference(out, in, lb, ub, guar);
	if (lb <= 0 && ub >= 0)
	    {
	    // probably "query_difference" is just approximate
	    // add a 0 and see if its satisfiable

	    Conjunct *test = copy_conj_same_relation();
	    test->problem->turnRedBlack();
	    skip_finalization_check++;

	    EQ_Handle e = test->add_EQ();
	    e.update_coef_during_simplify(in, -1);
	    e.update_coef_during_simplify(out, 1);
	    e.finalize();
	    assert(!simplify_conj(test, true, 0, 0));
	    // test was deleted by simplify_conj, as it was FALSE

	    skip_finalization_check--;
	    }
	}

    // check leading direction info
    if (leading_dir_valid_and_known())
	{
	Variable_ID in = input_var(guaranteed_leading_0s+1),
	    out = output_var(guaranteed_leading_0s+1);
	coef_t lb, ub;
	bool guar;
	query_difference(out, in, lb, ub, guar);
	if ((leading_dir < 0 && ub >= 0) ||
	    (leading_dir > 0 && lb <= 0))
	    {
	    // probably "query_difference" is just approximate
	    // add the negation of leading_dir and assert that
	    // the result is unsatisfiable;
	    // eg for leading_dir = +1 (in must be < out),
	    // add in >= out (in-out>=0) and assert unsatisfiable.

	    Conjunct *test = copy_conj_same_relation();
	    test->problem->turnRedBlack();
	    skip_finalization_check++;
	    
	    GEQ_Handle g = test->add_GEQ();
	    g.update_coef_during_simplify(in, leading_dir);
	    g.update_coef_during_simplify(out, -leading_dir);
	    g.finalize();

	    assert(!simplify_conj(test, true, 0, 0));
	    // test was deleted by simplify_conj, as it was FALSE

	    skip_finalization_check--;
	    }
	}
    }
#endif


Variable_ID Conjunct::declare(Const_String s) {
   return do_declare(s, Wildcard_Var);
}

Variable_ID Conjunct::declare() {
  return do_declare(Const_String(), Wildcard_Var);
}

Variable_ID Conjunct::declare(Variable_ID v) {
  return do_declare(v->base_name, Wildcard_Var);
}

Conjunct* Conjunct::really_conjunct()
    {
    return this;
    }



Variable_ID_Tuple* Conjunct::variables() {
  return &mappedVars;
}

Stride_Handle Conjunct::add_stride(int step, int preserves_level)
    {
    assert_not_finalized();
    Variable_ID wild = declare();
    int c;
    c = problem->newEQ();
    simplified = false;
    verified = false;
    if (! preserves_level)
	{
	if (leading_dir == 0)
	    possible_leading_0s = -1;
	// otherwise we must still have leading_dir, and thus no more 0's
	}
    problem->EQs[c].color = black;
    eqnnzero(&problem->EQs[c],problem->nVars);
    n_open_constraints++;
    EQ_Handle h = EQ_Handle(this, c);
    h.update_coef(wild,step);
    return h;
}

// This should only be used to copy constraints from simplified relations, 
// i.e. there are no quantified variables in c except wildcards.
EQ_Handle Conjunct::add_EQ(const Constraint_Handle &c, 
			   int /*preserves_level*/)  // currently unused
{
    EQ_Handle e = add_EQ();
    copy_constraint(e,c);
    return e;
}


EQ_Handle Conjunct::add_EQ(int preserves_level)
    {
    assert_not_finalized();
    int c;
    c = problem->newEQ();
    simplified = false;
    verified = false;
    if (!preserves_level)
	{
	if (leading_dir == 0)
	    possible_leading_0s = -1;
	// otherwise we must still have leading_dir, and thus no more 0's
	}
    problem->EQs[c].color = black;
    eqnnzero(&problem->EQs[c],problem->nVars);
    n_open_constraints++;
    return EQ_Handle(this, c);
}

// This should only be used to copy constraints from simplified relations, 
// i.e. there are no quantified variables in c except wildcards.
GEQ_Handle Conjunct::add_GEQ(const Constraint_Handle &c,
			     int /*preserves_level*/)  // currently unused
{ 
    GEQ_Handle g = add_GEQ();
    copy_constraint(g,c);
    return g;
}

GEQ_Handle Conjunct::add_GEQ(int preserves_level) {
    assert_not_finalized();
    int c;
    c = problem->newGEQ();
    simplified = false;
    verified = false;
    if (!preserves_level)
	{
	if (leading_dir == 0)
	    possible_leading_0s = -1;
	// otherwise we must still have leading_dir, and thus no more 0's
	}
    problem->GEQs[c].color = black;
    eqnnzero(&problem->GEQs[c],problem->nVars);
    n_open_constraints++;
    return GEQ_Handle(this, c);
}


Conjunct *Conjunct::find_available_conjunct() {
	return this;
	};

bool Conjunct::can_add_child() {
    return false;
}

void Conjunct::combine_columns() {

  int nvars = mappedVars.size(),i,j,k;

  for(i=1; i<=nvars; i++)
    for(j=i+1; j<=nvars; j++) {
	// If they are the same, copy into the higher numbered column.
        // That way we won't have problems with already-merged columns later
        assert(i != j);
	if(mappedVars[i] == mappedVars[j]) {
	    if (pres_debug)
		fprintf(DebugFile, "combine_col:Actually combined %d,%d\n",
		    j,i);
	    for(k=0; k<problem->nEQs; k++)
		problem->EQs[k].coef[j] += problem->EQs[k].coef[i];
	    for(k=0; k<problem->nGEQs; k++)
		problem->GEQs[k].coef[j] += problem->GEQs[k].coef[i];
	    zero_column(problem, i, 0, 0, problem->nEQs, problem->nGEQs);
	    // Create a wildcard w/no constraints.  temporary measure, 
	    // so we don't have to shuffle columns
	    Variable_ID zero_var = declare();
	    mappedVars[i] = zero_var;
	    break;
	}
    }
}

void Conjunct::finalize() {
// Debugging version of finalize; copy the conjunct and free the old one,
// so that purify will catch accesses to finalized constraints
//    assert(n_open_constraints == 0);
//    Conjunct *C = this->copy();
//    parent().replace_child(this, C);
//    delete this;
}

Conjunct::~Conjunct() {
  NR_CONJUNCTS--;
  delete problem;
  delete comp_problem;
}


//
// Cost = # of terms in DNF when negated
//  or CantBeNegated if too bad (i.e. bad wildcards)
//  or AvoidNegating if it would be inexact
//
// Also check pres_legal_negations --
//   If set to any_negation, just return the number
//   If set to one_geq_or_stride, return CantBeNegated if c > 1
//   If set to one_geq_or_eq, return CantBeNegated if not a single geq or eq
//

int Conjunct::cost() {
  int c;
  int i;
  int wc_no;
  int wc_j = 0;  // initialize to shut up the compiler

  // cost 1 per GEQ, and if 1 GEQ has wildcards, +2 for each of them

  c = problem->nGEQs;
  for(i=0; i<problem->nGEQs; i++) {
    wc_no = 0;
    for(int j=1; j<=problem->nVars; j++) if(problem->GEQs[i].coef[j]!=0) {
      Variable_ID v = mappedVars[j];
      if(v->kind()==Wildcard_Var) {
        wc_no++;
	c+=2;
        wc_j = j;
      }
    }
    if (wc_no > 1) return CantBeNegated;
    }

  for(i=0; i<problem->nEQs; i++) {
    wc_no = 0;
    for(int j=1; j<=problem->nVars; j++) if(problem->EQs[i].coef[j]!=0) {
      Variable_ID v = mappedVars[j];
      if(v->kind()==Wildcard_Var) {
        wc_no++;
        wc_j = j;
      }
    }

    if (wc_no == 0)	  // no wildcards
	c+=2;
    else if (wc_no == 1)  // one wildcard - maybe we can negate it
	{
        int i2;
        for(i2=0; i2<problem->nEQs; i2++)
            if(i != i2 && problem->EQs[i2].coef[wc_j]!=0)  break;
        if (i2 >= problem->nEQs)  // Stride constraint
            c++;
        else   // We are not ready to handle this
	    return CantBeNegated;
        }
    else		  // Multiple wildcards
	return CantBeNegated;
  }
  if (!exact) return AvoidNegating;

  if (pres_legal_negations == any_negation)
    {
      return c;
    }
  else
    {
      // single GEQ ok either way as long as no wildcards
      // (we might be able to handle wildcards, but I haven't thought about it)
      if (problem->nEQs==0 && problem->nGEQs<=1)
	{
	  if (c>1)  // the GEQ had a wildcard -- I'm not ready to go here.
	    {
	      if (pres_debug > 0)
		{
		  fprintf(DebugFile,
			  "Refusing to negate a GEQ with wildcard(s)"
			  " under restricted_negation;  "
			  "It may be possible to fix this.\n");
		}
	      return CantBeNegated;
	    }
	  return c;
	}
      else if (problem->nEQs==1 && problem->nGEQs==0)
	{
	  assert(c == 1 || c == 2);

	  if (pres_legal_negations == one_geq_or_stride)
	    {
	      if (c == 1)
		return c;     // stride constraint is ok
	      else
		{
		  if (pres_debug > 0)
		    {
		      fprintf(DebugFile,
			      "Refusing to negate a non-stride EQ under current pres_legal_negations.\n");
		    }
		  return CantBeNegated;
		}
	    }
	  else
	    {
	      assert(pres_legal_negations == one_geq_or_eq);
	      return c;
	    }
	}
      else
	{
	  if (pres_debug > 0)
	    {
	      fprintf(DebugFile,
		      "Refusing to negate multiple constraints under current pres_legal_negations.\n");
	    }
	  return CantBeNegated;
	}
    }
}


//
// Merge CONJ1 & CONJ2 -> CONJ.
// Action: MERGE_REGULAR or MERGE_COMPOSE: regular merge.
//         MERGE_GIST    make constraints from conj2 red, i.e.
//                        Gist Conj2 given Conj1 (T.S. comment). 
// Reorder columns as we go.
// Merge the columns for identical variables.  
// We assume we know nothing about the ordering of conj1, conj2.
//
// Does not consume its arguments
//
// Optional 4th argument gives the relation for the result - if
//  null, conj1 and conj2 must have the same relation, which will
//  be used for the result
//
// The only members of conj1 and conj2 that are used are: problem,
//  mappedVars and declare(), and the leading_0s/leading_dir members
//  and exact.
//
// NOTE: variables that are shared between conjuncts are necessarily 
// declared above, not here; so we can simply create columns for the 
// locals of each conj after doing the protected vars.  
//
Conjunct* merge_conjs(Conjunct* conj1, Conjunct* conj2,
                     Merge_Action action, Rel_Body *body)
    {
    // body must be set unless both conjuncts are from the same relation
    assert(body || conj1->relation() == conj2->relation());

    if (body == conj1->relation() && body == conj2->relation())
	body = 0;  // we test this later to see if there is a new body

    Conjunct *conj3 = new Conjunct(NULL, body ? body : conj2->relation());
    Problem *p1 = conj1->problem;
    Problem *p2 = conj2->problem;
    Problem *p3 = conj3->problem;
    int i;

    if (action != MERGE_COMPOSE)
	{
	conj1->assert_leading_info();
	conj2->assert_leading_info();
	}

    if(pres_debug>=2) 
	{
	use_ugly_names++;
	fprintf(DebugFile, ">>> Merge conjuncts: Merging%s:\n",
		(action == MERGE_GIST ? " for gist" :
	        (action == MERGE_COMPOSE ? " for composition" : "")));
	conj1->prefix_print(DebugFile);
	conj2->prefix_print(DebugFile);
	fprintf(DebugFile, "\n");
	use_ugly_names--;
	}



    switch(action) 
	{
    case MERGE_REGULAR:
    case MERGE_COMPOSE:
        conj3->exact=conj1->exact && conj2->exact;
        break;
    case MERGE_GIST:
        conj3->exact=conj2->exact;
	break;
	}

    if (action == MERGE_COMPOSE)
	{
	conj3->guaranteed_leading_0s=min(conj1->guaranteed_leading_0s,
					 conj2->guaranteed_leading_0s);
	conj3->possible_leading_0s=min((unsigned int) conj1->possible_leading_0s,
				       (unsigned int) conj2->possible_leading_0s);

	assert( conj3->guaranteed_leading_0s <= conj3->possible_leading_0s);

	// investigate leading_dir - not well tested code
	if (conj1->guaranteed_leading_0s<0 || conj2->guaranteed_leading_0s<0)
	    {
	    conj3->leading_dir = 0;
	    }
	else if (conj1->guaranteed_leading_0s == conj2->guaranteed_leading_0s)
	    if (conj1->leading_dir == conj2->leading_dir)
		conj3->leading_dir = conj1->leading_dir;
	    else
		conj3->leading_dir = 0;
	else if (conj1->guaranteed_leading_0s < conj2->guaranteed_leading_0s)
	    {
	    conj3->leading_dir = conj1->leading_dir;
	    }
	else // (conj1->guaranteed_leading_0s > conj2->guaranteed_leading_0s)
	    {
	    conj3->leading_dir = conj2->leading_dir;
	    }

	if (conj3->leading_dir == 0)
	    conj3->possible_leading_0s = min(conj3->relation()->n_inp(),
					     conj3->relation()->n_out());

	assert(conj3->guaranteed_leading_0s <= conj3->possible_leading_0s);
	assert(conj3->guaranteed_leading_0s == conj3->possible_leading_0s
		|| !conj3->leading_dir);
	}
    else if (!body)  // if body is set, who knows what leading 0's mean?
	{
	assert(action == MERGE_REGULAR || action == MERGE_GIST);

	int feasable = 1;

        int redAndBlackGuarLeadingZeros = omega::max(conj1->guaranteed_leading_0s,
					 conj2->guaranteed_leading_0s);
	if (action == MERGE_REGULAR) 
	    conj3->guaranteed_leading_0s= redAndBlackGuarLeadingZeros;
	else conj3->guaranteed_leading_0s=conj1->guaranteed_leading_0s;

	conj3->possible_leading_0s=min((unsigned)conj1->possible_leading_0s,
				       (unsigned)conj2->possible_leading_0s);
	if (conj3->possible_leading_0s < redAndBlackGuarLeadingZeros)
	    feasable = 0;
	else if (conj3->guaranteed_leading_0s == -1 
		|| conj3->possible_leading_0s > redAndBlackGuarLeadingZeros)
	    conj3->leading_dir = 0;
	else {
	    if (conj1->guaranteed_leading_0s == conj2->guaranteed_leading_0s)
		if (!conj1->leading_dir_valid_and_known())
		    conj3->leading_dir = conj2->leading_dir;
		else if (!conj2->leading_dir_valid_and_known())
		    conj3->leading_dir = conj1->leading_dir;
		else if (conj1->leading_dir * conj2->leading_dir > 0)
		    conj3->leading_dir = conj1->leading_dir; // 1,2 same dir
		else
		    feasable = 0;  // 1 and 2 go in opposite directions
	    else if (conj3->possible_leading_0s != conj3->guaranteed_leading_0s)
		conj3->leading_dir = 0;
	    else if (conj1->guaranteed_leading_0s<conj2->guaranteed_leading_0s)
		{
		assert(!conj1->leading_dir_valid_and_known());
		conj3->leading_dir = conj2->leading_dir;
		}
	    else 
		{
		assert(!conj2->leading_dir_valid_and_known());
		conj3->leading_dir = conj1->leading_dir;
		}
	    }

	if (!feasable)
	    {
	    if(pres_debug>=2)
		fprintf(DebugFile,
			">>> Merge conjuncts: quick check proves FALSE.\n");

	    // return 0 = 1

	    int e = p3->newEQ(); 
	    p3->EQs[e].color   = black;
	    p3->EQs[e].touched = 1;
	    p3->EQs[e].key     = 0;
	    p3->EQs[e].coef[0] = 1;

	    // Make sure these don't blow later assertions
	    conj3->possible_leading_0s = conj3->guaranteed_leading_0s = -1;
	    conj3->leading_dir = 0;

	    return conj3;
	    }
	}
    else // provided "body" argument but not composing, leading 0s meaningless
	{
	conj3->guaranteed_leading_0s = conj3->possible_leading_0s = -1;
	conj3->leading_dir = 0;
	}

    // initialize omega stuff

    for(i=0; i<p1->nGEQs+p2->nGEQs; i++) 
	{
	int e = p3->newGEQ();
	assert(e == i);
	p3->GEQs[e].color   = black;
	p3->GEQs[e].touched = 1;
	p3->GEQs[e].key     = 0;
	}
    for(i=0; i<p1->nEQs+p2->nEQs; i++) 
	{
	int e = p3->newEQ();
	assert(e == i);
	p3->EQs[e].color   = black;
	p3->EQs[e].touched = 1;
	p3->EQs[e].key     = 0;
	}

    assert(p3->nGEQs == p1->nGEQs + p2->nGEQs);
    assert(p3->nEQs == p1->nEQs + p2->nEQs);

    // flag constraints from second constraint as red, if necessary  
    if (action == MERGE_GIST) 
	{
	for(i=0; i<p2->nEQs; i++) 
	    {
	    p3->EQs[i+p1->nEQs].color = _red;
	    }
	for(i=0; i<p2->nGEQs; i++) 
	    {
	    p3->GEQs[i+p1->nGEQs].color = _red;
	    }
	}

    // copy constant column
    copy_column(p3, 0, p1, 0, 0, 0);
    copy_column(p3, 0, p2, 0, p1->nEQs, p1->nGEQs);
  
  // copy protected variables column from conj1
    int new_col = 1;
    Variable_Iterator VI(conj1->mappedVars);
    for(i=1; VI; VI++, i++) 
	{
	Variable_ID v = *VI;
	if(v->kind() != Wildcard_Var) 
	    {
	    conj3->mappedVars.append(v);
	    int fr_ix = i;
	    copy_column(p3, new_col, p1, fr_ix, 0, 0);
	    zero_column(p3, new_col, p1->nEQs, p1->nGEQs,
			p2->nEQs, p2->nGEQs);
	    new_col++;
	    }
	}
  
    // copy protected variables column from conj2,
  // checking if conj3 already has this variable from conj1
    for(i=1; i <= conj2->mappedVars.size(); i++) 
	{
	Variable_ID v = conj2->mappedVars[i];
	if(v->kind() != Wildcard_Var) 
	    {
	    int to_ix = conj3->mappedVars.index(v);
	    int fr_ix = i;
	    if(to_ix > 0) 
		{
		// use old column
		copy_column(p3, to_ix, p2, fr_ix, p1->nEQs, p1->nGEQs);
		}
	    else 
		{
		// create new column
		conj3->mappedVars.append(v);
		zero_column(p3, new_col, 0, 0, p1->nEQs, p1->nGEQs);
		copy_column(p3, new_col, p2, fr_ix, p1->nEQs, p1->nGEQs);
		new_col++;
		}
	    }
	}

    p3->safeVars = new_col-1;
  
  // copy wildcards from conj1
    for(i=1; i <= conj1->mappedVars.size(); i++) 
	{
	Variable_ID v = conj1->mappedVars[i];
	if(v->kind() == Wildcard_Var) 
	    {
	    Variable_ID nv = conj3->declare(v);
	    conj3->mappedVars.append(nv);
	    int fr_ix = i;
	    copy_column(p3, new_col, p1, fr_ix, 0, 0);
	    zero_column(p3, new_col, p1->nEQs, p1->nGEQs,
			p2->nEQs, p2->nGEQs);
	    new_col++;
	    }
	}
  
    // copy wildcards from conj2
    for(i=1; i <= conj2->mappedVars.size(); i++) 
	{
	Variable_ID v = conj2->mappedVars[i];
	if(v->kind() == Wildcard_Var) 
	    {
	    Variable_ID nv = conj3->declare(v);
	    conj3->mappedVars.append(nv);
	    int fr_ix = i;
	    zero_column(p3, new_col, 0, 0, p1->nEQs, p1->nGEQs);
	    copy_column(p3, new_col, p2, fr_ix, p1->nEQs, p1->nGEQs);
	    new_col++;
	    }
	}
 
    p3->nVars = new_col-1;
    checkVars(p3->nVars);
    p3->variablesInitialized = 1;
    for(i=1; i<=p3->nVars; i++)
      p3->var[i] = p3->forwardingAddress[i] = i;
    
    conj3->cols_ordered = true;
    conj3->simplified = false;
    conj3->verified = false;
    
    if(pres_debug>=2) 
	{
	use_ugly_names++;
	fprintf(DebugFile, ">>> Merge conjuncts: result is:\n");
	conj3->prefix_print(DebugFile);
	fprintf(DebugFile, "\n");
	use_ugly_names--;
	}

    conj3->assert_leading_info();

    return conj3;
    }




//
// Reorder variables by swapping.
// cols_ordered is just a hint that thorough check needs to be done.
// Sets _safeVars.
// 
void Conjunct::reorder() {
  if(!cols_ordered) {
    int var_no = mappedVars.size();
    int first_wild = 1;
    int last_prot = var_no;
    while(first_wild < last_prot) {
      for(; first_wild<=var_no && mappedVars[first_wild]->kind()!=Wildcard_Var;
	  first_wild++) ;
      for(; last_prot>=1 && mappedVars[last_prot]->kind()==Wildcard_Var;
	  last_prot--) ;
      if(first_wild < last_prot) {
	problem->swapVars(first_wild, last_prot);
	problem->variablesInitialized = false;
	Var_Decl *t = mappedVars[first_wild];
	mappedVars[first_wild] = mappedVars[last_prot];
	mappedVars[last_prot] = t;
	if(pres_debug) {
	  fprintf(DebugFile,
		  "<<<OrderConjCols>>>: swapped var-s %d and %d\n",
		  first_wild, last_prot);
	}
      }
    }

    int safe_vars;
    for(safe_vars=0;
	safe_vars<var_no && mappedVars[safe_vars+1]->kind()!=Wildcard_Var;
	safe_vars++) ;

#if ! defined NDEBUG
    for(int s = safe_vars ; s<var_no ; s++ )
	{
	assert(mappedVars[s+1]->kind() == Wildcard_Var);
	}
#endif

    problem->safeVars = safe_vars;
    cols_ordered = true;
  }
}



// Wherever possible, move function symbols to input tuple.
// This ensures that if in == out, red F(in) = x is redundant
// with black F(out) = x

void Conjunct::move_UFS_to_input()
    {  
    if (guaranteed_leading_0s > 0)
	{
	Set<Global_Var_ID> already_done;
	int remapped = 0;
	skip_finalization_check++;
	Rel_Body *body = relation();

	assert(body);
	
	for (Variable_ID_Iterator func(*body->global_decls());
	     func;
	     func++)
	    {
	    Global_Var_ID f = (*func)->get_global_var();
	    if (f->arity() <= guaranteed_leading_0s)
	      if (!already_done.contains(f) &&
   		body->has_local(f, Input_Tuple) &&
		body->has_local(f, Output_Tuple))
		{
		already_done.insert(f);

		// equatE f(in) = f(out)
		Variable_ID f_in  = body->get_local(f, Input_Tuple);
		Variable_ID f_out = body->get_local(f, Output_Tuple);
		if (f_in != f_out) {
			EQ_Handle e = add_EQ(1);
		
			e.update_coef_during_simplify(f_in, -1);
			e.update_coef_during_simplify(f_out, 1);
		
			f_out->remap = f_in;
			remapped = 1;
			}
		}	    
	    }

	if (remapped)
	    {
	    remap();
	    combine_columns();
	    reset_remap_field(*body->global_decls());
	    remapped = 0;
	    }
	
	skip_finalization_check--;
	}
    }





//
// Simplify CONJ.
// Return TRUE if there are solutions, FALSE -- no solutions.
//
int simplify_conj(Conjunct* conj, int ver_sim, int simplificationEffort, int color) {
  if (conj->verified 
	&& simplificationEffort <= conj->r_constrs 
	&& (conj->simplified  || simplificationEffort < 0)
	&& !color) {
	  if(pres_debug) {
	    fprintf(DebugFile, "$$$ Redundant simplify_conj ignored (%d,%d,%d)\n",ver_sim,simplificationEffort,color);
	    conj->prefix_print(DebugFile);
	    }
	  return 1;
	  };

  if (simplificationEffort < 0) simplificationEffort = 0;
  conj->move_UFS_to_input();
  conj->reorder();

  Problem *p = conj->problem;

  use_ugly_names++;

  int i;
  for(i=0; i<p->nGEQs; i++) {
    p->GEQs[i].touched = 1;
  }
  for(i=0; i<p->nEQs; i++) {
    p->EQs[i].touched = 1;
  }

  if(pres_debug) {
    fprintf(DebugFile, "$$$ simplify_conj (%d,%d,%d)[\n",ver_sim,simplificationEffort,color);
    conj->prefix_print(DebugFile);
  }

  assert(conj->cols_ordered);

  int ret_code;
  assert(p == conj->problem);
  if(!color) {
    ret_code = conj->simplifyProblem(ver_sim && ! conj->verified,0,simplificationEffort);
  } else {
    ret_code = conj->redSimplifyProblem(simplificationEffort,1);
    ret_code = (ret_code==redFalse ? 0 : 1);
  }
  assert(p->nSUBs==0);

  if(ret_code == 0) {
    if(pres_debug)
      fprintf(DebugFile, "] $$$ simplify_conj : false\n\n");
    delete conj;
    use_ugly_names--;
    return(false);
  }


  //
  // mappedVars is mapping from columns to Variable_IDs.
  // Recompute mappedVars for problem returned from ip.c
  //
  Variable_ID_Tuple new_mapped(0);  // This is expanded by "append"
  for (i=1; i<=p->safeVars; i++) {
    // what is now in column i used to be in column p->var[i]
    Variable_ID v = conj->mappedVars[p->var[i]];
    assert(v->kind() != Wildcard_Var);
    new_mapped.append(v);
  }

  /* Redeclare all wildcards that weren't eliminated. */
  free_var_decls(conj->myLocals);  conj->myLocals.clear();

  conj->mappedVars = new_mapped;  
  for (i = p->safeVars+1; i<=p->nVars; i++) {
    Variable_ID v = conj->declare();
    conj->mappedVars.append(v);
  }

  // reset var and forwarding address if desired.
  p->variablesInitialized = 1;
  for(i=1; i<=conj->problem->nVars; i++)
    conj->problem->var[i] = conj->problem->forwardingAddress[i] = i;
      
  if(pres_debug) {
    fprintf(DebugFile, "] $$$ simplify_conj\n");
    conj->prefix_print(DebugFile);
    fprintf(DebugFile, "\n");
  }


  use_ugly_names--;
  conj->simplified = true;
  conj->setup_anonymous_wildcard_names();

  return(true);
}

const char* get_var_name(unsigned int col, void * void_conj) {
#if defined PRINT_COLUMN_NUMBERS
  static char scum[512];
#endif
  Conjunct *c = (Conjunct *) void_conj;
  if (col == 0)
    return 0;
  Variable_ID v = c->mappedVars[col];
  assert(v!=0);
#if defined PRINT_COLUMN_NUMBERS
  strcpy(scum, v->char_name());
  sprintf(scum + strlen(scum), "(%d)", col);
  return scum;
#endif
  return v->char_name();
}

int Conjunct::rank() {
    Conjunct *C = this->copy_conj_same_relation();
    C->reorder();
    C->ordered_elimination(C->relation()->global_decls()->size());
    int C_rank = 0;
    for(Variable_Iterator vi = C->mappedVars; vi; vi++)
	if(C->find_column(*vi) > 0) C_rank++;
    delete C;
    return C_rank;

}


void Conjunct::query_difference(Variable_ID v1, Variable_ID v2,
				coef_t &lowerBound, coef_t &upperBound,
				bool &guaranteed)
    {
    int c1 = get_column(v1);
    int c2 = get_column(v2);
    assert(c1 && c2);
    problem->query_difference(c1, c2, lowerBound, upperBound, guaranteed);
    }


void Conjunct::query_variable_bounds(Variable_ID v, 
				     coef_t &lowerBound, coef_t &upperBound)
{
 int c = get_column(v);
 assert (c);
 problem->query_variable_bounds(c, &lowerBound, &upperBound);
}

bool Conjunct::query_variable_used(Variable_ID v)
{
 for (GEQ_Iterator g = GEQs(); g.live(); g.next()) {
	if ((*g).get_coef(v)) return true;
	}
 for (EQ_Iterator e = EQs(); e.live(); e.next()) {
	if ((*e).get_coef(v)) return true;
	}
 return false;
}


int Conjunct::simplifyProblem(int verify, int subs, int redundantElimination)
    {
    if (verified) verify = 0;
    int result = problem->simplifyProblem(verify, subs, redundantElimination);
    if (result == false && !exact)
	exact=true;
    assert(!(verified && verify && result == false));
    if (verify && result) verified = true;
    else if (!result) verified = false;
    return result;
    }

// not as confident about this one as the previous:
int Conjunct::redSimplifyProblem(int effort, int computeGist)
    {
    redCheck result = problem->redSimplifyProblem(effort, computeGist);
    if (result == redFalse && !exact)
	exact=true;
    return result;
    }
//
// Add given list of wildcards S to this Conjunct.
// Clears argument. (That's very important, otherwise those var_id's get freed)
// Push_exists takes responsibility for reusing or deleting Var_ID's;
//  here we reuse them.  Must also empty out the Tuple when finished (joins).
void Conjunct::push_exists(Variable_ID_Tuple &S) {
  for(Tuple_Iterator<Variable_ID> VI(S); VI; VI++) {
    (*VI)->var_kind = Wildcard_Var;
  }
  myLocals.join(S);       // Sets S to be empty.
  cols_ordered = false;
  simplified = false;
}

Conjunct *Formula::add_conjunct() {
    assert_not_finalized();
    assert(can_add_child());
    Conjunct *f = new Conjunct(this, myRelation);
    myChildren.append(f);
    return f;
}

// Compress/uncompress functions

bool Conjunct::is_compressed() {
  if(problem!=NULL && comp_problem==NULL) {
    return false;
  } else if(problem==NULL && comp_problem!=NULL) {
    return true;
  } else {
    PresErrAssert("Conjunct::is_compressed: bad conjunct");
    return false;
  }
}

void Conjunct::compress() {
  if(!is_compressed()) {    // compress
    comp_problem = new Comp_Problem(problem);
    delete problem;
    problem = NULL;
  }
}

void Conjunct::uncompress() {
  if(is_compressed()) {
    problem = comp_problem->UncompressProblem();
    delete comp_problem;
    comp_problem = NULL;
  }
}


Comp_Problem::Comp_Problem(Problem *problem) :
_nVars(problem->nVars), 
_safeVars(problem->safeVars),
_get_var_name(problem->get_var_name), 
_getVarNameArgs(problem->getVarNameArgs),
eqs(&problem->EQs[0],problem->nEQs,problem->nVars), 
geqs(&problem->GEQs[0],problem->nGEQs,problem->nVars) {
}

Comp_Constraints::Comp_Constraints(Eqn constrs,
				 int no_constrs, int no_vars) :
n_constrs(no_constrs), 
n_vars(no_vars) {
  coefs = new coef_t[(n_vars+1)*n_constrs];
  int e, v;
  for(e=0; e<n_constrs; e++) {
    for(v=0; v<=n_vars; v++) {
      coefs[coef_index(e,v)] = constrs[e].coef[v];
    }
  }
}

Comp_Constraints::~Comp_Constraints() {
  delete coefs;
}


Problem *Comp_Problem::UncompressProblem() {
  Problem *p = new Problem(eqs.n_constraints(), geqs.n_constraints());
  p->get_var_name     = get_var_name;
  p->getVarNameArgs = _getVarNameArgs;
  p->nVars          = _nVars;
  p->safeVars       = _safeVars;
  for(int i=1; i<=p->nVars; i++) {
		p->forwardingAddress[i] = i;
		p->var[i] = i;
		}
  eqs.UncompressConstr(&p->EQs[0], p->nEQs);
  geqs.UncompressConstr(&p->GEQs[0], p->nGEQs);
  return p;
}

void Comp_Constraints::UncompressConstr(Eqn constrs, short &pn_constrs) {
  int e, v;
  for(e=0; e<n_constrs; e++) {
    eqnnzero(&constrs[e], 0);
    for(v=0; v<=n_vars; v++) {
      constrs[e].coef[v] = coefs[coef_index(e,v)];
    }
    constrs[e].touched = 1;
  }
  pn_constrs = n_constrs;
}


void Conjunct::convertEQstoGEQs() { 
  simplify_conj(this,true,1,black);
  problem->convertEQstoGEQs();
}



} // end of namespace omega
