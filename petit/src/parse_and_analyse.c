/* parse_and_analyse.c */
#include <basic/bool.h>
#include <petit/petit_args.h>
#include <petit/lang-interf.h>
#include <petit/dd_misc.h>
#include <petit/dddir.h>
#include <petit/debug.h>
#include <petit/parse_and_analyse.h>
#include <petit/petity.h>
#include <petit/ssa.h>
#include <petit/linkio.h>
#include <petit/affine.h>
#include <petit/pres-interf.h>
#include <petit/arrayExpand.h>
#include <petit/find_affine.h>
#include <petit/vutil.h>
#include <petit/dddriver.h>
#include <petit/depcycles.h>
#include <petit/message.h>
#include <petit/ddodriver.h>
#include <petit/motif.h>
#include <petit/make.h>

namespace omega {

static void negate_node(node *n)
    {
    node *nn = new node;
    *nn = *n;
    nn->nodenext = NULL;
    nn->nodeprev = NULL;
    nn->nodeparent = NULL;

    n->nodeop = op_subtract;
    n->exptype = exprint;

    node *zero = make_const(0);

    PlaceNodeBelow(zero, n);
    PlaceNodeAfter(nn, zero);
    } /* negate_node */


static void traverse_indexes(node *nn, node *index)
    {
    if (nn->nodeop == op_index && (node *)(nn->nodevalue) == index)
	negate_node(nn);
    else
        for (node *n = nn->nodechild; n != NULL; n=n->nodenext)
            traverse_indexes(n, index);
    } /* traverse_indexes */


static void traverse_loops(node *nn)
    {
    if (nn->nodeop == op_do)
        {
        node *inc = nn->nodechild->nodenext->nodenext->nodenext;
        if (inc!=NULL && (inc->nodeop!=op_constant || inc->nodevalue!=1))
            {
            int back = (inc->nodeop == op_subtract &&
                        inc->nodechild->nodeop == op_constant &&
                        inc->nodechild->nodevalue == 0 &&
                        inc->nodechild->nodenext->nodeop == op_constant &&
                        inc->nodechild->nodenext->nodevalue == 1);
            if (! back)
		Error("Error: non unary loop steps not implemented yet");

	    negate_node(inc);

	    assert(nn->nodechild->nodeop == op_dolimit);

	    node *lb = nn->nodechild->nodenext;
	    node *ub = nn->nodechild->nodenext->nodenext;

	    if (lb->nodeop == op_max)
		for (node *m=lb->nodechild; m!=NULL; m=m->nodenext)
		    negate_node(m);
	    else
		negate_node(lb);

	    if (lb->nodeop == op_min)
		for (node *m=ub->nodechild; m!=NULL; m=m->nodenext)
		    negate_node(m);
	    else
		negate_node(ub);

	    traverse_indexes(nn->nodechild, nn->nodechild);
            }
        }

    for (node *n = nn->nodechild; n != NULL; n=n->nodenext)
        traverse_loops(n);
    } /* traverse_loops */


static void normalize_steps()
    {
    traverse_loops(Entry);
    } /* normalize_steps */


static int parse(char *file)
    {
    Assertions = NULL;
    Entry = NULL;
    ::yyin = fopen(file, "r");

    if (::yyin == 0)
        return false;
    else
        {
#if defined newTimeTrials || killTimeTrials || printKillRelations
        fprintf(debug, "\nParsing File \"%s\"\n", file);
#endif
        ::yyparse();

	if (petit_args.normalize) 
	    normalize_steps();

        fclose(yyin);
        yyin = 0;
	do_display(1);
        return true;
        }
    } /* parse */


static void build_new_info()
    {
    static bool first_time=true;
    link_lists();
    build_ssa();
    recognize_reductions(first_time);
    link_lists();
    find_affine_exprs();
    add_omega_vars(); 
    first_time = false;
    } /* build_new_info */




static void zap_all_old_info()
    {
    zap_affine_exprs();
    zap_dd_graph();
    zap_omega_vars();
    zap_ssa_graph();
    } /* zap_all_old_info */




static void zap_some_old_info()
    {
    zap_ssa_graph();
    zap_omega_vars();
    } /* zap_some_old_info */




static void analyse()
    {
#if ! defined BATCH_ONLY_PETIT
    char line[PETITBUFSIZ];
    sprintf(line, "WAIT: calculating data dependences");
    if (petit_args.motif_display)
        Message_Add(line);
#endif

    zap_all_old_info();
    build_new_info();

    if(petit_args.ivr_RepAffine)
        {
        ReplaceAffine(Entry);
        zap_some_old_info();
        build_new_info();
        }

    RmUnusedDcls();
    build_dd_graph( 0 );

#if ! defined BATCH_ONLY_PETIT
    sprintf(line, "Data dependences calculated, %s", brdd_tally());
    if (petit_args.motif_display)
        Message_Add( line );
#endif
    } /* analyse */




int re_analyse(int /*print*/) 
    {
    if (Entry != NULL) 
	{
        BRDD_Current = NULL;
        analyse();
        } 
    else 
	{
#if ! defined BATCH_ONLY_PETIT
        if (petit_args.motif_display)
            Message_Add("No current program");
#endif
	}

    return 0;
    } /* re_analyse */




static char GLOBAL_FILE[80];

int parse_and_analyse(char *file)
    {
    char line[PETITBUFSIZ];

    int found = 0;

    if (file != 0) 
	{
	sprintf(GLOBAL_FILE, "%s", file);
        petit_args.quiet=true;

        if (parse(file))
            {
            analyse();

            if (petit_args.doArrayExpn)
                ArrayExpansion(1);

            if(petit_args.doPrivatization)
                Privatization(0);

            if (petit_args.doArrayExpn || petit_args.doPrivatization)
                {
                /* flow cycles may have changed - davew */
                reset_flow_cycle_bits(false);
                FindFlowCycles(false,0);
                reset_flow_cycle_bits(true);
                FindFlowCycles(false,1);
                }

	    found = 1;

            sprintf(line, "Parsed %s, %s", file, brdd_tally());
            }
        else
            sprintf(line, "File %s not found", file);

#if ! defined BATCH_ONLY_PETIT
        if (petit_args.motif_display)
            Message_Add(line);
#endif
        }

    if (petit_args.motif_display)
	{
        do_display(1);
	}

    return found;
    } /* parse_and_analyse */




void parse_n_analyse(char *file)
    {
    (void) parse_and_analyse(file);
    } /* parse_n_analyse */




int do_reopen( int  )
    {
    parse_n_analyse(GLOBAL_FILE);
    return 0;
    } /* do_reopen */



#if ! defined BATCH_ONLY_PETIT
int do_newfile(int ) 
    {
    motif_getfile("File: ", parse_n_analyse, "*.t" );
    return 0;
    }/* do_newfile */
#endif



void compare_tests(int found)
    {
    petit_args.DDalgorithm=DDalg_epsilon;
    if (found)
        re_analyse(0);
    else
        {
        fprintf(stderr, "Cannot find file %s\n", petit_args.FileName);
        Exit(1);
        }
    } /* compare_tests */



void write_depends(int found, char *fname)
    {
    if (found)
        {
        if(petit_args.graph_write_dd_as_graph)
            brdd_graph(petit_args.DepWrFile,fname,petit_args.graph_by_statement,true);
        else
            brdd_write(0);

        if (petit_args.do_calculate_depnum)
           calculate_depnum();
        }
    else
        {
        fprintf(stderr, "File %s not found\n", petit_args.FileName);
        Exit(1);
        }
    } /* write_depends */

} // end omega namespace
