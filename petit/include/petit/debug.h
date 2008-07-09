/* debug.h,v 1.1.1.2 1992/07/10 02:40:09 davew Exp */
#ifndef Already_Included_Debug
#define Already_Included_Debug

#include <stdio.h>

namespace omega {

extern FILE *debug, *debug2, *statsfile;
extern int n_strange_occurances;

extern void strange_occurance(char *message);

extern void set_debug_files();

extern void debug_dump(char *filename);

}

#endif
