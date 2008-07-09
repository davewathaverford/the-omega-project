/* $Id: pres_rear.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <omega/pres_tree.h>
#include <omega/pres_conj.h>
#include <omega/Relation.h>
#include <omega/omega_i.h>

namespace omega {

/////////////////////////
//                     //
// Rearrange functions //
//                     //
/////////////////////////

//
// Rules:
// ~ (f1 | f2 | ... | fn) = ~f1 & ~f2 & ... & fn
// ~ ~ f = f
// Forall v: f = ~ (Exists v: ~ f)
// Exists v: (f1 | ... | fn) = (Exists v: f1) | ... | (Exists v: fn)
//

void Rel_Body::rearrange() {
  assert(children().length()==1);

  skip_finalization_check++;
  formula()->rearrange();
  skip_finalization_check--;

  if(pres_debug) {
    fprintf(DebugFile, "\n=== Rearranged TREE ===\n");
    prefix_print(DebugFile);
  }
}

void Formula::rearrange()
    {
    // copy list of children, as they may be removed as we work
    List<Formula*> kiddies = myChildren;

    for(List_Iterator<Formula*> c(kiddies); c; c++)
	(*c)->rearrange();
    }

//
// Push nots down the tree until quantifier or conjunct, rearrange kids
//
void F_Not::rearrange() {
  Formula *child = children().front();
  Formula *new_child, *f;

  switch(child->node_type()) {
  case Op_Or:
    parent().remove_child(this);
    new_child = parent().add_and();
    while(!child->children().empty()) {
      f = child->children().remove_front(); 
      F_Not *new_not = new_child->add_not(); 
      new_not->add_child(f); 
    }
    delete this;
    break;
//case Op_And:
//  parent().remove_child(this);
//  new_child = parent().add_or();
//  while(!child->myChildren.empty()) {
//    f = child->myChildren.remove_front(); 
//    F_Not *new_not = new_child->add_not(); 
//    new_not->add_child(f); 
//  }
//  delete this;
//  break;
  case Op_Not:
    parent().remove_child(this);
    f = child->children().remove_front(); 
    parent().add_child(f);
    delete this;
    f->rearrange();
    return;
  default:
    new_child = child;
    break;
  }

  new_child->rearrange();
}
	
//
// Convert a universal quantifier to "not exists not".
// Forall v: f = ~ (Exists v: ~ f)
//
void F_Forall::rearrange() {
  Formula &p = parent();
  p.remove_child(this);
  
  F_Not *topnot = p.add_not();
  F_Exists *exist = topnot->add_exists();
  for (Variable_ID_Iterator VI(myLocals); VI; VI++)
	(*VI)->set_kind(Exists_Var);
  exist->myLocals.join(myLocals);

  F_Not *botnot = exist->add_not();
  Formula *f = children().remove_front();
  botnot->add_child(f);

  delete this;

  botnot->rearrange();
}

//
// Exists v: (f1 | ... | fn) = (Exists v: f1) | ... | (Exists v: fn)
//
void F_Exists::rearrange() {
  Formula* child = children().front();
  switch(child->node_type()) {
  case Op_Or:
  case Op_Conjunct:
  case Op_Exists:
    child->push_exists(myLocals);
    parent().remove_child(this);
    parent().add_child(child);
    children().remove_front();
    delete this;
    break;
  default:
    break;
  }

  child->rearrange();
}
} // end of namespace omeganamespace omega {
