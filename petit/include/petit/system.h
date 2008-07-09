/* $Id: system.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_System
#define Already_Included_System

namespace omega {

extern void *build_do_auto_parallel();

extern int do_auto_parallel_epilog (int x);

extern void *build_do_dd_alg();

extern int do_dd_alg_epilog(int x);

extern void *build_do_system();

extern int do_system_epilog(int x);

}

#endif
