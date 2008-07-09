/* $Id: pres_gen.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <omega/pres_gen.h>
#include <basic/Exit.h>


namespace omega {

int     skip_finalization_check=0;
int     skip_set_checks=0;

int   pres_debug=0;
FILE *DebugFile=stderr;  // This is the default; it's best to set it yourself.

negation_control pres_legal_negations = any_negation;

//
// I/O utility functions.
//
void PresErrAssert(char *t) {
  fprintf(stdout, "\nERROR: %s\n", t);
  if(pres_debug) {
    fprintf(DebugFile, "\nERROR: %s\n", t);
  }
  Exit(1);
}



//
// Needed for gprof
//
#if defined PROFILE_MALLOCS
void* operator new(size_t n) 
{
    void *result = malloc (n < 1 ? 1 : n);
    if (result)
	return result;
    else {
	write(2,"Virtual memory exceeded in new\n",32);
	return 0;
    }
}

void operator delete (void* f) {
  if (f) free(f);
}
#endif


} // end of namespace omega
