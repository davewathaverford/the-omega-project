#if !defined _grab_lib_hack_h
#define _grab_lib_hack_h

#include <basic/bool.h>


namespace omega {

bool lib_hack(Conjunct *c, DNF *d, Rel_Body *rb);
extern bool WANT_SEGM_FAULT;

class grab_lib_hack {
 public:
   grab_lib_hack() { if (WANT_SEGM_FAULT)
                        lib_hack(NULL, NULL,NULL); 
                   }
};

static grab_lib_hack hack_hack;

} // end of namespace omega

#endif

