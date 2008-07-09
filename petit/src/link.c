/* link.c,v 1.1.1.2 1992/07/10 02:43:22 davew Exp */

#include <petit/tree.h>
#include <petit/link.h>

namespace omega {

/* What to use for 'prev' of next node, 'parent' of next node */
node *Link_Prev, *Link_Next, *Link_Parent;

/* initialize prev node */
void link_start( node *prev, node *parent )
{
    Link_Parent = parent;
    Link_Prev = prev;
    if( prev != NULL ){
	Link_Next = prev->nodenext;
    }else{
	Link_Next = parent->nodechild;
    }
}/* link_start */

/* Link in a new node at this point. */
void link_node( node *new_node )
{
    new_node->nodeprev = Link_Prev;
    new_node->nodeparent = Link_Parent;
    new_node->nodenext = Link_Next;
    /* leave child alone */

    if( Link_Prev != NULL ){
	Link_Prev->nodenext = new_node;
    }
    if( Link_Next != NULL ){
	Link_Next->nodeprev = new_node;
    }
    if(Link_Parent!=NULL && Link_Prev==NULL) {
	Link_Parent->nodechild = new_node;
    }

    Link_Prev = new_node;
}/* link_node */

/* unlink node, when it doesn't really belong */
void link_unlink( node *n )
{
    if( n->nodeprev != NULL )
	n->nodeprev->nodenext = n->nodenext;
    if( n->nodenext != NULL )
	n->nodenext->nodeprev = n->nodeprev;
    if( n->nodeparent != NULL && n->nodeparent->nodechild == n )
	n->nodeparent->nodechild = n->nodenext;

    n->nodeparent = NULL;
    n->nodeprev = NULL;
    n->nodenext = NULL;
}/* link_unlink */

/* Push and Pop scopes */
void link_pushscope( node *n )
{
    if( n == NULL ){
	Link_Parent = Link_Prev;
    }else{
	Link_Parent = n;
    }
    Link_Prev = Link_Parent->nodechild;
    if( Link_Prev == NULL ){
	Link_Next = NULL;
    }else{
	Link_Next = Link_Prev->nodenext;
    }
}/* link_pushscope */

void link_popscope(void)
{
    Link_Prev = Link_Parent;
    Link_Next = Link_Prev->nodenext;
    Link_Parent = Link_Prev->nodeparent;
}/* link_popscope */

} // end omega namespace
