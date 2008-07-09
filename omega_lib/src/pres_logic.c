/* $Id: pres_logic.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <omega/pres_logic.h>
#include <omega/pres_conj.h>
#include <omega/pres_quant.h>
#include <omega/omega_i.h>

namespace omega {


GEQ_Handle F_And::add_GEQ(int preserves_level) {
  assert_not_finalized();
  if (pos_conj == NULL || pos_conj->problem->nGEQs >= maxGEQs) {
    pos_conj = NULL;
    for(List_Iterator<Formula*> c(children()); c; c++)
      {
      if ((*c)->node_type()==Op_Conjunct &&
	  ((*c)->really_conjunct())->problem->nGEQs < maxGEQs) {
	pos_conj = (*c)->really_conjunct();
	break;
      }
    }
    if(!pos_conj) pos_conj = add_conjunct();// FERD -- set level if preserved?
  }
  return pos_conj->add_GEQ(preserves_level);
}


EQ_Handle F_And::add_EQ(int preserves_level) {
  assert_not_finalized();
  if (pos_conj == NULL || pos_conj->problem->nEQs >= maxEQs) {
    pos_conj = NULL;
    for(List_Iterator<Formula*> c(children()); c; c++)
      {
      if ((*c)->node_type()==Op_Conjunct &&
	  ((*c)->really_conjunct())->problem->nEQs < maxEQs) {
	pos_conj = (*c)->really_conjunct();
	break;
      }
    }
    if(!pos_conj) pos_conj = add_conjunct();//FERD-set level info if preserved?
  }
  return pos_conj->add_EQ(preserves_level);
}

Stride_Handle F_And::add_stride(int step, int preserves_level) {
  assert_not_finalized();
  if (pos_conj == NULL || pos_conj->problem->nEQs >= maxEQs) {
    pos_conj = NULL;
    for(List_Iterator<Formula*> c(children()); c; c++)
      {
      if ((*c)->node_type()==Op_Conjunct &&
	  ((*c)->really_conjunct())->problem->nEQs < maxEQs) {
	pos_conj = (*c)->really_conjunct();
	break;
      }
    }
    if(!pos_conj) pos_conj = add_conjunct();  // FERD -set level if preserved?
  }
  return pos_conj->add_stride(step, preserves_level);
}

GEQ_Handle F_And::add_GEQ(const Constraint_Handle &constraint,
			  int preserves_level) {
  assert_not_finalized();
  if (pos_conj == NULL || pos_conj->problem->nGEQs >= maxGEQs) {
    pos_conj = NULL;
    for(List_Iterator<Formula*> c(children()); c; c++)
      {
      if ((*c)->node_type()==Op_Conjunct &&
	  ((*c)->really_conjunct())->problem->nGEQs < maxGEQs) {
	pos_conj = (*c)->really_conjunct();
	break;
      }
    }
    if(!pos_conj) pos_conj = add_conjunct();// FERD -- set level if preserved?
  }
  return pos_conj->add_GEQ(constraint, preserves_level);
}


EQ_Handle F_And::add_EQ(const Constraint_Handle &constraint,
			int preserves_level) {
  assert_not_finalized();
  if (pos_conj == NULL || pos_conj->problem->nEQs >= maxEQs) {
    pos_conj = NULL;
    for(List_Iterator<Formula*> c(children()); c; c++)
      {
      if ((*c)->node_type()==Op_Conjunct &&
	  ((*c)->really_conjunct())->problem->nEQs < maxEQs) {
	pos_conj = (*c)->really_conjunct();
	break;
      }
    }
    if(!pos_conj) pos_conj = add_conjunct();//FERD-set level info if preserved?
  }
  return pos_conj->add_EQ(constraint,preserves_level);
}


void F_And::add_unknown() {
  assert_not_finalized();
  if (pos_conj == NULL) {
    for (List_Iterator<Formula*> c(children()); c; c++)
      {
      if ((*c)->node_type()==Op_Conjunct) {
         pos_conj = (*c)->really_conjunct();
	 break;
       }
    }
    if(!pos_conj) pos_conj = add_conjunct(); // FERD - set level if preseved?
  }
  pos_conj->make_inexact();
}     

Conjunct *F_Or::find_available_conjunct() {
	return 0;
	};
Conjunct *F_Not::find_available_conjunct() {
	return 0;
	};

Conjunct *F_And::find_available_conjunct() {
    for(List_Iterator<Formula*> child(children()); child; child++) {
      Conjunct *c = (*child)->find_available_conjunct();
      if (c) return c;
      };
    return 0;
    };


void F_Not::finalize() {
    assert(n_children() == 1);
    Formula::finalize();
}

bool F_Not::can_add_child() {
    return n_children() < 1;
}

F_And *F_And::and_with() {
    assert_not_finalized();
    assert(can_add_child());
    return this;
}

F_And::F_And(Formula *p, Rel_Body *r) : 
Formula(p,r), pos_conj(NULL) {
}

F_Or::F_Or(Formula *p, Rel_Body *r) : 
Formula(p,r){
}

F_Not::F_Not(Formula *p, Rel_Body *r) : 
Formula(p,r){
}

Formula *F_And::copy(Formula *parent, Rel_Body *reln) {
  F_And *f = new F_And(parent, reln);
  for(List_Iterator<Formula*> c(children()); c; c++)
    f->children().append((*c)->copy(f,reln));
  return f;
}

Formula *F_Or::copy(Formula *parent, Rel_Body *reln) {
  F_Or *f = new F_Or(parent, reln);
  for(List_Iterator<Formula*> c(children()); c; c++)
    f->children().append((*c)->copy(f,reln));
  return f;
}

Formula *F_Not::copy(Formula *parent, Rel_Body *reln) {
  F_Not *f = new F_Not(parent, reln);
  for(List_Iterator<Formula*> c(children()); c; c++)
    f->children().append((*c)->copy(f,reln));
  return f;
}
//
// Create F_Exists nodes below this F_Or.
// Copy list S to each of the created nodes.
// Push_exists takes responsibility for reusing or deleting Var_ID's;
//  here we delete them.  Must also empty out the Tuple when finished.
void F_Or::push_exists(Variable_ID_Tuple &S) {
  List<Formula*> mc;
  mc.join(children());

  while(!mc.empty()) {
    Formula *f = mc.remove_front();
    F_Exists *e = add_exists();

    copy_var_decls(e->myLocals, S);
    f->remap();
    reset_remap_field(S);

    e->add_child(f);
  }
  // Since these are not reused, they have to be deleted
  for(Tuple_Iterator<Variable_ID> VI(S); VI; VI++) {
      assert((*VI)->kind() == Exists_Var);
      delete *VI;
  }
  S.clear();
}

void F_Exists::push_exists(Variable_ID_Tuple &S) {
  myLocals.join(S);
  }

F_Not *Formula::add_not() {
    assert_not_finalized();
    assert(can_add_child());
    F_Not *f = new F_Not(this, myRelation);
    myChildren.append(f);
    return f;
}

F_And *Formula::add_and() {
    assert_not_finalized();
    assert(can_add_child());
    F_And *f = new F_And(this, myRelation);
    myChildren.append(f);
    return f;
}

F_And *Formula::and_with() {
    return add_and();
}

F_Or *Formula::add_or() {
    assert_not_finalized();
    assert(can_add_child());
    F_Or *f = new F_Or(this, myRelation);
    myChildren.append(f);
    return f;
}
} // end of namespace omega
