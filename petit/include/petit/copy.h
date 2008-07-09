/* copy.h,v 1.1.1.2 1992/07/10 02:39:42 davew Exp */

#ifndef Already_Included_Copy
#define Already_Included_Copy

namespace omega {

/* copy_subtree will copy 'nodenext' */
extern node* copy_subtree ( node* old, node *parent, node *prev );

/* copy_tree won't follow 'nodenext' */
extern node* copy_tree ( node* old );

}

#endif
