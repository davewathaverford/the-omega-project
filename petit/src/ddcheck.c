/* ddcheck.c,v 1.1.1.2 1992/07/10 02:42:17 davew Exp */

#if ! defined BATCH_ONLY_PETIT

#include <stdio.h>
#include <basic/bool.h>
#include <basic/assert.h>
#include <petit/tree.h>
#include <petit/message.h>
#include <petit/ddcheck.h>
#include <petit/make.h>
#include <petit/browsedd.h>
#include <petit/motif.h>
#include <petit/petit_args.h>

namespace omega {

ddnode *DDcheck_save_list;
ddnode *GLOBAL_START;

node *DDcheck_loop;
dddirection DDcheck_dir1, DDcheck_dir2, DDcheck_dir1check, DDcheck_dir2check;
unsigned int DDcheck_func;
int DDcheck_IR, DDcheck_doany;

void DDcheck_init(void)
{
    DDcheck_save_list = NULL;
}/* DDcheck_init */

void DDcheck_add( ddnode *dd )
{
   if (! ddisDead(dd))
     {
     dd->ddlink = DDcheck_save_list;
     DDcheck_save_list = dd;
     }
}/* DDcheck_add */

bool DDcheck_fini( char *string, int child )
{
int val;
char line[PETITBUFSIZ];

    if( DDcheck_save_list == NULL ) return 0;

    sprintf( line, "data dependence prevents %s", string );
    Message_Add( line );

    GLOBAL_START = DDcheck_save_list;
    assert(GLOBAL_MENU->root[child].menu_prolog != NULL);
    val=(GLOBAL_MENU->root[child].menu_prolog)
	    (GLOBAL_MENU->root[child].menu_parameter);
    if (val)
          {
          motif_menu(GLOBAL_MENU->child[child]);
	  if (GLOBAL_MENU->child[child]->refresh != NULL)
	      (GLOBAL_MENU->child[child]->refresh)(0);
          }
    return 1;
}/* DDcheck_fini */


/* visit a list of dd successors.
   for each, see if the successor lies within the loop in question.
   if so, see if it has a (<,>) dependence.
   return 1 if so. */
void DDcheck_list( ddnode *ddlist ) {
  ddnode *dd;
  unsigned int x1, x2, func;
  
  for( dd = ddlist; dd != NULL; dd = dd->ddnextsucc ){
    if( test_inloop( dd->ddsucc, DDcheck_loop ) ){
      /* test direction vector */
      x1 = (dd->dddir & DDcheck_dir1) == DDcheck_dir1check;
      x2 = (dd->dddir & DDcheck_dir2) == DDcheck_dir2check;
      x1 = x1 * 8;
      x2 = x2 * 4;
      func = 1 << (x1+x2);
      if( DDcheck_func & func ){
	/* Reduction dependencies don't prevent reordering transformations:
	   circulation, interchange, reversal.
	   Parallelize transformation: if all dependencies are reductions
	   make it doany instead of doall */
	if(dd->ddtype==ddreduce) {
	  DDcheck_doany=1;
	  continue;
	}
	DDcheck_add( dd );
      }
    }
  }
}/* DDcheck_list */

void DDcheck_body( node *body )
/* visit each node in the body of the loop.
   look for dependence arcs to other nodes in the body of the loop.
   if we find any, check them for (<,>) directions */
{
node *b;

    for( b = body; b != NULL; b = b->nodenext ){
	if( b->nodeddout != NULL ){
	    DDcheck_list( b->nodeddout );
	}
	DDcheck_body( b->nodechild );
    }
}/* DDcheck_body */


bool DDcheck( node *body, node *loop,
		  dddirection dddir1, dddirection dddir1check,
		  dddirection dddir2, dddirection dddir2check,
		  unsigned int func, int IgnoreReduce, char *string, int child)
{
    DDcheck_init();
    DDcheck_loop = loop;
    DDcheck_dir1 = dddir1;
    DDcheck_dir1check = dddir1check;
    DDcheck_dir2 = dddir2;
    DDcheck_dir2check = dddir2check;
    DDcheck_func = func;
    DDcheck_IR = IgnoreReduce;

    DDcheck_body( body );

    return DDcheck_fini( string, child );
}/* DDcheck */

int DDcheck_simple( node *body, node *loop,
			 dddirection dddir1, dddirection dddir1check,
			 dddirection dddir2, dddirection dddir2check,
			 unsigned int func, int IgnoreReduce)
{
    DDcheck_init();
    DDcheck_loop = loop;
    DDcheck_dir1 = dddir1;
    DDcheck_dir1check = dddir1check;
    DDcheck_dir2 = dddir2;
    DDcheck_dir2check = dddir2check;
    DDcheck_func = func;
    DDcheck_IR = IgnoreReduce;

    DDcheck_body( body );

    if( DDcheck_save_list == NULL ) return 0;
    return 1;
}/* DDcheck_simple */

ddnode *DDcheck_and_return( node *body, node *loop,
			 dddirection dddir1, dddirection dddir1check,
			 dddirection dddir2, dddirection dddir2check,
			 unsigned int func, int IgnoreReduce)
{
    DDcheck_init();
    DDcheck_loop = loop;
    DDcheck_dir1 = dddir1;
    DDcheck_dir1check = dddir1check;
    DDcheck_dir2 = dddir2;
    DDcheck_dir2check = dddir2check;
    DDcheck_func = func;
    DDcheck_IR = IgnoreReduce;

    DDcheck_body( body );

    return DDcheck_save_list;
}/* DDcheck_and_return */


void DDfix_list( ddnode *ddlist, node *loop, adjust_func adjust )
/* visit a list of dd successors.
   for each, see if the successor lies within the loop in question.
   if so, call adjust to adjust difference and direction. */
{
ddnode *dd;

    for( dd = ddlist; dd != NULL; dd = dd->ddnextsucc ){
	if( test_inloop( dd->ddsucc, loop ) ){
	    adjust( dd );
	}
    }
}/* DDfix_list */

void DDfix( node *body, node *loop, adjust_func adjust )
/* find dependence relations within the loop, call proc to adjust them */
{
node *b;

    for( b = body; b != NULL; b = b->nodenext ){
	if( b->nodeddout != NULL ){
	    DDfix_list( b->nodeddout, loop, adjust );
	}
	DDfix( b->nodechild, loop, adjust );
    }
}/* DDfix */

} // omega namespace

#endif
