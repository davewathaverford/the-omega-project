/* $Id: pres_beaut.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <omega/pres_tree.h>
#include <omega/pres_conj.h>
#include <omega/Relation.h>
#include <omega/omega_i.h>

namespace omega {

/////////////////////////
//                     //
// Beautify functions  //
//                     //
/////////////////////////


//
// f & true = f
// f | false = f
// f1 & f2 & ... & fn = Conjunct(f1,f2,...,fn)
//

void Rel_Body::beautify() {
  assert(children().length()==1);
  set_parent(NULL,this);

  skip_finalization_check++;
  formula()->beautify();
  Formula *child = formula();
  if(child->node_type()==Op_And && child->children().empty()) {
    remove_child(child);
    delete child;
    add_conjunct();
    
  }
  skip_finalization_check--;

  if(pres_debug) {
    fprintf(DebugFile, "\n=== Beautified TREE ===\n");
    prefix_print(DebugFile);
  }
  assert(children().length()==1);
}

void Formula::beautify()
    {
    // copy list of children, as they may be removed as we work
    List<Formula*> kiddies = myChildren;

    for(List_Iterator<Formula*> c(kiddies); c; c++)
	(*c)->beautify();
    }

void F_Exists::beautify() {
  Formula::beautify();
  assert(children().length()==1);

  if(myLocals.empty()) {
    parent().remove_child(this);
    parent().add_child(children().remove_front());
    delete this;
  } else {
    Formula *child = children().front();
    if(child->node_type() == Op_Conjunct
    	|| child->node_type() == Op_Exists) {
      child->push_exists(myLocals);
      parent().remove_child(this);
      parent().add_child(child);
      children().remove_front();
      delete this;
    }
  }
}

void F_Forall::beautify() {
  Formula::beautify();
  assert(children().length()==1);

  if(myLocals.empty()) {
    parent().remove_child(this);
    parent().add_child(children().remove_front());
    delete this;
  }
}


//
// The Pix-free versions of beautify for And and Or are a bit
// less efficient  than the previous code,  as we keep moving
// things from one list to another, but they do not depend on
// knowing that a Pix is valid after the list is updated, and
// they can always be optimized later if necessary.
// 

void F_Or::beautify() 
    {
    Formula::beautify();

    List<Formula*> uglies, beauties;
    uglies.join(children());  assert(children().empty());
#if ! defined NDEBUG
    foreach(c,Formula*,uglies,c->set_parent(0));
#endif

    while(!uglies.empty())
	{
	Formula *f = uglies.remove_front();
	if(f->node_type()==Op_And && f->children().empty() )
	    {
	    // smth | true = true
	    foreach(c,Formula*,uglies,delete c);
	    foreach(c,Formula*,beauties,delete c);
	    parent().remove_child(this);
	    parent().add_and();
	    delete f;
	    delete this;
	    return;
	    }
	else if(f->node_type()==Op_Or) 
	    {
	    // OR(f[1-m], OR(c[1-n])) = OR(f[1-m], c[1-n])
#if ! defined NDEBUG
	    foreach(c,Formula*,f->children(),c->set_parent(0));
#endif
	    uglies.join(f->children());
	    delete f;
	    }
	else
	    beauties.prepend(f);
	}

    if(beauties.length()==1) 
	{
	beauties.front()->set_parent(&parent());
	parent().remove_child(this);
	parent().add_child(beauties.remove_front());
	delete this;
	}
    else
	{
	foreach(c,Formula*,beauties,(c->set_parent(this),
				     c->set_relation(relation())));
	assert(children().empty());
	children().join(beauties);
	}
    }

void F_And::beautify()
    {
    Formula::beautify();

    Conjunct *conj = NULL;

    List<Formula*> uglies, beauties;
    uglies.join(children());  assert(children().empty());
#if ! defined NDEBUG
    foreach(c,Formula*,uglies,c->set_parent(0));
#endif

    while(!uglies.empty())
	{
	Formula *f = uglies.remove_front();
	if (f->node_type() == Op_Conjunct)
	    if(conj==NULL)
		conj = f->really_conjunct();
	    else
		{
		Conjunct *conj1 = merge_conjs(conj,
					     f->really_conjunct(),
					     MERGE_REGULAR);
		delete f;
		delete conj;
		conj = conj1;
		}
	else if(f->node_type()==Op_Or && f->children().empty())
	    {
	    // smth & false = false
	    foreach(c,Formula*,uglies,delete c);
	    foreach(c,Formula*,beauties,delete c);
	    parent().remove_child(this);
	    parent().add_or();
	    delete f;
	    delete conj;
	    delete this;
	    return;
	    }
	else if(f->node_type()==Op_And) 
	    {
	    //  AND(f[1-m], AND(c[1-n])) = AND(f[1-m], c[1-n])
#if ! defined NDEBUG
	    foreach(c,Formula*,f->children(),c->set_parent(0));
#endif
	    uglies.join(f->children());
	    delete f;
	    }
	else
	    beauties.prepend(f);
	}

    if(conj!=NULL)
	beauties.prepend(conj);

    if(beauties.length()==1) 
	{
	beauties.front()->set_parent(&parent());
	parent().remove_child(this);
	parent().add_child(beauties.remove_front());
	delete this;
	}
    else
	{
	foreach(c,Formula*,beauties,(c->set_parent(this),
				     c->set_relation(relation())));
	assert(children().empty());
	children().join(beauties);
	}
    }

void F_Not::beautify() {
  Formula::beautify();
  assert(children().length()==1);
  Formula *child = children().front();

  if(child->node_type()==Op_And && child->children().empty()) {
    // Not TRUE = FALSE
    parent().remove_child(this);
    parent().add_or();
    delete this;
  } else if (child->node_type()==Op_Or && child->children().empty()) {
    // Not FALSE = TRUE
    parent().remove_child(this);
    parent().add_and();
    delete this;
  }
}

void Conjunct::beautify() {
  if(is_true()) {
    parent().remove_child(this);
    parent().add_and();
    delete this;
  }
}

} // end of namespace omega
