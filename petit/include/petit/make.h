/* make.h,v 1.1.1.2 1992/07/10 02:40:42 davew Exp */
#ifndef Already_Included_Make
#define Already_Included_Make

#include <basic/bool.h>

namespace omega {


/* various utility functions for making data structures */

extern node *make_node( optype op );

extern void free_one_node( node *n );

extern void free_node( node *n );

extern node *make_const( int c );

extern node *make_op( node *l, optype op, node *r );

extern node *make_index( node *i );

extern void make_save( node *n );

extern void make_replace( node *n );

extern void make_switch_node( node *a, node *b );

extern void replace_index( node *n, bool follow, node *old, node *new_node );

extern void init_extra( unsigned int extra );

extern void init_extra2( unsigned int extra2 );

extern void init_extras( unsigned int extra, unsigned int extra2 );

extern bool test_inloop( node *n, node *loop );

extern bool test_tightly_nested( node *outer, node *inner );

}

#endif
