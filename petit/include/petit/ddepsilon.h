/* $Id: ddepsilon.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_DDEpsilon
#define Already_Included_DDEpsilon

namespace omega {


#define maxSubscr  10
#define maxEpsVars 20

#define DD_YES       2 /* there is dependence */
#define DD_NO        0 /* there is no dependence */
#define DD_QUESTION  1 /* we don't know yet */
#define DD_INEXACT   3 /* epsilon is inexact here ( for epsilon_preproc) */

#define BOUND_LOWER -1
#define BOUND_UPPER  1

#define UNKNOWN_BOUND 500000

#define VAL_NEVER_KNOWN 2 /* bound on subscript is unknown, and won't become known */
#define VAL_UNKNOWN     0 /* bound on subscript is unknown, but will become known */
#define VAL_KNOWN       1 /* boune on subscript is known */  


/* loop_context for the variable */
#define var_id_loop_cont(V) ((var_id_loop(V))->nodechild)

#define loop_start_node(LOOP) ((LOOP)->nodenext)
#define loop_end_node(LOOP)   ((LOOP)->nodenext->nodenext)


/* variable description */

struct EpsVarType {
  int 		lower;
  int 		upper;
  int           l_known;
  int           u_known;
  int           step;
  int 		ddnum; /* if <0, not common loop */
  Var_Id 	var;   
};

typedef EpsVarType * Epsvar_id;

struct BanerjeeInfoType {
  int used;
  int minval[3];
  int maxval[3];
  int min_known[4];
  int max_known[4];
} ;


/* common loop variable description */

struct EpsComvarType {
  Epsvar_id var;
  int       val;
  int       siv0_done;
  int       done;
  int       nused;
  int       coupled;
  BanerjeeInfoType sb[maxSubscr]; /* subscript expression bounds */
} ;

/* description of the variable in the subscript */

struct  EpsTermType {
  Epsvar_id var; /* variable id for this test */
  int a,b;       /* coefficients */
};

/* subscript description */

// WARNING: these structures are memcpy'd in ddepsilon.c; don't add any 
// class objects here without removing those (copy constructors would not
// be called)
struct EpsSubscrType {
   int done;
   int scalar;
   int nterms;
   EpsTermType terms[maxEpsVars];
   int lastcomvar_min;
   int lastcomvar_max;
   } ;

// WARNING: these structures are memcpy'd in ddepsilon.c; don't add any 
// class objects here without removing those (copy constructors would not
// be called)
struct SubscrBoundType{
int maxval;
int minval;
int max_known;
int min_known;
} ;

void epsilon_test(a_access a1, a_access a2,dir_and_diff_info * d_info,
                    situation * sit);
int  epsilon_preproc(a_access a1, a_access a2);

}

#endif
