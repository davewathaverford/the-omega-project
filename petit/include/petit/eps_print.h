/* $Id: eps_print.h,v 1.1.1.1 2000/06/29 19:24:24 dwonnaco Exp $ */
#ifndef Already_Included_EpsPrint
#define Already_Included_EpsPrint

void print_subscr(FILE *f, EpsSubscrType * s);
void print_eps_var(FILE *f, Epsvar_id v);
void print_ban_info(FILE *f, BanerjeeInfoType * bi);
void print_comvar(FILE *f, EpsComvarType *cv, int n);
void print_subscr_bounds(FILE * f, SubscrBoundType * s);

#endif
