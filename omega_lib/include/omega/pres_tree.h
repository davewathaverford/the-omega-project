#if ! defined _pres_tree_h
#define _pres_tree_h 1

//
// Header to include if you need all the classes to build
// a Presburger formula:
// variables, constraints, nodes for logical operations & quantifiers
//

#if ! defined _pres_var_h
#include <omega/pres_var.h>
#endif
#if ! defined _pres_cnstr_h
#include <omega/pres_cnstr.h>
#endif
#if ! defined _pres_logic_h
#include <omega/pres_logic.h>
#endif
#if ! defined _pres_quant_h
#include <omega/pres_quant.h>
#endif

#endif
