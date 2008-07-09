/* assert.h that calls abort as Exit(-1)
   for applications that do not care about Exit, regular assert may be used */

//#include <basic/Exit.h>
#include <stdlib.h>

#ifndef Already_Included_Assert
#define Already_Included_Assert 1

#undef assert
#undef _assert

# if ! defined(NDEBUG) || defined(ASSERTIONS_ANYWAY)
# include <stdio.h>
#ifdef WIN32
# define _assert(ex)	((!(ex)) ? ((void)fprintf(stderr,"\n\nAssertion \"%s\" failed: file \"%s\", line %d\n", ex, __FILE__, __LINE__), Exit(-2), 0) : 1)
#else
//# define _assert(ex)	((!(ex)) ? ((void)fprintf(stderr,"\n\nAssertion \"%s\" failed: file \"%s\", line %d\n", #ex, __FILE__, __LINE__), Exit(-2), 0) : 1)
# define _assert(ex)	(void)((!(ex)) ? ((void)fprintf(stderr,"\n\nAssertion \"%s\" failed: file \"%s\", line %d\n", #ex, __FILE__, __LINE__), exit(-2), 0) : 1)
#endif
# define assert(ex)	_assert(ex)
# else
# define _assert(ex)
# define assert(ex)
# endif

#endif /* Already_Included_Assert */
