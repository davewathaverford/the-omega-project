/* $Id: petit_args.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Petit_Args
#define Already_Included_Petit_Args

#include <basic/bool.h>


#include <petit/dddir.h>

namespace omega {

#define PETITBUFSIZ 256
#define DEPWRNAMELEN 256

#define MAX_UNIFORM_ARG_LENGTH 80
typedef struct
    {
    int argc;
    char **argv;
    char *FileName;
    unsigned int DDalgorithm;
    char DepWrFile[DEPWRNAMELEN];
    dd_flags depPrintFilter;
    int onlyPrintRealArrayDeps;
    int dep_analysis_debug;
    int dep_analysis_quick;
    int doOEcompare;
    int max_conjuncts;
    int testClosure;
    int valueClosure;
    int ivr_Assert;
    int ivr_DefEntryClass;
    int ivr_debug;
    int ivr_RemoveNoOp;
    int ivr_ShadowShadow;
    int ivr_on;
    int ivr_RepAffine;
    int ivr_SubstReals;
    int printPhis;
    int printAffine;
    int printShadow;
    int printIxArrays;
    int print_relations;
    int printNoDeadDeps;
    int print_SCC;
    int doEEdeps;
    int doPrivatization;
    int do_calculate_depnum;
    int doArrayExpn;
    int arrDefInOut;
    int quiet;
    int makeReductionOps;
    int repeatArrayExpn;
    int panicked_by_nonaffinity;
    int skipping_omega4;
    int skipping_omega5;
    int li2_only;
    int zappable_interesting_distance; // 0 == don't do zapping
    int just_zap_value_flow;
    int omitTopLevel;
    int omitScalars;
    int expandlimit;
    int lowerdefault;
    int upperdefault;
    bool graph_by_statement;
    bool graph_write_dd_as_graph;
    bool hpp_mode;
    bool hpp_only;
    bool motif_display;
    bool normalize;
    bool uniform;
    char uniform_args[MAX_UNIFORM_ARG_LENGTH];
    } petit_args_struct;

extern petit_args_struct petit_args;

extern void process_args(int argc, char *argv[]);

extern int br_compile_options(int dummy);

extern int br_options(int dummy);

}

#endif
