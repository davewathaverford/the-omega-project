/* $Id: pres_decl.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <omega/pres_decl.h>
#include <omega/omega_i.h>

namespace omega {

//
// Declare functions.
//
Variable_ID F_Declaration::do_declare(Const_String s, Var_Kind var_type) {
  Variable_ID v;
  assert(var_type != Global_Var);
  if(!s.null()) {
    v = new Var_Decl(s, var_type, 0);
  } else {
    v = new Var_Decl(var_type, 0);
  }
  myLocals.append(v);
  return v;
}

Variable_ID F_Declaration::declare(Const_String) {
  assert(0);  // must be declared in forall, exists, or conjunct
  return(NULL);
}

Section<Variable_ID> F_Declaration::declare_tuple(int n)
    {
    int first = myLocals.size()+1;

    for (int i=1 ; i<=n; i++)
	declare();

    return Section<Variable_ID>(&myLocals, first, n);
    }


void F_Declaration::finalize() {
    assert(n_children() == 1);
    Formula::finalize();
}

bool F_Declaration::can_add_child() {
    return n_children() < 1;
}


F_Declaration::F_Declaration(Formula *p, Rel_Body *r) : 
Formula(p,r),
myLocals(0) {
}

F_Declaration::F_Declaration(Formula *p, Rel_Body *r, Variable_ID_Tuple &S) :
Formula(p,r), myLocals(S) {
}

//
// Destruct declarative node.
// Delete variableID's themselves if they are not global.
//
F_Declaration::~F_Declaration() {
  free_var_decls(myLocals);
}

//Setup names for printing

void F_Declaration::setup_anonymous_wildcard_names() {
  for(Tuple_Iterator<Variable_ID> VI(myLocals); VI; VI++) {
    Variable_ID v = *VI;
    if (v->base_name.null()) v->instance = wildCardInstanceNumber++;
	};
    }

} // end of namespace omega
