#if ! defined _omega_i_h
#define _omega_i_h 1

#if ! defined _pres_var_h
#include <omega/pres_var.h>
#endif

#define Assert(c,t) if(!(c)) PresErrAssert(t)

namespace omega {

void PresErrAssert(char *t);

extern Rel_Body null_rel;

extern int     skip_finalization_check;
extern int     skip_set_checks;

// Global input and output variable tuples.

extern Global_Input_Output_Tuple input_vars;
extern Global_Input_Output_Tuple output_vars;
extern Global_Input_Output_Tuple &set_vars;

} // end of namespace omega


#if ! defined DONT_INCLUDE_TEMPLATE_CODE
// with g++258, everything will need to make Tuple<Relation>, as a
// function taking it as an argument is a friend of lots of classes
#include <omega/Relation.h>
#endif

#endif
