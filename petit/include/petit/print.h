/* $Id: print.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Print
#define Already_Included_Print

#include <stdio.h>
#include <petit/tree.h>

namespace omega {

void print_petit( node *Start, int follow );

extern void print_expr( node *n );

extern void print_node( node *n );

extern void print_header( node *n, unsigned int nest );

/* print 'petit' to debug file */
extern void debug_petit( node *start, int follownext );

/* print 'petit' to stdout file */
extern void print_petit_out( node *start, int follownext );

/* print 'petit' to named file */
extern void print_petit_file( node *start, int follownext );

/* print 'petit' to printBuf */
extern void print_to_buf( node *start, int follownext, int indent );

extern void print_template_stmt(node *start);

extern void print_template(node *start, node *finish);

extern void print_decls_out();

extern void print_decl_out(node *n);

extern void printit( char * );

extern int do_display(int always);

extern void writepgm(char *filename);

extern int do_writepgm(int dummy);

extern FILE *printout;
extern String printBuf;

extern node *first_printed_node;

/* print_where is now a global variable of type where */
typedef enum{
    print_in_printfile,
    print_in_debugfile,
    print_in_stdout,
    print_in_motif,
    print_in_printBuf} where;
extern where print_where;

} // end of namespace omega

#endif
