/* $Id: definitions.h,v 1.1.1.1 2000/06/29 19:24:24 dwonnaco Exp $ */
#ifndef Already_Included_Definitions
#define Already_Included_Definitions

#include <basic/bool.h>


#define set(v)      (v) = true
#define reset(v)    (v) = false

#define Assert(c,t)     {if (!(c)) ErrAssert(t);}
#define UserAssert(c,t) {if (!(c)) Error(t);}

#endif
