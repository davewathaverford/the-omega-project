/* print.c,v 1.1.1.2 1992/07/10 02:43:40 davew Exp */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <basic/bool.h>
#include <basic/assert.h>
#include <petit/Exit.h>
#include <petit/motif.h>
#include <petit/tree.h>
#include <petit/ops.h>
#include <petit/print.h>
#include <petit/debug.h>
#include <petit/language.h> /* languagetype definition (ms) */
#include <petit/makeint.h>  /* added these to get to program name (ms) */
#include <petit/affine.h>
#include <petit/ssa.h>
#include <petit/petit_args.h>
#include <petit/message.h>

namespace omega
{

String printBuf;
static char S[PETITBUFSIZ];
static int printed;
static int global_call = 0;

static int print_header_line;
static unsigned int continue_label;   /* gen by print_do, used by print_save_pos (ms) */
static int template_flag = 0;
FILE *printout;

/* used to link the browsable nodes together via the noderight field */
/* wak@cs.umd.edu 2/26/92 */
node *previous_node;

/* print_where is now a global variable of type "where" defined in print.h */
/* wak@cs.umd.edu 2/16/92 */
where print_where;


void printit( char *s )
{
    switch( print_where ){
    case print_in_printfile :
	fprintf( printout, s );
	break;

    case print_in_debugfile :
	fprintf( debug, s );
	break;

    case print_in_printBuf:
    case print_in_motif:
	{
	printBuf += s;
	printed += strlen(s);
	break;
	};
	

    default :
	printf( s );
    }
}/* printit */

void newline(void)
{
    switch( print_where ){
    case print_in_printfile :
	fprintf( printout, "\n" );
	break;

    case print_in_debugfile :
	fprintf( debug, "\n" );
	break;

    case print_in_printBuf:
    case print_in_motif:
	{
	printBuf += "\n";
	printed++;
	break;
	};
    default :
	printf( "\n" );
    }
}/* newline */

node *first_printed_node;

void print_save_position( node *n )
{
    /* link the browsable nodes via the noderight field */
    /* wak@cs.umd.edu 2/26/92 */
    if( print_where == print_in_motif)
        {
        n->nodeposition = printed;
	n->noderight = NULL;
	if (previous_node != NULL)
            previous_node->noderight = n;
        else
	    first_printed_node = n;
        previous_node = n;
	}

}/* print_save_position */

/* print a constant */
void print_const( node *n )
{
    sprintf( S, "%ld", n->nodevalue );
    printit( S );
}/* print_const */

void print_name( node *n )
{
symtabentry *sym;

    sym = (symtabentry*)n->nodevalue;
    printit( sym->symname );
}/* print_name */


void print_node( node *n ) {
/* Just print the single node at 'n' */
    node *nn;
    char *op = 0;

    switch( n->nodeop ){
    case op_dovec:
	if( language == fortran_alliant ){
	    /* print the directive in col 1, 'vector' in nested col (ms) */
	    sprintf( S, "cvd$%*s vector", (int)(n->nodevalue+1), " " );
	    printit( S );
	    newline();
	    sprintf( S, "%*s ", (int)(n->nodevalue+5), " " );
	    printit( S );
	}
	break;

    case op_dopar:
	if( language == fortran_alliant ){
	    /* print the directive in col 1, 'vector' in nested col (ms) */
	    sprintf( S, "cvd$%*s concur", (int)(n->nodevalue+1), " " );
	    printit( S );
	    newline();
	    sprintf( S, "%*s ", (int)(n->nodevalue+5), " " );
	    printit( S );
	}
	break;
    default:
	break;
    }

    switch(language) {
    case petitlang :
    case clang :
	op = get_op_petit_string( n->nodeop );
	break;
    case fortran_alliant :
	op = get_op_ftn_string( n->nodeop );
	break;
    }

    /* can not be done in op.tbl with present conversion rouitines */
    switch(n->nodeop) {
    case op_mod:  op = " % ";  break;
    default: break;
    }

    if (!op)
      {
	printit(" (null operation detected in print.c) ");
      }
    else
      {
	if( op[0] == '\0' ) {
	  switch( get_op_valtype( n->nodeop ) ){
	  case Osymbol :
	    print_name( n );
	    break;

	  case Onodesym :
	    if (template_flag && n->nodeop == op_index)
	      {
		char s[10];
		sprintf(s,"#%ld",((node*)n->nodevalue)->nodeparent->nodevalue);
		printit(s);
	      }
            else
	      {
	        nn = (node*)n->nodevalue;
	        print_name( nn );
	      }
	    break;

	  case Ovalue :
	    print_const( n );
	    break;
	  }
	} else {
	  printit( op );
	}
      }

    switch( n->nodeop ){
    case op_entry:
	/* print 'program' name (ms) */
	if ( language == fortran_alliant ) {
	    printit( " " );
	    printit(petit_args.FileName);
	}
	break;
    default:
	break;
    }
}/* print_node */

/* print_expr and print_var print a whole expression tree, */
/* saving the positions of each node in 'wposition' */

void print_var( node *n )
/* print a variable */
{
int call = global_call;
global_call = 0;
node *nn;
char ch[2];

    print_node( n );
    if( n->nodechild != NULL ){
	if (language == clang && ! call)
	    ch[0] = '[';
	else
	    ch[0] = '(';
	ch[1] = 0;
	for (nn = n->nodechild; nn != NULL; nn = nn->nodenext){
	    printit( ch );
	    print_expr( nn );
	    if (language == clang && ! call)
		{
		printit("]");
	        ch[0] = '[';
		}
	    else
	        ch[0] = ',';
	}
	if (language != clang || call)
	    printit( ")" );
    }
}/* print_var */

void print_with_precedence( node *nodechild, node *n, unsigned int x )
{
    if( get_op_precedence(nodechild->nodeop) <
	(get_op_precedence(n->nodeop)+x) ){
	printit( "(" );
	print_expr( nodechild );
	printit( ")" );
    }else{
	print_expr( nodechild );
    }
}/* print_with_precedence */

/* print an expression */
void print_expr( node *n )
{
node *lchild, *rchild;
char ch[2];

    switch( n->nodeop ){

    CASE_SCALAR_REF:
        print_save_position( n );
	printit(get_nodevalue_sym(get_nodevalue_node(n))->symname);
	if(petit_args.printPhis) {
	  if(n->nodeop == op_update) {
	    node *def_node = n->ud.def_node;
	    n->ud.def_node = n->nodeparent;
	    printit(str_var_index(n));
	    printit(",");
	    n->ud.def_node = def_node;
	  }
	  printit(str_var_index(n));
	}
	break;

    CASE_ARRAY_REF:
    case op_call:
	print_save_position( n );
	global_call = (n->nodeop == op_call);
	print_var( n );
	break;
    case op_rand:
	printit("rand()");
	break;

    case op_phi:
    case op_rtoi:
	{ node *p;
	  print_save_position( n );
	  print_node( n );
	  printit( "(" );
	  for(p=n->nodechild; p; p=p->nodenext) {
	    print_expr( p );
	    if(p->nodenext) printit( "," );
	  }
	  printit( ")" );
	}
	break;

    case op_subtract:
	/* special case: don't print zero for 0-expr */
	lchild = n->nodechild;
	rchild = lchild->nodenext;
	if( lchild->nodeop != op_constant || lchild->nodevalue != 0 ){
	    print_with_precedence( lchild, n, 0 );
	}
	print_save_position( n );
	print_node( n );
	print_with_precedence( rchild, n, 1 );
	break;

    case op_not:
        lchild = n->nodechild;
        print_save_position( n );
        printit (" not ");
        print_with_precedence(lchild, n, 1);
        break;
    case op_and:
    case op_or:
        lchild = n->nodechild;
        rchild = lchild->nodenext;
        print_with_precedence (lchild, n, 0);
        printit(" ");
        print_save_position( n );
        print_node( n ); 
        printit(" "); 
        print_with_precedence(rchild, n, 1);
        break;   
    case op_add:
    case op_multiply:
    case op_divide:
    case op_mod:
    case op_lt:
    case op_le:
    case op_eq:
    case op_ne:
    case op_gt:
    case op_ge:
	lchild = n->nodechild;
	rchild = lchild->nodenext;
	print_with_precedence( lchild, n, 0 );
	print_save_position( n );
	print_node( n );
	print_with_precedence( rchild, n, 1 );
	break;

    case op_exp:
	lchild = n->nodechild;
	rchild = lchild->nodenext;
	print_with_precedence( lchild, n, 1 );
	print_save_position( n );
	print_node( n );
	print_with_precedence( rchild, n, 0 );
	break;

    case op_floor_divide:
	printit( "floor(" );
	lchild = n->nodechild;
	rchild = lchild->nodenext;
	print_with_precedence( lchild, n, 0 );
	print_save_position( n );
	print_node( n );
	print_with_precedence( rchild, n, 1 );
	printit( ")" );
	break;

    case op_ceiling_divide:
	printit( "ceiling(" );
	lchild = n->nodechild;
	rchild = lchild->nodenext;
	print_with_precedence( lchild, n, 0 );
	print_save_position( n );
	print_node( n );
	print_with_precedence( rchild, n, 1 );
	printit( ")" );
	break;

    case op_sqrt:
	print_save_position( n );
	print_node( n );
	printit( "(" );
	print_expr( n->nodechild );
	printit( ")" );
	break;

    case op_max:
    case op_min:
	print_save_position( n );
	print_node( n );
	ch[0] = '(';
	ch[1] = 0;
	for( lchild = n->nodechild; lchild != NULL; lchild = lchild->nodenext ){
	    printit( ch );
	    print_expr( lchild );
	    ch[0] = ',';
	}
	printit( ")" );
	break;

    case op_stmt_number:
	print_save_position( n );
	break;

    default:
	print_save_position( n );
	print_node( n );
	break;
    }
    if(petit_args.printShadow && n->shadow) {
        printit("{");
	print_expr(n->shadow);
        printit("}");
    }
    if(petit_args.printAffine && n->nodeaffine && n->nodeaffine!=&not_affine) {
        affine_expr *na = n->nodeaffine;
	int i;
	char ss[80];
        printit("[");
	sprintf(ss, "%d", na->terms[0].coefficient);
	printit(ss);
	for(i=1; i<na->nterms; i++) {
	  affine_term *at = &na->terms[i];
	  sprintf(ss, "%+d*%s", at->coefficient, at->petit_var->symname);
	  printit(ss);
	}
        printit("]");
    }
}/* print_expr */

/* print a do-loop statement */
void print_do( node *n )
{
node *nn;
char ch[2];

    print_expr( n );    /* print appropriate 'do' */

    if( language == petitlang ){
	printit( " " );
    }else{
	continue_label += 10; /* generate a new label (ms) */
	sprintf( S, " %d ", continue_label );
	printit( S );
    }

    nn = n->nodechild;

    print_expr( nn );

    printit( " =" );
    ch[0] = ' ';
    ch[1] = 0;
    for( nn = nn->nodenext; nn != NULL; nn = nn->nodenext ) {
	printit( ch );
        ch[0] = ',';
	print_expr( nn );
        }
    
    if( language == petitlang ){
	printit( " do" );
    }
}/* print_do */

/* print an IF statement */
void print_if( node *n )
{
node *nn;

    print_expr( n );

    printit( " ( " );
    nn = n->nodechild;
    print_expr( nn );
    printit( " ) then" );
}/* print_if */


/* print a declaration statement */
void print_declare(node *n, unsigned int nest, int solo) {
  node *nn;
  char ch[2];
  symtabentry *sym, *psym;
  
  sym = (symtabentry*)n->nodevalue;

  if(solo || !(n->nodeprev && n->nodeprev->nodeop==op_declare &&
       n->nodeprev->nodesequence==n->nodesequence &&
       (psym=get_nodevalue_sym(n->nodeprev))->symtype==sym->symtype &&
       psym->symclass==sym->symclass)) {
    print_header( n, nest );

    if (language == petitlang) {
      switch(sym->symclass) {	   
      case CLunknown:                      break;
      case CLAUTO:    printit("auto ");    break;
      case CLCOMMON:  printit("common ");  break;
      case CLSTATIC:  printit("static ");  break;
      case CLFORMAL:  printit("formal ");  break;
      case CLPRIVATE: printit("private "); break;
      case CLIN:      printit("in ");      break;
      case CLOUT:     printit("out ");     break;
      case CLINOUT:   printit("inout ");   break;
      case CLBUILTIN: printit("builtin "); break;
      default:        ErrAssert("print_declare: illegal Petit class");
      }
    } else if(language == fortran_alliant) {
      switch(sym->symclass) {	   
      case CLunknown:                         break;
      case CLAUTO:       		      break;
      case CLCOMMON:  printit("common // ");  break;
      case CLSTATIC:  printit("save ");       break;
      case CLFORMAL:                          break;
      case CLBUILTIN: printit("intrinsic ");  break;
      case CLIN:      printit("in ");         break;
      case CLOUT:     printit("out ");        break;
      case CLINOUT:   printit("common // ");  break;
      case CLPRIVATE: printit("private ");    break;
      default:        ErrAssert("print_declare: illegal Fortran class");
      }
    } else if (language == clang) {
      switch(sym->symclass) {	   
      case CLunknown:                         break;
      case CLAUTO:       		      break;
      case CLCOMMON:                          break;
      case CLSTATIC:                          break;
      case CLFORMAL:                          break;
      case CLBUILTIN:                         break;
      case CLIN:                              break;
      case CLOUT:                             break;
      case CLINOUT:                           break;
      case CLPRIVATE:                         break;
      default:        ErrAssert("print_declare: illegal C class");
      }
    }
      
    if( sym->symtype == syminteger ){
      if (language == clang)
	printit( "int ");
      else
        printit( "integer " );
    }else if( sym->symtype == symreal ){
      if (language == clang)
        printit( "double " );
      else
        printit( "real " );
    }else if ( sym->symtype == symconst ){
      if( language == petitlang )
	printit( "const " );
      else if (language == fortran_alliant)
	printit( "parameter ( " );
      else
	printit( "int " );
    }else{
      sprintf( S, "type%d ", (int)sym->symtype );
      printit( S );
    }
  }
  
  print_expr( n );
  
  if( sym->symtype == symconst ){
    if ( language == fortran_alliant )
      sprintf( S, " = %ld )", sym->symvalue );
    else
      sprintf( S, " = %ld", sym->symvalue );
    printit( S );
  } else if(sym->symclass==CLBUILTIN ||
	    (n->nodechild==NULL && sym->symdims==1)) {
    printit("()");
  }else{
    nn = n->nodechild;
    if( nn != NULL ){
      if (language == clang)
	  ch[0] = '[';
      else
          ch[0] = '(';
      ch[1] = 0;
	for (nn = n->nodechild; nn != NULL; nn = nn->nodenext){
	printit( ch );

	if (language != clang)
	    {
	    print_expr( nn->nodechild );
	    print_expr( nn );
	    }
	print_expr( nn->nodechild->nodenext );

	if (language == clang)
	    {
	    printit("+1]");
	    ch[0] = '[';
	    }
        else
	    ch[0] = ',';
      }
      if (language != clang)
        printit( ")" );
    }
  }
}/* print_declare */


static void print_declare_end(node *n)
{
  symtabentry *sym, *psym;
  sym = (symtabentry*)n->nodevalue;
  if(!(n->nodenext && n->nodenext->nodeop==op_declare &&
       n->nodenext->nodesequence==n->nodesequence &&
       (psym=get_nodevalue_sym(n->nodenext))->symtype==sym->symtype &&
       psym->symclass==sym->symclass)) {
    if (language == clang)
      printit(";");
    newline();
  } else {
    printit(", ");
  }
} /* print_declare_end */


void print_decls_out( )
{
print_where = print_in_printBuf;
language = clang;
printBuf = "";
printed = 0;
for (node *n = Entry->nodechild; n!= NULL; n=n->nodenext)
    if (n->nodeop == op_declare)
        {
	print_declare(n, 0, 0);
        print_declare_end(n);
        }
}


void print_decl_out(node *n)
{
print_where = print_in_printBuf;
language = clang;
printBuf = "";
printed = 0;
assert(n->nodeop == op_declare);
print_declare(n, 0, 1);
}


/* print an assignment statement */
void print_assignment( node *n )
{
    print_expr( n->nodechild->nodenext );
    printit( " " );
    print_expr( n );
    printit( " " );
    print_expr( n->nodechild );
}/* print_assignment */

#define NEST_STEP 2

void print_header(node *n, unsigned int nest) {
  if (language == clang)
    return;
  if (language == petitlang) {
    if(print_header_line==2)
      sprintf(S, "%*s",
	      (int)(NEST_STEP*(nest-1)), "");
    else
      sprintf(S, "%3d:%*s", (int)n->nodesequence,
	      (int)(NEST_STEP*(nest-1)+1), " ");
  } else {
    sprintf(S, "      %*s", (int)(NEST_STEP*(nest-1)), "");
  }

  printit(S);
}/* print_header */


void print_comment( node *n )
{
symtabentry *sym;

    sym = (symtabentry*)n->nodevalue;
    switch ( language ){
    case petitlang:
	sprintf(S,"!%s",sym->symname);
	printit( S );
	break;
    case fortran_alliant:
	sprintf(S,"C       %s",sym->symname);
	printit( S );
	break;
    default:
	assert(0);
    }
}/* print_comment */

/* print a label number (continue) in the margin (ms) */
void print_label( unsigned int nest, unsigned int label )
{
    sprintf( S, "%5d %*s", label, (int)nest, " " );
    printit( S );
}/* print_label */

/* decide whether or not to print the header based on the node (ms) */
int print_the_header( node *n )
{
    switch ( language ){
    case petitlang:
	return ( print_header_line &&
		n->nodeop != op_stmt_number &&
		n->nodeop != op_exit &&
		!(!petit_args.printPhis && n->nodeop==op_assign &&
		  n->nodechild->nodeop==op_phi) &&
		n->nodeop != op_declare);
    case fortran_alliant:
	return ( print_header_line &&
		 n->nodeop != op_comment &&
		 n->nodeop != op_stmt_number &&
		 n->nodeop != op_exit &&
		!(!petit_args.printPhis && n->nodeop==op_assign &&
		  n->nodechild->nodeop==op_phi) &&
		 n->nodeop != op_declare &&
		 n->nodeop != op_dovec &&
		 n->nodeop != op_dopar );
    case clang:
	return 0;
    }
    return 0; /* dead code? */
}/* print_the_header */

static bool need_to_skip;

/* print petit program  or only a subtree*/
void print_pgm( node *start, int follownext, unsigned int nest )
{
node *n, *t, *e, *nextn;
unsigned int cont_label; /* recursively save generated 'continue' labels (ms) */

    for( n = start; n != NULL; n = nextn ){
	nextn = (node *) (follownext ? n->nodenext : NULL);
	if( print_the_header(n)) {
	  if(n->nodeop!=op_comment) {
	    print_header(n, nest);
	  } else {
	    print_header(n, 1);
	  }
	}

	need_to_skip = false;

	switch( n->nodeop ){
	case op_entry:
	    print_expr(n);
	    if(print_header_line) {
		newline();
	    }
	    nextn = (node *) (follownext ? n->nodechild : NULL);
	    break;

	case op_exit:
	    if(language==petitlang && print_header_line!=2) {
	      if(print_header_line) 
		print_header(n, nest);
	      print_expr(n);
	      if(print_header_line) 
		newline();
	    }
	    break;
	    
	CASE_DO:
	    print_do( n );
	    cont_label = continue_label; /* save generated label (ms) */
	    newline();
	    print_pgm( n->nodechild->nodechild, 1, nest+1 );

	    if ( language == petitlang ) {
		if( print_header_line ) print_header( n, nest );
		printit( "endfor" );
	    }else {
		print_label( nest, cont_label );
		printit( "continue" );
	    }
	    newline();
	    break;

	case op_if:
	    print_if( n );
	    newline();
	    t = n->nodechild->nodenext;
	    print_pgm( t->nodechild, 1, nest+1 );
	    e = t->nodenext;
	    if( e != NULL ){
		if( print_header_line ) print_header( n, nest );
		printit( "else" );
		newline();
		print_pgm( e->nodechild, 1, nest+1 );
	    }
	    if( print_header_line ) print_header( n, nest );
	    if ( language == petitlang )
		printit( "endif" );
	    else
		printit( "end if" );
	    newline();
	    break;

	case op_assert:
	    print_save_position( n );
	    printit("assert(");
	    print_expr(n->nodechild);
	    printit(")");
	    newline();
	    break;

	case op_declare:
	    print_declare(n, nest, 0);
            print_declare_end(n);
	    break;

	CASE_ASSIGN:
	    /* don't print assignments with phi-function */
	    if(petit_args.printPhis || n->nodechild->nodeop!=op_phi) {
	      print_assignment( n );
	      if (language == clang) 
	          printit(";");
	      else
	          newline();
	    }
	    break;

	case op_stmt_number:
	    break;

	case op_comment:
	    print_comment( n );
	    newline();
	    break;

	default:
	    print_expr( n );
	    if( nextn != NULL ) printit( ";" );
	    break;
	}
    }
}/* print_pgm */


void print_to_string( node *start, int follownext, unsigned int /*indent*/ )
{
    printBuf = "";
    printed = 0;

    continue_label = 0;   /* force first 'continue' label to be 10 (ms) */
    previous_node = NULL;
    print_pgm( start, follownext, 1 );

    if ( language == fortran_alliant && follownext ) {
	print_header( NULL, 1 );
	printit( "end" );
	newline();
	}
} /* print_to_string */


void motif_petit( node *start, int follownext)
{
  if(!petit_args.motif_display) return;

  where old_where;

  old_where = print_where;

  print_where = print_in_motif;
  print_header_line = 1;

  print_to_string(start, follownext, 1);
#if ! defined BATCH_ONLY_PETIT
  motif_display((char *)(const char *)printBuf);
#endif

  print_where = old_where;
} /* motif_petit */





/* Added a parameter top which is the number of the line to appear at the top */
/* wak@cs.umd.edu 2/16/92 */
void print_petit( node *start, int follownext)
{
    motif_petit(start, follownext);
}/* print_petit */





/* print petit program to a file */
/* file 'printout' must already be open */
void print_petit_file( node *start, int follownext )
{
where old_where;

    old_where = print_where;
    print_where = print_in_printfile;
    if ( language == petitlang )
	print_header_line = 2;
    else
	print_header_line = 1;

    continue_label = 0;   /* force first 'continue' label to be 10 (ms) */
    previous_node = NULL;
    print_pgm( start, follownext, 1 );

    if ( language == fortran_alliant && follownext ) {
	print_header( NULL, 1 );
	printit( "end" );
	newline();
    }
    print_where = old_where;
}/* print_petit_file */

/* print petit program to a printBuf */
void print_to_buf( node *start, int follownext, int indent )
{
where old_where;

    old_where = print_where;
    print_where = print_in_printBuf;

    if ( language == petitlang )
	{
	if (indent)
	    print_header_line = 2;
	else
	    print_header_line = 0;
	}
    else
	print_header_line = 1;

    print_to_string(start, follownext, indent);

    print_where = old_where;
}/* print_to_buf */

void print_template_stmt(node *start)
{
    template_flag = 1;
    language = clang;
    print_to_buf(start, 0, 0);
    template_flag = 0;
} /* print_template_stmt */


/* print petit program to stdout */
void print_petit_out( node *start, int follownext )
{
where old_where;

    old_where = print_where;
    print_where = print_in_stdout;
    if ( language == petitlang )
	print_header_line = 0;
    else
	print_header_line = 1;

    continue_label = 0;   /* force first 'continue' label to be 10 (ms) */
    previous_node = NULL;
    print_pgm( start, follownext, 1 );

    if ( language == fortran_alliant ) {
	print_header( NULL, 1 );
	printit( "end" );
	newline();
    }
    print_where = old_where;
}/* print_petit_out */

/* print petit program to debug */
void debug_petit( node *start, int follownext )
{
where old_where;

    old_where = print_where;
    print_where = print_in_debugfile;
    if ( language == petitlang )
	print_header_line = 0;
    else
	print_header_line = 1;

    continue_label = 0;   /* force first 'continue' label to be 10 (ms) */
    previous_node = NULL;
    print_pgm( start, follownext, 1 );

    if ( language == fortran_alliant ) {
	print_header( NULL, 1 );
	printit( "end" );
	newline();
    }
    print_where = old_where;
}/* debug_petit */



/* call print_petit and message display */
int do_display( int )
    {
    print_petit( Entry, 1 );
    return 0;
    }/* do_display */




void writepgm( char *filename )
    {
    char line[PETITBUFSIZ];
    if(filename[0] != '\0'  && Entry) 
        {
        printout = fopen( filename, "w" );
        if (! printout)
            {
            sprintf(line, "Can't write file %s", filename);
            Message_Add(line);
            return;
            }
        print_petit_file( Entry->nodechild, 1 );
        fclose( printout );
        }
    } /* writepgm */


#if ! defined BATCH_ONLY_PETIT
/*
 * print the program to a file.
 * edit the file.
 * parse the file.
 */
int do_writepgm( int  )
    {
    motif_getfile(" Output File: ", writepgm, "*" );
    return 0;
    }/* do_writepgm */
#endif

} // end namespace omega

