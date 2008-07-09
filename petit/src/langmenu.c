/* langmenu.c,v 1.1.1.2 1992/07/10 02:43:14 davew Exp */

#if ! defined BATCH_ONLY_PETIT

#include <stdio.h>
#include <petit/motif.h>
#include <petit/dddriver.h>
#include <petit/ddutil.h>
#include <petit/language.h>
#include <petit/langmenu.h>
#include <petit/message.h>
#include <petit/print.h>
#include <petit/tree.h>
#include <petit/Exit.h>

namespace omega {

void *build_do_translate()
    {
    /* translate (to fortran) menu. */
    static struct Menu_struc M4[] = {
        {"Fortran",0,0,   0,1},
        {"Petit",	  0,0,   0,3},
        {"Quit",	  0,Quit,0,0},
        {"Xcape",  0,0,   0,0},
        {0,	  0,0,   0,0 }};

    return Build_Menu( M4, 0, 1 );
    } /* build_do_translate */


/* menu for translation from petit -> fortran */
int do_translate_epilog( int x )
{
     if( x <= 0 ) return x;

     switch( x ){
     case 1: /* print alliant fortran program */
       language = fortran_alliant;
       break;

     case 3: /* save program in current format */
       language = petitlang;
       break;
     }

     /* wak@cs.umd.edu 2/16/92 */
     print_petit( Entry, 1 );
     return 0;
}/* do_translate_epilog */

} // end omega namespace
#endif
