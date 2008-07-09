/* $Id: uniform_args.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include "uniform/uniform_misc.h"
#include "uniform/uniform_args.h"
namespace omega {

uniform_args_struct uniform_args;


void process_uniform_args(char *arg)
    {
    char c;
    int j = 0;
    uniform_args.gen_do_closure = 0;
    uniform_args.non_data_ok = 1; 
    uniform_args.manual = 0;
    uniform_args.simple = 0;
    uniform_args.trace_uniform = 0; 

    uniform_args.Num_Procs = 10;
    uniform_args.n = 100;

    uniform_args.comps_per_comm = 5.0;

    while((c = arg[j]) != 0) 
        {
        j++;
        switch(c) 
            {
            case 'c':
                {
                uniform_args.gen_do_closure = 1; 
                break;
                }
            case 'd':
                {
                uniform_args.non_data_ok = 0; 
                break;
                }
            case 'm':
                {
                uniform_args.manual = 1; 
                break;
                }
            case 's':
                {
                uniform_args.simple = 1; 
                break;
                }
            case 't':
                {
                uniform_args.trace_uniform = 1; 
                break;
                }
            case 'p':
                {
                uniform_args.Num_Procs = 0;
                assert((c=arg[j]) >= '0' && c <= '9');
                while((c=arg[j]) >= '0' && c <= '9') 
                    {
                    uniform_args.Num_Procs = 
                        uniform_args.Num_Procs*10 + c-'0';
                    j++;
                    }
                break;
                }
            case 'n':
                {
                uniform_args.n = 0;
                assert((c=arg[j]) >= '0' && c <= '9');
                while((c=arg[j]) >= '0' && c <= '9') 
                    {
                    uniform_args.n = 
                        uniform_args.n*10 + c-'0';
                    j++;
                    }
                break;
                }
            case 'r':
                {
                uniform_args.comps_per_comm = 0;
                assert((c=arg[j]) >= '0' && c <= '9') ;
                while((c=arg[j]) >= '0' && c <= '9') 
                    {
                    uniform_args.comps_per_comm = 
                        uniform_args.comps_per_comm*10 + c-'0';
                    j++;
                    }
                if (arg[j] == '.')
                    {
                    j++;
                    int fp = 0, bp = 1;
                    while((c=arg[j])>='0' && c<='9') 
                        {
                        fp = fp*10 + c-'0';
                        j++;
                        bp = bp * 10;
                        }
                    uniform_args.comps_per_comm += 
                        (double)fp / (double)bp;
                    }
                break;
                }
            default:
                {
                UniformError("badly formed option to 'W' flag\n"
                      "px = set nprocs to x\n"
                      "c = do transitive closure\n"
                      "ix = log_10 (iters. per proc)\n"
                      "nx = iters. per loop\n"
                      "rx = comps. per comm.\n"
                      "x = work in batch mode\n");
                break;
                }
            }
        }
    } /* process_uniform_args */
}
