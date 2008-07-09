/* $Id: ssa.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/*********************************************************************
 *                                                                   *
 * Include file for Static Single Assignment calculation.            *
 * Written by Vadim Maslov vadik@cs.umd.edu 08/15/92.                *
 *                                                                   *
 *********************************************************************/
#ifndef Already_Included_Ssa
#define Already_Included_Ssa

#include <petit/tree.h>

namespace omega {

/*
 * Representation for defs and uses of petit AST block node.
 * We assume that for every AST block only one definition
 * of any given variable is visible at block end,
 * and uses of only one definition of variable are visible
 * from block beginning.
 */
#define MAXVARNO      250

typedef struct _var_def_use {
    node             *def_node;  /* node where variable is defined or NULL*/
    struct use_nodes *use_nodes; /* list of nodes where var. is used or NULL */
  } var_def_use;

typedef struct _VDU {                 /* array of variables */
  var_def_use a[MAXVARNO];
} VDU;

typedef struct use_nodes {    /* list of uses for one variable */
  node             *use_node; /* node where variable is used */
  struct use_nodes *next;     /* next use or NULL */
} *USE_NODES;

typedef unsigned short int       VARNO;

typedef symtabentry *STE;

/* def_use descr of DO loop for modification by IVR */
extern var_def_use *du_ind_loop;


/*
 * Function prototypes
 */
void      build_ssa(void);
void      recognize_reductions(bool);

char     *str_var_index(node *);

VARNO     FindVar(node *);
VARNO     LookVar(node *);
void      AddUseNodesList(USE_NODES *, node *);

void      zap_ssa_graph(void);

extern VARNO var_no;
//TTT
void print_tree_debug(void);

}

#endif /* Already_Included_Ssa */
