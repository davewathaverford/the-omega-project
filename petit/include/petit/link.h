/* link.h,v 1.1.1.2 1992/07/10 02:40:35 davew Exp */
#ifndef Already_Included_Link
#define Already_Included_Link

namespace omega {

/* initialize link procedure */
extern void link_start( node *prev, node *parent );

/* Link in a new node at this point. */
extern void link_node( node *new_node );

/* unlink node, when it doesn't really belong */
extern void link_unlink( node *n );

/* Push and Pop scopes */
extern void link_pushscope( node *n );

extern void link_popscope( void );

}

#endif
