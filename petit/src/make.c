/* $Id: make.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <basic/bool.h>
#include <petit/Exit.h>
#include <petit/tree.h>
#include <petit/ops.h>
#include <petit/vutil.h>
#include <petit/make.h>
#include <petit/affine.h>

namespace omega {

/* various utility functions for making data structures */

node *make_node( optype op ) {
  node *n = new node;
  
  n->noderight = NULL;
  n->nodecontext = NULL;     /* davew@cs.umd.edu 12/16/92 */
  n->nodeaffine = NULL;      /* davew@cs.umd.edu  3/24/92 */
  n->function = NULL;        /* davew@cs.umd.edu  8/03/94 */
  n->cover_depth = n->terminator_depth = -1;  /* davew 7/29/92 */
  n->nodeop = op;
  n->nodevalue = 0;
  n->exptype = exprunknown;
  n->nodenext = NULL;
  n->nodeprev = NULL;
  n->nodechild = NULL;
  n->nodeparent = NULL;
  n->nodelink = NULL;
  n->node_copy = NULL;
  n->nodesequence = Lines;
  n->nodeposition = 0;
  n->nodeddin = NULL;
  n->nodeddout = NULL;
  n->nodeextra = 0;
  n->ud.def_node = NULL;
  n->sc_region = 0;
  n->sc_number = 0;
  n->sc_lowlink = 0;
  n->sc_onStack = 0;
  n->is_loopfun = 0;
  n->ssag_node = NULL;
  n->shadow = NULL;
  n->version = 0;
  n->compound_stmt = 0;

  return n;
}/* make_node */

/*
 * Free one node.
 * This is rude version of FreeNode for use with original Petit stuff.
 */
void free_one_node(node *n) {
  if(is_mem_ref(n))
    DelRefList(n,0);

  FreeAffineExpr(n->nodeaffine);
  FreeDataDeps(n);

  n->nodeop = op_free;
  delete n;
}/* free_one_node */

/*
 * Free given tree.
 */
void free_node( node *n ) {
  node *nn, *nextnn;
  for( nn = n; nn != NULL; nn = nextnn ){
    nextnn = nn->nodenext;
    if( nn->nodechild != NULL ){
      free_node( nn->nodechild );
      nn->nodechild = NULL;
    }
    free_one_node(nn);
  }
}/* free_node */


node *make_const( int c )
{
node *n;
    n = make_node( op_constant );
    n->nodevalue = c;
    n->exptype = exprint;
    return n;
}/* make_const */

node *make_op( node *l, optype op, node *r )
{
node *n, *ll, *rr;

    ll = l;
    rr = r;
    if( rr == NULL ) return ll;
    else if( ll == NULL ){
	if( op == op_add ) return rr;
	else ll = make_const( 0 );
    }

    n = make_node( op );
    n->nodechild = ll;
    ll->nodeparent = n;
    ll->nodenext = rr;
    ll->nodeprev = NULL;
    rr->nodeparent = n;
    rr->nodenext = NULL;
    rr->nodeprev = ll;
    return n;
}/* make_op */

node *make_index( node *i )
{
node *n;
    n = make_node( op_index );
    //n->nodevalue = (unsigned int)i;
    n->nodevalue = (long unsigned int)i;
    n->exptype = exprrefint; /* all indexes are integer variables */
    return n;
}/* make_index */

struct {
    node *parent, *next, *prev, *child;
    bool full;
    }Save = {NULL,NULL,NULL,0};

void make_save( node *n )
/* save pointers from 'n' */
{
    if( Save.full ){
	fprintf(stderr, "Tried to overwrite saved node\n");
	Exit(1);
    }
    Save.full = 1;
    Save.parent = n->nodeparent;
    Save.prev = n->nodeprev;
    Save.next = n->nodenext;
    Save.child = NULL;   /* don't replace CHILD */
    if( n->nodeparent != NULL && n->nodeparent->nodechild == n )
	n->nodeparent->nodechild = NULL;
    if( n->nodenext != NULL ) n->nodenext->nodeprev = NULL;
    if( n->nodeprev != NULL ) n->nodeprev->nodenext = NULL;
    n->nodeparent = NULL;
    n->nodeprev = NULL;
    n->nodenext = NULL;
}/* make_save */

void make_replace( node *n )
/* put 'n' where the saved node was */
{
    if( !Save.full ){
	fprintf(stderr, "Tried to read empty saved node\n");
	Exit(1);
    }
    n->nodeparent = Save.parent;
    n->nodeprev = Save.prev;
    n->nodenext = Save.next;
    if( n->nodeparent != NULL && n->nodeparent->nodechild == NULL )
	n->nodeparent->nodechild = n;
    if( n->nodenext != NULL ) n->nodenext->nodeprev = n;
    if( n->nodeprev != NULL ) n->nodeprev->nodenext = n;
    Save.parent = NULL;
    Save.prev = NULL;
    Save.next = NULL;
    Save.full = 0;
}/* make_replace */

void make_switch_node( node *a, node *b )
{
node *anext, *aprev, *aparent, *achild;
node *bnext, *bprev, *bparent, *bchild;
node *c;

    if( a == NULL || b ==NULL ) return;

    anext = a->nodenext;
    aprev = a->nodeprev;
    aparent = a->nodeparent;
    achild = a->nodechild;
    bnext = b->nodenext;
    bprev = b->nodeprev;
    bparent = b->nodeparent;
    bchild = b->nodechild;

    if( bnext != a ){
	a->nodenext = bnext;
	if( bnext != NULL ) bnext->nodeprev = a;
    }else{
	a->nodenext = b;
    }
    if( bprev != a ){
	a->nodeprev = bprev;
	if( bprev != NULL ) bprev->nodenext = a;
    }else{
	a->nodeprev = b;
    }
    if( bparent != a ){
	a->nodeparent = bparent;
	if( bparent != NULL ){
	    if( bparent->nodechild == b ) bparent->nodechild = a;
	}
    }else{
	a->nodeparent = b;
    }
    if( bchild != a ){
	a->nodechild = bchild;
    }else{
	a->nodechild = b;
    }

    if( anext != b ){
	b->nodenext = anext;
	if( anext != NULL ) anext->nodeprev = b;
    }else{
	b->nodenext = a;
    }
    if( aprev != b ){
	b->nodeprev = aprev;
	if( aprev != NULL ) aprev->nodenext = b;
    }else{
	b->nodeprev = a;
    }
    if( aparent != b ){
	b->nodeparent = aparent;
	if( aparent != NULL ){
	    if( aparent->nodechild == a ) aparent->nodechild = b;
	}
    }else{
	b->nodeparent = a;
    }
    if( achild != b ){
	b->nodechild = achild;
    }else{
	b->nodechild = a;
    }


    if( a->nodechild != NULL ){
	for( c = a->nodechild; c!=NULL; c = c->nodenext ) c->nodeparent = a;
    }
    if( b->nodechild != NULL ){
	for( c = b->nodechild; c!=NULL; c = c->nodenext ) c->nodeparent = b;
    }
}/* make_switch_node */

void replace_index( node *n, bool follow, node *old, node *new_node )
{
node *m;

    for( m = n; m != NULL; m = (node *) (follow?m->nodenext:NULL) ){
	if( m->nodechild != NULL ){
	    replace_index( m->nodechild, 1, old, new_node );
	}
	if( m->nodeop == op_index && m->nodevalue == (long unsigned int)old ){
	    m->nodevalue = (long unsigned int)new_node;
	}
    }
}/* replace_index */

bool test_inloop( node *n, node *loop )
/* return 1 if we can reach 'loop' from 'n' by following nodeparent pointers */
{
node *nn;

    for( nn = n->nodeparent; nn!=NULL; nn = nn->nodeparent ){
	if( nn == loop ) return 1;
    }
    return 0;
}/* test_inloop */

bool test_tightly_nested( node *outer, node *inner )
/* return 0 if 'inner' is not tightly nested in 'outer' loop */
{
node *inter1, *inter2;

    inter2 = inner;
    do{
	for( inter1 = inter2->nodeparent;
		inter1 != NULL && inter1->nodeop != op_dolimit;
		inter1 = inter1->nodeparent ) ;

	if( inter1 == NULL ) return 0;

	inter1 = inter1->nodeparent;

	/* check that inter1 tightly surrounds inter2;
	   inter2->nodeparent == inter1->nodechild and
	   inter2->nodeprev == NULL and inter2->nodenext == NULL */

	if( inter2->nodeparent != inter1->nodechild ||
	    inter2->nodeprev != NULL || inter2->nodenext != NULL )
		return 0;

	inter2 = inter1;
    }while( inter1 != outer );

    return 1;
}/* test_tightly_nested */

} // end omega namespace
