/* $Id: ddepsdriver.h,v 1.1.1.1 2000/06/29 19:24:23 dwonnaco Exp $ */
#ifndef Already_Included_DDEpsDriver
#define Already_Included_DDEpsDriver

void epsilon_build_dd_graph(void);
int get_difdep_number(void);
int dd_eps_test(a_access access1,  a_access access2,
			ddnature oitype, ddnature iotype,
			unsigned int nest1, unsigned int nest2, 
			unsigned int bnest);

#endif
