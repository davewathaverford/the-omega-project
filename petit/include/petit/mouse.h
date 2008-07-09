/* mouse.h,v 1.1.1.2 1992/07/10 02:40:57 davew Exp */
#ifndef Already_Included_Mouse
#define Already_Included_Mouse

#include <petit/tree.h>

namespace omega {

extern int mouse_in_out;

extern void mouse_refresh_window();

extern void mouse_redo_window();

extern void mouse_close_window( void );

extern ddnode *mouse_nextdd( void );

extern ddnode *mouse_nextlinkdd( ddnode *dd );

extern ddnode *mouse_in_dd( node *n );

extern ddnode *mouse_out_dd( node *n );

extern ddnode *mouse_link_dd( void );

extern ddnode *mouse_loop_dd( void );

extern int mouse_find_loop_dd( node * );

extern int mouse_toggle( int dummy );

}

#endif
