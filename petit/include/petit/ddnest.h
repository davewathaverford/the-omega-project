/* ddnest.h,v 1.1.1.2 1992/07/10 02:39:51 davew Exp */
#ifndef Already_Included_DDnest
#define Already_Included_DDnest

namespace omega {

extern void dd_get_nests( node *onode, node *inode, unsigned int *onest, unsigned int *inest, unsigned int *bnest );

extern void dd_fix_nests( node *onode, node *inode, unsigned int onest, unsigned int inest, unsigned int bnest );

}

#endif
