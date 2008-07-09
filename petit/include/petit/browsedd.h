/* browsedd.h,v 1.1.1.2 1992/07/10 02:39:33 davew Exp */
#ifndef Already_Defined_BrowseDD
#define Already_Defined_BrowseDD

#include <basic/bool.h>


#define GRAPH_FLOW_COLOR "lightred"
#define GRAPH_ANTI_COLOR "lightblue"
#define GRAPH_OUTPUT_COLOR "darkcyan"
#define GRAPH_REDUCE_COLOR "gold"
#define GRAPH_FLOW_CLASS 1
#define GRAPH_ANTI_CLASS 2
#define GRAPH_OUTPUT_CLASS 3
#define GRAPH_REDUCE_CLASS 4

namespace omega {

extern int BRDD_mode;
extern int BRDD_show;
extern ddnode *BRDD_Current, *BRDD_link_start, *BRDD_Tagged;



extern char *brdd_print(ddnode *dd);

extern int brdd_write(int);

extern int do_graph(int dummy);

extern int brdd_graph(char *filename, char *inputfile, bool popup_vcg, 
		     bool graph_by_statement);

extern void write_deps(char *filename);

extern void write_deps(char *filename, char *inputfile, 
		       bool write_as_graph, bool graph_by_statement);

extern char *brdd_tally(void);

extern void *build_brdd_menu( void );

extern int brdd_menu( int dummy );

extern int brdd_menu_epilog( int dummy );

extern void brdd_start( node *start );

extern void brdd_display( void );

/* returns the current dd link */
extern node *brdd_current( void );

/* returns the line number of the current dd link */
extern unsigned int brdd_line( void );

extern void brdd_move(node* source, ddnode* dest);

}

#endif
