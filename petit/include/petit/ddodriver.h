/* $Id: ddodriver.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_DDOdriver
#define Already_Included_DDOdriver

#include <petit/tree.h>

namespace omega {

/*
   Driver routine for dependence testing with the omega test.

   A different driver is used for 2 reasons:
    - we use coverage information to decide whether or not to check for
      a flow dependence
    - we don't want the funky values for the loop nesting
 */

extern void initializeContainingLoops(node *loop, node *n);

extern void build_smaller_dd_graph();

extern int InDecl(node *n);

/* find # of loops containing n */
extern int node_depth(node *n);

extern int InDecl(node *n);

extern void calculate_depnum();

}

#endif
