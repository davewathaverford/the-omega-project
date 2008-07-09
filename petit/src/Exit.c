/* $Id: Exit.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

/**********************************************************
 *                                                        *
 * Error fighting and exit facilities                     *
 *                                                        *
 **********************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <petit/Exit.h>
#include <petit/debug.h>
#include <petit/omega2flags.h>
#include <petit/motif.h>
#include <petit/petit_args.h>

namespace omega {
#ifdef WIN32
#define Write(x) ;
#endif

static void RemoveIfEmpty(char *, FILE *);

static int mysignals = 0;

/*
 * if something bad happens
 */
void set_signals(void) {
#ifndef WIN32
  /* Set signal reactions */
/* Note that under Solaris 2.4 and other SYSV unices, upon receiving a signal,
   the handler is set back to SIG_DFL before the signal handler is run.  
   This shouldn't make any difference.  */
  if(!mysignals) {
    if(signal(SIGQUIT,&sigAbort) == (void(*)(int))SIGNAL_ERROR ||
       signal(SIGINT, &sigAbort) == (void(*)(int))SIGNAL_ERROR ||
       signal(SIGILL, &sigAbort) == (void(*)(int))SIGNAL_ERROR ||
       signal(SIGTRAP,&sigAbort) == (void(*)(int))SIGNAL_ERROR ||
       signal(SIGFPE, &sigAbort) == (void(*)(int))SIGNAL_ERROR ||
       signal(SIGBUS, &sigAbort) == (void(*)(int))SIGNAL_ERROR ||
       signal(SIGSEGV,&sigAbort) == (void(*)(int))SIGNAL_ERROR ||
       //       signal(SIGSYS, &sigAbort) == SIGNAL_ERROR ||
       false ) {  // this just makes it possible to comment out the above
      fprintf(stderr, "\nset_signals: can not set reaction to signals\n");
      Exit(-1);
    }
    mysignals = 1;
  }
#endif
}

/*
 * Assertion failed
 */
void ErrAssert(char *t) {
  fprintf(stderr,"\n%s: %s\n", &petit_args.argv[0][0], t);
  Exit(-2);
}

/*
 * User error
 */
void Error(char *t) {
  fprintf(stderr,"\n%s: %s\n", &petit_args.argv[0][0], t);
  Exit(1);
}

/*
 * Warning
 */
void Warning(char *t) {
  fprintf(stderr, "\n%s: %s -- just warning\n", &petit_args.argv[0][0], t);
}


/*
 * Quit menu command
 */
int Quit(int x) {

  /* Close and remove debug files (if they are empty) */
  RemoveIfEmpty("trace.out", debug);
  RemoveIfEmpty("aux.out", debug2);

  if(x<0) {
    fprintf(stderr, "Empty or bad menu (xcode = %d)\n", x);
    x=2;
  } else {
    x=0;
  }

  Exit(x);

  return(0);        /* eager to please compiler */
}


/*
 * Do closing procedures
 */

void Exit(int e)
{
#ifndef WIN32
  if(mysignals && signal(SIGTRAP, SIG_DFL) == (void(*)(int))SIGNAL_ERROR) {

      fprintf(stderr, "\nset_signals: can not set reaction to signals\n");
      exit(-4);
  }

#if ! defined BATCH_ONLY_PETIT
  motif_fini();
#endif

  switch(e) {
  case 0:
  case -1000:
    break;
  case 1:
    fprintf(stderr, "Exit apparently due to user error (exit code 1)\n");
    break;
  case 2:
  case -2:
    fprintf(stderr, "Exit apparently due to system limitation or error (exit code %d)\n", e);
    break;
  default:
    fprintf(stderr, "Something really bad happened (exit code %d)\n", e);
    break;
  }

  if(e<0)
  {
      char crash[1024];
      FILE *fp;

      crash[0]=0;
      char *home = getenv("PETITHOME");
      if (home) strcpy(crash, home);
      if (crash[0]) strcat(crash, "/misc/crash");
      if (crash[0]) fp = fopen(crash, "r");
      if(crash[0] && fp != NULL) {
	fclose(fp);
	system(crash);
      }
      char *env_var_for_core = "PETIT_DUMP_CORE";
      if (getenv(env_var_for_core))
	  {
	  abort();
	  }
      else
	  fprintf(stderr, "Not dumping core - set %s to generate core dump\n", env_var_for_core);
  }
#endif
  exit(e);
}

/*
 * Abort by bad signal
 */
#if SIG_HANDLER_HAS_ONE_ARG == 1
void sigAbort(int c)
#else
void sigAbort(int c, ...)
#endif

 {
#ifndef WIN32
  static struct sig1 {
    int   sig;
    int   core;
    char *text;
  } sigs[] = {
    {SIGINT,  0, "Interrupt"}, 
    {SIGQUIT, 1, "Quit"}, 
    {SIGILL,  1, "Illegal instruction"}, 
    {SIGTRAP, 1, "Trace trap"}, 
    {SIGABRT, 1, "Abort"}, 
    {SIGFPE,  1, "Floating arithmetic exception"}, 
    {SIGBUS,  1, "Bus error"}, 
    {SIGSEGV, 1, "Segmentation violation"}, 
    //    {SIGSYS,  1, "Bad argument to system call"}, 
  };
  static char s[80];
  int i;
  sprintf(s,"\n%s: signal %d: ", &petit_args.argv[0][0], c);
  Write(s);
  for(i=0; i< (int)(sizeof(sigs)/sizeof(struct sig1)); i++) {
    if(sigs[i].sig==c) {
      sprintf(s, "%s", sigs[i].text);
      Write(s);
      break;
    }
  }
  Write("\n");
  if (i >= (int)(sizeof(sigs)/sizeof(struct sig1)) || sigs[i].core) {
    Exit(-2);
  } else {
    Exit(0);
  }
#else
  Exit(c);
#endif
}

/*
 * We don't want to print anything if we don't have memory,
 * because buffer allocation problem may lead to cycling 
 */

void out_of_memory(void)
{
    fprintf(debug,"Memory Allocation failed\n");
    Write("Memory allocation failed -- not enough memory\n");
    Exit(-1000);
}

} // end of omega namespace

#ifndef WIN32
#include <sys/file.h>

namespace omega {

void Write(char *s) {
  static int fd = -1;
  int l;
  if(fd<0) {
    fd = open("/dev/tty",O_WRONLY,0);
    if(fd<0) exit(187);
  }
  l=strlen(s);
  if(write(fd,s,l)!=l) exit(188);
}

}
#endif

/*
 * Remove file with given name if it's empty
 */
namespace omega {
static
void RemoveIfEmpty(char *fname, FILE *file) {
  fclose(file);
  file = fopen(fname, "r");
  if(file==NULL) {
#if 0
    fprintf(stderr, "\nWarning: debug file %s was lost\n", fname);
#endif
    return;
  }
  if(fgetc(file) == EOF) {
    fclose(file);
    if(unlink(fname)!=0) {
#if 0
      fprintf(stderr, "Warning: can not remove debug file %s\n", fname);
#endif
    }
  } else {
    fclose(file);
  }
}

}
