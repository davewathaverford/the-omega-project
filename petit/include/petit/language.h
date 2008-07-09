/* language.h,v 1.1.1.2 1992/07/10 02:40:29 davew Exp */
#ifndef Already_Included_Language
#define Already_Included_Language

namespace omega {

typedef enum{ petitlang, fortran_alliant, clang } languagetype;

extern languagetype language;

}

#endif
