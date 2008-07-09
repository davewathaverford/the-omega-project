/* $Id: petit_args.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <basic/util.h>
#include <basic/bool.h>
#include <code_gen/spmd.h>
#include <omega/omega_core/debugging.h>
#include <omega/omega_core/oc.h>
#include <petit/dddriver.h>
#include <petit/ddodriver.h>
#include <petit/petit_args.h>
#include <petit/Exit.h>
#include <petit/build_stmt_rel.h>
#include <petit/definitions.h>
#include <petit/motif.h>
#include <petit/browse.h>

namespace omega {

#define MAXLINELEN 80


petit_args_struct petit_args;

struct option
    {
    int  *pflag;
    bool  def_value;
    char  symbol;
    char *explain;
    } options[] =
{{&petit_args.skipping_omega4   , false, '4', 
  "Skip Omega-4 (value-based analysis)"},
 {&petit_args.skipping_omega5   , false, '5', 
  "Skip Omega-5 (uninterpreted functions)"},
 {&petit_args.li2_only          , false, 'X', 
  "Treat not obviously polynomial cases as unknown (requires -5):"
  " control & subscript must be LI(2)-unit; limit negations."},
 {&petit_args.ivr_Assert        , false, 'A', 
  "Induction var. recognition assertions"},
 {&petit_args.dep_analysis_debug, false, 'D', 
  "Dependence analysis debug level: - D[0..4]"},
 {&petit_args.ivr_DefEntryClass , true , 'U', 
  "Make undefined var-s to have IN class"},
 {&petit_args.printPhis         , false, 'H', 
  "Print phi-functions and variable instances"},
 {&petit_args.ivr_debug         , false, 'I', 
  "SSA and IVR debug information"},
 {&petit_args.printAffine       , false, 'L', 
  "Print affine expressions in [ ]"},
 {&petit_args.max_conjuncts    , 0 , 'm', 
  "Maximum conjuncts to consider"},
 {&petit_args.ivr_RemoveNoOp    , true , 'N', 
  "Remove IVR-generated no-op assign stmts"},
 {&petit_args.dep_analysis_quick, false, 'f', 
  "Fast & approximate analyis"},
 {&petit_args.dep_analysis_debug, false, 'P', 
  "Presburger debug: -P[acopr][0..4]"},
 {&petit_args.printShadow       , false, 'S', 
  "Print shadow expressions in { }"},
 {&petit_args.testClosure       , false, 'T', 
  "Test transative closure"},
 {&petit_args.printIxArrays     , false, 'Y', 
  "Notify about index array occurences"},
 {&petit_args.print_relations   , false, 'b', 
  "Print Dependence Relations"},
 {&petit_args.arrDefInOut       , true , 'd', 
  "Make arrays INOUT by default"},
 {&petit_args.doEEdeps          , true , 'g', 
  "Do Entry/Exit node dependences"},
 {&petit_args.ivr_ShadowShadow  , true , 'h', 
  "Eliminate shadows of shadow expressions"},
 {&petit_args.ivr_on            , true , 'i', 
  "Induction variable recognition"},
 {&petit_args.onlyPrintRealArrayDeps   , false, 'j',
  "Only print dependences due to arrays of type Real"},
 {&petit_args.ivr_RepAffine     , false, 'l', 
  "Replace affine expr. with norm. form"},
 {&petit_args.printNoDeadDeps   , false, 'o', 
  "Don't print dead depends and their flags"},
 {&petit_args.doPrivatization   , false, 'p', 
  "Privatization"},
 {&petit_args.quiet             , true , 'q', 
  "Be quiet"},
 {&petit_args.makeReductionOps  , false, 'r', 
  "Recognize update operations"},
 {&petit_args.omitScalars      , false, 's', 
  "Skip analysis for scalars"},
 {&petit_args.omitTopLevel      , false, 't', 
  "Skip analysis for top level dependences"},
 {&petit_args.panicked_by_nonaffinity   , false, 'u',
  "Panic when there is a non-affine term during value-based or zap analysis"},
 {&petit_args.just_zap_value_flow , false,  'v', 
  "Do restrict 'zapping' analysis to value-based flow deps"},
 {&petit_args.doArrayExpn       , false, 'x', 
  "Array expansion (suboptions d, l, u)"},
 {&petit_args.repeatArrayExpn   , true , 'y', 
  "Repeat array expansion (if it's on)"},
 {&petit_args.ivr_SubstReals    , false, 'z', 
  "Substitute real scalars too"},
 {&petit_args.print_SCC         , false, 'C', 
  "Print SCC components"},
 {&petit_args.do_calculate_depnum,   false, 'K',  
  "Calculate number of dependencies"},
 {NULL,                0,     0,   NULL} };

const char *const other_options_explain =
"F controls dependence printing filter with suboptions: v, m, c, f, o, a\n"
"M set all presburger debug levels\n"
"O defines omega_core_debug\n"
"Q omega/epsilon test comparison? (enables petit_args.doOEcompare)\n"
"J enables petit_args.hpp_mode and petit_args.hpp_only\n"
"R write dependences to file -Rname for normal, or -R-name or -R--name\n"
"W enables -5 -r -w0, 'uniform', 'normalize', and copies subargs to uniform_args \n"
"a controls petit_args.DDalgorithm\n"
"w defines petit_args.zappable_interesting_distance\n"
"? prints command line arguments and exits\n"
;

static char *describe_options();

void process_args(int argc, char *argv[]) 
    {
    char line[MAXLINELEN];

    petit_args.argv = argv;
    petit_args.argc = argc;
    petit_args.FileName = NULL;
    petit_args.doOEcompare=0; 
    petit_args.depPrintFilter=0; 
    petit_args.DDalgorithm = DDalg_omega_preproc;
    petit_args.DepWrFile[0] = 0;
    petit_args.omitScalars = 0;
    petit_args.omitTopLevel = 0;
    petit_args.graph_by_statement=true;
    petit_args.graph_write_dd_as_graph=false;
    petit_args.hpp_mode=false;
    petit_args.hpp_only=false;
    petit_args.motif_display=true;
    petit_args.zappable_interesting_distance=4;
    petit_args.normalize = 0;

    struct option *popt;
#if ! defined NDEBUG
    char popt_already_found[256];
    memset(popt_already_found, 0, 256);
#endif

    for(popt=&options[0]; popt->pflag; popt++)
	{
#if ! defined NDEBUG
	assert(popt_already_found[popt->symbol]++ == 0);
#endif
        *(popt->pflag) = popt->def_value;
	}

    for(int i=0; i<argc; i++) 
        {
        int c, j;
        char *arg;

        if(i==0) /* read ~/.petitrc file for flags */
            {
            FILE *fin;
            sprintf(line, "%s/.petitrc", getenv("HOME"));
            fin = fopen(line, "r");
            if(fin) 
                {
                for(j=0; j<MAXLINELEN; j++) 
                    {
                    int c = fgetc(fin);
                    if(c==EOF || c=='\n') break;
                    line[j] = c;
                    }
                line[j] = 0;
                fclose(fin);
                arg = line;
                } 
            else 
                continue;
            } 
        else 
            arg = argv[i];

        if(arg[0] == '-') 
            {
            j = 1;
            while((c=arg[j++]) != 0) 
                {
                switch(c) 
                    {
                    case ' ':
			{
                        while(arg[j] == ' ') 
                            j++;
                        if(arg[j] == '-') 
                            {
                            j++;
                            continue;
                            }
                        if(arg[j] == 0)
                            break;
                        Error("file name in .petitrc file");
                        break;
			}

		    case '?':
		      {
			printf("%s", describe_options());
			Exit(0);
		      }
		    case 'w':
			{
			petit_args.zappable_interesting_distance =
			    atoi(&arg[j]);
			while(arg[j] != 0) j++;
			break;
			}

                    case 'D':
			{
                        if((c=arg[j])>='0' && c<='9') 
                            {
                            int level = c-'0';
                            petit_args.dep_analysis_debug = level;
                            j++;
                            } 
                        else 
                            petit_args.dep_analysis_debug = 2;
                        break;
			}

                    case 'F':
			{
                        while ((c=arg[j]) != ' ' && c)
                            {
                            switch(c) 
                                {
                                case 'v': 
                                    petit_args.depPrintFilter |= ddval; 
                                    break;
                                case 'm': 
                                    petit_args.depPrintFilter |= ddmem; 
                                    break;
                                case 'c': 
                                    petit_args.depPrintFilter |= ddloopCarried; 
                                    break;
                                case 'f': 
                                    petit_args.depPrintFilter |= ddIsFlow; 
                                    break;
                                case 'o': 
                                    petit_args.depPrintFilter |= ddIsOutput; 
                                    break;
                                case 'a': 
                                    petit_args.depPrintFilter |= ddIsAnti; 
                                    break;
                                default:
                                    break;
                                }
                            j++;
                            }
                        break;
			}

                    case 'P':
			{
                        if (! process_pres_debugging_flags(arg,j)) 
                            {
                            fprintf(stderr,
                                    "use Omega Library debugging like: "
                                    "{-P[facility][level]...}\n"
                                     "    a = all debugging flags\n"
                                     "    c = omega code\n"
                                     "    g = code generation\n"
                                     "    l = calculator\n"
                                     "    p = presburger functions\n"
                                     "    r = relational operators\n"
                                     " All debugging output goes to trace.out");
                            Error("badly formed Omega Library debug options");
                            }
                        break;
			}

                    case 'O':
			{
                        if((c=arg[j])>='0' && c<='3') 
                            {
                            omega_core_debug = c-'0';
                            j++;
                            } 
                        else 
                            omega_core_debug = 1;
                        break;
			}

                    case 'W':
			{
                        petit_args.uniform = 1;
                        petit_args.normalize = 1;
                        petit_args.zappable_interesting_distance = 0;
                        petit_args.skipping_omega5 = 1;
                        petit_args.makeReductionOps = 1;
			int jj = 0;
                        while ( jj < MAX_UNIFORM_ARG_LENGTH && arg[j] != '\0')
			    petit_args.uniform_args[jj++] = arg[j++];
                        petit_args.uniform_args[jj] = '\0';
                        break;
			}

		    case 'X':
			{
			    petit_args.li2_only  = true;
			    pres_legal_negations = one_geq_or_eq;
			}
		    case 'J':
			{
			    petit_args.hpp_mode = true;
			    petit_args.hpp_only = true;
			}
			break;

		    case 'M':
			{
                        // set all presburger flags but not omega flags
                        if((c=arg[j])>='0' && c<='3') 
			    {
                            int level = c-'0';
                            pres_debug=
				relation_debug=
				    code_gen_debug = level;
                            j++;
                            } 
			else 
                            pres_debug=
				relation_debug=
				    code_gen_debug = 1;
                        break;
			}

                    case 'a': 
			{
                        petit_args.DDalgorithm = 0;
                        while((c=arg[j]) >= '0' && c <= '9') 
			    {
                            petit_args.DDalgorithm = petit_args.DDalgorithm*10 + c-'0';
                            j++;
                            }
                        if( petit_args.DDalgorithm <= 0 ) 
			    petit_args.DDalgorithm = DDalg_omega_preproc;
                        break;
			}

                    case 'm': 
			{
                        petit_args.max_conjuncts = 0;
                        while((c=arg[j]) >= '0' && c <= '9') 
			    {
                            petit_args.max_conjuncts = petit_args.max_conjuncts*10 + c-'0';
                            j++;
                            }
                        break;
			}
                    case 'x':
			{
			petit_args.doArrayExpn = 1;
                        petit_args.expandlimit = MAXINT;
                        petit_args.lowerdefault = 0;
                        petit_args.upperdefault = 127;

                        while((c = arg[j]) != 0) 
			    {
                            j++;
                            switch(c) 
				{
                                case 'd': 
			            {
                                    petit_args.expandlimit = 0;
                                    while((c=arg[j]) >= '0' && c <= '9') 
			                {
                                        petit_args.expandlimit = 
				            petit_args.expandlimit*10 + c-'0';
                                        j++;
                                        }
                                    break;
			            }

                                case 'l': 
			            {
                                    petit_args.lowerdefault = 0;
                                    while((c=arg[j]) >= '0' && c <= '9') 
			                {
                                        petit_args.lowerdefault = 
				            petit_args.lowerdefault*10 + c-'0';
                                        j++;
			                }
                                    break;
			            }
                                case 'u': 
			            {
                                    petit_args.upperdefault = 0;
                                    while((c=arg[j]) >= '0' && c <= '9') 
			                {
                                        petit_args.upperdefault = 
				            petit_args.upperdefault*10 + c-'0';
                                        j++;
			                }
                                    break;
			            }
                                default:
				    Error("bad argument for array expansion\n");
			        }
			    }
                        break;
			}

		    case 'R':
			{
			// -R-name like old -Sname; -R--name like old -Vname
			if (arg[j] == '-')
			  {
			    j++;
			    petit_args.graph_write_dd_as_graph=true;
			    petit_args.graph_by_statement=true;
			    if (arg[j] == '-')
			      {
				j++;
				petit_args.graph_by_statement=false;
			      }
			  }
			int len=strlen(&arg[j]);
			if (len>=DEPWRNAMELEN)
			    ErrAssert("Write file name too long");
			strcpy(petit_args.DepWrFile, &arg[j]);
			j += len;
			break;
			}

                    case 'T':
			{
			int n=0;
                        petit_args.valueClosure=0;
                        petit_args.testClosure=1;
                        if (arg[j]=='h')
                            {
                            print_closure_help();
                            Exit(0);
                            }
                        while ((c=arg[j]) >= '0' && c<='9') 
			    {
                            n=n*10+c-'0';
                            j++;
                            }
			if (arg[j]=='v') 
			    {
                            petit_args.valueClosure=1;  
                            j++;
                            }
                        set_closure_flags(n);
                        break; 
			}

                    case 'Q':
			{
                        petit_args.DDalgorithm=DDalg_omega;
                        petit_args.doOEcompare=1;
                        break;
			}

                    default:
			{
                        for(popt=&options[0]; popt->pflag; popt++) 
			    {
                            if(popt->symbol == c) 
				{
                                *(popt->pflag) = 1 - *(popt->pflag);
                                goto FlagFound;
                                }
                            }
                        sprintf(line, "Unknown flag -%c", c);
                        Error(line);
			}
FlagFound:;
                    }
                }
            } 
        else 
	    {
            UserAssert(i>=1, "File name is not allowed in .petitrc file");
            UserAssert(petit_args.FileName==NULL, "Only one source file name is allowed");
            petit_args.FileName = arg;
            }
        }
    petit_args.motif_display = 
      (!petit_args.uniform && !petit_args.hpp_only && !petit_args.DepWrFile[0]);
    } /* process_args */

static void describe_compile_options(char *buf)
{
    strcat(buf,"NDEBUG:");
#ifdef NDEBUG
    strcat(buf, " defined\n");
    // FERD -- I'd like to do this: strcat(buf, " defined (" #NDEBUG ")\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "STILL_CHECK_MULT (detects overflow even with -DNDEBUG):");
#ifdef STILL_CHECK_MULT
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "SPEED:");
#ifdef SPEED
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "APROX:");
#ifdef APROX
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "newTimeTrials:");
#ifdef newTimeTrials
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "OMIT_GETRUSAGE:");
#ifdef OMIT_GETRUSAGE
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "eliminateRedundantConstraints:");
#ifdef eliminateRedundantConstraints
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "singleResult:");
#ifdef singleResult
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "clearForwardingPointers:");
#ifdef clearForwardingPointers
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "SKIP_OMEGA2:");
#ifdef SKIP_OMEGA2
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "SKIP_OMEGA4:");
#ifdef SKIP_OMEGA4
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "SKIP_OMEGA5:");
#ifdef SKIP_OMEGA5
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "SKIP_ZAPPABLE:");
#ifdef SKIP_ZAPPABLE
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "DAVEW_THESIS_REDUCTIONS:");
#ifdef DAVEW_THESIS_REDUCTIONS
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "skipping_all_tightening:");
#ifdef skipping_all_tightening
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "skipping_bailout:");
#ifdef skipping_bailout
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "cover_test_only_with_zeros:");
#ifdef cover_test_only_with_zeros
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "cover_test_the_zeros:");
#ifdef cover_test_the_zeros
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "naive_kills:");
#ifdef naive_kills
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "only_worry_about_cycles:");
#ifdef only_worry_about_cycles
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "dont_use_cover_analysis:");
#ifdef dont_use_cover_analysis
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "kill_with_some_obvious_constraints:");
#ifdef kill_with_some_obvious_constraints
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "ONLY_CHANGE_FLOW_LEVEL:");
#ifdef ONLY_CHANGE_FLOW_LEVEL
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "EXTRAVAGANT:");
#ifdef EXTRAVAGANT
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "DO_MONICA_TESTS:");
#ifdef DO_MONICA_TESTS
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "compareOmegaEpsilon:");
#ifdef compareOmegaEpsilon
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif

    strcat(buf, "BATCH_ONLY_PETIT:");
#ifdef BATCH_ONLY_PETIT
    strcat(buf, " defined\n");
#else
    strcat(buf, " not defined\n");
#endif
}

static void describe_run_options(char *buf, const int maxBuf)
{
    int bufLength = strlen(buf);
    struct option *popt;

    for(popt=&options[0]; popt->pflag; popt++)
        {
        char line1[200], line2[10];

        if(*(popt->pflag) != (int) popt->def_value)
            sprintf(line2, "-%c", popt->symbol);
        else
            sprintf(line2, " %c", popt->symbol);
        sprintf(line1,"%s  %-45s  %s\n",line2,popt->explain,
                *(popt->pflag)?"ON":"off");
        assert(strlen(line1)<200);
        strncat(buf, line1,maxBuf-bufLength-5);
        bufLength += strlen(line1);
        }

	strncat(buf, other_options_explain, maxBuf-bufLength-5);
}

static char * describe_options()
{
    const int maxBuf = 20000;
    static char buf[maxBuf];
    if (buf[0] == '\0')
      {
	describe_compile_options(buf);
	describe_run_options(buf, maxBuf);
	buf[maxBuf-1] = '\0';
      }

    return buf;
}

#if ! defined BATCH_ONLY_PETIT

int br_options(int)
    {
    const int maxBuf = 20000;
    static char buf[maxBuf];
    if (buf[0] == '\0')
      describe_run_options(buf, maxBuf);

    motif_lowlight( br_current() );

    motif_display(buf);
    return 0;
    } /* br_options */



int br_compile_options(int) 
    {
    char buf[5000];
    buf[0] = '\0';

    motif_lowlight( br_current() );

    describe_compile_options(buf);

    motif_display(buf);
    return 0;
    } /* br_compile_options */

#endif

} // end omega namespace
