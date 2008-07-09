/* compare.c,v 1.1.1.2 1992/07/10 02:42:11 davew Exp */

#include <basic/bool.h>
#include <petit/tree.h>
#include <petit/ops.h>
#include <petit/compare.h>

namespace omega {

bool compare_trees( node *a, node *b, int nxt )
{
    if( a == NULL && b == NULL ) return 1;
    if( a == NULL || b == NULL ){
	return 0;
    }
    if( a->nodeop != b->nodeop ){
	return 0;
    }

    switch( a->nodeop ){
    /* compare 'value' field */
    CASE_DO:
    case op_dolimit :
    case op_declare :
    case op_constant :
    case op_real :
    case op_index :
    CASE_MEMREF:
    case op_call:
	if( a->nodevalue != b->nodevalue ){
	    return 0;
	}
    default:
	break;
    }

    if( !compare_trees( a->nodechild, b->nodechild, 1 ) ){
	return 0;
    }
    if( nxt && !compare_trees( a->nodenext, b->nodenext, 1 ) ){
	return 0;
    }

    return 1;
}/* compare_trees */

bool compare_equal( node *a, node *b )
{
    if( compare_trees( a, b, 0 ) ){
	return 1;
    }else{
	return 0;
    }
}/* compare_equal */

} // end omega namespace
