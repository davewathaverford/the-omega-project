/* ddcheck.h,v 1.1.1.2 1992/07/10 02:39:44 davew Exp */
#ifndef Already_Included_DDcheck
#define Already_Included_DDcheck

#include <basic/bool.h>

namespace omega {

extern ddnode *GLOBAL_START;

/* each dependence relation in the loop is checked with:
    dd->dddir & dddir1 == dddir1check => test1
    dd->dddir & dddir2 == dddir2check => test2
   the results of these two tests are combined and compared to
   the bit vector 'func': 0xabcd, where a, b, c, d are either 0 or 1.
   a = test1 & test2
   b = test1 & !test2
   c = !test1 & test2
   d = !test1 & !test2
 */
extern bool DDcheck( node *body, node *loop,
		    dddirection dddir1, dddirection dddir1check,
		    dddirection dddir2, dddirection dddir2check,
		    unsigned int func, int IgnoreReduce, char *string, int child);

extern int DDcheck_simple( node *body, node *loop,
			   dddirection dddir1, dddirection dddir1check,
			   dddirection dddir2, dddirection dddir2check,
			   unsigned int func, int IgnoreReduce);

extern ddnode *DDcheck_and_return( node *body, node *loop,
			   dddirection dddir1, dddirection dddir1check,
			   dddirection dddir2, dddirection dddir2check,
			   unsigned int func, int IgnoreReduce);

typedef void (*adjust_func)( ddnode *dd );

extern void DDfix( node *body, node *loop, adjust_func adjust );

/* for each dependence relation within the body of the loop, call adjust */

/* for self-generated DD lists, initialize the list */
extern void DDcheck_init( void );

/* add a node to the dd list */
extern void DDcheck_add( ddnode *dd );

/* check for null list */
extern bool DDcheck_fini( char *string, int child);

}

#endif
