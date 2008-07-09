/* $Id: kill.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_kill
#define Already_Included_kill

#include <basic/bool.h>
#include <petit/coverlist.h>

namespace omega {

bool kill_possible(dd_current dd, dd_current f, dd_current o,
                   int commonDepth, int allow_partial);

bool kill_possible(dd_current dd, dd_current f, dd_current o);

void do_partial_covers_kill(a_access r);
void verify_kills(a_access r);

}

#endif

