/* $Id: petit.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#include <stdio.h>
#include <stdlib.h>
#if !defined(EXCLUDE_UNIFORM)
#include <uniform/uniform.h>
#endif
#include <petit/Exit.h>
#include <petit/petit_args.h>
#include <petit/debug.h>
#include <petit/f2p.h>
#include <petit/depend_filter.h>
#include <petit/parse_and_analyse.h>
#include <petit/build_stmt_rel.h>
#include <petit/motif.h>
#include <petit/message.h>
#include <petit/browse.h>
#include <petit/system.h>
#include <petit/langmenu.h>
#include <petit/timer.h>
#include <petit/print.h>
#include <petit/hpp.h>

#include <petit/affine.h>
#include <petit/lang-interf.h>

using namespace omega;

static void grab_memory()
    {
    char *initial_memory_grab = getenv("PETIT_INITIAL_MEMORY_GRAB");
    long unsigned int grab_amount;

    if (initial_memory_grab)
        grab_amount = atol(initial_memory_grab);
    else
        grab_amount = 5242800; // default to ~ 5M

    void *foo = malloc(grab_amount);

    if (foo) free((char *)foo);
    } /* grab_memory */



int main(int argc, char *argv[]) 
    {
    set_signals();  // set reactions to signals

    grab_memory(); // Preallocate some memory so latter allocations are faster
  
    all_debugging_off(); // all presburger debugging off

    process_args(argc, argv);

    convert_to_petit(); // convert from Fortran if .t extension

    set_debug_files();

    depend_filter_init(); 

#if ! defined BATCH_ONLY_PETIT

    if (petit_args.motif_display) 
	{
        motif_init(argc, argv);
        Message_Startup();    
	}

#endif

    int found = parse_and_analyse(petit_args.FileName);

    if (petit_args.DepWrFile[0]) 
        write_depends(found, petit_args.FileName);
    else if (petit_args.doOEcompare) 
        compare_tests(found);
    else if (petit_args.testClosure) 
        test_closure(found);
    else if (petit_args.uniform)
        {
        if (!found)
            Error("Input file not found, exiting\n");

#if !defined(EXCLUDE_UNIFORM)
        uniform(petit_args.uniform_args);
#else
        Error("Uniform is not included in this executable\n");
#endif
        }
    else if (petit_args.hpp_only)
	hpp_generate(0);
    else
        {
#if ! defined BATCH_ONLY_PETIT

        struct Menu_struc M_initial_nonhpf[] = 
            {{"Browse",build_br_browse,   br_browse,   br_browse_epilog,   1},
             {"Reopen",0,                 do_reopen,   0,                  1},
             {"Parse" ,0,                 do_newfile,  0,                  1},
             {"CalcDD",0,                 re_analyse,  0,                  1},
             {"System",build_do_system,   0,           do_system_epilog,   1},
             {"Trans", build_do_translate,0,           do_translate_epilog,1},
             {"Graph", 0,                 do_graph,    0,                  1},
             {"Write", 0,                 do_writepgm, 0,                  1},
             {"Msgs",  build_Message_Menu,0,           do_display,         2},
             {"Quit",  0,                 Quit,        0,                  0},
             {0,       0,                 0,           0,                  0}};

        struct Menu_struc M_initial_hpf[] = 
            {{"Browse",build_br_browse,   br_browse,   br_browse_epilog,   1},
             {"Reopen",0,                 do_reopen,   0,                  1},
             {"Parse" ,0,                 do_newfile,  0,                  1},
             {"CalcDD",0,                 re_analyse,  0,                  1},
             {"System",build_do_system,   0,           do_system_epilog,   1},
             {"Trans", build_do_translate,0,           do_translate_epilog,1},
             {"Graph", 0,                 do_graph,    0,                  1},
             {"HPF",   build_hpp,         0,           do_display,         1},
             {"Write", 0,                 do_writepgm, 0,                  1},
             {"Msgs",  build_Message_Menu,0,           do_display,         2},
             {"Quit",  0,                 Quit,        0,                  0},
             {0,       0,                 0,           0,                  0}};


        if(petit_args.hpp_mode)
	    motif_loop(Build_Menu(M_initial_hpf, do_display, 0));
        else
	    motif_loop(Build_Menu(M_initial_nonhpf, do_display, 0));
#else

        brdd_write(0);  

#endif
        }

    Exit(0);
    return 0; // shut the compiler up
    } /* main */

