/* $Id: add-assert.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#ifndef Already_Included_AddAssert
#define Already_Included_AddAssert 1

#include <basic/bool.h>
#include <petit/lang-interf.h>
#include <petit/dddir.h>

namespace omega {

dd_flags possible_to_eliminate(dd_current dd);
bool try_to_eliminate(dd_current dd);
Relation zap_conditions(dd_current dd);

}

#endif
