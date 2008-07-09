/* $Id: linkio.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#include <stdio.h>

#include <basic/assert.h>
#include <petit/tree.h>
#include <petit/ops.h>
#include <petit/linkio.h>

namespace omega {
/* traverse tree looking for fetch and store nodes and link them together */


void linkio_node( node *n )
{
    node *nn, *nd;

    for( nn=n; nn != NULL; nn = nn->nodenext ){
	assert(nn->nodeop != op_free);
	switch( nn->nodeop ){
	case op_assign:
	    linkio_node( nn->nodechild );
            break;

	CASE_MEMREF:
	case op_call:
	    nd = (node*)nn->nodevalue;
	    nn->nodelink = nd->nodelink;
	    nd->nodelink = nn;
	    linkio_node( nn->nodechild );
	    break;

	case op_dolimit:
	    /* do bounds before stuff in loop body */
	    /* added 3/20/92 davew@cs.umd.edu */
	    /* because coverage test requires that we do a */
	    /* bottom-to-top traversal of the uses of the variable */
	    linkio_node( nn->nodenext );
	    /* above call does lower bound, upper bound, step */
	    linkio_node( nn->nodechild );
	    return;

	default:
	    linkio_node( nn->nodechild );
	}
	/* may not reach this point any more */
    }
    /* may not reach this point any more */
}/* linkio_node */


/* look for fetch and store nodes */
/* link them up */

void link_lists(void)
{
node *nd;
nametabentry *n;

    for( n = NameHead; n != NULL; n = n->namenext ){
	nd = n->namest.symdecl;
	assert(nd == NULL || nd->nodeop != op_free);
	if( nd != NULL ){
	    nd->nodelink = NULL;
	}
    }
    linkio_node( Entry );
}/* link_lists */

} // end omega namespace
