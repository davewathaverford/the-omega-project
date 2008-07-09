/* $Id: arrayExpand.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/******************************************************
 *                                                    *
 * Array expansion and privatization.                 *
 * Written by Vadim Maslov vadik@cs.umd.edu 10/26/92. *
 *                                                    *
 ******************************************************/

#ifndef Already_Included_Array_Expand
#define Already_Included_Array_Expand

#include <petit/tree.h>

namespace omega {

int  ArrayExpansion(int);
int  ZapExpand(ddnode *);
int  Privatization(int);

}

#endif
