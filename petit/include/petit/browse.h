/* browse.h,v 1.1.1.2 1992/07/10 02:39:31 davew Exp */
#ifndef Already_Included_Browse
#define Already_Included_Browse

#include <petit/tree.h>

namespace omega {

/* go to next interesting node */
extern node *br_go_to_another( node *n );

extern node *Browse_current, *Browse_back, *prev_current;
extern node *Browse_markers[26];
extern int Browse_mode;
extern void *GLOBAL_BROWSE;
extern void *GLOBAL_BROWSEDD;
extern int NORMAL_ENTRY;

/* find the next loop or variable from the current position */
extern int br_find( int which );

extern node *br_find_next( node *n );

extern int br_refresh( int );

/* return the node currently being browsed */
extern node *br_current( void );

/* return the line number of the node currently being browsed */
extern unsigned int br_line( void );

extern void br_move_it( node *new_node );

extern void *build_br_browse( void );
extern int br_browse( int dummy );
extern int br_browse_epilog( int dummy );

}

#endif
