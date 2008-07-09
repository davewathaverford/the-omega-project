/* $Id: ssa.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

/*********************************************************************
 *                                                                   *
 * Static Single Assignment calculation, and IVR/FS calls.           *
 * Written by Vadim Maslov vadik@cs.umd.edu 08/15/92.                *
 *                                                                   *
 *********************************************************************/

#include <basic/bool.h>
#include <petit/ssa.h>
#include <petit/ivr.h>
#include <petit/vutil.h>
#include <petit/tree.h>
#include <petit/ops.h>
#include <petit/make.h>
#include <petit/print.h>
#include <petit/debug.h>
#include <petit/Exit.h>
#include <petit/definitions.h>
#include <petit/petit_args.h>

namespace omega {


static STE vars[MAXVARNO];	       /* array of variables used in SSA */
VARNO var_no;		       /* number of variables in this array */

var_def_use *du_ind_loop;

static int ivr_print;


static void      zap_use_nodes_list(pSTMT);
static VDU       bld_sub_ssa(node *);

static void      ResetVdu(VDU *);
static void      UnionUses(var_def_use *, var_def_use *);
static void      DefUses(USE_NODES, node *);
static node     *MakePhi(STE, node *, node *, node *);
static node     *MakeNode(optype,node *,node *,node *,node *);
static void      PrintDU(char *, var_def_use *);
static void      SubsShadows(pEXPR);
static void      ElimShadowShadow(pEXPR);
static void      SetDefaultClasses(void);


/*
 * Build Static Single Assignment Graph from AST
 */


void build_ssa(void) 
    {
    var_no = 0;
    ivr_print = false;

    InitSSAgraph();
    bld_sub_ssa(Entry);

    if(petit_args.ivr_DefEntryClass)
	SetDefaultClasses();
    
    if(petit_args.ivr_ShadowShadow)
	traverse_exprs(Entry, &SubsShadows);

    if(petit_args.ivr_RemoveNoOp)
	NoOpListDelete();

    traverse_stmts(Entry, &zap_use_nodes_list);

    }



/* Skip recognizing reductions first time through if array expansion is on --
   it tends to interfere.  Later, after array expansion, we force a re_analyse
   so that reductions get recognized. */
void recognize_reductions(bool first_time) {
    if(petit_args.makeReductionOps && (!petit_args.doArrayExpn || !first_time))
	traverse_stmts(Entry, &RecUpdate);
}


/*
 * Sets classes for scalar variables, based on SSA graph
 */
static 
void SetDefaultClasses(void) 
    {
    USE_NODES un;
    for(un=Entry->ud.use_nodes; un; un=un->next) 
	{
	if(!DefinedAtBegin(un->use_node)) 
	    {
	    symtabentry *sym =
		get_nodevalue_sym(get_nodevalue_node(un->use_node));
	    switch(sym->symclass) 
		{
	    case CLOUT:
		sym->symclass = CLINOUT;
		break;
	    case CLunknown:
	    case CLAUTO:
		sym->symclass = CLIN;
		break;
	    case CLPRIVATE:
		Warning("bld_sub_ssa: private variable used outside its scope");
		break;
	    default:
		ErrAssert("SetDefaultClasses: object of wrong class encountered");
		break;
		}
	    }
	}
    }

/*
 * This is called once after building SSA graph to get rid of
 * assignment use lists. They are needed only for dead stmt
 * elimination.
 */
static
void zap_use_nodes_list(pSTMT p) 
    {
    if(p==Entry || is_assign(p)) 
	{ 
	USE_NODES un, nun;
	for(un=p->ud.use_nodes; un; un=nun) 
	    {
	    /* we don't null def_node here because we need it for dep. testing */
	    nun=un->next;
	    delete un;
	    }
	p->ud.use_nodes = NULL;
	}
    }

/*
 * Build Static Single Assignment Graph directly
 * from petit Abstract Syntax Tree @ pointer ap.
 * At each step we do computation (stmt1, stmt2) -> stmt1.
 * Call induction variable recognition for DO loops.
 */
static
VDU bld_sub_ssa(node *ap) 
    {
    VDU vdu1;        /* here we collect effect of sequence of AST statements */
    VARNO vno;
    node *apnext;
    ResetVdu(&vdu1);
    for(; ap!=NULL; ap=apnext) 
	{
	VDU vdu2;	   /* here we collect effect of one AST statement */
	apnext = ap->nodenext;
	ap->ud.def_node = NULL;
	AddSSAgraph(ap);
	switch(ap->nodeop) 
	    {
	case op_if:
	    {
	    VDU vdu3, vdu4;
	    node *p = ap->nodechild->nodenext;
	    vdu2 = bld_sub_ssa(ap->nodechild);            /* condition */
	    vdu3 = bld_sub_ssa(p->nodechild);             /* THEN branch */
	    if(p->nodenext) 
		{
		vdu4 = bld_sub_ssa(p->nodenext->nodechild); /* ELSE branch */
		}
	    else 
		{
		ResetVdu(&vdu4);
		}
	    for(vno=0; vno<var_no; vno++) 
		{
		var_def_use *du2 = &vdu2.a[vno];
		var_def_use *du3 = &vdu3.a[vno];
		var_def_use *du4 = &vdu4.a[vno];
		UnionUses(du2, du3);
		UnionUses(du2, du4);
		if(du3->def_node && du4->def_node) 
		    {
		    node *phi = MakePhi(vars[vno], du3->def_node, du4->def_node, ap);
		    PlaceNodeAfter(phi, ap);
		    ap = ap->nodenext;	                  /* to bypass inserted phi */
		    du2->def_node = phi;
		    }
		else if(du3->def_node || du4->def_node) 
		    {
		    node *dnode = du4->def_node ? du4->def_node : du3->def_node;
		    node *phi = MakePhi(vars[vno], NULL, dnode, ap);
		    PlaceNodeAfter(phi, ap);
		    ap = ap->nodenext;	                  /* to bypass inserted phi */
		    du2->def_node = phi;
		    AddUseNodesList(&du2->use_nodes, phi->nodechild->nodechild);
		    }
		}
	    }
	    break;

	CASE_DO:
	    {
	    VDU vdu3;
	    node *p;
	    int ssag_start;

	    ResetVdu(&vdu2);
	    for(p=ap->nodechild->nodenext; p; p=p->nodenext) 
		{
		vdu3 = bld_sub_ssa(p);                /* from, to, step */
		for(vno=0; vno<var_no; vno++)
		    UnionUses(&vdu2.a[vno], &vdu3.a[vno]);
		}
	    ssag_start = no_ssag_nodes;
	    p = ap->nodechild->nodechild;
	    vdu3 = bld_sub_ssa(p);                  /* loop body */
	    for(vno=0; vno<var_no; vno++) 
		{
		node *def_node = vdu3.a[vno].def_node;
		if(def_node) 
		    {
		    node *decl = get_nodevalue_node(def_node->nodechild->nodenext);
		    symtabentry *sym = get_nodevalue_sym(decl);
		    /* private variables don't let var. value to get out of loop */
		    if(!(sym->symclass==CLPRIVATE &&
			 decl->nodeparent && decl->nodeparent->nodeparent==ap)) 
			{
			node *phi = MakePhi(vars[vno], NULL, def_node, ap);
			PlaceNodeBefore(phi, p);
			p = phi;                     /* to insert next phi before this */
			vdu2.a[vno].def_node = phi;
			AddUseNodesList(&vdu2.a[vno].use_nodes,
					phi->nodechild->nodechild);
			DefUses(vdu3.a[vno].use_nodes, phi);
			}
		    }
		else 
		    {
		    UnionUses(&vdu2.a[vno], &vdu3.a[vno]);
		    }
		}

	    du_ind_loop = &vdu2.a[0];
	    if(petit_args.ivr_debug)
		PrintDU("Just before IVR:", du_ind_loop);
	    DoIvrFs(ap, ssag_start);
	    du_ind_loop = NULL;
	    }
	    break;

	case op_assert:
	    vdu2 = bld_sub_ssa(ap->nodechild);        /* assertion */
	    break;

	case op_entry:
	    vdu2 = bld_sub_ssa(ap->nodechild);        /* program body */

	    /* Initial values are defined by Entry node */
	    for(vno=0; vno<var_no; vno++) 
		{
		USE_NODES un = vdu2.a[vno].use_nodes;
		if(un) 
		    {
		    DefUses(un, Entry);
		    vdu2.a[vno].use_nodes = NULL;
		    /* after elimination of dead statements we may reassign
		       classes for variables which don't have definition */
		    if(!DefinedAtBegin(un->use_node) && !petit_args.ivr_DefEntryClass) 
			{
			char sss[80];
			symtabentry *sym =
			    get_nodevalue_sym(get_nodevalue_node(un->use_node));
			sprintf(sss, "variable `%s' is used before it's defined\n",
				sym->symname);
			Warning(sss);
			}
		    }
		}

	    /* Final values are used by ExitNode */
	    for(vno=0; vno<var_no; vno++) 
		{
		node *dn = vdu2.a[vno].def_node;
		if(dn && UsedAtEnd(dn->nodechild->nodenext)) 
		    {
		    AddUseNodesList(&vdu2.a[vno].def_node->ud.use_nodes, ExitNode);
		    }
		}

	    du_ind_loop = &vdu2.a[0];
	    if(petit_args.ivr_debug) PrintDU("Just before IVR at top level:", du_ind_loop);
	    DoIvrFs(NULL, 0);
	    du_ind_loop = NULL;

	    /* Free use nodes lists to maintain purity of petit */
	    for(vno=0; vno<var_no; vno++)
		FreeUseNodeList(vdu2.a[vno].use_nodes);

	    return(vdu2);

	case op_declare: 
	case op_stmt_number:
	case op_comment:
	case op_exit:
	    continue;
	    break;

	case op_store:
	    ap->ud.def_node = ap->nodeparent;
	    vno = LookVar(ap);
	    vdu1.a[vno].def_node = ap->nodeparent;
	    return(vdu1);

	case op_index:       /* loop variables are not uses */
	    return(vdu1);

	case op_fetch:
	    vno = LookVar(ap);
	    AddUseNodesList(&vdu1.a[vno].use_nodes, ap);
	    return(vdu1);

	case op_update:	 /* var op= expr: both def and use */
	    ap->ud.def_node = ap->nodeparent;
	    vno = LookVar(ap);
	    vdu1.a[vno].def_node = ap->nodeparent;
	    AddUseNodesList(&vdu1.a[vno].use_nodes, ap);
	    return(vdu1);

	default:             /* assume that the node is expression operation */
	    {
	    node *p;
	    Assert(is_expr(ap), "bld_sub_ssa: expression op expected");
	    ResetVdu(&vdu2);
	    for(p=ap->nodechild; p; p=p->nodenext) 
		{
		VDU vdu3 = bld_sub_ssa(p);
		for(vno=0; vno<var_no; vno++) 
		    {
		    UnionUses(&vdu2.a[vno], &vdu3.a[vno]);
		    if(vdu3.a[vno].def_node) vdu2.a[vno].def_node=vdu3.a[vno].def_node;
		    }
		}
	    }
	    if(!is_assign(ap)) 
		{
		return(vdu2);
		}
	    break;
	    }

	if(petit_args.ivr_debug) 
	    {
	    PrintPgm("Statement analyzed:", ap, false);
	    PrintDU("One Statement", &vdu2.a[0]);
	    }

	/* composition of two consecutive statements */
	for(vno=0; vno<var_no; vno++) 
	    {
	    if(vdu1.a[vno].def_node && vdu2.a[vno].use_nodes) 
		{
		DefUses(vdu2.a[vno].use_nodes, vdu1.a[vno].def_node);
		}
	    else 
		{
		UnionUses(&vdu1.a[vno], &vdu2.a[vno]);
		}
	    if(vdu2.a[vno].def_node) vdu1.a[vno].def_node=vdu2.a[vno].def_node;
	    }
	if(petit_args.ivr_debug) PrintDU("Stmt Sequence", &vdu1.a[0]);
	}
    return(vdu1);
    }



#define MAXPRINTNODES 500

/*
 * Print index of scalar variable (beauty aid)
 */
char *str_var_index(node *p) 
    {
    static struct 
    {
	unsigned long int  nodevalue;
	node   *def_node;
    } nodes[MAXPRINTNODES];
    static int nodeno;
    static char s[80];
    int i, index;
    node *def_node;
  
    if(!ivr_print) 
	{
	ivr_print=true;
	nodeno=0;
	}
    def_node = p->ud.def_node;
    if(def_node==Entry) 
	{
	return("_E");
	}
    if(def_node==NULL) 
	{
	return("_0");
	}
    index=1;
    for(i=0; i<nodeno; i++) 
	{
	if(nodes[i].nodevalue==p->nodevalue) 
	    {
	    if(nodes[i].def_node==def_node) break;
	    index++;
	    }
	}
    if(i>=nodeno) 
	{
	Assert(nodeno<MAXPRINTNODES, "str_var_index: too many nodes");
	nodes[nodeno].def_node=def_node;
	nodes[nodeno].nodevalue=p->nodevalue;
	nodeno++;
	}
    sprintf(s, "_%d", index);
    return(s);
    }

} // end omega namespace

/*
 * ==================== Utilities for SSA =================
 */

#include <string.h>

namespace omega {

/*
 * Reset variables use/def descriptor
 */
static
void ResetVdu(VDU *pvdu) 
    {
    memset((char*)pvdu, 0, sizeof(VDU));
    }

/*
 * Find variable in vars array.
 * Return variable index in vars or var_no if not found.
 */
VARNO FindVar(node *p) 
    {
    VARNO i;
    STE ste = get_nodevalue_sym(get_nodevalue_node(p));
    for(i=0; i<var_no; i++) 
	{
	if(vars[i] == ste) break;
	}
    return(i);
    }

/*
 * Locate variable in variable table.
 */
VARNO LookVar(node *p) 
    {
    VARNO vno = FindVar(p);
    if(vno<var_no) return(vno);
    Assert(var_no <= MAXVARNO, "LookVar: too many variables");
    vars[var_no] = get_nodevalue_sym(get_nodevalue_node(p));
    return(var_no++);
    }

/*
 * Insert to use nodes list for one variable given element
 */
void AddUseNodesList(USE_NODES *pun, node *use_node) 
    {
    USE_NODES un = new use_nodes;
    if(petit_args.ivr_debug) fprintf(debug, "+++AddUseNodesList: p=%p\n", use_node);
    un->use_node = use_node;
    un->next = *pun;
    *pun = un;
    }

/*
 * Make union for two uses
 */
static
void UnionUses(var_def_use *pto, var_def_use *pfr) 
    {
    if(pfr->use_nodes) 
	{
	USE_NODES *pun;
	for(pun = &pto->use_nodes; *pun; pun = &((*pun)->next)) ;
	*pun = pfr->use_nodes;
	}
    }

/*
 * Make every use in given list un reference def_node,
 * and def_node have lists referencing all its uses.
 */
static
void DefUses(USE_NODES un, node *def_node) 
    {
    USE_NODES *pun;
    for(pun = &def_node->ud.use_nodes; *pun; pun = &(*pun)->next) ;
    *pun = un;
    for(; un; un=un->next) 
	{
	un->use_node->ud.def_node = def_node;
	}
    }

/*
 * Make phi-function
 */
static
node *MakePhi(STE ste, node *arg1, node *arg2, node *phi_parent) 
    {
    node *symdecl = ste->symdecl;
    node *np, *sp;
    Lines++;
    np = MakeNode(op_fetch,NULL,NULL,symdecl,arg2);
    np = MakeNode(op_fetch,NULL,np,symdecl,arg1);
    sp = MakeNode(op_store,NULL,NULL,symdecl,NULL);
    np = MakeNode(op_phi,np,sp,NULL,NULL);
    np->ud.phi_parent = phi_parent;
    np = MakeNode(op_assign,np,NULL,NULL,NULL);
    sp->ud.def_node = np;
    return(np);
    }

/*
 * Make AST node with child, next, value, def_node references.
 * Add next and parent references to what lies below currently created node.
 * SSA specilal: add node to SSA graph nodes list.
 */
static
node *MakeNode(optype op, node *child, node *next, node *value, node *def_node)
    {
    node *np = make_node(op);
    if(petit_args.ivr_debug)
	fprintf(debug, "+++MakeNode: np=%p op=%s\n", np, get_op_name(op));
    np->nodechild = child;
    if(child) 
	{
	node *p;
	for(p=child; p; p=p->nodenext) p->nodeparent = np;
	}
    np->nodenext = next;
    if(next) next->nodeprev = np;
    np->nodevalue = (unsigned long int)value;
    np->ud.def_node = def_node;
    if(def_node) AddUseNodesList(&def_node->ud.use_nodes, np);
    AddSSAgraph(np);
    AddRefList(np);
    return(np);
    }

/*
 * Print def-use list (beauty aid)
 */
static 
void PrintDU(char *t, var_def_use *pvdu) 
    {
    VARNO vno;
    fprintf(debug, ">>>DefUse list: %s\n", t);
    for(vno=0; vno<var_no; vno++) 
	{
	var_def_use *du = &pvdu[vno];
	USE_NODES un;
	fprintf(debug, ">>>Var=%s: def_node=%p  use_nodes=", vars[vno]->symname, du->def_node);
	for(un=du->use_nodes; un; un=un->next)  fprintf(debug, "%p ", un->use_node);
	fprintf(debug, "\n");
	}
    fprintf(debug, "\n");
    }


/*
 * Remove phi-functions
 */
static
void RmPhis(pSTMT p) 
    {
    if(p->nodeop==op_assign && p->nodechild->nodeop==op_phi) 
	{
	/* Vadim's comment - 
           we already killed use lists, so don't bother 
           about SSA graph anymore */
        
	RemoveNode(p,false);
	}
    }

/*
 * Delete a shadow node if it exists
 */

static void DeleteShadow(pEXPR p) {
 if (p->shadow) {
    FreeTree(p->shadow,1);
    p->shadow=NULL;
 } 
}  

/*
 * Zap SSA graph.
 * Part of zapping done at next cal to build_ssa: def_node, use_nodes
 */
void zap_ssa_graph(void) 
    {
    nametabentry **pname;
    traverse_exprs(Entry, &DeleteShadow); 
//  traverse_exprs_shadow(Entry,&FixDefNodes);
    traverse_stmts(Entry, &RmPhis);

    for(pname=&NameHead; *pname!=NULL; ) 
	{
	if((*pname)->namest.original) 
	    {
	    nametabentry *n = *pname;
	    *pname = (*pname)->namenext;
	    if(n->namest.symname)
		delete n->namest.symname;
	    delete n;
	    }
	else 
	    {
	    pname = &(*pname)->namenext;
	    }
	}
    }




/*
 * Substitute shadows of shadow expressions.
 */
static 
void SubsShadows(pEXPR e) 
    {
    if(e->shadow) 
	{
	traverse_expr1(e->shadow,ElimShadowShadow);
	}
    }

static 
void ElimShadowShadow(pEXPR e) 
    {
    if(e->shadow) 
	{
	traverse_expr1(e->shadow,ElimShadowShadow);
	PlaceNodeInstead(e->shadow,e);
	e->shadow = NULL;
	FreeTree(e,0);
	}
    }



static char * node_name (node *p) {
 if (p!=NULL)
    return get_op_name(p->nodeop);
 else
    return "";
}

void print_node_content(node *p, FILE * f, int level) {
   if (p==NULL)
      return;
   int i;
   fprintf(f,"\n");
   for (i=0;i<level*2; i++)
     fprintf(f," ");
   fprintf(f,"NODE %p - %s ", p, get_op_name(p->nodeop));
  if (is_mem_ref(p)) 
      fprintf(f,"Var %s ", (get_nodevalue_sym(get_nodevalue_node(p)))->symname);
    fprintf(f,"\n");
   for (i=0;i<level*2; i++)
     fprintf(f," ");
       

   fprintf(f,"prev %p(%s), next %p(%s), child %p(%s), parent %p(%s), link %p(%s)\n ",
      p->nodeprev, node_name(p->nodeprev), p->nodenext, node_name(p->nodenext),
      p->nodechild, node_name(p->nodechild), p->nodeparent, node_name(p->nodeparent),
      p->nodelink, node_name(p->nodelink));
   for (i=0;i<level*2; i++)
     fprintf(f," ");
        
   switch (p->nodeop) {
   case op_phi: fprintf(f, "Phi parent = %p (%s) ", p->ud.phi_parent,
                           node_name(p->ud.phi_parent));
                break;
   case op_assign:
       {
                fprintf(f, "Use nodes are: ");
                for (use_nodes *un=p->ud.use_nodes; un!=NULL; un=un->next)
                  fprintf(f, "%p(%s)  ", un->use_node, node_name(un->use_node));
                break;
       }
   CASE_MEMREF:
                fprintf(f, "Def node is: %p (%s) ", p->ud.def_node, 
                        node_name(p->ud.def_node));
                break;
   default:
                break;
   }
 }


void print_subtree_debug(node *p, FILE * f, int level) {
  if (p!=NULL) {
     print_node_content(p, f, level);
     if (p->shadow) {
       fprintf(f,"\nSHADOW-------------------------->\n");
       print_subtree_debug(p->shadow,f,level);
       fprintf(f,"\nEND_SHADOW<-----------------------\n"); 
     }
     for (node * c=p->nodechild; c; c=c->nodenext)
        print_subtree_debug(c,f,level+1);
 }
}  

void print_tree_debug() {
   print_subtree_debug(Entry, debug, 0);
 }
             
} // end of namespace omega
