#include <omega.h>

/*
    This function first checks for a substitution using "subs",
    and then tries a variety of things to see if its possible to
    create a substitution using mod (and maybe div), for some
    cases that are required by time skewing with tcodegen.
    Hopefull at some point this functionality will be generalized
    and integrated into the Substitutions class.
 */

namespace omega {

String get_sub(Relation &R, int o, Substitutions &subs);

}
