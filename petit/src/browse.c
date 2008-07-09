/* browse.c,v 1.1.1.2 1992/07/10 02:42:03 davew Exp */

/* Interactively browse around the data structures */

#include <stdio.h>
#include <omega/Relations.h>
#include <petit/tree.h>
#include <petit/browse.h>
#include <petit/motif.h>
#include <petit/message.h>
#include <petit/browsedd.h>
#include <petit/print.h>
#include <petit/ops.h>
#include <petit/mouse.h>
#include <petit/makeint.h>
#include <petit/ivr-lang-interf.h>
#include <petit/Exit.h>
#include <petit/petit_args.h>

namespace omega {

node* br_go_to_another( node *n )
{
node *nn;

    if( n->nodechild != NULL ) return n->nodechild;
    for( nn = n; nn != NULL; nn = nn->nodeparent ){
	if( nn->nodenext != NULL ) return nn->nodenext;
    }
    return Entry;
}/* br_go_to_another */


#if ! defined BATCH_ONLY_PETIT

void *GLOBAL_BROWSE;
int  Browse_mode;
node *Browse_current, *Browse_back, *prev_current;
node *Browse_markers[26];

int NORMAL_ENTRY = 1;

/* Return the node currently being browsed, if any */
/* wak@cs.umd.edu 2/16/92 */

node *br_current(void)
{
    if ( Browse_mode )
        return Browse_current;
    else
        return NULL;
} /* br_current */

void br_move_it( node *new_node )
{
    if( new_node == NULL ){
	printf( "\007" );
    }else{
        motif_lowlight( prev_current ); 
	if( new_node != Browse_current )
	    {
	    Browse_back = Browse_current;
	    Browse_current = new_node;
	    }
        motif_highlight( Browse_current ); 
	prev_current = Browse_current;
    }
}/* br_move_it */


static node *Browse_Most_Recent = NULL;

int br_refresh( int always )
{
    if( always )
        print_petit( Entry, 1 );

    br_move_it( Browse_current );
    Browse_Most_Recent = Browse_current;
    return 0;
}/* br_refresh */


node *br_find_next( node *n )
{
node *nn;

    if( n == NULL ){
	nn = NULL;
    }else{
	switch( n->nodeop ){
    
	CASE_DO:
	    nn = n->nodechild;
	    if( nn != NULL ) nn = nn->nodechild;
	    break;
    
	case op_dolimit:
	case op_entry:
	    nn = n->nodechild;
	    break;
    
	default:
	    /* go to 'next', as in the following loop */
	    nn = NULL;
	}
    }


    if( nn != NULL && nn->nodeop == op_stmt_number ) nn = nn->nodenext;

    if( nn == NULL ){
        for( nn = n; nn != NULL && nn->nodenext == NULL; nn = nn->nodeparent ) ;
	if( nn != NULL ) nn = nn->nodenext;
        }
    if( nn != NULL ) return nn;
    return Entry;
}/* br_find_next */

int br_move( int where ) {
/* Move Browse_current according to the value of 'where';
   -1 = back (to previous position)
   1 = down
   2 = up
   3 = left
   4 = right
   5 = link (if any)
   6 = to declare node (for op_fetch, op_store )
   7 = 'next' (defined for each op-type)
   8 = go to definition using SSA graph
 */
    node *n;
    static node *v0 = NULL;
    void *wptr;

    n = NULL; /* by default */
    switch( where ){
    case -1: /* back */
	n = Browse_back;
	break;
	
    case 1: /* down */
	n = Browse_current->nodechild;
	break;

    case 2: /* up */
	n = Browse_current->nodeparent;
	break;

    case 3: /* left */
	n = Browse_current->nodeprev;
	break;

    case 4: /* right */
	n = Browse_current->nodenext;
	break;

    case 5: /* link */
	n = Browse_current->nodelink;
	if( n != NULL ) break;
	/* else, go to head of list */

    case 6: /* to declare node */
	switch( Browse_current->nodeop ){
	CASE_MEMREF:
	case op_index:
	case op_call:
	    n = (node*)Browse_current->nodevalue;
	    break;
	default:
	    break;
	}
	break;

    case 7: /* to 'next' */
	n = br_find_next( Browse_current );
	break;

    case 8: /* to scalar definition */
	switch(Browse_current->nodeop) {
	CASE_MEMREF:
	  n = Browse_current->ud.def_node;
	  break;

	case op_phi:
	  n = Browse_current->ud.phi_parent;
	  break;
        default:
	  break;
	}
	break;

    case 9: /* to scalar use */
	v0 = Browse_current;
	n = ssa_sink(v0, &wptr);
	break;

    case 10: /* to next scalar use */
	if(v0==NULL) {
	  n = NULL;
	} else {
	  n = ssa_next_sink(Browse_current, v0, &wptr);
	}
	break;
    }

    /* Check that n is not NULL and set Browse_back appropriately */
    /* wak@cs.umd.edu 2/16/92 */
    if( n == NULL ){
	printf( "\007" );
    }else{
	if( n != Browse_current ){
            Browse_back = Browse_current;
	    mouse_close_window();
	    Browse_current = n;
	}
    }

    return 0;
}/* br_move */


void do_br_mark( char *s )
{
char ch;
unsigned int i;

    ch = s[0];
    if( ch >= 'A' && ch <= 'Z' ) i = ch - 'A';
    else if( ch >= 'a' && ch <= 'z' ) i = ch - 'a';
    else{
	Message_Add( "illegal mark character" );
	printf( "\007" );
	return;
    }
    Browse_markers[i] = Browse_current;
}/* do_br_mark */


int br_mark( int dummy )
/* save current position in a marker */
{
    dummy = dummy; /* dead code */
    motif_getword( "Mark [a-z]: " , do_br_mark);
    return 0;
}/* br_mark */


void do_br_goto( char *s )
{
char ch;
unsigned int i;

    ch = s[0];
    if( ch >= 'A' && ch <= 'Z' ) i = ch - 'A';
    else if( ch >= 'a' && ch <= 'z' ) i = ch - 'a';
    else{
	Message_Add( "illegal mark character" );
	printf( "\007" );
	return;
    }
    if( Browse_markers[i] == NULL ){
	Message_Add( "mark does not exist" );
	printf( "\007" );
	return;
    }
    mouse_close_window();
    Browse_current = Browse_markers[i];
    br_refresh(0);
    return;
}/* br_goto */


int br_goto( int dummy )
/* goto saved position from a marker */
{

    dummy = dummy; /* dead code */
    motif_getword( "Mark [a-z]: ", do_br_goto );
    return 0;
}/* br_goto */


int br_find( int which )
{
node *n;
int more;

    more = 1;
    for( n = br_go_to_another( Browse_current );
	more;
	n = more ? br_go_to_another( n ) : n ){
	switch( n->nodeop ){
	case op_entry:
	    more = 0;
	    break;

	  CASE_DO:
	    if( which == 1 ) more = 0;
	    break;

	  CASE_MEMREF:
	    if( which == 2 ) more = 0;
	    break;

	case op_index:
	    if( which == 4 ) more = 0;
	    break;

	case op_add:
	case op_subtract:
	case op_multiply:
	case op_divide:
	case op_floor_divide:
	case op_ceiling_divide:
	case op_sqrt:
	case op_exp:
	case op_min:
	case op_max:
	case op_mod:
	case op_lt:
	case op_le:
	case op_eq:
	case op_ne:
	case op_ge:
	case op_gt:
	    if( which == 5 ) more = 0;
	    break;

	  CASE_ASSIGN:
	    if( which == 6 ) more = 0;
	    break;

	case op_stmt_number:
	default:
	    break;
	}
    }

    /* Check that n is not NULL and set Browse_back appropriately */
    /* wak@cs.umd.edu 2/16/92 */
    if( n == NULL ){
	printf( "\007" );
    }else{
	if( n != Browse_current ){
            Browse_back = Browse_current;
	    mouse_close_window();
	    Browse_current = n;
        }
    }
    return 0;
}/* br_find */



int br_browse( int )
{
unsigned int i;

    mouse_close_window();

    if (NORMAL_ENTRY)
	{
        Browse_current = Entry;
        Browse_back = Entry;
        prev_current = NULL;
	}
    NORMAL_ENTRY = 1;

    if (Entry != NULL) 
	{
        for( i=0; i<26; ++i )
	    Browse_markers[i] = NULL;
        
        Browse_mode = 1;
	br_refresh(0);
	return 1;
        } 
    else 
        {
	Message_Add( "There is no program to browse" );
	return 0;
	}
}
    

int br_browse_epilog( int )
{
	mouse_close_window();
    
        /* Flag that no nodes should be highlighted from now on */
        /* wak@cs.umd.edu 2/16/92 */
        Browse_mode = 0; 
    
        return 0;
}/* br_browse_epilog */



int unhighlight( int )
{
  motif_lowlight( Browse_current ); 
  return 1;
} /* unhighlight */




void *build_Compile_Menu()
    {
    static struct Menu_struc M1[] = {
        {"Xcape",0,0,   0,1},
        {"Quit", 0,Quit,0,0},
        {0,      0,0,   0,0 }};

    return Build_Menu( M1, br_compile_options, 0 );

    } /* build_Compile_Menu */


void *build_Options_Menu()
    {
    static struct Menu_struc M1[] = {
        {"Xcape",0,0,   0,1},
        {"Quit", 0,Quit,0,0},
        {0,      0,0,   0,0 }};

    return Build_Menu( M1, br_options, 0 );

    } /* build_Options_Menu */


void *build_br_browse()
{

/* browse menu */
static struct Menu_struc M1[] = {
    {"COpts",build_Compile_Menu,  0,                 br_refresh,           1},
    {"DD",   build_brdd_menu,     brdd_menu,         brdd_menu_epilog,     1},
    {"Loop", 0,                   br_find,           0,                    1},
    {"Optns",build_Options_Menu,  0,                 br_refresh,           1},
    {"Msgs" ,build_Message_Menu,  0,                 br_refresh,           1},
    {"Xcape",0,                   unhighlight,       0,                    0},
    {0,	    0,                   0,                 0,                    0}};

    GLOBAL_BROWSE = Build_Menu(M1, br_refresh, 0);
    return GLOBAL_BROWSE;
} /* build_br_browse */

#endif

} // end of namespace omega
