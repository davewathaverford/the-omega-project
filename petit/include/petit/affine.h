/* $Id: affine.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#ifndef Already_Included_Affine
#define Already_Included_Affine 1


#include <basic/bool.h>
#include <petit/lang-interf.h>

namespace omega {

/* This file defines the affine_expr structure and macros & functions
   that are independent of petit - that is, code that examines existing
   affine expressions.  Code that builds affine expressions from the
   parse tree, or part of the parse tree, is in find_affine.h, as that
   code is petit-specific.
 */

#define aeMaxVars 12

typedef struct symtabentry *Var_Id;
typedef node *a_access;

struct affine_term {
    Var_Id petit_var;   /* pointer to symbol table entry */
    int    coefficient; /* co-efficient */
    int    arity;       /* arity of the correponding omega var */
};

// WARNING: these structures are memcpy'd in some files; don't add any 
// class objects here without removing those (copy constructors would not
// be called)
struct affine_expr {
    int  nterms;		   /* # terms, including constant */
    affine_term  terms[aeMaxVars]; /* 1st entry var is always 0, (the const) */
    affine_expr *other_branch;     /* if min or max */
};


#define is_affine(AE)          ( (AE) != &not_affine )
#define node_is_affine(NODE)   ( (NODE)->nodeaffine != (void *)&not_affine )

extern bool nodes_subs_are_affine(a_access A);
/* the above is needed only for one assertion - for some
   array access A, return true iff all subscripts of A are affine */

extern affine_expr not_affine;
/* affine_expr should point to not_affine if expression is not affine */


/* compare 2 affine exprs.
   return 1 if different, 0 if same
 */
int   CmpAffineExprs(affine_expr *, affine_expr *);

/* return a copy allocated with malloc */
affine_expr *CopyAffineExpr(affine_expr *);
void         FreeAffineExpr(affine_expr *);
char *       print_rep(const affine_expr *);  // return static char array

}

#endif
