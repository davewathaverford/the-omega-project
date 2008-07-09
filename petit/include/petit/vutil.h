/* $Id: vutil.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/**********************************************************
 *                                                        *
 * General purpose functions (utilities).                 *
 * Written by Vadim Maslov vadik@cs.umd.edu 08/15/92.     *
 *                                                        *
 **********************************************************/
#ifndef Already_Included_Vutil
#define Already_Included_Vutil

#include <basic/assert.h>
#include <petit/Exit.h>  // get Vadim's Exit.h for petit, not just Omega Library
#include <petit/tree.h>

namespace omega {

int       DefinedAtBegin(node *);
int       UsedAtEnd(node *);

void      PlaceNodeAfter(node *, node *);
void      PlaceNodeBefore(node *, node *);
void      PlaceNodeBelow(node *, node *);
void      PlaceNodeInstead(node *, node *);
void      PlaceShadow(node *);
void      SetNodeSeq(node *, unsigned int);
void      RemoveNode(node *, int);
void      RmStmtNumber(node *);
node     *MkStmtNumber(node *);

void      AddRefList(node *);
node     *DelRefList(node *, int);
node     *CopyTree(node *, int);
nametabentry *CopySymTabEntry(symtabentry *, char *name);
void      AddEdgeSSAgraph(node *);

void      FreeTree(node *, int);
void      FreeNode(node *, int);
void      FreeUseNodeList(struct use_nodes *);
void      FreeNameTabEntry(symtabentry *);
void      FreeDataDeps(node *);
void      DelUseFromLists(node *);
node     *ExprNode(node *, optype, node *, int);

void      PrintPgm(char *, node *, int);
void      ReplaceAffine(node *);

node     *GetSub(node *, int);
node     *EmbracingLoop(node *, int);
void      RmUnusedDcls(void);

node     *MakeConst(long int, int);

}

#endif /* Already_Included_Vutil */
