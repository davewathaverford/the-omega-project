/* $Id: find_affine.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Find_Affine
#define Already_Included_Find_Affine 1

#include <petit/affine.h>

namespace omega {

/*
   Traverse the parse tree, trying to associate affine functions with
   expressions that form loop bounds and array subscripts

   if expr is not affine, associate not_affine with it.

   if expr is an upper or lower bound on a loop index, we may
      have several affine expressions if there are appropriate min
      or max expressions
 */

void find_affine_exprs();
void find_affine_exprs_in(node *n);
void zap_affine_exprs();  /* free space */
void r_zap_affine_exprs(node *n);  /* free space */

/* for terms that are not affine, how do they change? */
unsigned int expr_invariant_at_depth(node *expr);

}

#endif
