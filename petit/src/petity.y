/* $Id: petity.y,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

/* declarations section */
%{
#include <string.h>
#include <stdlib.h>
#include <basic/assert.h>
#include <basic/Exit.h>
#include <petit/lang-interf.h>
#include <petit/make.h>
#include <petit/tree.h>
#include <petit/vutil.h>
#include <petit/petity.h>
#ifdef WIN32
#include <io.h>
#define isatty _isatty
#include <malloc.h>
#define alloca _alloca
#endif

#define AllowComments 1

namespace omega {

static void parse_die();
static int parse_intval();
static void parse_linknode( node *new_node );
static void parse_unlinknode();
static void parse_linknodeto( node *old, node *new_node );
static void parse_pushscope( node *n );
static void parse_popscope();
static node *parse_binaryop( node *left, optype op, node *right );
static void parse_forloop(optype op, symtabentry *index,
			       node *lower, node *upper, node *inc);
static void parse_endforloop();

static void parse_stmt_index( node *n );
static void parse_check_stmt_number();
static void parse_init();
static void yyerror( char *s);
symtabentry *add_symbol(const char *name, int new_int,
			symboltype symtype, exprtype exptype,
			nametabentry *onam);
static void CheckEnd();
static symtabentry *parse_doid_symbol();

/* global variables: */
/* line number */
unsigned long int Lines;

/* Entry node */
node *Entry;

/* Exit node */
node *ExitNode;

/* List of assertions */
node *Assertions = NULL;

/* Current head of name table */
nametabentry *NameHead;

/* communication from lexer */
/* name of ID */
char P_IDname[MAXIDLENGTH], P_INTname[MAXIDLENGTH];

/* line number of next token */
unsigned long int P_Lines;
} // end of namespace omega

#ifdef WIN32
#include <io.h>
#define isatty _isatty
#endif
#include "petit/petitl.c"

namespace omega {
/* current nest level in loops */
unsigned long int P_LoopNest;

#include <petit/ddutil.h>
char *loopsyms[maxnest];

/* current statement number */
unsigned long int P_StmtNumber;

/* A node pointer */
node *P_Node;

/* A symbol table entry pointer */
symtabentry *P_Sym, *P_Sym1, *P_DeclSym;

/* type of symbol being declared */
symboltype P_SaveType;

var_class SaveClass;

/* What to use for 'prev' of next node, 'parent' of next node */
node *P_NextPrev, *P_NextParent;

/* To allow undeclared variables appear in array bounds */
int NoNewOldCheck = 0;

/* to prohibit RETURN statements from appearing not at the end */
int ShouldBeEnd;

} // end of namespace omega

using namespace omega;

/* end of declarations section */
%}

%union {
    int iv;
    node *np;
    symtabentry *stp;
    optype op;
    }

%token <iv> TCOMMENT
%token <iv> TASSERT
%token <iv> TBY
%token <iv> TCEILING TCOLON TCOMMA TCONST
%token <iv> TDO TDOANY
%token <iv> TELSE TENDFOR TENDIF TEQ TEQUAL TEXP
%token <iv> TFLOAT TFLOOR TFOR
%token <iv> TGE TGT
%token <iv> TRAND TID TIF TINT TINTEGER
%token <iv> TLE TLT TLP
%token <iv> TMAX TMIN TMINUS TMOD
%token <iv> TNE
%token <iv> TAND TOR TNOT
%token <iv> TPARFOR TPLUS
%token <iv> TREAL TRP
%token <iv> TSEMI TSLASH TSQRT TSTAR
%token <iv> TTHEN TTO
%token <iv> TVECFOR
%token <iv> TLCB TRCB
%token <iv> TFORMAL TCOMMON TSTATIC TAUTO TPRIVATE TBUILTIN 
%token <iv> TIN TOUT TINOUT
%token <iv> TRETURN
%token <iv> TPLUSEQUAL TSTAREQUAL TMAXEQUAL TMINEQUAL

%start pgm

%left TOR
%left TAND
%left TNOT
%nonassoc TEQ TGE TGT TLE TLT TNE
%left TPLUS TMINUS
%left TSLASH TSTAR
%left TMAX TMIN
%nonassoc TMOD
%right TEXP

%type <np> assocop 
%type <iv> cexpr
%type <iv> type class
%type <np> expr optforby
%type <np> lhs
%type <stp> newid doid
%type <np> oldid
%type <op> asgnoper

%%
/* rules section */

pgm :
        {
	  parse_init();
	  ShouldBeEnd = 0;
	}
        stlist  
        {
	  P_Node = make_node(op_exit);
	  ExitNode = P_Node;
	  parse_linknode(P_Node);
	}
;


stlist :
	stmt 
|
	stlist TSEMI stmt
;

stmt :
|	stassert
|	stcomment
|	stasgn      {CheckEnd();}
|	stasgnoper  {CheckEnd();}
|	stconst
|	stfor       {CheckEnd();}
|	stforall    {CheckEnd();}
|	stif        {CheckEnd();}
|	stparfor    {CheckEnd();}
|	stdoany     {CheckEnd();}
|	stdecl
|       streturn    {ShouldBeEnd=1;}
;

streturn  : TRETURN {}
|           TRETURN expr {free_node($2);}
;

stassert  : TASSERT TLP expr TRP
		{
		    parse_check_stmt_number();   /* what the heck... */
		    P_Node = make_node( op_assert );
		    P_Node->nodelink = Assertions;
		    Assertions = P_Node;
		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );
		    parse_linknode( $3 );
		    parse_popscope();
		}
;

stcomment : TCOMMENT  {
                if(1) {
		    P_Sym = parse_symbol(-1);
		    P_Node = make_node( op_comment );
		    P_Node->nodevalue = (unsigned long int)P_Sym;
		    parse_linknode( P_Node );
		    P_Sym->symtype = symcomment;
		    P_Sym->symdims = (unsigned) -1;
		    P_Sym->symdecl = P_Node;

		    P_Sym->exptype = exprint; /* cexpr's are only integers */
		  }
		}
;

stasgn :
	lhs TEQUAL expr
		{
		  node *rp = $3;

		  parse_check_stmt_number();
		  P_Node = make_node( op_assign );
		  
		  /* make sure types are compatible */
		  /* note: refreal can accept any type */
		  if( $1->exptype == exprrefint && rp->exptype != exprint ) {
		    rp = make_node(op_rtoi);
		    rp->exptype = exprrefint;
		    rp->nodechild = $3;
		    ($3)->nodeparent = rp;
		  }
		  
		  parse_linknodeto( P_Node, rp );
		  parse_linknodeto( P_Node, $1 );
		  parse_linknode( P_Node );
		}
;


stasgnoper : lhs asgnoper expr 
                {
		  optype *pop;
		  
		  parse_check_stmt_number();
		  P_Node = make_node($2);
		  
		  /* make sure types are compatible */
		  /* note: refreal can accept any type */
		  if( $1->exptype == exprrefint && $3->exptype != exprint ) {
		    fprintf(stderr, "*** can't assign real to integer\n");
		    parse_die();
		  }

		  parse_linknodeto( P_Node, $3 );
		  parse_linknodeto( P_Node, $1 );
		  parse_linknode( P_Node );

		  pop = &P_Node->nodechild->nodenext->nodeop;
		  if(*pop==op_store) {
		    *pop = op_update;
		  } else if(*pop==op_store_array) {
		    *pop = op_update_array;
		  }
                }
;


asgnoper : TPLUSEQUAL  {$$ = op_add_assign;}
|          TSTAREQUAL  {$$ = op_mul_assign;}
|          TMAXEQUAL   {$$ = op_max_assign;}
|          TMINEQUAL   {$$ = op_min_assign;}
;


lhs :
	TID
		{
		    P_Sym = parse_symbol(0);
		    if( P_Sym->symdims > 0 ){
			fprintf(stderr, "*** lhs array must have %d subscripts: %s\n", 
				P_Sym->symdims, P_Sym->symname );
			parse_die();
		    }
		    if( P_Sym->symtype == symconst ){
			fprintf(stderr, "*** lhs must be variable: %s\n",
				P_Sym->symname );
			parse_die();
		    }
		    if( P_Sym->symtype == symindex ){
			fprintf(stderr, "*** lhs cannot be loop index: %s\n",
				P_Sym->symname );
			parse_die();
		    }

		    P_Node = make_node( op_store );
		    P_Node->nodevalue = (unsigned long int)P_Sym->symdecl;

		    P_Node->exptype = P_Sym->exptype; /* refint or refreal */

		    $$ = P_Node;
		}
|
	TID
		{
		    P_Sym = parse_symbol(0);
		    if( P_Sym->symdims <= 0 ){
			fprintf(stderr, "*** not an array: %s\n", P_Sym->symname );
			parse_die();
		    }
		    P_Node = make_node( op_store_array );
		    P_Node->nodevalue = (unsigned long int)P_Sym->symdecl;

		    if( P_Sym->exptype == exprintarray )
			P_Node->exptype = exprrefint;
		    else
		    if( P_Sym->exptype == exprrealarray )
			P_Node->exptype = exprrefreal;
		    else
			P_Node->exptype = exprunknown;

		    parse_linknode( P_Node );
		    parse_pushscope( P_Node ); 
		    $<np>$ = P_Node;
		    /* save it because it can be reassigned down there */
		    P_Sym1 = P_Sym; 
		}
	TLP list TRP
		{
		    /* make sure all subscripts are integer expressions */
		    int count = 0;
		    for( P_Node = ($<np>2)->nodechild; P_Node != NULL;
				      P_Node = P_Node->nodenext ){
		        count++;
			if( P_Node->exptype != exprint ) {
			    fprintf(stderr, "*** All subscripts must be integer\n" );
			    parse_die();
			}
		    }
		    if(count != (int)P_Sym1->symdims) {
			fprintf(stderr, "*** Wrong number of subscripts in %s (store)\n", P_Sym1->symname );
			parse_die();
		    }

		    parse_popscope();
		    parse_unlinknode();
		    $$ = $<np>2;
		}
;

stconst :
       TCONST constlist {}
;

constlist :
	constdecl
|
	constlist TCOMMA constdecl
;

constdecl :
	newid TEQUAL cexpr
		{
		    P_Sym = $1;
		    P_Node = make_node( op_declare );
		    P_Node->nodevalue = (unsigned long int)P_Sym;
		    parse_linknode( P_Node );
		    P_Sym->symtype = symconst;
		    P_Sym->symdims = (unsigned) -1;
		    P_Sym->symvalue = $3;
		    P_Sym->symdecl = P_Node;

		    P_Sym->exptype = exprint; /* cexpr's are only integers */
		}
;

stfor :
	TDO doid TEQUAL expr forto expr optforby TSEMI
	    { parse_forloop( op_do, $2, $4, $6, $7 ); }
	    stmt
            { parse_endforloop(); }
|
	TDO doid TEQUAL expr forto expr optforby
	    { parse_forloop( op_do, $2, $4, $6, $7 ); }
	    TLCB stlist TRCB
	    { parse_endforloop(); }
|
	TFOR doid TEQUAL expr forto expr optforby
	    { parse_forloop( op_do, $2, $4, $6, $7 ); }
	    TDO stlist TENDFOR
	    { parse_endforloop(); }
;


optforby :          {$$ = NULL;}
|       forby expr  {$$ = $2;}
;


stforall :
	TVECFOR doid TEQUAL expr forto expr optforby 
		{
		    parse_forloop( op_dovec, $2, $4, $6, $7 );
		}
	TDO stlist TENDFOR
		{
		    parse_endforloop();
		}
;

stparfor :
	TPARFOR doid TEQUAL expr forto expr optforby
		{
		    parse_forloop( op_dopar, $2, $4, $6, $7 );
		}
	TDO stlist TENDFOR
		{
		    parse_endforloop();
		}
;

stdoany :
	TDOANY doid TEQUAL expr forto expr optforby
		{
		    parse_forloop( op_doany, $2, $4, $6, $7 );
		}
	TDO stlist TENDFOR
		{
		    parse_endforloop();
		}
;

forto :
        TTO {}
|
	TCOMMA {}
|
	TCOLON {}
;

forby :
	TBY {}
|
	TCOMMA {}
|
	TCOLON {}
;

stif :
	TIF expr 
		{
		    parse_check_stmt_number();
		    P_Node = make_node( op_if );

		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );
		    parse_linknode( $2 );
		    P_Node = make_node( op_then );
		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );
		}
	TTHEN stlist
	elsepart
	TENDIF
		{
		    parse_popscope();
		    parse_popscope();
		}
;


elsepart :
	TELSE 
		{   parse_popscope();
		    P_Node = make_node( op_else );
		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );
		}
	stlist
|
;

stdecl :  class type {SaveClass=(var_class) $1; P_SaveType=(symboltype) $2;} decllist
;

class :          {$$ = CLunknown;}
|       TAUTO    {$$ = CLAUTO;}
|       TCOMMON  {$$ = CLCOMMON;}
|       TSTATIC  {$$ = CLSTATIC;}
|       TFORMAL  {$$ = CLFORMAL;}
|       TIN      {$$ = CLIN;}
|       TOUT     {$$ = CLOUT;}
|       TINOUT   {$$ = CLINOUT;}
|       TPRIVATE {$$ = CLPRIVATE;}
|       TBUILTIN {$$ = CLBUILTIN;}
;

type :  TINTEGER  {$$=syminteger;}
|       TREAL     {$$=symreal;}
;


decllist :
	decl
|
	decllist TCOMMA decl
;

decl :
	newid
		{
		    P_DeclSym = $1;
		    P_Node = make_node( op_declare );
		    P_Node->nodevalue = (unsigned long int)P_DeclSym;
		    parse_linknode( P_Node );
		    P_DeclSym->symtype = P_SaveType;
		    P_DeclSym->symdims = 0;
		    P_DeclSym->symdecl = P_Node;
		    if(SaveClass == CLBUILTIN) {
		      fprintf(stderr, "Not function %s\n", $1->symname);
		      parse_die();
		    }
		    P_DeclSym->symclass =
		      SaveClass==CLunknown ? CLAUTO : SaveClass;

		    /* determine the type based on declaration */
		    if( P_SaveType == syminteger )
			P_DeclSym->exptype = exprrefint;
		    else if( P_SaveType == symreal )
			P_DeclSym->exptype = exprrefreal;
		    else
			P_DeclSym->exptype = exprunknown;
		}
        init
|
	newid
		{
		    P_DeclSym = $1;
		    P_Node = make_node( op_declare );
		    P_Node->nodevalue = (unsigned long int)P_DeclSym;
		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );
		    P_DeclSym->symtype = P_SaveType;
		    P_DeclSym->symdecl = P_Node;
		    P_DeclSym->symclass = SaveClass==CLunknown ?
		      (1 ? CLINOUT : CLAUTO) : SaveClass;
		}
	TLP {NoNewOldCheck=1;} boundlist {NoNewOldCheck=0;} TRP
		{
		    /* determine the type based on declaration */
		    if(P_DeclSym->symdims!=DIMSFUN || SaveClass!=CLBUILTIN) {
		      if(P_DeclSym->symdims==DIMSFUN) {
			P_DeclSym->symdims = 1;
		      }
		      if( P_SaveType == syminteger )
			P_DeclSym->exptype = exprintarray;
		      else if( P_SaveType == symreal )
			P_DeclSym->exptype = exprrealarray;
		      else
			P_DeclSym->exptype = exprunknown;
		    } else {
		      if( P_SaveType == syminteger )
			P_DeclSym->exptype = exprintfun;
		      else if( P_SaveType == symreal )
			P_DeclSym->exptype = exprrealfun;
		      else
			P_DeclSym->exptype = exprunknown;
		    }

		    parse_popscope();
		}
        init
;


init :
|       TEQUAL init1 {}
|       TEQUAL TLCB initlist TRCB {}
;

initlist : init1
|          initlist TCOMMA init1

init1 : expr {free_node($1);}
;


boundlist :
                                {   P_DeclSym->symdims = DIMSFUN; }
|	bound	                {   P_DeclSym->symdims = 1; }
|	boundlist TCOMMA bound	{   P_DeclSym->symdims++; }
;

bound :
	expr
		{
		    P_Node = make_node( op_bounds );
		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );

		    P_Node = make_node( op_constant );
		    P_Node->nodevalue = 1;
		    parse_linknode( P_Node );

		    P_Node = $1;
		    parse_linknode( P_Node );
		    parse_popscope();
		}
|
	expr TCOLON expr
		{
		    P_Node = make_node( op_bounds );
		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );

		    P_Node = $1;
		    parse_linknode( P_Node );

		    P_Node = $3;
		    parse_linknode( P_Node );
		    parse_popscope();
		}
;

newid :
	TID
		{   $$ = parse_symbol(1); }
;

doid :
	TID
		{   $$ = parse_doid_symbol(); }
;

cexpr :
	TID
		{   
		    P_Sym = parse_symbol(0);
		    $$ = P_Sym->symvalue;
		    /* don't allow variables in constant expressions */
		    if( P_Sym->symtype != symconst ){
			fprintf(stderr, "*** not a constant: %s\n", P_Sym->symname );
			parse_die();
		    }
		}
|
	TINT
		{   $$ = parse_intval(); }
|
	cexpr TPLUS cexpr
		{   $$ = $1 + $3; }
|
	cexpr TMINUS cexpr
		{   $$ = $1 - $3; }
|
	TMINUS cexpr
		{   $$ = - $2; }
|
	TPLUS cexpr
		{   $$ = $2; }
|
	cexpr TSTAR cexpr
		{   $$ = $1 * $3; }
|
	TLP cexpr TRP
		{   $$ = $2; }
;

oldid :
	TID
		{   $$ = parse_symbolnode(); }
;

expr :
	oldid
		{
		    $$ = $1;
		    if($1->nodeop == op_fetch_array) {
		        P_Node = (node*)$1->nodevalue;
			P_Sym = (symtabentry*)P_Node->nodevalue;
			fprintf(stderr,
				"*** array must have %d subscripts: %s\n",
				P_Sym->symdims, P_Sym->symname );
			parse_die();
		    }
		    if($1->nodevalue == 0) {
		        fprintf(stderr,	"*** You use variable (loop index) which is not completely declared ***\n");
			parse_die();
		    }
		}
|
	oldid
		{
	            P_Node = (node*)$1->nodevalue;
		    P_Sym = (symtabentry*)P_Node->nodevalue;
		    if( $1->nodeop == op_fetch ){
			fprintf(stderr, "*** not an array or function call: %s\n", P_Sym->symname );
			parse_die();
		    }
		    else if( $1->nodeop == op_index ){
			fprintf(stderr, "*** not an array or function call: %s\n", P_Sym->symname );
			parse_die();
		    }
		    parse_linknode( $1 );
		    parse_pushscope( $1 );
		}
	TLP list TRP
		{   parse_popscope();
		    parse_unlinknode();

	            P_Node = (node*)$1->nodevalue;
		    P_Sym = (symtabentry*)P_Node->nodevalue;
		    if($1->nodeop != op_call) {
		      int count = 0;
		      /* make sure all subscripts are integer expressions */
		      for( P_Node = $1->nodechild; P_Node != NULL;
			  P_Node = P_Node->nodenext ) {
			count++;
			if( P_Node->exptype != exprint ) {
			  fprintf(stderr, "*** All subscripts must be integer\n" );
			  parse_die();
			}
		      }
		      if(count != (int)P_Sym->symdims) {
			fprintf(stderr, "*** Wrong number of subscripts in %s (load)\n", P_Sym->symname );
			parse_die();
		      }
		    }

		    $$ = $1;
		}
|
	TINT
		{
		    P_Node = make_node( op_constant );
		    P_Node->nodevalue = parse_intval();
		    P_Node->exptype = exprint;
		    $$ = P_Node;
		}
|
	TFLOAT
		{
		    P_Sym = parse_symbol(-1);
		    P_Node = make_node( op_real );
		    P_Node->nodevalue = (unsigned long int)P_Sym;
		    P_Node->exptype = exprreal;
		    $$ = P_Node;
		}
|
	TLP expr TRP
		{   $$ = $2; }
|
	expr TPLUS expr
		{   $$ = parse_binaryop($1,op_add,$3); }
|
	expr TMINUS expr
		{   $$ = parse_binaryop($1,op_subtract,$3); }
|
	TPLUS expr
		{   $$ = $2; }
|
	TMINUS expr
		{   $$ = parse_binaryop(make_const(0),op_subtract,$2); }
|
	expr TSTAR expr
		{   $$ = parse_binaryop($1,op_multiply,$3); }
|
	expr TSLASH expr
		{   $$ = parse_binaryop($1,op_divide,$3); }
|
	expr TEXP expr
		{   $$ = parse_binaryop($1,op_exp,$3); }
|
	expr TLT expr
		{   $$ = parse_binaryop($1,op_lt,$3); }
|
	expr TLE expr
		{   $$ = parse_binaryop($1,op_le,$3); }
|
	expr TEQ expr
		{   $$ = parse_binaryop($1,op_eq,$3); }
|
	expr TNE expr
		{   $$ = parse_binaryop($1,op_ne,$3); }
|
	expr TGE expr
		{   $$ = parse_binaryop($1,op_ge,$3); }
|
	expr TGT expr
		{   $$ = parse_binaryop($1,op_gt,$3); }
|
        expr TAND expr
                {   $$ = parse_binaryop($1,op_and,$3); }
|
        expr TOR expr
                {   $$ = parse_binaryop($1,op_or,$3); }
|
        TNOT expr
                {   $$ = parse_binaryop($2,op_not,NULL); }
|  
	expr TMOD expr
		{   $$ = parse_binaryop($1,op_mod,$3); }
|
	expr TMAX expr
		{   $$ = parse_binaryop($1,op_max,$3); }
|
	expr TMIN expr
		{   $$ = parse_binaryop($1,op_min,$3); }
|
	TSQRT TLP expr TRP
		{   $$ = parse_binaryop(NULL,op_sqrt,$3); }
|
	TRAND TLP TRP
                {   $$ = make_node( op_rand );
                    $$->exptype = exprint;
                }
|
	TFLOOR TLP expr TSLASH expr TRP
		{   $$ = parse_binaryop($3,op_floor_divide,$5);
		    $$->exptype = exprint;
		}
|
	TCEILING TLP expr TSLASH expr TRP
		{   $$ = parse_binaryop($3,op_ceiling_divide,$5);
		    $$->exptype = exprint;
		}
|
	assocop
		{
		    parse_linknode( $1 );
		    parse_pushscope( $1 );
		}
	TLP list TRP
		{
		    parse_popscope();
		    parse_unlinknode();

		    /* if any of the list's expressions is real, so is this */
		    $1->exptype = exprint;
		    for( P_Node = $1->nodechild;
			 P_Node != NULL; P_Node = P_Node->nodenext ) {
			if( P_Node->exptype != exprint ) {
			    $1->exptype = exprreal;
			}
		    }
		    $$ = $1;
		}
;

assocop :
	TMAX
		{ $$ = make_node( op_max ); }
|
	TMIN
		{ $$ = make_node( op_min ); }
;

list :
	expr
		{   parse_linknode( $1 ); }
|
	list TCOMMA expr
		{   parse_linknode( $3 ); }
;

%%
/* program section */

namespace omega {

static 
void CheckEnd() {
  if(ShouldBeEnd) {
    Error("RETURN statement at the middle of a program");
  }
}


/* parse_die */
static void parse_die()
{
    fprintf(stderr, "Ended on line %ld at %s\n", Lines, yytext );
    Exit(1);
}/* parse_die */

/* Utility routine ... get the integer */
static int parse_intval()
{
int val, p;
char ch;

    p = 0;
    val = 0;
    while( (ch=P_INTname[p]) >= '0' && ch <= '9' ){
	val = val * 10 + (ch - '0');
	p = p + 1;
    }
    return val;
}/* parse_intval */

/* Link in a new node at this point. */
static void parse_linknode( node *new_node )
{
    new_node->nodeprev = P_NextPrev;
    new_node->nodeparent = P_NextParent;

    if( P_NextPrev != NULL )
	P_NextPrev->nodenext = new_node;
    if( P_NextParent != NULL && P_NextParent->nodechild == NULL )
	P_NextParent->nodechild = new_node;

    P_NextPrev = new_node;
}/* parse_linknode */

/* unlink node, when it doesn't really belong */
static void parse_unlinknode()
{
node *old;
    old = P_NextPrev;

    P_NextPrev = old->nodeprev;

    if( P_NextParent != NULL && P_NextParent->nodechild == old )
	P_NextParent->nodechild = NULL;
    if( P_NextPrev != NULL )
	P_NextPrev->nodenext = NULL;

    old->nodeprev = NULL;
    old->nodeparent = NULL;
}/* parse_unlinknode */

static void parse_linknodeto( node *old, node *new_node )
{
node *last;
    for(last = old->nodechild;
	last != NULL && last->nodenext != NULL;
	last = last->nodenext ) ;

    new_node->nodeprev = last;
    new_node->nodeparent = old;

    if( last != NULL )
	last->nodenext = new_node;
    if( old->nodechild == NULL )
	old->nodechild = new_node;
}/* parse_linknodeto */

/* Push and Pop scopes */
static void parse_pushscope( node *n )
{
    if( n == 0 ){
	P_NextParent = P_NextPrev;
    } else {
	P_NextParent = n;
    }
    P_NextPrev = NULL;
}/* parse_pushscope */

static void parse_popscope()
{
    P_NextPrev = P_NextParent;
    P_NextParent = P_NextPrev->nodeparent;
}/* parse_popscope */


static exprtype expression_type( node *left, optype op, node *right )
{
exprtype result;

    /* assume the expression's type is integer until proven otherwise */
    result = exprint;

    switch( op ) {
    case op_add:
    case op_subtract:
    case op_multiply:
    case op_divide:
    case op_exp:
    case op_mod:
    case op_max:
    case op_min:
    case op_floor_divide:
    case op_ceiling_divide:
      /* if either expression is real, the result is real */
      if( left != NULL ) {
	  if( left->exptype != exprint && left->exptype != exprrefint ) {
	      result = exprreal;
	  }
      }
      if( right != NULL ) {
	  if( right->exptype != exprint && right->exptype != exprrefint ) {
	      result = exprreal;
	  }
      }
      break;

    case op_lt:
    case op_le:
    case op_eq:
    case op_ne:
    case op_ge:
    case op_gt:
      /* type already set to integer, which these are */
      break;

    case op_sqrt:
      /* type is always real in this case */
      result = exprreal;
      break;

    default:
      /* default is unknown, which had better not happen! */
      result = exprunknown;
      break;
    }

    return( result );
}/* expression_type */

static node *parse_binaryop( node *left, optype op, node *right )
{
node *n;

    n = make_node( op );

    /* get the expression's type */
    n->exptype = expression_type( left, op, right );

    if( left  != NULL ) parse_linknodeto( n, left  );
    if( right != NULL ) parse_linknodeto( n, right );
    return n;
}/* parse_binaryop */

static void parse_forloop(optype op, symtabentry *index,
			node *lower, node *upper, node *inc) {
    node *donode, *doindex;
    int i;

    if(!(P_LoopNest<maxnest))
      ErrAssert("parse_forloop: too deep loop nesting level");
    for(i=0; i<(int)P_LoopNest; i++) {
      if(strcmp(loopsyms[i],index->symname)==0) {
	fprintf(stderr,
		"Loop parameter `%s' is already used in outside loop\n",
		index->symname);
	parse_die();
      }
    }
    loopsyms[P_LoopNest]=index->symname;

    donode = make_node( op );
    parse_linknode( donode );
    ++P_LoopNest;
    donode->nodevalue = P_LoopNest;
    parse_pushscope( donode );

    doindex = make_node( op_dolimit );
    doindex->nodevalue = (unsigned long int)index;
    parse_linknode( doindex );

    parse_linknode( lower );
    parse_linknode( upper );
    if( inc != NULL ) parse_linknode( inc );

    parse_pushscope( doindex );

    index->symtype = symindex;
    index->symdecl = doindex;
    index->exptype = exprrefint; /* we only allow integer index variables */

    /* make sure the lower and upper loop bounds are integer expressions */
    if( lower->exptype != exprint ) {
	fprintf(stderr, "*** lower bound not integer expression\n" );
	parse_die();
    }
    else
    if( upper->exptype != exprint ) {
	fprintf(stderr, "*** upper bound not integer expression\n" );
	parse_die();
    }
    else
    if( inc != NULL ) {
	if( inc->exptype != exprint ) {
	    fprintf(stderr, "*** increment not integer expression\n" );
	    parse_die();
	}
    }
}/* parse_forloop */

static void parse_endforloop()
{
symtabentry *index;

    index = (symtabentry*)P_NextParent->nodevalue;
    index->symtype = symoldindex;
    parse_popscope();
    parse_popscope();
    --P_LoopNest;
}/* parse_endforloop */

nametabentry *parse_findsym()
{
nametabentry *f;

    for( f = NameHead; f != NULL; f = f->namenext ){
	if( strcmp( f->namest.symname, P_IDname ) == 0 &&
	    f->namest.symtype != symoldindex ){
	     return f;
	}
    }
    return NULL;
}/* parse_findsym */

symtabentry *add_symbol(const char *name, int new_int,
			symboltype symtype, exprtype exptype,
			nametabentry *onam)
{
  nametabentry *nam;

#if 0  
  fprintf(stderr, "At line %ld, adding symbol %s to name table\n", Lines, name);
  for (nam = NameHead; nam; nam = nam->namenext)
  {
      fprintf(stderr, "   %s\n", nam->namest.symname);
  }
#endif

  nam = new nametabentry;
  if (!nam) out_of_memory();
  nam->namenext = NameHead;
  NameHead = nam;
  nam->namest.symtype = symtype;
  nam->namest.symdims = 0;
  nam->namest.symdecl = NULL;
  nam->namest.def_node = NULL;
  nam->namest.symname = (char*)calloc( strlen( name )+1, 1 );
  if (!nam->namest.symname) out_of_memory();
  strcpy( nam->namest.symname, name );
  nam->namest.nte = nam;
  nam->namest.original = NULL;
  nam->namest.exptype = exptype;
  nam->namest.symoffset = 0;	  /* set later if needs memory */
  nam->namest.symtag = UNTAGGED;
  nam->namest.omega_vars = 0;
  if(new_int==0 && onam==NULL) {
    node *n = make_node( op_declare );
    n->nodevalue = (unsigned long int)(&nam->namest);
    nam->namest.symdecl = n;
    nam->namest.symclass = CLFORMAL;
    PlaceNodeBefore(n,Entry->nodechild);
  }
  return &(nam->namest);
}

/*
 * Find/create symbol table entry.
 * new_int : 0 - name should already exist or you'll get error message,
 *       1 - name should not exist or get error message,
 *       2 - create new name even if the entry with this name exists already
 */
symtabentry *parse_symbol(int new_int) {
  nametabentry *nam = parse_findsym();
  nametabentry *onam = nam;
  
  if(NoNewOldCheck) {
    if(nam)
      return &(nam->namest);
  } else {
    if( new_int == 0 ){
      if( nam == NULL ){
	fprintf(stderr, "*** `%s' is undeclared variable ***\n", P_IDname);
	parse_die();
      }
      return &(nam->namest);
    }
    if( new_int == 1 ) {
      if( nam != NULL ){
	fprintf(stderr, "Warning: variable `%s' declared twice, 2nd declaration on line %ld\n", P_IDname, Lines);
	return &(nam->namest);
      }
    }
  }

  return add_symbol(P_IDname, new_int, syminteger, exprrefint, onam);
       /* we'll set the type later */
}/* parse_symbol */

/*
 * create symbol table entry for loop, and for loop step count
 * (the latter is needed so that we can have a var_id for the omega test)
 */

static symtabentry *parse_doid_symbol()
{
    symtabentry *id, *stepsym;
    char loop_step_name[256];
    sprintf(loop_step_name, "<%s-trip>", P_IDname);
    stepsym = add_symbol(loop_step_name, 1, symloopstep, exprrefint, 0);
    id = parse_symbol(2);
    assert(&(id->nte->namenext->namest) == stepsym);
    return id;
}
							      
/*
 * Make node referencing already existing variable
 */
node *parse_symbolnode()
{
node *nod;
symtabentry *sym;

    sym = parse_symbol( 0 );

    if( sym->symtype == symindex ){
	nod = make_node( op_index );
	nod->nodevalue = (unsigned long int)sym->symdecl;
	nod->exptype = exprint;	       /* all index expressions are integer */
    } else if( sym->symtype == symconst ) {
	nod = make_node( op_constant );
	nod->nodevalue = (unsigned long int)sym->symvalue;
	nod->exptype = exprint;	       /* all constants are integers */
    } else if( sym->symdims == 0 ) {   /* determine type of scalar var */
	nod = make_node( op_fetch );
	nod->nodevalue = (unsigned long int)sym->symdecl;
	if( sym->exptype == exprrefint )
	    nod->exptype = exprint;
	else if( sym->exptype == exprrefreal )
	    nod->exptype = exprreal;
	else
	    nod->exptype = exprunknown;
    } else if(sym->symdims == DIMSFUN) {   /* builtin function */
	nod = make_node( op_call );
	nod->nodevalue = (unsigned long int)sym->symdecl;
	if( sym->exptype == exprintfun )
	    nod->exptype = exprint;
	else if( sym->exptype == exprrealfun )
	    nod->exptype = exprreal;
	else
	    nod->exptype = exprunknown;
    } else if( sym->symdims > 0 ) {
	nod = make_node( op_fetch_array );
	nod->nodevalue = (unsigned long int)sym->symdecl;
	if( sym->exptype == exprintarray )
	    nod->exptype = exprint;     /* if int array, then just int */
	else
	if( sym->exptype == exprrealarray )
	    nod->exptype = exprreal;    /* if real array, then just real */
	else
	    nod->exptype = exprunknown;
    } else {
	fprintf(stderr, "unknown symbol %s\n", sym->symname );
	parse_die();
	nod = 0;  // make g++ calm down about initialization
    }
    return nod;
}/* parse_symbolnode */

static void parse_stmt_index( node *n )
{
node *index;

    if( n == NULL ) return;
    parse_stmt_index( n->nodeparent );
    if( n->nodeop == op_dolimit ){
	index = make_node( op_index );
	index->nodevalue = (unsigned long int)n;
	parse_linknode( index );
    }
}/* parse_stmt_index */

static void parse_check_stmt_number()
/* Put an op_stmt_number here. */
{
node *stmt_number;

    if( P_NextParent != NULL && P_NextParent->nodeop == op_dolimit ){
	++P_StmtNumber;
	stmt_number = make_node( op_stmt_number );
	stmt_number->nodevalue = P_StmtNumber;
	parse_linknode( stmt_number );
	parse_pushscope( stmt_number );
	parse_stmt_index( stmt_number->nodeparent );
	parse_popscope();
    }
}/* parse_check_stmt_number */

/* Initialize everything. */
static void parse_init()
{
    Lines = 1;
    P_Lines = 1;
    P_LoopNest = 0;
    P_StmtNumber = 0;

    /* Initialize entry point. */
    Entry = make_node( op_entry );

    /* Initialize node links. */
    P_NextPrev = NULL;
    P_NextParent = Entry;

    NameHead = NULL;
}/* parse_init */

void yyerror( char *s)
{
    fprintf(stderr, "%s\n", s );
    parse_die();
}/* yyerror */

} // end of namespace omega
