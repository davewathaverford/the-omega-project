/* $Id: RelBody.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <basic/assert.h>
#include <basic/util.h>
#include <omega/RelBody.h>
#include <omega/Relation.h>
#include <omega/pres_tree.h>
#include <omega/pres_conj.h>
#include <omega/omega_i.h>

namespace omega {


Rel_Body null_rel;
bool Rel_Body::is_null() const {
  return(this == &null_rel);
}


int Rel_Body::max_ufs_arity()
    {
    int ma = 0, a;
    for (Variable_ID_Iterator v(*global_decls()); v; v++)
	{
	a = (*v)->get_global_var()->arity();
	if (a > ma)
	    ma = a;
	}
    return ma;
    }

int Rel_Body::max_ufs_arity_of_set()
    {
    int ma = 0, a;
    for (Variable_ID_Iterator v(*global_decls()); v; v++)
	if ((*v)->function_of() == Set_Tuple)
	{
	a = (*v)->get_global_var()->arity();
	if (a > ma)
	    ma = a;
	}
    return ma;
    }
int Rel_Body::max_ufs_arity_of_in()
    {
    int ma = 0, a;
    for (Variable_ID_Iterator v(*global_decls()); v; v++)
	if ((*v)->function_of() == Input_Tuple)
	{
	a = (*v)->get_global_var()->arity();
	if (a > ma)
	    ma = a;
	}
    return ma;
    }

int Rel_Body::max_ufs_arity_of_out()
    {
    int ma = 0, a;
    for (Variable_ID_Iterator v(*global_decls()); v; v++)
	if ((*v)->function_of() == Output_Tuple)
	{
	a = (*v)->get_global_var()->arity();
	if (a > ma)
	    ma = a;
	}
    return ma;
    }

int Rel_Body::max_shared_ufs_arity()
    {
    int ma = 0, a;
    for (Variable_ID_Iterator v(*global_decls()); v; v++)
      for (Variable_ID_Iterator v2(*global_decls()); v2; v2++)
	if (*v != *v2 
		&& (*v)->get_global_var() == (*v2)->get_global_var()
		&& (*v)->function_of() != (*v2)->function_of())
	{
	a = (*v)->get_global_var()->arity();
	if (a > ma)
	    ma = a;
	}
    return ma;
    }

//
// Input and output variables.
//
void Rel_Body::name_input_var(int nth, Const_String S) {
  assert(1 <= nth && nth <= number_input && (!is_set() || skip_set_checks > 0));
  In_Names[nth] = S;
}

void Rel_Body::name_output_var(int nth, Const_String S) {
  assert(1<= nth && nth <= number_output && (!is_set() || skip_set_checks > 0));
  Out_Names[nth] = S;
}

void Rel_Body::name_set_var(int nth, Const_String S) {
  assert(1 <= nth && nth <= number_input && is_set());
  In_Names[nth] = S;
}

int Rel_Body::n_inp() const   
   { assert(!is_null() && (!is_set()||skip_set_checks>0)); return number_input;}
int Rel_Body::n_out() const
   { assert(!is_null() && (!is_set()||skip_set_checks>0)); return number_output;}
int Rel_Body::n_set() const
   { assert(!is_null() && (is_set()||skip_set_checks>0));  return number_input;  }

Variable_ID Rel_Body::input_var(int nth) {
  assert(!is_null());
  assert(!is_set() || skip_set_checks>0);
  assert(1 <= nth && nth <= number_input);
  input_vars[nth]->base_name = In_Names[nth];
  return input_vars[nth];
}

Variable_ID Rel_Body::output_var(int nth) {
  assert(!is_null());
  assert(!is_set() || skip_set_checks>0);
  assert(1<= nth && nth <= number_output);
  output_vars[nth]->base_name = Out_Names[nth];
  return output_vars[nth];
}

Variable_ID Rel_Body::set_var(int nth) {
  assert(!is_null());
  assert(is_set() || skip_set_checks>0);
  assert(1 <= nth && nth <= number_input);
  input_vars[nth]->base_name = In_Names[nth];
  return input_vars[nth];
}



//
// Add the AND node to the relation.
// Useful for adding restraints.
//
F_And *Rel_Body::and_with_and() {
  assert(!is_null());
  if (is_simplified())
    DNF_to_formula();
  relation()->finalized = false;
  Formula *f = rm_formula();
  F_And *a = add_and();
  a->add_child(f);
  return a;
}

//
// Add constraint to relation at the upper level.
//
EQ_Handle Rel_Body::and_with_EQ() {
  assert(!is_null());
  if (is_simplified())
    DNF_to_formula();
  assert(! is_shared());  // The relation has been split.
  relation()->finalized = false;
  return find_available_conjunct()->add_EQ();
}

EQ_Handle Rel_Body::and_with_EQ(const Constraint_Handle &initial) {
  assert(!is_null());
  assert(initial.relation()->is_simplified());
  EQ_Handle H = and_with_EQ();
  copy_constraint(H, initial);
  return H;
}


GEQ_Handle Rel_Body::and_with_GEQ() {
  assert(!is_null());
  if (is_simplified())
    DNF_to_formula();
  assert(! is_shared());  // The relation has been split.
  relation()->finalized = false;  // We are giving out a handle.
				// We should evantually implement finalization
                                // of subtrees, so the existing formula cannot
                                // be modified.
  return find_available_conjunct()->add_GEQ();
}

GEQ_Handle Rel_Body::and_with_GEQ(const Constraint_Handle &initial) {
  assert(!is_null());
    assert(initial.relation()->is_simplified());
    GEQ_Handle H = and_with_GEQ();
    copy_constraint(H, initial);
    return H;
}



Conjunct *Rel_Body::find_available_conjunct() {
    Conjunct *c;
  assert(!is_null());

    if (children().empty()) {
      c = add_conjunct();
    } else {
      assert(children().length() == 1);
      Formula *kid = children().front();  // RelBodies have only one child
      c = kid->find_available_conjunct();
      if (c==NULL) {
	remove_child(kid);
	F_And *a = add_and();
	a->add_child(kid);
	c = a->add_conjunct();
      };
    } 
    return c;
}

void Rel_Body::finalize() {
  assert(!is_null());
    if (!is_finalized())
	assert(! is_shared());  // no other pointers into here
    finalized = true;
    if (! children().empty())
	children().front()->finalize();  // Can have at most one child
}

// Null Rel_Body
// This is the only rel_body constructor that has ref_count initialized to 1;
// That's because it's used to construct the global Rel_Body "null_rel". 
// Unfortunately because we don't know in what order global constructors will
// be called, we could create a global relation with the default relation
// constructor (which would set the null_rel ref count to 1), and *then* 
// call Rel_Body::Rel_Body(), which would set it back to 0, leaving a relation
// that points to a rel_body with it's ref_count set to 0!  So this is done as
// a special case, in which the ref_count is always 1.
Rel_Body::Rel_Body() :
Formula(0, this), 
ref_count(1),
status(under_construction),
number_input(0), number_output(0),
In_Names(0), Out_Names(0),
simplified_DNF(NULL),
r_conjs(0), 
finalized(true),
_is_set(false)
{
}


Rel_Body::Rel_Body(int n_input, int n_output) :
Formula(0, this),
ref_count(0),
status(under_construction),
number_input(n_input), number_output(n_output), 
In_Names(n_input), Out_Names(n_output),
simplified_DNF(NULL),
r_conjs(0), 
finalized(false),
_is_set(false)
{ 
  if(pres_debug) {
    fprintf(DebugFile, "+++ Create Rel_Body::Rel_Body(%d, %d) = 0x%p +++\n",
		n_input, n_output, this);
  }
  int i; 
  for(i=1; i<=number_input; i++) {
    In_Names[i] = Const_String();
  }
  for(i=1; i<=number_output; i++) {
    Out_Names[i] = Const_String();
  }
}

Rel_Body::Rel_Body(Rel_Body *r) :
Formula(0, this),
ref_count(0),
status(r->status),
number_input(r->number_input), number_output(r->number_output),
In_Names(r->number_input), Out_Names(r->number_output),
simplified_DNF(NULL),
r_conjs(r->r_conjs), 
finalized(r->finalized),
_is_set(r->_is_set)
{
  if(pres_debug) {
    fprintf(DebugFile, "+++ Copy Rel_Body::Rel_Body(Rel_Body * 0x%p) = 0x%p +++\n",
		r, this);
    prefix_print(DebugFile);
  }

  int i;
  for(i=1;i<=r->number_input;i++) In_Names[i] = r->In_Names[i];
  for(i=1;i<=r->number_output;i++) Out_Names[i] = r->Out_Names[i];
  copy_var_decls(Symbolic,r->Symbolic);

  if(!r->children().empty() && r->simplified_DNF==NULL) {
    Formula *f = r->formula()->copy(this,this);
    f->remap();
    children().append(f);
  } else if(r->children().empty() && r->simplified_DNF!=NULL) {
    simplified_DNF = r->simplified_DNF->copy(this);
    simplified_DNF->remap();
  } else {			// copy NULL relation
  }

  reset_remap_field(r->Symbolic);
}


Rel_Body::Rel_Body(Rel_Body *r, Conjunct *c) :
Formula(0, this),
ref_count(0),
status(uncompressed),
number_input(r->number_input), number_output(r->number_output),
In_Names(r->number_input), Out_Names(r->number_output),
r_conjs(0), 
finalized(r->finalized),
_is_set(r->_is_set)
{
  if(pres_debug) {
    fprintf(DebugFile, "+++ Copy Rel_Body::Rel_Body(Rel_Body * 0x%p, Conjunct * 0x%p) = 0x%p +++\n",r,c,this);
  }

  int i;
  for(i=1;i<=r->number_input;i++) In_Names[i] = r->In_Names[i];
  for(i=1;i<=r->number_output;i++) Out_Names[i] = r->Out_Names[i];
  copy_var_decls(Symbolic,r->Symbolic);

  // assert that r has as many variables as c requires, or that c is from r
  assert(r == c->relation());
  assert(r->simplified_DNF != NULL);
  simplified_DNF = new DNF;
  simplified_DNF->add_conjunct(c->copy_conj_diff_relation(this,this));
  single_conjunct()->remap();

  reset_remap_field(r->Symbolic);
}

Rel_Body::~Rel_Body() {
  if(pres_debug) {
    fprintf(DebugFile, "+++ Destroy Rel_Body::~Rel_Body() 0x%p +++\n", this);
  }
  free_var_decls(Symbolic);
  if(simplified_DNF != NULL) {
    delete simplified_DNF;
  }
}

//
// Take a relation that has been simplified and convert it 
// back to formula form.  
//
void Rel_Body::DNF_to_formula() {
  assert(!is_null());
  if (simplified_DNF != NULL) {
    simplified_DNF->DNF_to_formula(this);
    simplified_DNF = NULL;
  status = under_construction;
  }
}

bool Rel_Body::can_add_child() {
    assert(this != &null_rel);
    return n_children() < 1;
}



// ********************
//  Simplify functions
// ********************


extern int s_rdt_constrs;



//
// Simplify a given relation.
// Store the resulting DNF in the relation, clean out the formula.
//
void Rel_Body::simplify() {
  Rel_Body::simplify(false,false);
}

void Rel_Body::simplify(int rdt_conjs, int rdt_constrs) {
  if(simplified_DNF == NULL) {
    finalized = true;
    if(children().empty()) {
      simplified_DNF = new DNF;
    } else {
      if(pres_debug) {
        if(DebugFile==NULL) {
  	DebugFile = fopen("test.out", "w");
  	if(DebugFile==NULL)
  	  fprintf(stderr, "Can not open file test.out\n");
        }
      }

      assert(children().length()==1);
      if(pres_debug) {
	fprintf(DebugFile, "=== %p Rel_Body::simplify(%d, %d) Input tree (%d) ===\n",
		this,rdt_conjs,rdt_constrs,r_conjs);
	prefix_print(DebugFile);
      }
      verify_tree();

      beautify();
      verify_tree();

      rearrange();
      verify_tree();

      beautify();
      verify_tree();

      s_rdt_constrs = rdt_constrs;
      if(pres_debug) {
        fprintf(DebugFile, "\n=== In simplify, before DNFize ===\n");
        prefix_print(DebugFile);
      }
      DNFize();
      if(pres_debug) {
        fprintf(DebugFile, "\n=== In simplify, after DNFize ===\n");
        prefix_print(DebugFile);
      }
      verify_tree();


      simplified_DNF->rm_redundant_inexact_conjs();
      verify_tree();

      if (rdt_conjs > 0 && !simplified_DNF->is_definitely_false() && simplified_DNF->length() > 1)
{
        simplified_DNF->rm_redundant_conjs(rdt_conjs-1);
        verify_tree();
      }
      

      if(pres_debug) {
        fprintf(DebugFile, "\n=== Resulting Relation ===\n");
        prefix_print(DebugFile);
      }
    }

  } else {
    /* Reprocess DNF to get rid of redundant stuff */

    if (rdt_constrs < 0) return;
    simplified_DNF->rm_redundant_inexact_conjs();

    if (rdt_conjs > r_conjs) {
      if(pres_debug) 
	fprintf(DebugFile,"=== Rel_Body::simplify() redundant CONJUNCTS ===\n");
      simplified_DNF->rm_redundant_conjs(rdt_conjs-1);
    }
    if (rdt_constrs > 0 ) {
      if(pres_debug) 
	fprintf(DebugFile,"=== Rel_Body::simplify() redundant CONSTR-S ===\n");
      s_rdt_constrs = rdt_constrs;
      simplified_DNF->simplify();
    }
  }

  r_conjs = rdt_conjs;

  for(DNF_Iterator D(simplified_DNF); D.live(); D.next()) {
    D.curr()->set_relation(this);
    D.curr()->set_parent(this);
  }
}


// ******************
//  Query functions
// ******************


//
// Check if relation has a single conjunct formula and return this conjunct.
//
Conjunct *Rel_Body::single_conjunct() {
  simplify();
  return simplified_DNF->single_conjunct();
}

bool Rel_Body::has_single_conjunct() {
  simplify();
  return simplified_DNF->has_single_conjunct();
}

//
// Remove and return first conjunct
//
Conjunct *Rel_Body::rm_first_conjunct() {
  simplify();
  return simplified_DNF->rm_first_conjunct();
}


//
//
//
void Rel_Body::query_difference(Variable_ID v1, Variable_ID v2,
			      coef_t &lowerBound, coef_t &upperBound, bool &guaranteed)
    {
    simplify();

    coef_t _lb, _ub;
    int first = 1;
    bool _g;
    lowerBound = negInfinity;  // default values if no DNF's
    upperBound = posInfinity;
    guaranteed = 0;

    for (DNF_Iterator D(simplified_DNF); D.live(); D.next())
	{
	(*D)->query_difference(v1, v2, _lb, _ub, _g);
	if (first) 
	    {
	    lowerBound = _lb;
	    upperBound = _ub;
	    guaranteed = _g;
	    first = 0;
	    }
	else
	    {
	    guaranteed = guaranteed && _g;
	    lowerBound = omega::min(lowerBound, _lb);
	    upperBound = omega::max(upperBound, _ub);
	    }
	}
    }
//
//
//
void Rel_Body::query_variable_bounds(Variable_ID v, 
				     coef_t &lowerBound, coef_t &upperBound)
    {
    simplify();

    coef_t _lb, _ub;
    int first = 1;
    lowerBound = negInfinity;  // default values if no DNF's
    upperBound = posInfinity;

    for (DNF_Iterator D(simplified_DNF); D.live(); D.next())
	{
	(*D)->query_variable_bounds(v, _lb, _ub);
	if (first) 
	    {
	    lowerBound = _lb;
	    upperBound = _ub;
	    first = 0;
	    }
	else
	    {
	    lowerBound = omega::min(lowerBound, _lb);
	    upperBound = omega::max(upperBound, _ub);
	    }
	}
    }

//
// Simplify formula if needed and return the resulting DNF.
//
DNF* Rel_Body::query_DNF() {
  return(query_DNF(false,false));
}

DNF* Rel_Body::query_DNF(int rdt_conjs, int rdt_constrs) {
  simplify(rdt_conjs, rdt_constrs);
  return(simplified_DNF);
}

//
// Other formula queries.
//

// Interpret UNKNOWN as true, then check satisfiability
// i.e., check if the formula simplifies to FALSE, since the library
// will never say that if the *known* constraints are unsatisfiable by 
// themselves.
bool Rel_Body::is_upper_bound_satisfiable() {
  int tmp = s_rdt_constrs;
  s_rdt_constrs = -1;
  simplify();
  s_rdt_constrs = tmp;
  return(!simplified_DNF->is_definitely_false());
}

// Interpret UNKNOWN as false, then check satisfiability
// i.e., check if there exist any exact conjuncts in the solution
bool Rel_Body::is_lower_bound_satisfiable() {
  int tmp = s_rdt_constrs;
  s_rdt_constrs = -1;
  simplify();
  s_rdt_constrs = tmp;
  for(DNF_Iterator d(simplified_DNF); d; d++)
    if((*d)->is_exact()) return true;
  return false;
}

bool Rel_Body::is_satisfiable() {
  assert(is_lower_bound_satisfiable() == is_upper_bound_satisfiable());
  return is_upper_bound_satisfiable();
}

// Check if we can easily determine if the formula evaluates to true.
bool Rel_Body::is_obvious_tautology() {
  int tmp = s_rdt_constrs;
  s_rdt_constrs = 0;
  simplify();
  s_rdt_constrs = tmp;
  return(simplified_DNF->is_definitely_true());
}

// Expensive check to determine if the formula evaluates to true.
bool Rel_Body::is_definite_tautology() {
  if(is_obvious_tautology()) return true;
  Relation l =  Lower_Bound(Relation(*this,1));
  return !(Complement(l).is_upper_bound_satisfiable());
}

bool Rel_Body::is_unknown() {
  simplify();
  return(has_single_conjunct() && single_conjunct()->is_unknown());
}

//
// Get accuracy status of the relation
//

Rel_Unknown_Uses Rel_Body::unknown_uses()
    {
    if (!is_simplified())
	simplify();
    
    Rel_Unknown_Uses local_status=0; 
    int n_conj=0;

    for (DNF_Iterator c(simplified_DNF); c; c++)
	{
	n_conj++;
	if ((*c)->is_exact())
	    local_status |= no_u;
	else if ((*c)->is_unknown())
	    local_status |= or_u;
	else
	    local_status |= and_u;
	}

    if (n_conj == 0)
	{
	assert(local_status == 0);
	local_status = no_u;
	}
    assert(local_status);
#if ! defined NDEBUG
    Rel_Unknown_Uses impossible = (and_u | or_u);
    assert( (local_status & impossible) != impossible);
#endif

    return local_status;
    }

void Rel_Body::interpret_unknown_as_false() {
  simplify();
  simplified_DNF->remove_inexact_conj();  
}
       
void Rel_Body::interpret_unknown_as_true() {
  simplify();
  for(DNF_Iterator d(simplified_DNF); d; d++)
    (*d)->interpret_unknown_as_true();
}
       


void Rel_Body::reverse_leading_dir_info()
    {
    if (is_simplified())
	{
	for (DNF_Iterator c(simplified_DNF); c; c++)
	    (*c)->reverse_leading_dir_info();
	}
    else
	{
	assert(!simplified_DNF);
	assert(children().size() == 1);
	children().front()->reverse_leading_dir_info();
	}
    }

//
// Rel_Body::DNFize just DNF-izes its child node and calls verify
//

DNF* Rel_Body::DNFize()
    {
#if defined(INCLUDE_COMPRESSION)
    assert(!this->is_compressed());
#endif
    if (! simplified_DNF)
	{
	simplified_DNF = children().remove_front()->DNFize();

	int mua = max_shared_ufs_arity();
	if (mua > 0)
	    {
	    if (pres_debug)
		{
		fprintf(DebugFile, "\n=== In DNFize, before LCDNF ===\n");
		prefix_print(DebugFile);
		}

	    simplified_DNF->make_level_carried_to(mua);
	    }

	if(pres_debug)
	    {
	    fprintf(DebugFile, "\n=== In DNFize, before verify ===\n");
	    prefix_print(DebugFile);
	    }

	simplified_DNF->simplify();
        }

    assert(children().length() == 0);

    return simplified_DNF;
    }

void Rel_Body::make_level_carried_to(int level)
    {
    if (!simplified_DNF)
	{
	DNFize();
	}

    assert(simplified_DNF && children().empty());

    simplified_DNF->make_level_carried_to(level);
    }
//
// if direction==0, move all conjuncts with >= level leading 0's to return
//            else  move all conjuncts with level-1 0's followed by
//		    the appropriate signed difference to returned Relation
//

Relation Rel_Body::extract_dnf_by_carried_level(int level, int direction)
    {
    if (!simplified_DNF)
	{
	DNFize();
	}

    assert(simplified_DNF && children().empty());

    simplified_DNF->make_level_carried_to(level);

    Relation extracted(n_inp(), n_out());
    extracted.copy_names(*this);
    assert(extracted.rel_body->children().empty());
    assert(extracted.rel_body->simplified_DNF == NULL);
    extracted.rel_body->simplified_DNF = new DNF;
    extracted.rel_body->Symbolic = Symbolic;

    DNF *remaining = new DNF;
    Conjunct *curr;

    for (curr = simplified_DNF->rm_first_conjunct();
	 curr;
	 curr = simplified_DNF->rm_first_conjunct())
	{
	assert(curr->guaranteed_leading_0s >= level ||
	       curr->guaranteed_leading_0s == curr->possible_leading_0s);
	assert(curr->possible_leading_0s >= 0);

	curr->assert_leading_info();

	if ((direction == 0 && curr->guaranteed_leading_0s >= level) ||
	    (curr->guaranteed_leading_0s == level-1 &&
	     curr->leading_dir_valid_and_known() &&
	     curr->leading_dir * direction > 0))
	    {
	    extracted.rel_body->simplified_DNF->add_conjunct(curr);
	    }
	else
	    {
	    remaining->add_conjunct(curr);
	    }
	}
    delete simplified_DNF;
    simplified_DNF = remaining;

#if ! defined NDEBUG
    for (DNF_Iterator rc(simplified_DNF); rc; rc++)
	(*rc)->assert_leading_info();

    for (DNF_Iterator ec(extracted.rel_body->simplified_DNF); ec; ec++)
	(*ec)->assert_leading_info();
#endif

    finalize();
    extracted.finalize();
    return extracted;
    }

//Compress/uncompress functions

bool Rel_Body::is_compressed() {
#if defined(INCLUDE_COMPRESSION)
  if(is_simplified()) {
    for(DNF_Iterator p(simplified_DNF); p.live(); p.next()) {
      if(p.curr()->is_compressed())
	return true;
    }
  }
  return false;
#else
  return true; // This allows is_compressed assertions to work
#endif
}

void Rel_Body::compress() {
#if !defined(INCLUDE_COMPRESSION)
    return;
#else
  if (status == compressed)
    return;
  if (pres_debug)
    fprintf(DebugFile,">>> Compressing relation %p\n",this);
  simplify();
  for(DNF_Iterator p(simplified_DNF); p.live(); p.next()) {
    p.curr()->compress();
  status = compressed;
  }
#endif
}

void Rel_Body::uncompress() {
#if !defined(INCLUDE_COMPRESSION)
  return;
#else
  if (pres_debug)
    fprintf(DebugFile,"<<< Uncompressing relation %p\n",this);
  assert(is_simplified());
  for(DNF_Iterator p(simplified_DNF); p.live(); p.next()) {
    p.curr()->uncompress();
  status = uncompressed;
  }
#endif
}

} // end of namespace omeganamespace omega {
