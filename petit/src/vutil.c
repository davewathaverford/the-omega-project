/* $Id: vutil.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

/**********************************************************
 *                                                        *
 * General purpose functions (utilities).                 *
 * Written by Vadim Maslov vadik@cs.umd.edu 08/15/92.     *
 *                                                        *
 **********************************************************/

#include <stdio.h>
#include <string.h>
#include <basic/bool.h>
#include <petit/pres-interf.h>
#include <petit/vutil.h>
#include <petit/ssa.h>
#include <petit/ivr.h>
#include <petit/ops.h>
#include <petit/debug.h>
#include <petit/make.h>
#include <petit/copy.h>
#include <petit/affine.h>
#include <petit/lang-interf.h>
#include <petit/definitions.h>
#include <petit/petit_args.h>

namespace omega {

/* functions used only in this file */
static void 	MYcopy_fixtree(node *new_node, int flag); 
static int 	DelUseNodeList(USE_NODES *pun, node *p);

/*
 * Returns TRUE if variable referenced by given node
 * is defined at the beginning by Entry
 */



int DefinedAtBegin(node *n) {
  switch(get_nodevalue_sym(get_nodevalue_node(n))->symclass) {
  case CLCOMMON:
  case CLSTATIC:
  case CLFORMAL:
  case CLINOUT:
  case CLIN:
    return(true);
  case CLAUTO:
  case CLPRIVATE:
  case CLOUT:
    return(false);
  default:
    ErrAssert("DefinedAtBegin: bad variable class");
    return(0);
  }
}


/*
 * Returns TRUE if variable referenced by given node
 * is use at the end by ExitNode
 */
int UsedAtEnd(node *n) {
  switch(get_nodevalue_sym(get_nodevalue_node(n))->symclass) {
  case CLCOMMON:
  case CLSTATIC:
  case CLFORMAL:
  case CLINOUT:
  case CLOUT:
    return(true);
  case CLAUTO:
  case CLPRIVATE:
  case CLIN:
    return(false);
  default:
    ErrAssert("UsedAtEnd: bad variable class");
    return(0);
  }
}


/*
 * Place statement new after statement p
 */
void PlaceNodeAfter(node *new_node, node *p) {
  new_node->nodenext = p->nodenext;
  if(p->nodenext) p->nodenext->nodeprev = new_node;
  new_node->nodeprev = p;
  p->nodenext = new_node;
  new_node->nodeparent = p->nodeparent;
}


/*
 * Place statement new before statement p
 */
void PlaceNodeBefore(node *new_node, node *p) {
  Assert(p!=NULL, "PlaceNodeBefore: NULL ptr to stmt");
  if(p->nodeprev==NULL) {
    p->nodeparent->nodechild = new_node;
    new_node->nodeparent = p->nodeparent;
    new_node->nodenext = p;
    p->nodeprev = new_node;
  } else {
    PlaceNodeAfter(new_node, p->nodeprev);
  }
}

/*
 * Place node new below node old.
 * There are no other nodes below old.
 */
void PlaceNodeBelow(node *new_node, node *old) {
  old->nodechild = new_node;
  new_node->nodeparent = old;
}

/* 
 * place shadow of the node to have the same parent and prev node
 * as an original node
 */  

void PlaceShadow(node * p) {
  if (p->shadow) {
    p->shadow->nodeparent=p->nodeparent;
    p->shadow->nodeprev=p->nodeprev;
  }
}


/*
 * Place node new instead of node old.
 * Rewrite nodesequence's of new with that of old.
 */
void PlaceNodeInstead(node *new_node, node *old) {
  node *parent = old->nodeparent;
  node *next   = old->nodenext;
  node *prev   = old->nodeprev;
  Assert(parent!=NULL, "PlaceNodeInstead: parent == NULL");
  if(parent->nodechild == old) parent->nodechild = new_node;
  new_node->nodeparent = parent;
  if(next) {
    next->nodeprev = new_node;
    new_node->nodenext = next;
  }
  if(prev) {
    prev->nodenext = new_node;
    new_node->nodeprev = prev;
  }
  SetNodeSeq(new_node, old->nodesequence);
}

/*
 * Set nodesequence at given tree
 */
void SetNodeSeq(node *p, unsigned int ns) {
  node *np;
  for(np=p->nodechild; np; np=np->nodenext)
    SetNodeSeq(np, ns);
  p->nodesequence = ns;
}

/*
 * Remove given subtree. Free it if needed.
 * free: >=1 remove from IVR lists
 *         0 remove only from nodelink list
 *        -1 just free tree
 *       <-2 just remove from the AST location, not free it
 */
void RemoveNode(node *ap, int free) {
  if(ap->nodeprev) {
    ap->nodeprev->nodenext = ap->nodenext;
    if(ap->nodenext) ap->nodenext->nodeprev = ap->nodeprev;
  } else {
    Assert(ap->nodeparent!=NULL, "RemoveNode: no parent");
    Assert(ap->nodeparent->nodechild==ap, "RemoveNode: bad links");
    ap->nodeparent->nodechild = ap->nodenext;
    if(ap->nodenext) ap->nodenext->nodeprev = NULL;
  }

  if(petit_args.ivr_debug) {
    fprintf(debug, "\nRemoveNode: ap=%p, nodeseq=%d free=%d\n",
	    ap, ap->nodesequence, free);
  }

  if(free >= -1) {
    FreeTree(ap,free);
  } else {
    ap->nodenext = NULL;
    ap->nodeprev = NULL;
    ap->nodeparent = NULL;
  }
}

/*
 * Remove statement number node before given node
 */
void RmStmtNumber(node *p) {
  if(p->nodeprev && p->nodeprev->nodeop==op_stmt_number) {
    RemoveNode(p->nodeprev,true);
  }
}

/*
 * Make op_stmt_number node for a given statement node.
 * op_stmt_number should list all its surrounding loops.
 */
node *MkStmtNumber(node *p) {
  node *stmt = make_node(op_stmt_number);
  node *chn = NULL;
  while((p=loop_containing(p)) != Entry) {
    node *np = make_index((node*)p->nodechild);
    if(chn==NULL) {
      PlaceNodeBelow(np,stmt);
      chn = stmt->nodechild;
    } else {
      PlaceNodeBefore(np,chn);
      chn = np;
    }
  }
  return(stmt);
}


/*
 * Add node to the list of references.
 */
void AddRefList(node *p) {
  if(is_mem_ref(p)) {
    symtabentry *ste = get_nodevalue_sym(get_nodevalue_node(p));
    if(petit_args.ivr_Assert) {
      node **pln;
      for(pln = &ste->symdecl; *pln; pln = &((*pln)->nodelink)) {
	Assert(*pln!=p, "AddRefList: ref already in list");
	if(petit_args.ivr_debug>1) fprintf(debug, "el=%p ", *pln);
      }
      *pln = p;
      p->nodelink = NULL;
    } else {
      p->nodelink = ste->symdecl->nodelink;
      ste->symdecl->nodelink = p;
    }
    if(petit_args.ivr_debug) fprintf(debug, "+++AddRefList: ref %p added to list\n", p);
  }
}

/*
 * Delete given node from the list of references.
 * Delete declaration and symbol table entry if variable is not used anymore.
 * Return pointer to the next node.
 * If ass is TRUE check that node was present on list.
 */
node *DelRefList(node *p, int ass) {
  symtabentry *ste = get_nodevalue_sym(get_nodevalue_node(p));
  node **pln;

  for(pln = &ste->symdecl; *pln; pln = &((*pln)->nodelink)) {
    if(petit_args.ivr_debug>1) fprintf(debug, "el=%p ", *pln);
    if(*pln == p) {
      if(petit_args.ivr_debug)
	fprintf(debug, "---DelRefList: remove %p from list\n", p);
      *pln = p->nodelink;
      goto NodeDeleted;
    }
  }

// my fix - Tatiana 4/22/95
// the reference to the node can be not found in the case of the double
// declarations. Try to find it in the reference list that starts
// at its own declaration

if (ste->symdecl != get_nodevalue_node(p)) {
  node * dymmy_link=get_nodevalue_node(p);
  for (pln=&dymmy_link; *pln; pln = &((*pln)->nodelink)) {
    if (*pln == p) {
      *pln = p->nodelink;
      goto NodeDeleted;
     }
  }
}



  if(ass) {
    ErrAssert("DelRefList: node not found");
  } else {
    return(NULL);
  }

 NodeDeleted:;
  return(p->nodelink);
}


/*
 * CopyTree which works correctly for small trees without declarations.
 * flag: -1  just copy tree
 *        0  copy tree and add copied variable ref-s to ref-s list (nodelink)
 *        1  the same + add to IVR lists
 */
node *CopyTree(node *old, int flag) {
  node *new_node, *oldnext;

  if(old == NULL)
    return NULL;
  oldnext = old->nodenext;
  old->nodenext = NULL;

  new_node = copy_subtree(old, NULL, NULL);
  old->nodenext = oldnext;

  MYcopy_fixtree(new_node, flag);
  return(new_node);
}


/*
 * Rewritten copy_fixtree which doesn't spoil nodevalue
 * if declarations are not copied.
 */
static
void MYcopy_fixtree(node *new_node, int flag) {
  new_node->nodesequence = Lines;
  new_node->nodeddin = NULL;
  new_node->nodeddout = NULL;

  if(flag>=0)
    AddRefList(new_node);

  if(flag) {
    AddSSAgraph(new_node);
    AddEdgeSSAgraph(new_node);
  } else {
    new_node->ud.def_node = NULL;
  }

  if(new_node->nodechild != NULL) MYcopy_fixtree(new_node->nodechild, flag);
  if(new_node->nodenext  != NULL) MYcopy_fixtree(new_node->nodenext, flag);
}

/*
 * Copy symbol table entry
 * ! List of omega vars connected to this entry is not copied !
 */
nametabentry *CopySymTabEntry(symtabentry *ste, char *name) {
  nametabentry *nte = new nametabentry;
  if (!nte) out_of_memory();
  
  /* NO: memcpy((char*)&nte->namest, (char*)ste,sizeof(struct symtabentry)); */
  nte->namest = *ste;

  nte->namenext = ste->nte->namenext;
  ste->nte->namenext = nte;
  nte->namest.nte = nte;
  
  nte->namest.symname = new char[strlen(name)+1];
  if (nte->namest.symname==NULL) out_of_memory();
  strcpy(nte->namest.symname, name);

  nte->namest.omega_vars = 0;  


  return(nte);
}


/*
 * Add edges to SSA graph for given fetch.
 */
void AddEdgeSSAgraph(node *new_node) {
  if(new_node->nodeop==op_fetch || new_node->nodeop==op_update) {
    if(new_node->ud.def_node==NULL && du_ind_loop!=NULL) {
      /* Variable doesn't know yet where it's defined */
      VARNO i = FindVar(new_node);
      Assert(i<var_no, "MYcopy_fixtree: variable not in vars array");
      AddUseNodesList(&du_ind_loop[i].use_nodes, new_node);
    } else if(new_node->ud.def_node) {
      /* Variable's definition is already known */
      AddUseNodesList(&new_node->ud.def_node->ud.use_nodes, new_node);
    }
  }
}

/*
 * Free tree.
 * flag: 
 *   -1      remove variable ref-s from ref-s list
 *           remove nodes of the tree from IVR lists
 *           remove nodes from the SSA graph
 *
 *    0      remove a tree 
 *           remove variable ref-s from ref-s list
 *
 *    1      remove a tree 
 *           remove variable ref-s from ref-s list
 *           remove nodes of the tree from IVR lists
 *           remove nodes from the SSA graph
 */
void FreeTree(node *n, int flag) {
  node *p, *np;
  assert(n->nodeop != op_free);
  for(p=n->nodechild; p; p=np) {
    np = p->nodenext;
    FreeTree(p,flag);
  }
  FreeNode(n,flag);
}

/*
 * Free one node, maybe, not completely.
 * Eliminate the node from various lists where it could be mentioned.
 *
 * Flags (summary, see also at FreeTree):
 *     | SSA graph | memory
 * ----+-----------+-------
 *   -1      X
 *    0                X
 *    1      X         X
 */
void FreeNode(node *p, int flag) {
  if(flag>=0) {
    switch(p->nodeop) {
    CASE_MEMREF:
      DelRefList(p,0);
      break; 
    case op_dolimit:
      FreeNameTabEntry(get_nodevalue_sym(p));
      break;
    default:
      break;
    }
  }

  if(flag) {
    /* remove freed node from the list of SSA graph nodes */
    RemoveSSAgraph(p);
    
    if((p->nodeop==op_store || p->nodeop==op_update) && du_ind_loop!=NULL) {
      VARNO i = FindVar(p);
      Assert(i<var_no, "DelDefNode: variable not in vars array");
      if(du_ind_loop[i].def_node == p->nodeparent)
	du_ind_loop[i].def_node = NULL;
    }

    DelUseFromLists(p);

    if(is_assign(p)) {
      FreeUseNodeList(p->ud.use_nodes);
    }

    p->ud.def_node = NULL;
  }

  if(flag>=0) {
    FreeAffineExpr(p->nodeaffine);
    FreeDataDeps(p);
    if(p->shadow) {
      FreeTree(p->shadow, 1);
    }
    
    p->nodeop = op_free;
    delete p;
  }
}

/*
 * Free USE_NODES list
 */
void FreeUseNodeList(USE_NODES un) {
  USE_NODES nun;
  for(; un; un=nun) {
    nun=un->next;
    un->use_node->ud.def_node = NULL;
    delete un;
  }
}

/*
 * Remove given symbol table entry from name table
 */
void FreeNameTabEntry(symtabentry *ste) {
  nametabentry **pn;
  for(pn=&NameHead; *pn!=NULL; ) {
    nametabentry *n = *pn;
    if(&n->namest == ste) {
      *pn = n->namenext;
      if(n->namest.symname)
	delete n->namest.symname;
      delete n;
      break;
    } else {
      pn = &n->namenext;
    }
  }
}

/*
 * Free dependences coming from/to this node
 */
void FreeDataDeps(node *n) {
  ddnode *dd, *ddn;

  /* free dependences coming to the node */
  for(dd=n->nodeddin; dd; dd=ddn) {
    ddnode **pdd;
    ddn = dd->ddnextpred;
    for(pdd=&dd->ddpred->nodeddout;
	*pdd && *pdd!=dd;
	pdd=&(*pdd)->ddnextsucc) ;
    Assert(*pdd, "free_one_node: wrong ddlinks #1");
    *pdd = (*pdd)->ddnextsucc;
    delete dd;
  }

  /* free dependences coming from the node */
  for(dd=n->nodeddout; dd; dd=ddn) {
    ddnode **pdd;
    ddn = dd->ddnextsucc;
    for(pdd=&dd->ddsucc->nodeddin;
	*pdd && *pdd!=dd;
	pdd=&(*pdd)->ddnextpred) ;
    Assert(*pdd, "free_one_node: wrong ddlinks #2");
    *pdd = (*pdd)->ddnextpred;
    delete dd->dddiff;
    delete dd;
  }
}

/*
 * Delete use of a variable from IVR lists
 */
void DelUseFromLists(node *p) {
  if(p->nodeop==op_fetch || p->nodeop==op_update) {
    int nd = 0;
    if(du_ind_loop) {
      VARNO i = FindVar(p);
      Assert(i<var_no, "FreeNode: variable not in vars array");
      nd += DelUseNodeList(&du_ind_loop[i].use_nodes, p);
    }
    if(p->ud.def_node)
      nd += DelUseNodeList(&p->ud.def_node->ud.use_nodes, p);
    Assert(nd==1 || (du_ind_loop==NULL&&nd==0),
	   "FreeNode: ref mentioned in <> 1 list");
  }
}
  
/*
 * Delete given use node from given use list.
 * Returns number of actually deleted references.
 */
static
int DelUseNodeList(USE_NODES *pun, node *p) {
  USE_NODES un;
  for(; *pun && (*pun)->use_node!=p; pun=&((*pun)->next)) ;
  un = *pun;
  if(un) {
    *pun = un->next;
    if(petit_args.ivr_debug) fprintf(debug, "---DelUseNodeList: p=%p\n", p);
    delete un;
    return(1);
  } else {
    return(0);
  }
}


/*
 * Create expression node, do some trivial optimizations.
 * Maintains variable reference lists and IVR lists.
 * The flag `as' has the same meaning as in CopyTree/FreeTree.
 */
node *ExprNode(node *p1, optype op, node *p2, int as) {
  node *np;
  Assert(p2!=NULL, "ExprNode: don't like arg2 == NULL");
  if((p1->exptype!=exprint && p1->exptype!=exprrefint) ||
     (p2->exptype!=exprint && p2->exptype!=exprrefint)) {
    np = make_op(p1,op,p2);
    np->exptype = p1->exptype;
    if(np->exptype==exprrefint)  np->exptype=exprint;
    if(np->exptype==exprrefreal) np->exptype=exprreal;
    if(as) AddSSAgraph(np);
    return(np);
  }
  if(p1->nodeop==op_constant && p2->nodeop==op_constant) {
    unsigned long int v1 = p1->nodevalue;
    unsigned long int v2 = p2->nodevalue;
    long int  v;
    switch(op) {
    case op_add     : v=v1+v2; break;
    case op_subtract: v=v1-v2; break;
    case op_multiply: v=v1*v2; break;
    case op_divide  : v=v1/v2; break;
    case op_ceiling_divide: v=(v1+v2-1)/v2; break;
    default:     goto MakeNode;
    }
    if(v < 0) {			/* all numbers in petit are positive ! */
      op = op_subtract;
      p1->nodevalue = 0;
      p2->nodevalue = -v;
      goto MakeNode;
    }
    p1->nodevalue = v;
    FreeTree(p2,as);
    return(p1);
  }
  /* commutativity */
  if((get_op_precedence(p1->nodeop)>get_op_precedence(p2->nodeop) ||
      p1->nodeop==op_constant) && (op==op_add || op==op_multiply)) {
    node *t;
    t=p1; p1=p2; p2=t;
  }
  /* a+0 -> a, a*1 ->a, a*0 -> 0, etc. */
  if(p2->nodeop==op_constant) {
    long int v2 = p2->nodevalue;
    if(op==op_add || op==op_subtract) {
      if(v2==0) {
	FreeTree(p2,as);
	return(p1);
      } else if(v2 < 0) {
	op = (op==op_add ? op_subtract : op_add);
	p2->nodevalue = -v2;
      }
    } else if(op==op_multiply || op==op_divide || op==op_ceiling_divide) {
      if(v2==1) {
	FreeTree(p2,as);
	return(p1);
      } else if(v2==0) {
	Assert(op==op_multiply, "ExprNode: division by zero in YOUR PROGRAM");
	FreeTree(p1,as);
	p2->nodevalue = 0;
	return(p2);
      }
    }
  }
  /* distribution */
  if(op == op_multiply &&
     (p1->nodeop == op_add || p1->nodeop == op_subtract)) {  
    node *np1 = ExprNode(CopyTree(p2,as), op_multiply,
			 CopyTree(p1->nodechild,as), as);
    node *np2 = ExprNode(CopyTree(p2,as), op_multiply,
			 CopyTree(p1->nodechild->nodenext,as),as);
    np = ExprNode(np1,p1->nodeop,np2,as);
    FreeTree(p1,as);
    FreeTree(p2,as);
    return(np);
  }
  /* associativity */
  if((op==op_add || op==op_subtract) && p2->nodeop==op_constant &&
     (p1->nodeop==op_add || p1->nodeop==op_subtract)) {
    optype nop = (p1->nodeop==op ? op_add : op_subtract);
    np = ExprNode(CopyTree(p1->nodechild,as), p1->nodeop,
		  ExprNode(CopyTree(p1->nodechild->nodenext,as), nop,
			   CopyTree(p2,as), as), as);
    FreeTree(p1,as);
    FreeTree(p2,as);
    return(np);
  }
 MakeNode:
  np = make_op(p1,op,p2);
  np->exptype = exprint;
  if(as) AddSSAgraph(np);
  return(np);
}


/*
 * Print program in frame (beauty aid)
 */
void PrintPgm(char *t, node *p, int f) {
  fprintf(debug, "\n=== %s ===\n", t);
  debug_petit(p, f);
  fprintf(debug, "\n");
}


/*
 * Replace affine expressions with their shortest form.
 */
void ReplaceAffine(node *ap) {
  for(; ap!=NULL; ap=ap->nodenext) {
    switch(ap->nodeop) {
    case op_if:
      ReplaceAffine(ap->nodechild);                           /* condition */
      ReplaceAffine(ap->nodechild->nodenext->nodechild);      /* THEN branch */
      if(ap->nodechild->nodenext->nodenext)
	ReplaceAffine(ap->nodechild->nodenext->nodenext->nodechild);  /* ELSE branch */
      break;

    CASE_DO:
      ReplaceAffine(ap->nodechild->nodenext);       
      ReplaceAffine(ap->nodechild->nodenext->nodenext);       
      ReplaceAffine(ap->nodechild->nodenext->nodenext->nodenext);       
      ReplaceAffine(ap->nodechild->nodechild);                /* loop body */
      break;

    case op_entry:
      ReplaceAffine(ap->nodechild);                           /* program body */
      break;

    case op_assert:
    case op_declare: 
    case op_stmt_number:
    case op_comment:
    case op_exit:
      continue;

    CASE_ASSIGN:
      ReplaceAffine(ap->nodechild);
      ReplaceAffine(ap->nodechild->nodenext);
      break;

    default: 
      Assert(is_expr(ap), "ReplaceAffine: expression op expected");
      if(ap->nodeaffine && node_is_affine(ap) && ap->nodeaffine->other_branch==NULL &&
	 (ap->exptype==exprint || ap->exptype==exprrefint)) {
	affine_expr *ae=ap->nodeaffine;
	node *lp, *rp;
	int i;
	Assert(ae->terms[0].petit_var==NULL, "ReplaceAffine: first term var is not NULL");
	lp = NULL;
	for(i=1; i<ae->nterms; i++) {
	  Var_Id var_id=ae->terms[i].petit_var;
	  int coef = ae->terms[i].coefficient;
	  node *cp;
	  if(var_id_is_index(var_id)) {
	    rp = make_node(op_index);
	    rp->nodevalue = (unsigned long int)var_id->symdecl;
	  } else {
	    rp = make_node(op_fetch);
	    if(var_id->original)
	      var_id = var_id->original;
	    rp->nodevalue = (unsigned long int)var_id->symdecl;
	    AddRefList(rp);
	  }
	  rp->exptype = exprrefint;
	  cp = make_const(coef<0 ? -coef : coef);
	  rp = ExprNode(rp, op_multiply, cp, false);
	  if (lp == NULL)
	      lp = rp;
          else
	      lp = ExprNode(lp, coef<0 ? op_subtract : op_add, rp, false);
	}
	int coef = ae->terms[0].coefficient;
	rp = MakeConst((coef<0 ? -coef : coef), false);
	if (lp == NULL)
	    lp = rp;
	else
	    lp = ExprNode(lp, coef<0 ? op_subtract : op_add, rp, false);

	lp->nodeaffine = ae;
	PlaceNodeInstead(lp, ap);
	ap->nodeaffine = NULL;
	FreeTree(ap, true);
      } else {
	node *p, *np;
	for(p=ap->nodechild; p; p=np) {
	  np=p->nodenext;
	  ReplaceAffine(p);
	}
      }
      return;
    }
  }
}

/*
 * Get a subscript no. sn from array reference ae
 */
node *GetSub(node *ae, int sn) {
  int i;
  node *sub = ae->nodechild;
  for(i=0; i<sn; i++)
    sub = sub->nodenext;
  return(sub);
}

/*
 * Return loop which "embraces" statement p at level dl
 */
node *EmbracingLoop(node *p, int dl) {
  node *loop;
  for(loop=p;
      !(is_do(loop) && (int)loop->nodevalue==dl);
      loop = loop_containing(loop))
    Assert(loop->nodeop!=op_entry, "EmbracingLoop: bad loop");
  return(loop);
}

/*
 * Remove unused declarations.
 * Can't remove declarations in DelRefList if nodelink list is empty,
 * because in interchange we can have a situation when
 * there is no references to a variable only temporarily.
 */
void RmUnusedDcls(void) {
  nametabentry **pn, *n;
  node *dcl;

  for(pn=&NameHead; *pn!=NULL; ) {
    n = *pn;
    dcl = n->namest.symdecl;
    if(dcl && dcl->nodeop==op_declare && dcl->nodelink==NULL) {
      RemoveNode(dcl,false);
      *pn = n->namenext;
      if(n->namest.symname)
	delete n->namest.symname;
      delete n;
    } else {
      pn = &n->namenext;
    }
  }
}


/*
 * Makes constant keeping in mind tricks with negative numbers
 */
node *MakeConst(long int value, int addssa) {
  node *lp, *rp;
  if(value >= 0) {
    lp = make_const(value);
    if(addssa) AddSSAgraph(lp);
  } else {
    lp = make_const(0);
    if(addssa) AddSSAgraph(lp);
    rp = make_const(-value);
    if(addssa) AddSSAgraph(rp);
    lp = make_op(lp, op_subtract, rp);
    lp->exptype = exprint;
    if(addssa) AddSSAgraph(lp);
  }
  return(lp);
}

} // end of omega namespace
