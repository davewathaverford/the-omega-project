/* ddnest.c,v 1.1.1.2 1992/07/10 02:42:25 davew Exp */

#include <petit/tree.h>
#include <petit/ops.h>
#include <petit/ddutil.h>
#include <petit/ddnest.h>

namespace omega {

/* get nest level of common loops surrounding the two references. */

void dd_get_nests( node *onode, node *inode, unsigned int *onest, unsigned int *inest, unsigned int *bnest )
{
    node *oparent, *iparent;

    /* find nearest enclosing loop of each node by searching for the dolimit */
    *onest = 0;
    for( oparent = onode->nodeparent; 
	oparent != NULL && oparent->nodeop != op_dolimit; 
	oparent = oparent->nodeparent ){ }
    if( oparent != NULL ){
	oparent = oparent->nodeparent;
	*onest = oparent->nodevalue;
    }
    *inest = 0;
    for( iparent = inode->nodeparent; 
	iparent != NULL && iparent->nodeop != op_dolimit; 
	iparent = iparent->nodeparent ){ }
    if( iparent != NULL ){
	iparent = iparent->nodeparent;
	*inest = iparent->nodevalue;
    }

    /* 'oparent' and 'iparent' now point to op_do or are null. */

    if( oparent == NULL || iparent == NULL ){
	*bnest = 0;
    }else{
	while( oparent != iparent ){
	    if( oparent->nodevalue > iparent->nodevalue ){
		/* pop out oparent once more */
		for( oparent = oparent->nodeparent; 
		    oparent != NULL && oparent->nodeop >= op_dolimit; 
		    oparent = oparent->nodeparent ){ }
		if( oparent == NULL ) iparent = NULL;
	    }else{
		for( iparent = iparent->nodeparent; 
		    iparent != NULL && iparent->nodeop >= op_dolimit; 
		    iparent = iparent->nodeparent ){ }
		if( iparent == NULL ) oparent = NULL;
	    }
	}
	if( oparent != NULL ) *bnest = oparent->nodevalue;
	else *bnest = 0;
    }

    /* Have nest level of the common loop, and each reference. 
       To simplify matters (really!), temporarily increment the nest
       level of the loops that surround 'inode' but not 'onode'
       so that we have the following loops:
         1..bnest -> surrounds both onode and inode
         bnest+1..onest -> surrounds only onode
         onest+1..inest -> surrounds only inode.
       The amount by which to increment the nest level of these
       loops is 'onest-bnest'. */
    if( *onest > *bnest && *inest > *bnest ){
	for( iparent = inode->nodeparent;
	    iparent != NULL;
	    iparent = iparent->nodeparent ){
	    switch( iparent->nodeop ){
	    CASE_DO:
		if( iparent->nodevalue > *bnest ){
		    iparent->nodevalue += (*onest-*bnest);
		}else{
		    iparent = Entry; /* break out of loop early */
		}
		break;

	    default:
		break;
	    }
	}
    }
}/* dd_get_nests */

void dd_fix_nests( node *onode, node *inode, unsigned int onest, unsigned int inest, unsigned int bnest )
{
node *iparent;
    onode = onode; /* dead code */
    /* replace do nests the way they were */
    if( onest > bnest && inest > bnest ){
	for( iparent = inode->nodeparent;
	    iparent != NULL;
	    iparent = iparent->nodeparent ){
	    switch( iparent->nodeop ){
	    CASE_DO:
		if( iparent->nodevalue > bnest ){
		    iparent->nodevalue -= (onest-bnest);
		}else{
		    iparent = Entry; /* break out of loop early */
		}
		break;

	    default:
		break;
	    }
	}
    }
}/* dd_fix_nests */

} // end omega namespace
