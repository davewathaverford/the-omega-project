/* $Id: build_stmt_rel.h,v 1.1.1.1 2000/06/29 19:24:22 dwonnaco Exp $ */
#ifndef Already_Included_BuildStmtRel
#define Already_Included_BuildStmtRel

#include <omega/Relations.h>
#include <petit/tree.h>
#include <petit/lang-interf.h>

#define myMaxStmts 50
#define myOmitScalars 0

struct StmtDep {
     Relation   dep_rel;        // dependence relation
     int        exists;         // is edge already here?  
     int        is_closure;     // is already closure? 
   };

struct Stmt {
     node     * tree_node;
     a_access 	access;
     int 	depth;
     Relation 	bounds;
   };

extern void set_closure_flags(int n);

extern void print_closure_help(void);

extern void test_closure(int found);

#endif
