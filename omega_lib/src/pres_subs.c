
#include <omega/pres_subs.h>

namespace omega {

Substitutions::Substitutions(Relation &input_R, Conjunct *input_c) {
    int i;
    r = new Relation(input_R,input_c);
    c = r->single_conjunct();
    c->reorder_for_print();
    c->ordered_elimination(r->global_decls()->length());
    int num_subs = c->problem->nSUBs; 
    subs = new eqn[num_subs];
    for(i = 0; i < num_subs; i++)
	subs[i] = SUBs[i];
    subbed_vars.reallocate(num_subs);
    /* Go through and categorize variables as:
         1) substituted, 2) not substituted, 3) wildcard
       Safevars number of variables were not able to be substituted.
       nVars number of total variables, including wildcards.
       nSUBs is the number of substitutions.
       nSUBs + nVars == the number of variables that went in. 
       Then reset var and forwardingAddress arrays in the problem,
       so that they will correctly refer to the reconstructed
       mappedVars. */
    Variable_ID_Tuple unsubbed_vars(c->problem->safeVars);
    for(i = 1; i <= c->mappedVars.size(); i++) 
	if(c->mappedVars[i]->kind() != Wildcard_Var) {
	    int addr = c->problem->forwardingAddress[i];
	    assert(addr == c->find_column(c->mappedVars[i]) && addr != 0);
	    if(addr < 0) {
		assert(-addr <= subbed_vars.size());
		subbed_vars[-addr] = c->mappedVars[i];
	    } else {
		assert(addr <= unsubbed_vars.size());
		unsubbed_vars[addr] = c->mappedVars[i];
	    }
	} else {
	    // Here we don't redeclare wildcards, just re-use them.
	    unsubbed_vars.append(c->mappedVars[i]);
	}
    assert(unsubbed_vars.size() + subbed_vars.size() == c->mappedVars.size());
    c->mappedVars = unsubbed_vars; /* These are the variables that remain */
     
    for(int col = 1; col <= c->problem->nVars; col++){
      c->problem->var[col] = col;
      c->problem->forwardingAddress[col] = col;
    }
}

Substitutions::~Substitutions() {
    delete [] subs;
    delete r;
}

bool Substitutions::substituted(Variable_ID v) {
    return (subbed_vars.index(v) > 0);
}

Sub_Handle Substitutions::get_sub(Variable_ID v){
    assert(substituted(v) && "No substitution for variable");
    return Sub_Handle(this,subbed_vars.index(v)-1,v);
}


bool Substitutions::sub_involves(Variable_ID v, Var_Kind kind){
  assert(substituted(v));
  for(Constr_Vars_Iter i = get_sub(v); i; i++)
    if ((*i).var->kind() == kind)
      return true;
  return false;
}


//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


int Sub_Iterator::live() const {
  return current <= last;
}

void Sub_Iterator::operator++() { this->operator++(0); }

void Sub_Iterator::operator++(int) {
    current++;
}

Sub_Handle Sub_Iterator::operator*() {
  assert(s && current <= last && "Sub_Iterator::operator*: bad call");
  return Sub_Handle(s,current,s->subbed_vars[current+1]);
} 

Sub_Handle Sub_Iterator::operator*() const {
  assert(s && current <= last && "Sub_Iterator::operator*: bad call");
  return Sub_Handle(s,current,s->subbed_vars[current+1]);
} 


//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



Sub_Handle::Sub_Handle(Substitutions *_s, int _e, Variable_ID _v) :
Constraint_Handle(_s->c,&(_s->subs),_e), v(_v) {}

String Sub_Handle::print_to_string() const {
  relation()->setup_names();
  return v->name() + " = " +  this->print_term_to_string();
}

String Sub_Handle::print_term_to_string() const {
  /* The horrible truth is that print_term_to_string is a member
     function of Conjunct, (and then Problem below it) but uses
     nothing from there but the names, so you can pass it a pointer to
     an equation that isn't even part of the conjunct. */
  relation()->setup_names();
  return c->print_term_to_string(&((*eqns)[e]));
}


/*
String Sub_Handle::print_to_string() const {
    return c->problem->print_EQ_to_string(&((*eqns)[e]));
}

String Sub_Handle::print_term_to_string() const {
    return c->print_term_to_string(&(*eqns[e]));
}
*/

} // end of namespace omeganamespace omega {
