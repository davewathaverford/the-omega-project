/* $Id: uniform.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

/*----------------------------------------------------------------------------  
    This is the entry procedure for the Uniform methods transformation 
system. The parameter passed to this procedure should be the command line
arguments that pertain to the transformation system. The transformed code is
output to the file "spmd.c".
------------------------------------------------------------------------------*/

#include <stdio.h>
#include <uniform/uniform_args.h>
#include <uniform/uniform.h>
#include <uniform/uniform_misc.h>
#include <uniform/depend_rel.h>
#include <uniform/space_mappings.h>
#include <uniform/select.h>
#include <uniform/time_mappings.h>
#include <uniform/simple_codegen.h>
namespace omega {

void uniform(char * uniform_args)
    {
    int i;
    process_uniform_args(uniform_args);

    uniform_debug = fopen( "uniform.trace" , "w" );
    if (!uniform_debug)
        {
        fprintf(stderr, "Can't open debug file \"uniform.trace\"\n");
        uniform_debug = stderr;
        }
     setbuf(uniform_debug,0);

     uniform_debug2 = fopen( "spmd.c" , "w" );
     if (!uniform_debug2)
         {
         fprintf(stderr, "Can't open output file \"spmd.c\"\n");
         uniform_debug2 = stderr;
         }

    nr_statements = 0;
    stmt_info = Tuple<sinfo>(0);
    all_depends = depend_graph(0);
    val_flow_depends = depend_graph(0);

    traverse();

    if (nr_statements == 0)
	UniformError("program has no executable statements");

    if (nr_statements >= max_stmts)
	{
	char s[80];
	sprintf(s, "nr_statements %d exceed maximum of %d", 
		nr_statements, max_stmts);
	UniformError(s);
	}

    global_max_nest = 0;
    for (i=0; i<nr_statements; i++)
        global_max_nest = max(global_max_nest, stmt_info[i+1].nest);

    was_negative.resize(nr_statements, global_max_nest+1);
    for (i=0; i<nr_statements; i++)
        determine_steps(i);

    find_names();

    build_depend_relation_graph();

    T1.resize(maxLevels, nr_statements*2);

    fprintf(uniform_debug2, "/*************************************************"
	    "****************************\n");

    fprintf(uniform_debug2, "commmand line arguments: %s\n", uniform_args);

    minimize_communication();

    select_time_mappings();

    fprintf(uniform_debug2, "**************************************************"
	    "**************************/\n\n");

    simple_codegen();
    } /* uniform */
}
