/* $Id: arrayExpand.h,v 1.1.1.1 2000/06/29 19:24:22 dwonnaco Exp $ */
/******************************************************
 *                                                    *
 * Array expansion and privatization.                 *
 * Written by Vadim Maslov vadik@cs.umd.edu 10/26/92. *
 *                                                    *
 ******************************************************/

#ifndef Already_Included_Array_Expand
#define Already_Included_Array_Expand

#include <petit/tree.h>

int  ArrayExpansion(int);
int  ZapExpand(ddnode *);
int  Privatization(int);

#endif
