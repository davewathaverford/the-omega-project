/* $Id: tree.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Tree
#define Already_Included_Tree

#include <basic/bool.h>
#include <omega/Relations.h>
#include <basic/Bag.h>
#include <basic/List.h>
#include <petit/dddir.h>
#include <petit/classes.h>

namespace omega {

/*
 * Strongly connected component number
 */
typedef unsigned short int SCCNO;


/* Tree operators are given as 'optype' */

typedef enum{
#include "op.typ"
} optype;

/* The entries in this table gives character string names for each
* operator, as well as other information.
* The entry in OPvaltype gives the interpretation of the 'nodevalue'
* field for that type of operator.
* */
/* no interpretation */
#define Onone		0
/* integer value, DO nest level, other constant integer value */
#define Ovalue		1
/* pointer to a symbol table entry */
#define Osymbol		2
/* pointer to a node which has a pointer to a symbol table entry */
#define Onodesym	3

struct OPinforec{
    char *OPname;
    unsigned char OPvaltype;
    unsigned char OPprecedence;
    char *OPpetitstring;
    char *OPftnstring;
} ;
extern struct OPinforec OPinfo[];

#define get_op_name(op)		OPinfo[(unsigned int)(op)].OPname
#define get_op_valtype(op)	OPinfo[(unsigned int)(op)].OPvaltype
#define get_op_precedence(op)	OPinfo[(unsigned int)(op)].OPprecedence
#define get_op_petit_string(op)	OPinfo[(unsigned int)(op)].OPpetitstring
#define get_op_ftn_string(op)	OPinfo[(unsigned int)(op)].OPftnstring


/* the different "types" of expressions */
typedef enum {
#include "exprtype.typ"
} exprtype;

struct EXPRinforec{
    char *EXPRname;
} ;
extern struct EXPRinforec EXPRinfo[];

#define get_expr_name(expr)		EXPRinfo[(unsigned int)(expr)].EXPRname


/* Basic data structure is an abstract syntax tree. */

/* The basic abstract syntax tree element is a 'node' */

/* new filed nodeaffine represents an expression as an affine function
   of variables in the program.  It is needed only in the omega test,
   and should be initialized to 0.  see affine.h for more info
   davew@cs.umd.edu 3/24/92 */


struct affine_expr;

typedef struct node {
    optype         nodeop;	 /* this node operation */
    unsigned long int         nodevalue;	 /* value, polymorphic */
    exprtype       exptype;      /* (useful if this is expr node) */
    struct node *  nodenext;
    struct node *  nodeprev;
    struct node *  nodechild;
    struct node *  nodeparent;
    struct node *  nodecontext;  /* efficiency hack for omega test */
    struct node *  nodelink;	 /* link to the nest reference to this var. */
    struct node *  noderight;
    unsigned long int         nodeextra;
    struct node *  node_copy;
    unsigned int           nodesequence; /* sequence number (such as a line number) */
    unsigned int           nodeposition; /* position on the window */
    struct ddnode *nodeddout;	 /* outcoming dependences */
    struct ddnode *nodeddin;     /* incoming dependences */
    struct affine_expr *nodeaffine;    /* needed only in omega test;
					  initialize to 0 */
    struct symtabentry *function;      /* omega - initialize to 0 */
    int cover_depth, terminator_depth; /* omega -  initialize to -1, -1 */

    /* === Induction Variable Recognition & SSA stuff */
    union {
      /* op_fetch, op_update, op_store:
	 op_assign node where variable is assigned to */
      struct node      *def_node;

      /* op_assign, op_oper_assign:
	 list of op_fetch/op_update nodes in which this assignment is used */
      struct use_nodes *use_nodes;

      /* op_phi:  op_do or op_if node which created this phi-fun */
      struct node      *phi_parent;
    } ud;

    SCCNO        sc_region;  /* SCC number */
    SCCNO        sc_number;  /* for FindSCC */
    SCCNO        sc_lowlink; /* for FindSCC */
    bool         sc_onStack; /* for FindSCC */
    bool         is_loopfun; /* flag: is closed form function of loop var. */
    Set<node*>   edges;	     /* For various tmp uses, include dep cycles */
    struct node **ssag_node;  /* for efficiency purposes */

    struct node *shadow;	/* shadow expression */

    unsigned int compound_stmt;
    unsigned int version;

    /* === */
} node;

/* Main entry point. */
extern node *Entry;

extern node *ExitNode;
extern unsigned long int Lines;

/* List of assertions, linked by nodelink pointers */
extern node *Assertions;

/* The symbol table consists of 26 possible symbols, */
/* indexed by name, character a - z */

/* different symbol types */

typedef enum{
#include "symtype.typ"
} symboltype;

struct SYMinforec{
    char *symtypename;
};
extern struct SYMinforec SYMinfo[];

#define get_symtype_name(symtype)	SYMinfo[(unsigned int)(symtype)].symtypename

/* new field symtag is used in the omega test to "tag" variables
   used in expressions so that we know if we are using the same one
   repeatedly.  The tag was originally a int (used for column number),
   but for the new "Presburger Interface", this field will be used as
   a pointer to a global variable declaration.  It should still be
   initialized to "UNTAGGED".
 */

#define UNTAGGED 0

#define symvalue_unknown	(-100000000)

struct symtabentry {
    symtabentry();
    ~symtabentry();
    symboltype symtype;	        /* syminteger or symreal or symconst */
    var_class  symclass;	/* storage class */
    unsigned int       symdims;		/* count dimensions */
    node *     symdecl;		/* pointer to op_declare node */
    char *     symname;		/* symbol name */
    unsigned long int     symvalue;	/* initial value (for const) */
    exprtype   exptype;	        /* similar to symtype, but different */
    bool       sym_con_var;	/* flag: tag_var_as_changeable_or_constant()*/
    unsigned int       symoffset;	/* offset from frame ptr in memory   */
    node *     def_node;	/* where this scalar is defined  */
    struct nametabentry *nte;      /* pointer to nametabentry  */
    struct symtabentry  *original; /* original symtabentry for var. instance */
    struct symtabentry  *symcopy;  /* copy of this node */
    void *     symtag;
    List<Omega_Var *> * omega_vars; /* list of pointers to omega vars */
    Relation *alignment;  /* alignment for arrays */
    String template_name;   /* template it is distributed over */
};

typedef struct nametabentry {
    struct nametabentry *namenext;
    symtabentry          namest;
} nametabentry;

extern nametabentry *NameHead;

typedef enum{
#include "ddtype.typ"
} ddnature;

struct DDinforec{
    char *ddtypename;
};
extern struct DDinforec DDinfo[];

#define get_ddtype_name(ddtype)	DDinfo[(unsigned int)(ddtype)].ddtypename


#define maxCommonNest 7		/* maximal number of common loops */

extern char* DDdirname[];	/* defined for 0-15 */

typedef int *DDdifference;

struct ddnode {
    ddnode(int nest);
    ddnode(ddnode *ddold);
    ~ddnode();

    ddnature       ddtype;	/* flow, anti, output, reduction */
    node          *ddpred, *ddsucc;
    struct ddnode *ddnextpred, *ddnextsucc;
    struct ddnode *ddlink;	/* chain of saved dependence arcs */
    unsigned int           ddnest;
    dddirection    ddrestraint; /* restraint vector 3/29/92 davew@cs.umd.edu */
    dddirection    dddir;
    dd_flags       ddflags;
    DDdifference   dddiff;
    unsigned long int         ddextra;
    signed char    loop_dir[maxCommonNest+1];  /*  1: step is positive, 
						  -1: step is negative */
    Relation       *dd_relation;
#if defined STUDY_KILL_USE
    int            kill_use_count;/* temporary used for study of output deps */
#endif
};
#if defined STUDY_KILL_USE
extern int number_of_kills_for_use_count;
#endif

#define is_stor_dep(dd) ((dd)->ddtype==ddanti || (dd)->ddtype==ddoutput)
#define is_flow_dep(dd) ((dd)->ddtype==ddflow || (dd)->ddtype==ddreduce)

/* Get the interpreted 'nodevalue' entry */
#define get_nodevalue_sym(n)	((symtabentry*)((n)->nodevalue))
#define get_nodevalue_int(n)	((int)((n)->nodevalue))
#define get_nodevalue_node(n)	((node*)((n)->nodevalue))

}

#endif
