/* $Id: Exit.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/**********************************************************
 *                                                        *
 * Error fighting and exit facilities, include file       *
 *                                                        *
 **********************************************************/

#ifndef Already_Included_Exit
#define Already_Included_Exit

#include <basic/Exit.h>
#include <signal.h>

namespace omega {

/* SunOS has ... after the int argument; who knows about others. */
#if !defined(SIG_HANDLER_HAS_ONE_ARG)
#  if (defined(SYSV) || defined(__svr4__) || defined(bsdi))
#    define SIG_HANDLER_HAS_ONE_ARG 1
#  else
#    define SIG_HANDLER_HAS_ONE_ARG 0
#  endif
#endif
/* System V uses SIG_ERR, some others use -1 */
#if !defined(SIG_RETURNS_SIG_ERR)
#  if (defined(SYSV) || defined(__svr4__))
#    define SIG_RETURNS_SIG_ERR 1
#  else
#    define SIG_RETURNS_SIG_ERR 0
#  endif
#endif


#if SIG_RETURNS_SIG_ERR == 1
#    define SIGNAL_ERROR SIG_ERR
#else
#    define SIGNAL_ERROR ((void(*)(...)) -1)
#endif
#if SIG_HANDLER_HAS_ONE_ARG == 1
     void      sigAbort(int);
#else
     void      sigAbort(int, ...);
#endif


void      set_signals(void);
extern void      ErrAssert(char *);
extern void      Error(char *);
void      Warning(char *);

int      Quit(int dummy );



extern void out_of_memory();

void      Write(char *);

}

#endif
