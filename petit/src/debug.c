/* debug.c,v 1.1.1.2 1992/07/10 02:42:51 davew Exp */

#include <stdio.h>
#include <omega/pres_gen.h>
#include <petit/debug.h>

namespace omega {

FILE *debug, *debug2, *statsfile;
int n_strange_occurances = 0;


void strange_occurance(char *message)
{
    n_strange_occurances++;
    fprintf(debug, message);
}



void set_debug_files()
    {
#ifdef TC_STATS
    statsfile = fopen("tc.out","w");
    if (!statsfile)
        {
        fprintf(stderr,"Can't open ./tc.out\n");
        Exit(1);
        }
#endif

    /* set the default debug file to trace.out */
    /* change made by unborkar@cs.umd.edu 2/20/92 */
    debug = fopen( "trace.out" , "w" );
    if (!debug)
        {
        fprintf(stderr, "Can't open debug file \"trace.out\"\n");
        debug = stderr;
        }
     setbuf(debug,0);
     debug2 = fopen( "aux.out" , "w" );
     if (!debug2)
         {
         fprintf(stderr, "Can't open debug file \"aux.out\"\n");
         debug2 = stderr;
         }
#if !defined NDEBUG || defined newTimeTrials || printKillRelations
     setbuf(debug,0);
     setbuf(debug2,0);
#endif
     setOutputFile(debug);

     DebugFile = debug;
    } /* set_debug_files */



void debug_dump(char *filename)
    {
    if (filename[0] != 0)
        {
        if (debug != stderr && debug != stdout) 
            fclose(debug);
        if ((strcmp(filename,"stderr") == 0 ) ||
            (strcmp(filename,"STDERR") == 0 ) )
            debug = stderr;
        else if ((strcmp(filename,"stdout") == 0) ||
                 (strcmp(filename,"STDOUT") == 0) )
            debug = stdout;
        else 
            {
            debug = fopen( filename, "w" );
            if (!debug) 
                {
                fprintf(stderr,"Can't open debug file \"%s\"\n",filename);
                debug = stderr;
                }
#if !defined NDEBUG || defined newTimeTrials || printKillRelations
            setbuf(debug,0);
#endif
            }
        setOutputFile(debug);
        }
    } /* debug_dump */

} // end namespace omega
