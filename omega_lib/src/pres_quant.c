/* $Id: pres_quant.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <omega/pres_quant.h>
#include <omega/omega_i.h>

namespace omega {

F_Forall::F_Forall(Formula *p, Rel_Body *r) : 
F_Declaration(p,r){
}

F_Exists::F_Exists(Formula *p, Rel_Body *r) : 
F_Declaration(p,r){
}

F_Exists::F_Exists(Formula *p, Rel_Body *r, Variable_ID_Tuple &S) :
F_Declaration(p,r,S) {
}


Formula *F_Forall::copy(Formula *parent, Rel_Body *reln) {
  F_Forall *f = new F_Forall(parent, reln);
  copy_var_decls(f->myLocals, myLocals);
  for(List_Iterator<Formula*> c(children()); c; c++)
    f->children().append((*c)->copy(f,reln));
  reset_remap_field(myLocals);
  return f;
}

Formula *F_Exists::copy(Formula *parent, Rel_Body *reln) {
  F_Exists *f = new F_Exists(parent, reln);
  copy_var_decls(f->myLocals, myLocals);
  for(List_Iterator<Formula*> c(children()); c; c++)
    f->children().append((*c)->copy(f,reln));
  reset_remap_field(myLocals);
  return f;
}

Variable_ID F_Forall::declare(Const_String s) {
  return do_declare(s, Forall_Var);
}

Variable_ID F_Forall::declare() {
  return do_declare(Const_String(), Forall_Var);
}

Variable_ID F_Forall::declare(Variable_ID v) {
  return do_declare(v->base_name, Forall_Var);
}


Variable_ID F_Exists::declare(Const_String s) {
  return do_declare(s, Exists_Var);
}

Variable_ID F_Exists::declare() {
  return do_declare(Const_String(), Exists_Var);
}

Variable_ID F_Exists::declare(Variable_ID v) {
  return do_declare(v->base_name, Exists_Var);
}


Conjunct *F_Forall::find_available_conjunct() {
	return 0;
	};
Conjunct *F_Exists::find_available_conjunct() {
    	assert(children().length() == 1 || children().length() == 0);
	if (children().length() == 0)
	    return 0;
	else 
	    return children().front()->find_available_conjunct();
        }

F_Exists *Formula::add_exists() {
    assert_not_finalized();
    assert(can_add_child());
    F_Exists *f = new F_Exists(this, myRelation);
    myChildren.append(f);
    return f;
}

F_Exists *Formula::add_exists(Variable_ID_Tuple &S) {
    assert_not_finalized();
    assert(can_add_child());
    F_Exists *f = new F_Exists(this, myRelation, S);
    myChildren.append(f);
    return f;
}

F_Forall *Formula::add_forall() {
    assert_not_finalized();
    assert(can_add_child());
    F_Forall *f = new F_Forall(this, myRelation);
    myChildren.append(f);
    return f;
}


} // end of namespace omeganamespace omega {
