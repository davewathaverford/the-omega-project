#include <basic/bool.h>
#include <omega.h>

// THIS FILE IS USED TO PROVIDE THE CORRECT ONE-PASS
// LINKING FOR THE OMEGA LIBRARY 

// ALL FUNCTIONS THAT ARE REFERRED FROM THIS FILE ARE
// ALWAYS LINKED. THESE FUNCTIONS INCLUDE:
//   ALL THE VIRTUAL FUNCTIONS  (ALWAYS LINKED IN ANY CASE)
//   FUNCTIONS USED IN lib_hack (LINKED BECUASE OF OUR HACK)
// THIS ALLOWS ALSO TO USE THIS FILE TO GENERATE FILE ubiquitous.names
// THAT CONTAINS LIST OF THE FUNCTIONS THAT ARE ALWAYS LINKED

namespace omega {


bool WANT_SEGM_FAULT = false;

bool lib_hack(Conjunct *c, DNF * d, Rel_Body * rb) {
bool result=false;
 if (WANT_SEGM_FAULT) {
// relation constructors
  Relation r(0,0);
  Relation rc(r,c);
  Relation rbi(*rb,1);
  Relation r0;
  Relation rr(r);

// input/output variable functions
  r.n_inp();
  r.n_out();
  r.n_set();
  r.input_var(1);
  r.output_var(1);
  r.set_var(1);       
  
//other functions
  rr=r;
  bool l=r.is_upper_bound_satisfiable();
  d->prefix_print(NULL);

  result=true;
 }
return result;
}


} // end omega namespace
