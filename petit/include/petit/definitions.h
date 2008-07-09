/* $Id: definitions.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Definitions
#define Already_Included_Definitions

#include <basic/bool.h>

namespace omega {

#define set(v)      (v) = true
#define reset(v)    (v) = false

#define Assert(c,t)     {if (!(c)) ErrAssert(t);}
#define UserAssert(c,t) {if (!(c)) Error(t);}
}

#endif
