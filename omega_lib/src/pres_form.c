/* $Id: pres_form.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <omega/pres_form.h>
#include <omega/pres_tree.h>
#include <omega/pres_conj.h>
#include <omega/Relation.h>
#include <omega/omega_i.h>

namespace omega {

//
// Children and parents.
//
void Formula::remove_child(Formula *kid)
    {
    assert(&kid->parent() == this);
    if (myChildren.front() == kid) 
	myChildren.del_front();
    else
	{
	List_Iterator<Formula*> j,k;
	for(j=List_Iterator<Formula*>(myChildren); *j != kid && j; k=j, j++)
	    ;
	
	if (k)
	    myChildren.del_after(k);
	else
	    Assert(0,"Child to be removed not found in child list");
	}
    }


void Formula::add_child(Formula *kid) {
    assert(can_add_child());
    myChildren.append(kid);
    kid->myParent = this;
    kid->myRelation = this->relation();
}

void Formula::replace_child(Formula *child, Formula* new_child) {
    assert(&child->parent() == this);
    for(List_Iterator<Formula *> LI(myChildren);  LI;  LI++)
	if(*LI == child)
	{
	    *LI = new_child;
	    new_child->myParent = this;
	    new_child->myRelation = this->relation();
	    break;
	}
}

void Formula::set_parent(Formula *parent, Rel_Body *reln) {
  myParent = parent;
  myRelation = reln;
  for(List_Iterator<Formula*> c(myChildren); c; c++)
    (*c)->set_parent(this,reln);
}

//
// Function that sets myRelation pointers in a tree.
//
void Formula::set_relation(Rel_Body *r) {
  myRelation = r;
  for(List_Iterator<Formula *> FI(myChildren); FI; FI++)
    (*FI)->set_relation(r);
}


//
// Function that descends to conjuncts to merge columns
//
void Formula::combine_columns() {
  foreach(child,Formula *,myChildren,child->combine_columns());
}


void Formula::finalize()
    {
    for(List_Iterator<Formula*> c(children()); c; c++)
	(*c)->finalize();
    }

bool Formula::can_add_child() {
    return true;
}



Conjunct *Formula::really_conjunct()
    {
    assert(0 && "really_conjunct() called on something that wasn't");
    return NULL;
    }

Formula::Formula(Formula *p, Rel_Body *r) : 
myParent(p), myRelation(r){
}


void Formula::verify_tree() // should be const
    {
#if ! defined NDEBUG
    Any_Iterator<Formula*> c = myChildren.any_iterator();
    for (; c; c++)
	{
	assert((*c)->myParent==this);
	assert((*c)->myRelation==this->myRelation);
	(*c)->verify_tree();
	}
#endif
    }

Formula *Formula::copy(Formula *, Rel_Body *) {
  assert(0);
  return NULL;
}

Formula::~Formula()
    {
    for(List_Iterator<Formula*> c(myChildren); c; c++)
	delete *c;
    myChildren.clear();
    }


void Formula::assert_not_finalized() {
  if (!skip_finalization_check) {
    assert(! relation()->is_finalized());
    assert(! relation()->is_shared());
  }
}

void Formula::reverse_leading_dir_info()
    {
    for(List_Iterator<Formula*> c(myChildren); c; c++)
	(*c)->reverse_leading_dir_info();
    }

void Formula::invalidate_leading_info(int changed)
    {
    for(List_Iterator<Formula*> c(myChildren); c; c++)
	(*c)->invalidate_leading_info(changed);
    }

void Formula::enforce_leading_info(int guaranteed, int possible, int dir)
    {
    for(List_Iterator<Formula*> c(myChildren); c; c++)
	(*c)->enforce_leading_info(guaranteed, possible, dir);
    }

//
// Push_exists functions.
// Push exists takes responsibility for the Variable_ID's in the Tuple.
// It should:
//    * Re-use them, or
//    * Delete them.
void Formula::push_exists(Variable_ID_Tuple &) {
  assert(0);
}
} // end of namespace omega
