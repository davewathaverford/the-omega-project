/* $Id: arrayExpand.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

/******************************************************
 *                                                    *
 * Array expansion and privatization.                 *
 * Written by Vadim Maslov vadik@cs.umd.edu 10/26/92. *
 *                                                    *
 ******************************************************/

#include <stdio.h>
#include <string.h>
#include <basic/bool.h>
#include <petit/arrayExpand.h>
#include <petit/vutil.h>
#include <petit/affine.h>
#include <petit/make.h>
#include <petit/lang-interf.h>
#include <petit/pres-interf.h>
#include <petit/print.h>
#include <petit/definitions.h>
#include <petit/ops.h>
#include <petit/ddodriver.h>
#include <petit/petity.h>
#include <petit/petit_args.h>
#include <petit/parse_and_analyse.h>
#include <petit/debug.h>

namespace omega {

static int touched;

static void  ZapVarStoreDeps(node *);
static void  InitCCstuff(node *);
static void  ConnectNeighbours(node *);
static node *CopyRenameCoCo(node *, char *);
static int   GoodIndexes(affine_expr *, Var_Id);


/*
 * Zap as many storage dependencies as possible by array expansion
 */
int ArrayExpansion(int print) {
    nametabentry *n, *nn;
    touched = 0;
    for(n=NameHead; n!=NULL; n=nn) {
      node *dcl = n->namest.symdecl;
      nn = n->namenext;
      if(dcl==NULL || dcl->nodeop!=op_declare || n->namest.symclass==CLBUILTIN)
	continue;
      
      ZapVarStoreDeps(dcl);
    }

    /* Reductions interfere w/array expansion, so they are delayed when it
       is turned on; so we force re_analyse so they get recognized now.   */
    if(touched || petit_args.makeReductionOps) 
      re_analyse(print);

  if(print) {
      omega::print_petit(Entry, 1);
  }
    //print_petit(Entry, 1);

  return(0);
}


/*
 * Zap a storage dependence by array expansion
 */
int ZapExpand(ddnode *dd) {
  touched = 0;
  ZapVarStoreDeps(get_nodevalue_node(dd->ddpred));
  return(touched);
}


static int firstWarning = 1;

static void make_tree_constant(node *bexpr, int def)
    {
    switch (bexpr->nodeop)
	{
	case op_constant:
	    return;
        case op_add:
        case op_subtract:
        case op_multiply:
        case op_divide:
        case op_min:
        case op_max:
	    {
		for (node *tmp = bexpr->nodechild; tmp!=NULL; tmp=tmp->nodenext)
		    make_tree_constant(tmp, def);
		break;
	    }
	default:
            if (firstWarning)
		{
	        fprintf(DebugFile, "Warning: "
	            "unable to accurately derive bounds for expanded array\n");
                print_to_buf(bexpr, 0, 0);
		fprintf(DebugFile, "loop bound contains (%s)\n", 
			(const char *) printBuf);
		}
            firstWarning = 0;
	    bexpr->nodeop = op_constant;
	    bexpr->nodevalue = def;
	    free_node(bexpr->nodechild);
            bexpr->nodechild = NULL;
	}
    } /* make_tree_constant */




#define MAXCOCONO 200
static node *cocos[MAXCOCONO];	/* nodes of connected component (CC) */
int coco_no;			/* number of nodes in CC */
int conn_comp_no;		/* CC number */
node *nr1;			/* next node on nodelink list */

/* 
 * Zap storage dependencies for a given variable.
 * To do this, find all conneceted components for given variabe,
 * rename them and try to expand.
 * sc_region serves as a connected component number.
 */
static
void ZapVarStoreDeps(node *dcl) {
  node *r1;
	
  if(dcl->nodeop!=op_declare)
    return;

  InitCCstuff(dcl);
  
  for(r1=dcl->nodelink; r1; r1=nr1) {
    nr1 = r1->nodelink;
    if (is_constant(r1)) continue; //this can happen because of make_constant_tree - T.S.
    if(InDecl(r1) || is_phi_access(r1)) continue;
    Assert(r1==Entry || r1==ExitNode || get_nodevalue_node(r1)==dcl,
	   "ZapVarStoreDeps: wrong links");
    if(r1->sc_region==0) {
      node *ndcl = dcl;
      int i;
      r1->sc_region = ++conn_comp_no;
      coco_no = 0;
      cocos[coco_no++] = r1;
      ConnectNeighbours(r1);

      /* Now we have list of nodes for one CC */
      /* Rename a component if it allows us to break a storage dependence */
      for(i=0; i<coco_no; i++) {
	ddnode *dd;
	for(dd=cocos[i]->nodeddout; dd; dd=dd->ddnextsucc) {
	  if(!ddisDead(dd) && is_stor_dep(dd) &&
	     dd->ddsucc->sc_region!=r1->sc_region &&
	     dd->ddsucc->nodevalue==r1->nodevalue)
	    goto RenameCC;
	}
	for(dd=cocos[i]->nodeddin; dd; dd=dd->ddnextpred) {
	  if(!ddisDead(dd) && is_stor_dep(dd) &&
	     dd->ddpred->sc_region!=r1->sc_region &&
	     dd->ddpred->nodevalue==r1->nodevalue)
	    goto RenameCC;
	}
      }
      goto AfterRenameCC;

    RenameCC:;      /* Add declaration and symtabentry for new variable */
      ndcl=CopyRenameCoCo(dcl,"_r%d");
      nr1=dcl->nodelink;

      /* Try to do scalar (array) expansion to kill storage dep-s in CC */
    AfterRenameCC:;
      { int dl;
	for(dl=1; dl<=maxnest; dl++) {
	  int KillableStorDeps = 0;

	  for(i=0; i<coco_no; i++) {
	    node *p = cocos[i];
	    ddnode *dd;
	    int LiveDeps = 0;

	    /* Don't expand globally visible things */
	    if(p==Entry || p==ExitNode)
	      goto AfterDl;

	    for(dd=p->nodeddout; dd; dd=dd->ddnextsucc) {
	      if(!ddisDead(dd) && dd->ddsucc->sc_region==r1->sc_region){
		LiveDeps = 1;
		if(dl > (int)dd->ddnest) goto AfterDl;
		if(is_flow_dep(dd)) {
		  if(dd->dddiff[dl] != 0) goto NextDl;
		} else if(is_stor_dep(dd)) {
		  if(dd->dddiff[dl] != 0) KillableStorDeps = 1;
		}
	      }
	    }

	    if(!LiveDeps) goto AfterDl;
	  }

	  if(KillableStorDeps && 
	     (int)get_nodevalue_sym(ndcl)->symdims < petit_args.expandlimit) {
	    /* Level dl is good for expansion here, so do it */
	    node *p, *loop;
	    node *bexpr1, *bexpr2, *bound, *b;

	    /* Rename expanded CC first to avoid clashes with other CCs */
	    if(ndcl==dcl)
	      {
	      ndcl = CopyRenameCoCo(dcl,"_e%d");
	      nr1=dcl->nodelink;
	      }

	    /* Find the loop responsible for expansion */
	    p = cocos[0];
	    loop = EmbracingLoop(p,dl);

	    /* Declaration */
	    get_nodevalue_sym(ndcl)->symdims++;
	    bound = make_node(op_bounds);
	    Lines = ndcl->nodesequence;
	    bexpr1 = CopyTree(loop->nodechild->nodenext,false);
	    make_tree_constant(bexpr1, petit_args.lowerdefault);
	    PlaceNodeBelow(bexpr1, bound);
            PlaceShadow(bexpr1); //my fix - Tatiana 4/22/95
	    bexpr2 = CopyTree(loop->nodechild->nodenext->nodenext,false);
	    make_tree_constant(bexpr2, petit_args.upperdefault);
	    PlaceNodeAfter(bexpr2, bexpr1);
            PlaceShadow(bexpr2); //my fix - Tatiana 4/22/95
	    if(is_array_ref(p)) {
	      for(b=ndcl->nodechild; b->nodenext; b=b->nodenext) ;
	      PlaceNodeAfter(bound, b);
	    } else if(is_scalar_ref(p)) {
	      PlaceNodeBelow(bound, ndcl);
	    } else {
	      ErrAssert("ZapVarStoreDeps: wrong reference");
	    }

	    /* References */
	    for(i=0; i<coco_no; i++) {
	      p = cocos[i];
	      if(is_array_ref(p)) {
		node *ix;
		for(ix=p->nodechild; ix->nodenext; ix=ix->nodenext) ;
		PlaceNodeAfter(make_index(loop->nodechild), ix);
	      } else if(is_scalar_ref(p)) {
		switch(p->nodeop) {
			case op_fetch: 
				p->nodeop = op_fetch_array;
				break;
                        case op_store: 
				p->nodeop = op_store_array;
				break;
                        case op_update:
				p->nodeop = op_update_array;
				break;
			default:
				break;
			}
		PlaceNodeBelow(make_index(loop->nodechild), p);
	      }
	    }

	    touched = 1;
	  }
	  NextDl:;
	}
        AfterDl:;
      }
    }
  }
}


/*
 * Initialize connected component stuff
 */
static
void InitCCstuff(node *dcl) {
  node *r1;
  conn_comp_no = 0;

  for(r1=dcl->nodelink; r1; r1=r1->nodelink)
    r1->sc_region = 0;

  Entry->sc_region = 0;
  ExitNode->sc_region = 0;
}


/*
 * Connect to CC neighboring nodes of the given node
 */
static
void ConnectNeighbours(node *r1) {
  ddnode *dd;

  for(dd=r1->nodeddout; dd; dd=dd->ddnextsucc) {
    if(!ddisDead(dd) && is_flow_dep(dd)) {
      node *r2 = dd->ddsucc;
      Assert(r1==Entry || r1==ExitNode || r2==Entry || r2==ExitNode ||
	     get_nodevalue_node(r2)==get_nodevalue_node(r1),
	     "ConnectNeighbours: bad links #1");
      if(r2->sc_region==0) {
	r2->sc_region = r1->sc_region;
	Assert(coco_no<MAXCOCONO, "ConnectNeighbours: too many nodes in CC");
	cocos[coco_no++] = r2;
	ConnectNeighbours(r2);
      }
    }
  }

  for(dd=r1->nodeddin; dd; dd=dd->ddnextpred) {
    if(!ddisDead(dd) && is_flow_dep(dd)) {
      node *r2 = dd->ddpred;
      Assert(r1==Entry || r1==ExitNode || r2==Entry || r2==ExitNode ||
	     get_nodevalue_node(r2)==get_nodevalue_node(r1),
	     "ConnectNeighbours: bad links #2");
      if(r2->sc_region==0) {
	r2->sc_region = r1->sc_region;
	Assert(coco_no<MAXCOCONO, "ConnectNeighbours: too many nodes in CC");
	cocos[coco_no++] = r2;
	ConnectNeighbours(r2);
      }
    }
  }
}

/*
 * Rename a given connected component.
 * Return declaration of renamed CC
 * Templates for names must end with %d
 */
static
node *CopyRenameCoCo(node *dcl, char *name_template) {
  int i, nl;
  node *ndcl;
  symtabentry *ste;
  symtabentry *nste;
  int NewNameNo = 0;
  nametabentry *n;
  int variable_class;
  
  /* Don't rename CC which contains Entry or ExitNode */
  for(i=0; i<coco_no; i++) {
    node *p = cocos[i];
    if(p==Entry || p==ExitNode)
      return(dcl);
  }

  ste = get_nodevalue_sym(dcl);
  Lines = dcl->nodesequence;
  ndcl = CopyTree(dcl,false);
  ndcl->nodelink = NULL;
  ndcl->nodesequence = dcl->nodesequence;

  /* Create short and unique name */
  strcpy(P_IDname, ste->symname);
  nl = strlen(P_IDname);
 NextName:;
  sprintf(&P_IDname[nl], name_template, NewNameNo);
  if(NewNameNo==0) P_IDname[strlen(P_IDname)-1]=0;
  for(n=NameHead; n!=NULL; n=n->namenext) {
    node *d = n->namest.symdecl;
    if(d==NULL || d->nodeop!=op_declare)
      continue;
    if(strcmp(n->namest.symname,P_IDname)==0) {
      char *s = n->namest.symname+strlen(n->namest.symname);
      while(--s>=n->namest.symname && *s>='0' && *s<='9') ;
      NewNameNo = 0;
      while(*++s>='0' && *s<='9') NewNameNo = NewNameNo*10 + *s-'0';
      NewNameNo++;
      goto NextName;
    }
  }

  nste = parse_symbol(2);
  nste->symtype = ste->symtype;
  nste->symclass = CLAUTO;
  nste->symdims = ste->symdims;
  nste->symdecl = ndcl;
  nste->exptype = ste->exptype;
  add_omega_var(nste);
  ndcl->nodevalue = (unsigned long int) nste;
  PlaceNodeAfter(ndcl,dcl);
  
  /* Change references in CC to new variable, find out the class of var. */
  variable_class=CLAUTO;
  for(i=0; i<coco_no; i++) {
    node *p = cocos[i];
    DelRefList(p,1);
    p->nodevalue = (unsigned long int) ndcl;
    AddRefList(p);
  }

  touched = 1;
  return(ndcl);
}


/*
 * Array privatization
 */
int Privatization(int print) {
  nametabentry *n, *nn;
  touched = 0;
  for(n=NameHead; n!=NULL; n=nn) {
    node *dcl = n->namest.symdecl;
    node *r1;

    nn = n->namenext;
    if(dcl==NULL || dcl->nodeop!=op_declare ||
       n->namest.symdims==0 || n->namest.symclass==CLBUILTIN)
      continue;
	
    InitCCstuff(dcl);
    
    for(r1=dcl->nodelink; r1; r1=nr1) {
      nr1 = r1->nodelink;
      if(InDecl(r1) || is_phi_access(r1)) continue;
      Assert(r1==Entry || r1==ExitNode || get_nodevalue_node(r1)==dcl,
	     "Privatization: wrong links");
      if(r1->sc_region==0) {
	Var_Id loop_var;
	int dl, i;
	node *ndcl = dcl;
	symtabentry *nste = get_nodevalue_sym(ndcl);

	r1->sc_region = ++conn_comp_no;
	coco_no = 0;
	cocos[coco_no++] = r1;
	ConnectNeighbours(r1);
	loop_var = 0;
	
	/* Now we have a list of nodes for one CC */
	/* Look if this CC has 0 dependence differences at some level dl */
	for(dl=1; dl<=maxnest; dl++) {
	  int isub;

	  for(i=0; i<coco_no; i++) {
	    ddnode *dd;
	    node *p = cocos[i];
	    node *lp;

	    /* Don't privatize globally visible variables */
	    if(p==Entry || p==ExitNode) goto AfterDl;

	    lp = loop_containing(p);
	    if(lp==Entry || (int)lp->nodevalue<dl) goto AfterDl;
	    
	    for(dd=p->nodeddout; dd; dd=dd->ddnextsucc) {
	      if(!ddisDead(dd) && dd->ddsucc->sc_region==r1->sc_region){
		if(dl > (int)dd->ddnest) goto AfterDl;
		if(is_flow_dep(dd) && dd->dddiff[dl]!=0) goto NextDl;
	      }
	    }
	  }

	  /* All flow/reduction dependencies have diff. 0 at level dl */
	  /* Look for subscripts to be eliminated */
	  loop_var = get_nodevalue_sym(EmbracingLoop(cocos[0],dl)->nodechild);
	  for(isub=0; isub<(int)nste->symdims; isub++) {
	    node *sub;
	    int ixtype;

	    sub = GetSub(cocos[0],isub);
	    if(!node_is_affine(sub))
	      goto NextIsub;

	    ixtype = GoodIndexes(sub->nodeaffine,loop_var);
	    if(ixtype==0)
	      goto NextIsub;

	    for(i=1; i<coco_no; i++) {
	      node *p = GetSub(cocos[i],isub);
	      if(!(node_is_affine(p) &&
		 GoodIndexes(p->nodeaffine,loop_var) &&
		 CmpAffineExprs(p->nodeaffine,sub->nodeaffine)==0))
		goto NextIsub;
	    }

	    /* === We can eliminate a subscript here === */
	    /* Rename expanded CC first to avoid clashes with other CCs */
	    if(ndcl==dcl) {
	      ndcl = CopyRenameCoCo(dcl,"_p%d");
	      nr1=dcl->nodelink;
	      nste = get_nodevalue_sym(ndcl);
	    }
	    
	    /* Remove the subscript from declaration */
	    nste->symdims--;
	    nste->symclass = CLPRIVATE;
	    RemoveNode(GetSub(ndcl,isub),false);
	    
	    /* Remove the subscript from all references */
	    for(i=0; i<coco_no; i++) {
	      node *p = cocos[i];
	      RemoveNode(GetSub(p,isub),false);
	      if(nste->symdims==0) {
		switch(p->nodeop) {
			case op_fetch_array: 
				p->nodeop = op_fetch;
				break;
                        case op_store_array: 
				p->nodeop = op_store;
				break;
                        case op_update_array:
				p->nodeop = op_update;
				break;
			default:
				break;
			}
		}	
	    }
	    
	    touched = 1;
	    isub--;

	    NextIsub:;
	  }
	  Assert(GetSub(cocos[0],isub)==NULL, "Privatization: wrong sub-s");

	  /* If no privatizer-carrying subscript was deleted then
	   * stop privatization for this CC */
	  NextDl:;
	}

	/* Here we completed privatizing CC */
	/* Now move declaration of private variable to the right loop */
        AfterDl:;
	if (loop_var) {
	  RemoveNode(ndcl,-2);
	  PlaceNodeBefore(ndcl, loop_var->symdecl->nodechild);
	}
      }
    }
  }

  
  if(touched) {
    //fprintf(DebugFile,"After Privatization:\n");
    //debug_petit(Entry,1);
    re_analyse(print);
  }
 
  if(print)
    print_petit(Entry, 1);

  return(0);
}

/*
 * Return 1 : ae is constant
 *        2 : ae is loop variable currently being privatized
 *        0 : ae is none of above
 */
static
int GoodIndexes(affine_expr *ae, Var_Id petit_var) {
  switch(ae->nterms) {
  case 1:  return(1);
  case 2:  return(ae->terms[1].petit_var==petit_var ? 2 : 0);
  default: return(0);
  }
}

} // end of namespace omega
