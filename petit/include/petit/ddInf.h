/* $Id: ddInf.h,v 1.1.1.1 2000/06/29 19:24:23 dwonnaco Exp $ */
/* headers for ddInf.c */

#ifndef Already_Included_DDInf
#define Already_Included_DDInf

#include <basic/bool.h>

extern bool storeInf;

/* counting atomic DD vectors */

void resetAtomCount(void);
void addAtomCount(dir_and_diff_info * d_info);

/* printing */

void n_atom_output(unsigned int alg,  node * a1, node * a2);
void class_inf_output(node * a1, node * a2);

/* dependence classification information */

void setApplReason(int reason);
void setEpsInf(int kind);
int  getEpsInf(void);
#endif
