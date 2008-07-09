/* $Id: pres_cnstr.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <omega/pres_cnstr.h>
#include <omega/pres_conj.h>
#include <omega/Relation.h>
#include <omega/omega_i.h>


namespace omega {

Constraint_Handle::Constraint_Handle(Conjunct *_c, Eqn * _eqns, int _e) : 
c(_c), eqns(_eqns), e(_e) {
}

GEQ_Handle::GEQ_Handle(Conjunct *_c, int _e) :
Constraint_Handle(_c,&(_c->problem->GEQs),_e) {
}

bool Constraint_Handle::is_const(Variable_ID v){
    bool is_const=true;
    for(Constr_Vars_Iter cvi(*this, false); cvi && is_const; cvi++)
        is_const = ((*cvi).coef == 0 || ((*cvi).var == v && (*cvi).coef !=0));
    return is_const;
}

bool EQ_Handle::operator==(const Constraint_Handle &that){
    Constraint_Handle &e1=*this;
    const Constraint_Handle &e2=that;
    int sign = 0;
    for(Constr_Vars_Iter cvi(e1, false); cvi; cvi++) {
	coef_t c1 = (*cvi).coef;
	coef_t c2 = e2.get_coef((*cvi).var);
	if (sign == 0) sign = (c1*c2>=0?1:-1);
        if (sign*c1 != c2) return false;
	}
    assert(sign != 0);
    {for(Constr_Vars_Iter cvi(e2, false); cvi; cvi++) {
	coef_t c1 = e1.get_coef((*cvi).var);
	coef_t c2 = (*cvi).coef;
        if (sign*c1 != c2) return false;
	}
    }
    return sign * e1.get_const() == e2.get_const();
}

bool GEQ_Handle::operator==(const Constraint_Handle &that){
    Constraint_Handle &e1=*this;
    const Constraint_Handle &e2=that;
    for(Constr_Vars_Iter cvi(e1, false); cvi; cvi++) {
	coef_t c1 = (*cvi).coef;
	coef_t c2 = e2.get_coef((*cvi).var);
        if (c1 != c2) return false;
	}
    {for(Constr_Vars_Iter cvi(e2, false); cvi; cvi++) {
	coef_t c1 = e1.get_coef((*cvi).var);
	coef_t c2 = (*cvi).coef;
        if (c1 != c2) return false;
	}
    }
    return e1.get_const() == e2.get_const();
}




void GEQ_Handle::negate() {
  assert(! this->relation()->is_simplified());
  int i;
  for(i=1; i<=c->problem->nVars; i++) {
    (*eqns)[e].coef[i] = -(*eqns)[e].coef[i];
  }
  (*eqns)[e].coef[0] = -(*eqns)[e].coef[0]-1;
}

bool Constraint_Handle::has_wildcards() const {
      Constr_Vars_Iter C(*this, true);
      if (C.live())
      {
          assert(C.curr_var()->kind() == Wildcard_Var);
	  assert(C.curr_coef() != 0);
	  return 1;
      }
      return 0;
 }

int Constraint_Handle::max_tuple_pos() const {
      int m = 0, pos;
      for( Constr_Vars_Iter C(*this, false); C.live() ; C.next()) {
	  switch (C.curr_var()->kind()) {
	    case Input_Var: 
	    case Output_Var: 
		pos = C.curr_var()->get_position();
		if (m < pos) m = pos;
	    default: 
		;
	    };
      }
	return m;
}


EQ_Handle::EQ_Handle(Conjunct *_c, int _e) :
Constraint_Handle(_c,&(_c->problem->EQs),_e) {
}

//
// Update functions.
//
void Constraint_Handle::update_coef(Variable_ID D, coef_t I) {
  assert(! this->relation()->is_simplified());
  assert(D != 0);
  // The next two assertions are somewhat high-cost.
#if !defined(NDEBUG)
  skip_set_checks++;
  assert((D->kind() != Input_Var || D->get_position() <= this->relation()->n_inp()));
  assert((D->kind() != Output_Var || D->get_position() <= this->relation()->n_out()));
  skip_set_checks--;
#endif
  int col = c->get_column(D);
  (*eqns)[e].coef[col] += I;
}

void Constraint_Handle::update_const(coef_t I) {
  assert(! this->relation()->is_simplified());
  (*eqns)[e].coef[0] += I;
}


// update a coefficient of a variable that already exists in mappedvars

void Constraint_Handle::update_coef_during_simplify(Variable_ID D, coef_t I) {
  assert(D != 0);
  int col = c->get_column(D);
  (*eqns)[e].coef[col] += I;
}

void Constraint_Handle::update_const_during_simplify(coef_t I) {
  (*eqns)[e].coef[0] += I;
}

//
// Get functions.
//

coef_t Constraint_Handle::get_coef(Variable_ID v) const {
  assert(this->relation()->is_simplified());
  assert(v != 0);
  int col = c->find_column(v);
  if(col == 0) {
    return 0;
  } else {
    return (*eqns)[e].coef[col];
  }
} 

coef_t Constraint_Handle::get_coef_during_simplify(Variable_ID v) const {
  assert(v != 0);
  int col = c->find_column(v);
  if(col == 0) {
    return 0;
  } else {
    return (*eqns)[e].coef[col];
  }
} 

coef_t Constraint_Handle::get_const() const {
  assert(this->relation()->is_simplified());
  return((*eqns)[e].coef[0]);
}

coef_t Constraint_Handle::get_const_during_simplify() const {
  return((*eqns)[e].coef[0]);
}

Variable_ID Constraint_Handle::get_local(const Global_Var_ID G)
    {
    return relation()->get_local(G);
    }
Variable_ID Constraint_Handle::get_local(const Global_Var_ID G,
					Argument_Tuple of)
    {
    return relation()->get_local(G, of);
    }

void Constraint_Handle::finalize() {
  c->n_open_constraints--;
}

void Constraint_Handle::multiply(int multiplier) {
  int i;
  assert(! this->relation()->is_simplified());
  for(i=1; i<=c->problem->nVars; i++) {
    (*eqns)[e].coef[i] = (*eqns)[e].coef[i] * multiplier;
  }
  (*eqns)[e].coef[0] = (*eqns)[e].coef[0] * multiplier;
}

Rel_Body *Constraint_Handle::relation() const {
  return c->relation();
}


//
// Variables of constraint iterator.
//
Constr_Vars_Iter::Constr_Vars_Iter(const Constraint_Handle &ch, bool _wild_only) :
eqns(ch.eqns),
e(ch.e),
prob(ch.c->problem),
vars(ch.c->mappedVars),
wild_only(_wild_only) {
    assert(vars.size() == prob->nVars);
    for(current=1; current<=prob->nVars; current++) {
	if((*eqns)[e].coef[current]!=0 && 
	   (!wild_only || vars[current]->kind()==Wildcard_Var)) 
	    return;
    }
}

int Constr_Vars_Iter::live() const
    {
    return (current<=prob->nVars);
    }


void Constr_Vars_Iter::operator++() { this->operator++(0); }

void Constr_Vars_Iter::operator++(int) {
    for(current++ ; current <=prob->nVars; current++)
	if((*eqns)[e].coef[current]!=0 && 
	   (!wild_only || vars[current]->kind()==Wildcard_Var))
	    return;
}


Variable_ID Constr_Vars_Iter::curr_var() const {
  assert(current <= prob->nVars);
  return vars[current];
}

coef_t Constr_Vars_Iter::curr_coef() const {
    assert(current <= prob->nVars);
    return (*eqns)[e].coef[current];
}

Variable_Info Constr_Vars_Iter::operator*() const
    {
    assert(current <= prob->nVars);
    return Variable_Info(vars[current],(*eqns)[e].coef[current]);
    }

//
// Constraint iterator.
//
Constraint_Iterator Conjunct::constraints() {
  return Constraint_Iterator(this);
}

Constraint_Iterator::Constraint_Iterator(Conjunct *_c) : c(_c), current(0),
 last(c->problem->nGEQs-1), eqns(&(c->problem->GEQs)) {
    if(!this->live()) (*this)++; // switch to EQ's if no GEQs
 }

int Constraint_Iterator::live() const {
  return current <=last;
}

void Constraint_Iterator::operator++() { this->operator++(0); }

void Constraint_Iterator::operator++(int) {
    if(++current > last)
	if(eqns == &(c->problem->GEQs)) { // Switch to EQs
	    eqns = &(c->problem->EQs);
	    current = 0;
	    last = c->problem->nEQs-1;
	}
}

Constraint_Handle Constraint_Iterator::operator*() {
  Assert(c && eqns && current <= last, "Constraint_Iterator::operator*: bad call");
  return Constraint_Handle(c,eqns,current);
} 

Constraint_Handle Constraint_Iterator::operator*() const {
  Assert(c && eqns && current <= last, "Constraint_Iterator::operator*: bad call");
  return Constraint_Handle(c,eqns,current);
} 


//
// EQ iterator.
//
EQ_Iterator Conjunct::EQs() {
  return EQ_Iterator(this);
}

EQ_Iterator::EQ_Iterator(Conjunct *_c) : c(_c) {
    last = c->problem->nEQs-1;
    current = 0;
}

int EQ_Iterator::live() const {
  return current <= last;
}

void EQ_Iterator::operator++() { this->operator++(0); }

void EQ_Iterator::operator++(int) {
    current++;
}

EQ_Handle EQ_Iterator::operator*() {
  Assert(c && current <= last, "EQ_Iterator::operator*: bad call");
  return EQ_Handle(c,current);
} 

EQ_Handle EQ_Iterator::operator*() const {
  Assert(c && current <= last, "EQ_Iterator::operator*: bad call");
  return EQ_Handle(c,current);
} 


//
// GEQ iterator.
//
GEQ_Iterator Conjunct::GEQs() {
  return GEQ_Iterator(this);
}

GEQ_Iterator::GEQ_Iterator(Conjunct *_c) : c(_c) {
    last = c->problem->nGEQs-1;
    current = 0;
}

int GEQ_Iterator::live() const {
  return current <= last;
}

void GEQ_Iterator::operator++() { this->operator++(0); }

void GEQ_Iterator::operator++(int) {
    current++;
}


GEQ_Handle GEQ_Iterator::operator*() {
  Assert(c && current <= last, "GEQ_Iterator::operator*: bad call");
  return GEQ_Handle(c,current);
} 

GEQ_Handle GEQ_Iterator::operator*() const {
  Assert(c && current <= last, "GEQ_Iterator::operator*: bad call");
  return GEQ_Handle(c,current);
} 


void copy_constraint(Constraint_Handle H, const Constraint_Handle initial) 
{
  skip_set_checks++;
//  assert(H.relation()->n_inp() == initial.relation()->n_inp());
//  assert(H.relation()->n_out() == initial.relation()->n_out());
    
  H.update_const_during_simplify(initial.get_const_during_simplify());
  if (H.relation() == initial.relation())  
  {
      for( Constr_Vars_Iter C(initial, false); C.live() ; C.next())
      {
	  assert(C.curr_var()->kind()!= Forall_Var &&
		 C.curr_var()->kind()!= Exists_Var);
	  if (C.curr_var()->kind()!= Wildcard_Var)
	      H.update_coef_during_simplify(C.curr_var(), C.curr_coef());
	  else
              // Must add a new wildcard,
              // since they can't be used outside local Conjunct
	      H.update_coef_during_simplify(H.c->declare(), C.curr_coef());
      }
  } else {
      Rel_Body *this_rel = H.relation();
      for( Constr_Vars_Iter C(initial, false); C.live() ; C.next())
      {
	  switch (C.curr_var()->kind()) {
	    case Forall_Var:
	    case Exists_Var:
	      Assert(0, "Can't copy quantified constraints across relations");
	      break;
	    case Wildcard_Var:
	       // for each wildcard var we see, create a new wildcard
	       // will lead to lots of wildcards, but Wayne likes it
               // that way
	    {
		H.update_coef_during_simplify(H.c->declare(), C.curr_coef());
		break;
	    }
	    case Input_Var: //use variable_ID of corresponding position
	    {
		int pos = C.curr_var()->get_position();
		assert(this_rel->n_inp() >= pos);
		Variable_ID V = this_rel->input_var(pos);
		H.update_coef_during_simplify(V, C.curr_coef());
		break;
	    }
	    case Output_Var:  //use variable_ID of corresponding position
	    {
		int pos = C.curr_var()->get_position();
		assert(this_rel->n_out() >= pos);
		Variable_ID V = this_rel->output_var(pos);
		H.update_coef_during_simplify(V, C.curr_coef());
		break;
	    }

	    case Global_Var:  // get this Global's Var_ID in this relation
	    {
	     Variable_ID V;
	     Global_Var_ID G = C.curr_var()->get_global_var();
	     if (G->arity() == 0)
		     V = this_rel->get_local(G);
	     else 
		     V = this_rel->get_local(G,C.curr_var()->function_of());
	     H.update_coef_during_simplify(V, C.curr_coef());
	     break;
	    }
	  default:
	      Assert(0, "copy_constraint: variable of impossible type");
	  }  /* end switch  */
      }      /* end for     */
  }          /* end if      */
  skip_set_checks--;
}            /* copy_constraint */

} // end of namespace omega
