/* $Id: farkas.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Affine_Closure
#define Already_Included_Affine_Closure

#include <omega/Relation.h>

namespace omega {

typedef enum {Basic_Farkas, 
		Decoupled_Farkas,
		Linear_Combination_Farkas,
		Positive_Combination_Farkas,
		Affine_Combination_Farkas, 
		Convex_Combination_Farkas } Farkas_Type;


extern Relation Farkas(NOT_CONST Relation &R, Farkas_Type op);
extern coef_t farkasDifficulty;

extern Global_Var_ID coefficient_of_constant_term;

} // end of namespace omega

#endif
