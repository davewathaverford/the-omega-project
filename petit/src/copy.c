/* copy.c,v 1.1.1.2 1992/07/10 02:42:15 davew Exp */

#include <stdio.h>
#include <string.h>
#include <omega/Relations.h>
#include <petit/tree.h>
#include <petit/ops.h>
#include <petit/make.h>
#include <petit/makeint.h>
#include <petit/copy.h>
#include <petit/vutil.h>
#include <petit/affine.h>

namespace omega {

ddnode::ddnode(int nest) {
  if(nest <= 0) {
    dddiff = NULL;
  } else {
    dddiff = new int[nest+1];
  }
  dd_relation = NULL;
}

ddnode::ddnode(ddnode *ddold) {
  ddtype  = ddold->ddtype;
  ddpred  = ddold->ddpred;
  ddsucc  = ddold->ddsucc;
  ddnextpred = ddold->ddnextpred;
  ddnextsucc = ddold->ddnextsucc;
  ddlink  = ddold->ddlink;
  ddnest  = ddold->ddnest;
  ddrestraint = ddold->ddrestraint;
  dddir   = ddold->dddir;
  ddflags = ddold->ddflags;
  ddextra = 0;
  if(ddold->dd_relation!=NULL) {
    dd_relation = new Relation(*(ddold->dd_relation));
  } else {
    dd_relation = NULL;
  }
  int i;
  if(ddnest <= 0) {
    dddiff = NULL;
  } else {
    dddiff = new int[ddnest+1];
    for(i=0; i<=(int)ddnest; i++)
      dddiff[i] = ddold->dddiff[i];
  }
  for(i=1; i<=(int)ddold->ddnest; i++) {
    loop_dir[i] = ddold->loop_dir[i];
  }
}

ddnode::~ddnode() {
  delete dddiff;
  delete dd_relation;
}



static int DepCopy = 0;

static node *copy1_subtree(node *, node *, node *);
static void copy_fixtree(node *);

/* 
 * copy 'old' node.  return pointer to 'new'.
 * recursively copy all links.
 */
node *copy_subtree(node *old, node *parent, node *prev) {
  node *new_node = copy1_subtree(old, parent, prev);
  copy_fixtree(old);
  return(new_node);
} /* copy_subtree */


static
node *copy1_subtree(node *old, node *parent, node *prev) {
  node *new_node = new node(*old);
//  make_node(old->nodeop);
// NO: Ack! this was bad
//   memcpy((char*)new_node, (char*)old, sizeof(struct node));
  
  /* save pointer from old to new for later fixup */
  old->node_copy = new_node;

  switch(new_node->nodeop) {
  case op_declare:
  case op_dolimit:
  case op_real:
    /* if symbol table is also copied */
    { symtabentry *new_sym = get_nodevalue_sym(new_node)->symcopy;
      if(new_sym != NULL) {
	new_node->nodevalue = (unsigned long int) new_sym;
	new_sym->symdecl = new_node;
      }
    }
    break;

  CASE_MEMREF:
  case op_index:
  case op_call:
    { node *new_decl = get_nodevalue_node(new_node)->node_copy;
      if(new_decl != NULL) 
	new_node->nodevalue = (unsigned long int) new_decl;
    }
    break;

  default:
    break;
  }

  /* value stays the same or, if a pointer, gets fixed later */
  /* next, prev, child, parent will get fixed here */
  /* link gets fixed later */
  /* sequence stays the same */
  /* wposition is probably junk */
  /* ddout gets copied later */
  /* ddin gets NULL here, fixed later */
  
  new_node->nodeparent = parent;
  new_node->nodeprev = prev;
  new_node->nodelink = NULL;

  if(!DepCopy) {
    new_node->nodeddin = NULL;
    new_node->nodeddout = NULL;
  }

#if 0
  /* original petit code not always frees what was allocated */
  AddRefList(new_node);		
#endif

  new_node->nodeaffine = CopyAffineExpr(old->nodeaffine);

  /* should it be 1 or flag ??? */
  new_node->shadow = CopyTree(old->shadow,1);
  
  if( old->nodechild != NULL )
    new_node->nodechild = copy1_subtree( old->nodechild, new_node, NULL );
  if( old->nodenext  != NULL )
    new_node->nodenext  = copy1_subtree( old->nodenext,  parent, new_node );
  
  return new_node;
} /* copy1_subtree */


/*
 * Nullify node_copy at old tree.
 */
static
void copy_fixtree(node *old) {
  old->node_copy = NULL;
  if(old->nodechild != NULL) copy_fixtree(old->nodechild);
  if(old->nodenext  != NULL) copy_fixtree(old->nodenext);
}


node *copy_tree( node *old )
{
    node *new_node, *oldnext;
    if( old == NULL ) return NULL;
    oldnext = old->nodenext;
    old->nodenext = NULL;
    new_node = copy_subtree( old, NULL, NULL );
    old->nodenext = oldnext;
    return new_node;
}/* *copy_tree */


/*
 * Copy data dependence
 */
static
ddnode* copy_ddout(ddnode *ddold) {
    ddnode *ddnew = new ddnode(ddold);
    node *old = ddold->ddpred;
    ddnew->ddpred = (old->node_copy ? old->node_copy : old);
    old = ddold->ddsucc;
    ddnew->ddsucc = (old->node_copy ? old->node_copy : old);
    ddnew->ddnextpred = ddnew->ddsucc->nodeddin;
    ddnew->ddsucc->nodeddin = ddnew;
    ddnew->ddnextsucc = NULL;
    if(ddold->ddnextsucc != NULL) {
      ddnew->ddnextsucc = copy_ddout(ddold->ddnextsucc);
    }
    return ddnew;
} /* copy_ddout */

static
void copy_dd( node *new_node ) {
    node *n;
    for( n = new_node; n != NULL; n = n->nodenext ){
        if( n->nodeddout != NULL ) n->nodeddout = copy_ddout( n->nodeddout );
        if( n->nodechild != NULL ) copy_dd( n->nodechild );
    }
}/* copy_dd */

} // end omega namespace
