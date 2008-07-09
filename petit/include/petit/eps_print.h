/* $Id: eps_print.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_EpsPrint
#define Already_Included_EpsPrint

namespace omega {

void print_subscr(FILE *f, EpsSubscrType * s);
void print_eps_var(FILE *f, Epsvar_id v);
void print_ban_info(FILE *f, BanerjeeInfoType * bi);
void print_comvar(FILE *f, EpsComvarType *cv, int n);
void print_subscr_bounds(FILE * f, SubscrBoundType * s);

}

#endif
