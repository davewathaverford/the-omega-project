/* $Id: Relation.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <omega/Relation.h>
#include <omega/Relations.h>
#include <omega/pres_dnf.h>
#include <omega/pres_conj.h>
#include <omega/Rel_map.h>
#include <omega/omega_i.h>
#include <omega/omega_core/debugging.h>

namespace omega {

//
// Create null relation.
//
Relation::Relation() : rel_body(&null_rel)  {
  rel_body->ref_count = 1;
}

Relation Relation::Null() {
  return Relation();
}

bool Relation::is_null() const {
  return(rel_body == &null_rel);
}


//
// Create a relation. Its will be built later.
//
Relation::Relation(int n_input, int n_output) {
  rel_body = new Rel_Body(n_input,n_output);
  rel_body->ref_count = 1;
}

//
//  Create a Set: a relation constructor, but won't allow implicit conversions
//  from int

#if ! defined NONCOERCIBLE_WONT_WORK_AT_ALL
Relation::Relation(NonCoercible<int> nci) 
#else
Relation::Relation(NonCoercibleInt nci) 
#endif
{
    rel_body = new Rel_Body(value(nci), 0);
    rel_body->ref_count = 1;
    markAsSet();
}

Relation::Relation(Rel_Body &r, int) {
    rel_body = &r;
    r.ref_count++;
   }

Relation Relation::Empty(const Relation &R) {
   if (R.is_set()) return Relation(R.n_set());
   else return Relation(R.n_inp(),R.n_out());
   }
//
// Create relation which is FALSE or TRUE.
//

Relation Relation::True(const Relation &R) {
   if (R.is_set()) return True(R.n_set());
   else return True(R.n_inp(),R.n_out());
   }
    
Relation Relation::False(const Relation &R) {
   if (R.is_set()) return False(R.n_set());
   else return False(R.n_inp(),R.n_out());
   }
    
Relation Relation::Unknown(const Relation &R) {
   if (R.is_set()) return Unknown(R.n_set());
   else return Unknown(R.n_inp(), R.n_out());
   }


Relation Relation::True(int setvars) {
    Relation R(setvars);
    R.add_and();
    R.finalize();
    return R;
}

Relation Relation::True (int in, int out) {
    Relation R(in,out);
    R.add_and();
    R.finalize();
    return R;
}

Relation Relation::False (int setvars) {
    Relation R(setvars);
    R.add_or();
    R.finalize();
    return R;
}

Relation Relation::False (int in, int out) {
    Relation R(in,out);
    R.add_or();
    R.finalize();
    return R;
}


Relation Relation::Unknown (int setvars) {
    Relation R(setvars);
    R.add_and();
    R.finalize();
    R.simplify();
    Conjunct * c= R.single_conjunct();
    c->make_inexact();
    return R;
  }

Relation Relation::Unknown (int in, int out) {
    Relation R(in,out);
    R.add_and();
    R.finalize();
    R.simplify();
    Conjunct * c= R.single_conjunct();
    c->make_inexact();
    return R;
  }


//
// Copy a relation.
//
Relation::Relation(const Relation &r) {
#if defined(INCLUDE_COMPRESSION)
  assert(!r.is_compressed());
#endif
  if (r.is_finalized()) {
    rel_body = r.rel_body;
    rel_body->ref_count++;
  } else {
    assert(! r.rel_body->is_shared());
    rel_body = new Rel_Body(r.rel_body);
    rel_body->ref_count = 1;
  }
}

//
// Copy relation r and replace formula in it with conjunct c.
// Wayne (TM) function.
//
Relation::Relation(const Relation &r, Conjunct *c) {
  rel_body = new Rel_Body(r.rel_body, c);
  rel_body->ref_count = 1;
}


//
// Assign a relation r to this relation.
//
Relation &Relation::operator=(const Relation &r) {
#if defined(INCLUDE_COMPRESSION)
  assert (!r.is_compressed());
#endif
 
  /* === Destroy this === */
  assert(rel_body->ref_count >= 1);
  if(rel_body!=&null_rel && --(rel_body->ref_count)==0) {
    delete rel_body;
  }
  
  /* === Copy r to this === */
  if (r.is_finalized()) {
    rel_body = r.rel_body;
    rel_body->ref_count++;
  } else {
    assert(! r.rel_body->is_shared());
    rel_body = new Rel_Body(r.rel_body);
    rel_body->ref_count = 1;
  }
  return *this;
}


int operator==(const Relation &, const Relation &) { 
    assert(0 && "You rilly, rilly don't want to do this.\n");
    abort();
    return 0;
}

void Relation::copy_names(Rel_Body &r) {
  int t;
  if (is_set()) {
    for(t = 1; t <= r.n_set(); t++) 
        name_set_var(t,r.set_var(t)->base_name);
    }
  else {
    for(t = 1; t <= r.n_inp(); t++) 
        name_input_var(t,r.input_var(t)->base_name);
    for(t = 1; t <= r.n_out(); t++) 
        name_output_var(t,r.output_var(t)->base_name);
    };
  }


// Like makeSet (see Relations.c), but won't invert the relation -- 
// fails if it has output instead of input variables.  Called in Relation 
// functions just after a MapRel, so that we know there are no outputs anyway.

void Relation::markAsSet() {
  assert(!is_null());
  assert(is_set() || (n_inp() >= 0 && n_out() == 0));
  if (!is_set()) split();  // split if we'll modify this
  rel_body->_is_set = true;
  invalidate_leading_info();
}

void Relation::markAsRelation() {
  assert(!is_null());
  if (is_set()) split();  // split if we'll modify this
  rel_body->_is_set = false;
}



Relation::~Relation() {
  assert(rel_body->ref_count >= 1);
  assert(this->is_null() == (rel_body == &null_rel));
  if(rel_body!=&null_rel && --(rel_body->ref_count)==0) {
      if (rel_body == &null_rel) abort();
      delete rel_body;
  }
}



//
// One of the representatives using the body wants to be changed.
// Create a separate body for this rep not to damage other reps.
// Return address of the body. Old rep point to new body.
//
Rel_Body *Relation::split() {
  assert(rel_body != &null_rel && "Error: Attempt to modify a null relation");
  assert (rel_body->ref_count >= 1);
  if(!(rel_body==&null_rel || rel_body->ref_count==1)) {
    if(pres_debug) {
      fprintf(DebugFile, "+++ SPLIT relation +++\n");
    }
    Rel_Body *new_body = new Rel_Body(rel_body);
    new_body->ref_count = 1;
    rel_body->ref_count--;
    rel_body = new_body;
    if(pres_debug>=2) {
      fprintf(DebugFile, " copying 0x%p to give 0x%p\n", this, rel_body);
    }
  }
  return (rel_body);
}

} // end of namespace omeganamespace omega {
