/* $Id: ivr-lang-interf.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

/************************************************************
 *                                                          *
 * Language Interface for IVR and Forward Substitution (FS) *
 * System-dependent part.                                   *
 * Written by Vadim Maslov vadik@cs.umd.edu 01/03/93.       *
 *                                                          *
 ************************************************************/

#include <stdio.h>
#include <string.h>
#include <basic/bool.h>
#include <petit/ivr-lang-interf.h>
#include <petit/ssa.h>
#include <petit/ivr.h>
#include <petit/tree.h>

namespace omega {

/*
 * Go down at SSA graph using petit AST.
 * --- supplied to IVR lang-interf.
 */
node *ssa_source(node *v, void ** /*wptr*/) { // wptr unused
  node *w;
  switch(v->nodeop) {
  case op_fetch:
    w = v->ud.def_node;
    if(w && is_assign(w)) 
      w = w->nodechild->nodenext;
    break;
  case op_store:
  case op_update:
    w = v->nodeprev;
    break;
  case op_entry:
    w = NULL;
    break;
  default:
    w = v->nodechild;
    break;
  }
  if(w && w->shadow)  
    w = w->shadow;
  return(w);
}

/*
 * Go right from node v at SSA graph using petit AST.
 * Originally we went down from node v0.
 * --- supplied to IVR lang-interf.
 */
node *ssa_next_source(node *v, node *v0, void ** /*wptr*/) { // wptr unused
  node *w;
  /* If node was freed by IVR, stop */
  switch(v0->nodeop) {
  case op_fetch:
  case op_store:
    w = NULL;
    break;
  case op_update:
    if(v->nodenext && v->nodenext->nodeop==op_update) {
      w = v0->ud.def_node;
      if(w && is_assign(w))
	w = w->nodechild->nodenext;
    } else {
      w = NULL;
    }
    break;
  default:
    w = v->nodenext;
    break;
  }
  if(w && w->shadow)
    w = w->shadow;
  return(w);
}

/*
 * Go down at SSA graph for remove.
 * Is not likely to work outside IVR/FS, see zap_use_nodes_list
 * --- supllied to lang-interf.
 */
node *ssa_sink(node *v, void **wptr) {
  if(v->nodeop==op_store || v->nodeop==op_update || v==Entry) {
    USE_NODES un = v==Entry ? v->ud.use_nodes : v->nodeparent->ud.use_nodes;
    *wptr = (void*) un;
    return (node*) (un==NULL ? NULL : un->use_node);
  } else if(v->nodeop==op_exit) {     /* op_exit uses final values */
    return(NULL);
  } else {
    node *w=v->nodeparent;
    if(is_assign(w))
      w=v->nodenext;
    else if(!is_expr(w))
      w=NULL;
    return(w);
  }
}

/*
 * Go right at SSA graph for remove.
 * Go from node v keeping in mind that v0 is node from which we went down.
 * --- supllied to lang-interf.
 */
node *ssa_next_sink(node *v, node *v0, void **wptr) {
  if(v0->nodeop==op_store || v0->nodeop==op_update || v==Entry) {
    USE_NODES un = (*((USE_NODES *) wptr))->next;
    *wptr = (void*) un;
    return (node *) (un ? un->use_node : NULL);
  } else {
    return(NULL);
  }
}

/*
 * Place new statement after given statement.
 * Set the correct line number,
 * and make op_stmt_number needed by Wayne's stuff.
 */
void place_new_stmt_after(pEXPR new_stmt, pEXPR old) {
  node *psn;

  PlaceNodeAfter(new_stmt, old);
  SetNodeSeq(new_stmt, ++Lines);

  psn = MkStmtNumber(new_stmt);
  PlaceNodeBefore(psn, new_stmt);
}

/*
 * Make assignment to the given scalar variable
 */
pSTMT make_scalar_assgmt(pEXPR lp, EXPR_OPER /*op*/, pEXPR rp) { // op unused
  node *np = make_op(rp, op_assign, lp);
  if(rp->shadow) {
    rp->shadow->nodeparent = rp->nodeparent;

//    rp->shadow->nodenext = rp->nodenext;
    rp->shadow->nodeprev = rp->nodeprev;
  }
  lp->ud.def_node = np;
  du_ind_loop[LookVar(lp)].def_node = np;
  return(np);
}

/*
 * Make a scalar store operation for given variable
 */
pEXPR make_scalar_store(Var_Id var) {
  pEXPR rp = make_node(op_store);    
  rp->nodevalue = (unsigned long int)var->symdecl;
  AddRefList(rp);
  AddSSAgraph(rp); 
  return(rp);
}

/*
 * Make a scalar fetch operation for given variable
 */
pEXPR make_scalar_fetch(Var_Id var) {
  pEXPR rp = make_node(op_fetch);    
  rp->nodevalue = (unsigned long int)var->symdecl;
  AddRefList(rp);
  AddSSAgraph(rp);
  AddEdgeSSAgraph(rp);
  return(rp);
}

/*
 * Turn update to store and += to :=
 */
void turn_update_to_store(pEXPR p) {
  p->nodeparent->nodeop = op_assign;
  DelUseFromLists(p);
  p->nodeop = op_store;
}

/*
 * Turn store to update and := to += (or *=)
 */
void turn_store_to_update(pEXPR n, EXPR_OPER op, pEXPR fetch) {
  n->nodeparent->nodeop = op;
  if(n->nodeop==op_store) {
    n->nodeop = op_update;
    n->ud.def_node = fetch->ud.def_node;
    if(fetch->ud.def_node)
      AddUseNodesList(&fetch->ud.def_node->ud.use_nodes, n);
  } else if(n->nodeop==op_store_array) {
    n->nodeop = op_update_array;
  } else  {
    ErrAssert("turn_store_to_update: wrong op in assignment");
  }
}

/*
 * Place shadow expression sh to node e.
 */
void place_shadow_expr(pEXPR sh, pEXPR e) {
  e->shadow = sh;
  sh->nodeparent = e->nodeparent;
  /* do we really need these two ? */
  sh->nodeprev   = e->nodeprev;
  sh->nodenext   = e->nodenext;
}


/*
 * Traverse program statements (depth first).
 * Call given function at each node.
 */
void traverse_stmts(pSTMT ap, void (*func) (pSTMT)) {
  pSTMT np;
  for(; ap!=NULL; ap=np) {
    np = ap->nodenext;
    switch(ap->nodeop) {
    case op_if:
      { node *p = ap->nodechild->nodenext;
	traverse_stmts(p->nodechild,func);              /* THEN branch */
	if(p->nodenext)
	  traverse_stmts(p->nodenext->nodechild,func);  /* ELSE branch */
      }
      break;

    CASE_DO:
      traverse_stmts(ap->nodechild->nodechild,func);    /* loop body */
      break;

    case op_entry:
      traverse_stmts(ap->nodechild,func);               /* program body */
      break;

    default:
      break;
    }

    (*func)(ap);
  }
}


/*
 * Traverse an expression (depth first).
 * Call given function at each node.
 */
void traverse_expr1(pEXPR e, void (*func) (pEXPR)) {
  pEXPR p, np;
  if(e) {
    for(p=e->nodechild; p; p=np) {
      np = p->nodenext;
      traverse_expr1(p,func);
    }
    (*func)(e);
  }
}
/*
 * Traverse an expression (depth first) and SHADOW expressions
 * Call given function at each node.
 */
void traverse_expr1_shadow(pEXPR e, void (*func) (pEXPR)) {
  pEXPR p, np;
  if(e) {
    for(p=e->nodechild; p; p=np) {
      np = p->nodenext;
      traverse_expr1_shadow(p,func);
    }
    if (e->shadow)
        traverse_expr1_shadow(e->shadow,func);
    (*func)(e);
  }
}

/*
 * Traverse all expressions in a program.
 * Call given function at each node.
 */
void traverse_exprs(pSTMT ap, void (*func) (pEXPR)) {
  pSTMT np;
  for(; ap!=NULL; ap=np) {
    np = ap->nodenext;
    switch(ap->nodeop) {
    case op_if:
      { node *p = ap->nodechild->nodenext;
	traverse_expr1(ap->nodechild,func);             /* condition */
	traverse_exprs(p->nodechild,func);              /* THEN branch */
	if(p->nodenext)
	  traverse_exprs(p->nodenext->nodechild,func);  /* ELSE branch */
      }
      break;

    CASE_DO:
      { node *p = ap->nodechild;
	traverse_expr1(p->nodenext,func);               /* loop parameters */
	traverse_expr1(p->nodenext->nodenext,func); 
	traverse_expr1(p->nodenext->nodenext->nodenext,func); 
	traverse_exprs(p->nodechild,func);              /* loop body */
      }
      break;

    case op_entry:
      traverse_exprs(ap->nodechild,func);               /* program body */
      break;

    case op_declare: 
    case op_comment:
    case op_exit:
      break;

    case op_stmt_number:
      traverse_expr1(ap->nodechild,func); 
      break;

    case op_assert:
      traverse_expr1(ap->nodechild,func);               /* assertion */
      break;

    CASE_ASSIGN:
      { node *p;
	for(p=ap->nodechild; p; p=p->nodenext) 
	  traverse_expr1(p,func);  
      }
      break;

    default:
      ErrAssert("traverse_exprs: expr op in statements");
    }
  }
}

/*
 * Traverse all expressions in a program including SHADOW!
 * Call given function at each node.
 */
void traverse_exprs_shadow(pSTMT ap, void (*func) (pEXPR)) {
  pSTMT np;
  for(; ap!=NULL; ap=np) {
    np = ap->nodenext;
    switch(ap->nodeop) {
    case op_if:
      { node *p = ap->nodechild->nodenext;
	traverse_expr1_shadow(ap->nodechild,func);             /* condition */
	traverse_exprs_shadow(p->nodechild,func);              /* THEN branch */
	if(p->nodenext)
	  traverse_exprs_shadow(p->nodenext->nodechild,func);  /* ELSE branch */
      }
      break;

    CASE_DO:
      { node *p = ap->nodechild;
	traverse_expr1_shadow(p->nodenext,func);               /* loop parameters */
	traverse_expr1_shadow(p->nodenext->nodenext,func); 
	traverse_expr1_shadow(p->nodenext->nodenext->nodenext,func); 
	traverse_exprs_shadow(p->nodechild,func);              /* loop body */
      }
      break;

    case op_entry:
      traverse_exprs_shadow(ap->nodechild,func);               /* program body */
      break;

    case op_declare: 
    case op_comment:
    case op_exit:
      break;

    case op_stmt_number:
      traverse_expr1_shadow(ap->nodechild,func); 
      break;

    case op_assert:
      traverse_expr1_shadow(ap->nodechild,func);               /* assertion */
      break;

    CASE_ASSIGN:
      { node *p;
	for(p=ap->nodechild; p; p=p->nodenext) 
	  traverse_expr1_shadow(p,func);  
      }
      break;

    default:
      ErrAssert("traverse_exprs: expr op in statements");
    }
  }
}

int OldStyleAffineExprs = false;

/*
 * Find symbol table entry for a variable access.
 * Treat each definition of scalar as a separate variable.
 */
Var_Id get_ssa_scalar_id(pEXPR e) {
  nametabentry *nte;
  symtabentry *ste = get_nodevalue_sym(get_nodevalue_node(e));
  node *def_node;
  char name[80];

  if(ste->symdecl->nodeop!=op_declare) {
    return(ste);
  }

  def_node = e->ud.def_node;
  if(OldStyleAffineExprs) {
    Assert(def_node==NULL,
	   "get_ssa_scalar_id: OldStyleAffineExprs and def_node!=NULL");
    return(ste);
  } else {
    if(def_node==NULL) {
      ErrAssert("GetSymPhi: !OldStyleAffineExprs and def_node==NULL");
      def_node = Entry;
    }
  }

  /*
   * All entries for instances of variable go after entry for
   * original variable, to save search time.
   */
  for(nte=ste->nte->namenext; nte && nte->namest.original; nte=nte->namenext) {
    if(nte->namest.original==ste && nte->namest.def_node==def_node) {
      return(&nte->namest);
    }
  }
  
  sprintf(name, "%s%s", ste->symname, str_var_index(e));
  nte = CopySymTabEntry(ste, name);
  nte->namest.symtag = UNTAGGED;
  nte->namest.def_node = def_node;
  nte->namest.symdecl = NULL;
  nte->namest.original = ste;

  return(&nte->namest);
}


/*
 * Makes sym_con_var in var_id to tell whether (for a given reference)
 * given variable is symbolic constant (sym_con_var=TRUE)
 * or 2-instance variable (sym_con_var=FALSE).
 *
 * Algorithm: for def and use find outermost loop which is not constant
 * and which surrounds them. Actually record loop/stmt one level deeper.
 * If this loops are different then it's a symbolic constant.
 *
 * Can change status only from UNTAGGED to SymbConst or ChVar,
 * or from SymbConst or ChVar, not any other way.
 */
void tag_var_as_changeable_or_constant(Var_Id var, pEXPR e, int leading_0s) {
  if(var->symtag==UNTAGGED || var->sym_con_var==true) {
    node *n_d, *n_u, *nn;
    
    n_d = var->def_node;

    /* For zapping (or whatever) we want to have symbolic constants old way */
    if(leading_0s<0 && n_d!=NULL && n_d!=Entry) {
      var->sym_con_var = false;
      return;
    }
    
    while(n_d!=Entry && (nn=loop_containing(n_d))!=Entry &&
	  (int)nn->nodevalue>leading_0s)
      n_d = nn;

    n_u = e;
    while(n_u!=Entry && (nn=loop_containing(n_u))!=Entry &&
	  (int)nn->nodevalue>leading_0s)
      n_u = nn;
    
    var->sym_con_var = (!(is_do(n_d) && is_do(n_u) && n_d==n_u));
  }
}

} // end omega namespace

/* go through the parse tree and call get_ssa_scalar_id for every use
   of the scalar
 */  
