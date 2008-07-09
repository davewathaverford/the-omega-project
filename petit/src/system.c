/* $Id: system.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#if ! defined BATCH_ONLY_PETIT

#include <stdio.h>
#include <petit/dddriver.h>
#include <petit/Exit.h>
#include <petit/print.h>
#include <petit/message.h>
#include <petit/tree.h>
#include <petit/debug.h>
#include <petit/arrayExpand.h>
#include <petit/petit_args.h>
#include <petit/motif.h>
#include <petit/system.h>

namespace omega {

void *build_do_auto_parallel()
    {
    /* auto-parallelize menu. */
    static struct Menu_struc M4[] = {
        {"ExpandArr",0,                 ArrayExpansion,0,              1},
        {"Privatize",0,                 Privatization, 0,              1},
        {"Msgs",     build_Message_Menu,0,             do_display,     2},
        {"Quit",     0,                 Quit,          0,              0},
        {"Xcape",    0,                 0,             0,              0},
        {0,             0,                 0,             0,              0} };

    return Build_Menu( M4, do_display, 0 );
    } /* build_do_auto_parallel */



int do_auto_parallel_epilog (int x)
    {
    if (x <= 0) 
	return x;
    return 1;
    } /* do_auto_parallel_epilog */




void *build_do_dd_alg()
    {
    /* dd menu. */
    static struct Menu_struc M4[] = 
	{{"Omega", 0,              0,    0,                DDalg_omega},
         {"Eps",   0,              0,    0,                DDalg_epsilon},
         {"Banerjee", 0,           0,    0,                DDalg_banerjee},
         {"Omega eps", 0,          0,    0,                DDalg_omega_preproc},
         {"Msgs",  build_Message_Menu,0, do_display,       1},
         {"Quit",  0,              Quit, 0,                0},
         {"Xcape", 0,              0,    0,                0},
         {0,       0,              0,    0,                0}};

    return Build_Menu( M4, do_display, 5 );
    } /* build_do_dd_alg */




int do_dd_alg_epilog(int x)
    {
    if (x <= 0) 
	return x;
    petit_args.DDalgorithm = x;
    return 1;
    } /* do_dd_alg_epilog */




void *build_do_system()
    {
    /* system menu. */
    static struct Menu_struc M4[] = {
    {"Auto",  build_do_auto_parallel,0,             do_auto_parallel_epilog,1},
    {"DDalg", build_do_dd_alg,       0,             do_dd_alg_epilog,       1},
    {"File",  0,                     0,             0,                      6},
    {"Output",0,                     0,             0,                      2},
    {"Write", 0,                     0,             0,                      5},
    {"Msgs",  build_Message_Menu,    0,             do_display,             2},
    {"Quit",  0,                     Quit,          0,                      0},
    {"Xcape", 0,                     0,             0,                      0},
    {0,       0,                     0,             0,                      0}};

    return Build_Menu( M4, do_display, 0 );
    } /* build_do_system */





int do_system_epilog(int x)
    {
    if (x <= 0) 
	return x;
    switch (x)
	{
        case 1:
            return 0;

        case 2:
            if (debug != stderr && debug != stdout) 
		fclose( debug );
            debug = fopen( "trace.out", "a" );
            if (!debug) 
		{
                fprintf(stderr, "Can't open debug file \"trace.out\"\n");
                debug = stderr;
                }
#if !defined NDEBUG || defined newTimeTrials || printKillRelations
            setbuf(debug,0);
#endif
            setOutputFile(debug);
            break;

        case 5:
            debug_petit( Entry, 1 );
            break;

        case 6:
            motif_getfile( "Debug File: ", debug_dump, "*" );
            break;
        }
    return 0;
    }/* do_system_epilog */

} // end namespace omega

#endif
