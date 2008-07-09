/* $Id: ivr.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

/**********************************************************
 *                                                        *
 * Subroutines related to induction variable recognition. *
 * System-independent part.                               *
 * Written by Vadim Maslov vadik@cs.umd.edu 08/15/92.     *
 *                                                        *
 **********************************************************/

#include <stdio.h>
#include <basic/bool.h>
#include <basic/assert.h>
#include <basic/util.h>
#include <petit/lang-interf.h>
#include <petit/ivr.h>
#include <petit/debug.h>
#include <petit/definitions.h>
#include <petit/petit_args.h>

namespace omega {

static void      IvrAddToScc(pEXPR);
static void      SccToInd(pEXPR);
static void      replace_expr_with_another(pEXPR, pEXPR);



/* =========== Building SSA graph ============ */

#define MAXSSANODES 12000
pEXPR ssag_nodes[MAXSSANODES];  /* array of nodes in SSA graph */
int   no_ssag_nodes;	        /* number of nodes in this array */

/*
 * Initialize SSA graph
 */
void InitSSAgraph(void)    
    {
    no_ssag_nodes = 0;
    }


/*
 * Add node to the list of nodes in SSA graph.
 * Returns the argument.
 */
pEXPR AddSSAgraph(pEXPR p)
    {
    if(is_ssag_node(p)) 
	{
	if(no_ssag_nodes>=MAXSSANODES) 
	    {
	    int i, nulls=0;
	    for(i=0; i<no_ssag_nodes; i++)
		if(ssag_nodes[i]==NULL) nulls++;
	    fprintf(debug, "\nAddSSAgraph: nodes=%d NULL nodes=%d\n", i, nulls);
	    ErrAssert("AddSSAgraph: too many SSA graph nodes");
	    }
	if(petit_args.ivr_Assert) 
	    {
	    int i;
	    for(i=0; i<no_ssag_nodes; i++)
		Assert(ssag_nodes[i]!=p, "AddSSAgraph: duplicate reference");
	    }
	if(petit_args.ivr_debug) 
	    {
	    fprintf(debug, "+++AddSSAgraph: p=%p no_ssag_nodes=%d\n",
		    p, no_ssag_nodes);
	    }
	scc_pssag_list(p) = &ssag_nodes[no_ssag_nodes];
	ssag_nodes[no_ssag_nodes++] = p;
	}
    return(p);
    }

/*
 * Remove a node from SSA graph
 */
void RemoveSSAgraph(pEXPR p)
    {
    if(scc_pssag_list(p) && *scc_pssag_list(p)==p) 
	{
	*scc_pssag_list(p) = NULL;
	}
    }


/* ======= Induction Variable Recognition & Forward Substitution ====== */

/* loop for which we do IVR */
static pSTMT ind_loop;
static SCCNO sc_vrxno;       /* number of vertexes in SCC */


/* what to initialize for FindScc */
pEXPR pt_stack[MAXPTSTACK];  /* FindSCC: stack */
int   ps_i;		     /* FindSCC: stack position */
SCCNO vertex_no;             /* FindSCC: number of current vertex */
SCCNO sc_region;	     /* FindSCC: number of current SCC */

/* parameters for FindScc, invariant for all recursive calls */
pEXPR (*GoDownFun)(pEXPR, void**); 
pEXPR (*GoRightFun)(pEXPR, pEXPR, void**);
void  (*AddToScc)(pEXPR); 
void  (*ProcessScc)(pEXPR);
char   *FindSccTxt; 

#define MAXNOOPSTMTNO 300
static pSTMT NoOpStmts[MAXNOOPSTMTNO];
static int   NoOpStmtNo = 0;



/* 
 * Do induction variable recognition and forward substitution.
 * This function should be called for every loop and for outer level
 * starting from inner loops and going outside.
 */
void DoIvrFs(pSTMT loop, int ssag_start)
    {
    int i;

    if(!petit_args.ivr_on)
	return;

    if(petit_args.ivr_debug) 
	{
	fprintf(debug, "\n\nDoIvrFs: loop=%p ssag_start=%d\n", loop, ssag_start);
	print_program("Just before IVR:", loop);
	}

    /* Initialize Tarjan algorithm */
    ind_loop = loop;
    vertex_no = 0;
    ps_i = 0;
    sc_region = 1;
    for(i=0; i<no_ssag_nodes; i++) 
	{
	pEXPR v = ssag_nodes[i];
	if(v) 
	    {
	    scc_number(v) = 0;
	    scc_region(v) = 0;
	    reset(scc_onstack(v));
	    reset(scc_isloopfun(v));
	    scc_pssag_list(v) = &ssag_nodes[i];
	    }
	}
  
    /* Find SCCs and apply IVR to them */
    GoDownFun  = &ssa_source; 
    GoRightFun = &ssa_next_source;
    AddToScc   = &IvrAddToScc; 
    ProcessScc = &SccToInd;
    FindSccTxt = "IVR"; 
    for(i=ssag_start; i<no_ssag_nodes; i++) 
	{
	pEXPR w = ssag_nodes[i];
	if(w && scc_number(w)==0)
	    FindSCC(w);
	}
    }


/*
 * Find Maximal Strongly Connected Components (SCCs) in given graph.
 * Algorithm by R. Tarjan, SIAM J. Comput., v. 1(1972), n. 2, pp. 146-160.
 * It produces SCCs in order which is reverse of topological sort.
 */
SCCNO FindSCC(pEXPR v)
    {
    /* should be callee-specific */
    Assert(is_ssag_node(v) || is_assign(v), "FindSCC: not SSAG node");
    scc_number(v) = scc_lowlink(v) = ++vertex_no;

    if(petit_args.ivr_debug) 
	{
	fprintf(debug, "+++FindSCC(%s): op=%s v=%p nodeseq=%d ps_i=%d sc_num=%d\n",
		FindSccTxt, expr_oper_name(expr_op(v)), v, expr_lineno(v),
		ps_i, scc_number(v));
#ifndef NOT_PETIT
	fprintf(debug, "   child=%p next=%p parent=%p prev=%p",
		v->nodechild, v->nodenext, v->nodeparent, v->nodeprev);
#endif
	fprintf(debug, "\n");
	}
  
    /* put v on stack of points */
    Assert(ps_i < MAXPTSTACK, "FindSCC: Point Stack overflow");
    pt_stack[ps_i++] = v;
    set(scc_onstack(v));

    {
    pEXPR w, wnext;
    void *wptr;
    for(w=(*GoDownFun)(v,&wptr); w; w=wnext) 
	{
	wnext = (*GoRightFun)(w,v,&wptr);
        if(petit_args.ivr_debug)  {
	   fprintf(debug,"  following edge from %p to %p\n", v,w);
	   }
        
	if(scc_number(w) == 0) 
	    {
	    /* (v, w) is a tree arc 
	     *
	     * *w may be freed/replaced at call time,
	     * so we avoid reading w after FindScc.
	     * min is macro -- that's why cll can not
	     * be substituted in the next line.
	     */
	    SCCNO cll = FindSCC(w);
	    scc_lowlink(v) = min(scc_lowlink(v), cll);
	    }
	else if(scc_number(w) < scc_number(v)) 
	    {
	    /* (v,w) is frond or cross-link */
	    if(scc_onstack(w)) 
		{
		scc_lowlink(v) = min(scc_lowlink(v), scc_number(w));
		}
	    }
	}
    }

    {
    SCCNO lowlink = scc_lowlink(v);  /* memorize it, because v can be freed */
    if(lowlink == scc_number(v)) 
	{   /* v is root of SCC, start new SCC */
	pEXPR w;
	sc_vrxno = 0;
	while(ps_i>0 && scc_number(w=pt_stack[ps_i-1]) >= scc_number(v)) 
	    {
	    /* put w in current component */
	    scc_region(w) = sc_region;
	    reset(scc_onstack(w));
	    (*AddToScc)(w);
	    ps_i--;
	    }
	(*ProcessScc)(v);		      /* v can be freed at this call */
	sc_region++;
	}
    
    return(lowlink);
    }
    }


/* 
 * Add vertex to SCC for IVR/FS.
 */
static
void IvrAddToScc(pEXPR /*w*/) // w unused
    {
    sc_vrxno++;
    }


/*
 * Process given SCC to discover induction variable.
 */
static
void SccToInd(pEXPR v)
    {
    if(petit_args.ivr_debug) 
	{
	fprintf(debug,"***SccToInd: op=%s v=%p nodeseq=%d sc_num=%d sc_vrxno=%d\n",
		expr_oper_name(expr_op(v)), v, expr_lineno(v), scc_number(v), sc_vrxno);
	}
  
    if(sc_vrxno==1) 
	{
	/*
	 * When node depends on previously visited SCC do forward substitution:
	 *
	 * Values of phi-functions and array references are not propagated.
	 * Example: a1 and a2 can not be substituted in 2nd assignment.
	 * for k = i to n do
	 *   a1 = a(i,k)
	 *   a2 = a(j,k)
	 *   a(x,y) = c * a1 + s*a2
	 *   a(j,k) = -s * a1 + c*a2
	 * endfor
	 */
	if(expr_is_const(v) || expr_is_loop_index(v) || stmt_is_entry_node(v)) 
	    {
	    set(scc_isloopfun(v));

	    }
	else if(expr_is_scalar_fetch(v)) 
	    {
	    /*
	     * Allow substitution of expressions containing
	     * not only known values listed above but also scalar variables.
	     * It's done mostly for integer variables, because substitution
	     * of real variables doesn't give much for dependence analysis.
	     */
	    pEXPR p = ssa_source(v,NULL);
	    if(petit_args.ivr_SubstReals || expr_type_is_int(v))
		set(scc_isloopfun(v));
      
	    if(p==NULL) 
		{
		/* vars not defined in our scope are considered to have known value*/
		set(scc_isloopfun(v));

		}
	    else if(expr_is_scalar_store(p) && scc_isloopfun(p) &&
		    !expr_is_phi(expr_parent(v))) 
		{
		pEXPR np;
		np = ssa_source(p,NULL);
		np = copy_expr_tree(np);
		if(petit_args.ivr_debug) 
		    {
		    print_program("$$$ forward substitute was", v);
		    print_program("$$$ forward substitute now", np);
		    fprintf(debug, "   SccToInd: is_loopfun=%d !\n", scc_isloopfun(np));
		    }
		replace_expr_with_another(np, v);
		scc_region(np) = scc_region(v);
		scc_number(np) = scc_number(v);
		scc_lowlink(np) = scc_lowlink(v);
		scc_onstack(np) = scc_onstack(v);
		free_expr_tree(v,0);
		}

	    }
	else 
	    {
	    pEXPR np;
	    Assert(is_ssag_node(v), "SccToInd: not SSAG node");
	    for(np=ssa_source(v,NULL); np; np=ssa_next_source(np,v,NULL)) 
		{
		if(!scc_isloopfun(np)) goto NoFun;
		}
/*
 * Old rule: substitute expressions without array ref-s and phi-functions
 *    if(!expr_is_phi(v) && !expr_is_array_reference(v))
 *	set(scc_isloopfun(v));
 *
 * New rule: substitute only affine expressions of loop indices and scalars.
 *
 * Also check that scalars that are arguments of expression being substituted
 * have the same value in the new place as they had in the old place
 * (not implemented yet).
 */
	    if(expr_is_scalar_store(v) ||
	       expr_op(v)==op_add || expr_op(v)==op_subtract ||
	       expr_op(v)==op_max || expr_op(v)==op_min ||
	       (expr_op(v)==op_multiply &&
		(expr_is_int_const(bin_op_arg_left(v)) ||
		 expr_is_int_const(bin_op_arg_right(v))))) 
		{
		set(scc_isloopfun(v));
		}
	      NoFun:;
	    }

	if(petit_args.ivr_debug)
	    fprintf(debug, "   SccToInd: is_loopfun=%d\n", scc_isloopfun(v));

	}
    else if(petit_args.ivr_on) 
	{
	pEXPR p, phi, pstep, init_value;
	INT_CONST inc_value, loc_inc, loop_step;
	int FirstIV;
    
	/* Find out if SCC is a family of basic induction variables */
	p = v;
	inc_value = 0;
	phi = NULL;
	do 
	    {
	    pEXPR np;
	    if(scc_region(p) != sc_region) return;
	    np = ssa_source(p,NULL);

	    if(expr_is_scalar_fetch(p) || expr_is_scalar_store(p)) 
		{

		}
	    else if(expr_is_scalar_update(p)) 
		{
		if(!expr_is_int_const(np))
		    return;
		if(expr_op(assgmt_parent(p))==op_add_assign) 
		    {
		    inc_value += int_const_value(np);
#if 0
		    }
		else if(expr_op(assgmt_parent(p))==op_sub_assign) 
		    {
		    inc_value -= int_const_value(np);
#endif
		    }
		else 
		    {
		    return;
		    }
		np = ssa_next_source(np,p,NULL);

		}
	    else if(expr_is_phi(p)) 
		{
		if(phi_parent(p)!=ind_loop || phi!=NULL)
		    return;
		phi = p;
		np = ssa_next_source(np,p,NULL);

		}
	    else if(expr_op(p)==op_add || expr_op(p)==op_subtract) 
		{
		pEXPR arg1 = np;
		pEXPR arg2 = ssa_next_source(arg1,p,NULL);
		if(expr_op(p)==op_add && expr_is_int_const(arg1)) 
		    {
		    pEXPR t=arg1; arg1=arg2; arg2=t;
		    }
		if(!expr_is_int_const(arg2))
		    return;
		if(expr_op(p)==op_add)
		    inc_value += int_const_value(arg2);
		else
		    inc_value -= int_const_value(arg2);
		np = arg1;

		}
	    else 
		{
		return;
		}
	    p = np;
	    } while(p != v);
    
	/* Check that loop step divides induction variable step */
	pstep = loop_val_step(ind_loop);
	if(pstep == NULL) 
	    {
	    loop_step = 1;
	    }
	else if(!expr_is_int_const(pstep)) 
	    {
	    return;
	    }
	else 
	    {
	    loop_step = int_const_value(pstep);
	    }
	if(inc_value%loop_step != 0)
	    return;
    
	/* Now we know that SCR is basic IV family. So we make replacement. */
	v = phi;  p = phi;
	loc_inc = inc_value;
	init_value = NULL;                 /* keep complier from complaining */
	set(FirstIV);
	do 
	    {
	    pEXPR np = ssa_source(p,NULL);
	    if(expr_is_phi(p)) 
		{
		Assert(where_defined(np)==NULL, "SccToInd: init value ptr != NULL");
		init_value = np;
		np = ssa_next_source(np,p,NULL);

		}
	    else if(expr_is_scalar_fetch(p)) 
		{

		}
	    else if(expr_op(p)==op_add || expr_op(p)==op_subtract) 
		{
		pEXPR arg1 = np;
		pEXPR arg2 = ssa_next_source(arg1,p,NULL);
		if(expr_op(p)==op_add && expr_is_int_const(arg1)) 
		    {
		    pEXPR t=arg1; arg1=arg2; arg2=t;
		    }
		if(expr_op(p)==op_add)
		    loc_inc -= int_const_value(arg2);
		else
		    loc_inc += int_const_value(arg2);
		np = arg1;
		free_expr_node(arg2,0);

		}
	    else if(expr_is_scalar_update(p) || expr_is_scalar_store(p)) 
		{
		/* Replace right hand side of assignment with closed form of IV */
		pEXPR lp, rp;
		lp = make_loop_index_ref(ind_loop); 
		rp = copy_expr_tree(loop_val_start(ind_loop));
		lp = make_expr_node(lp, op_subtract, rp);
		rp = make_constant(inc_value/loop_step);
		lp = make_expr_node(lp, op_multiply, rp);
		rp = make_constant(loc_inc);
		lp = make_expr_node(lp, op_add, rp);
		rp = copy_expr_tree(init_value);
		lp = make_expr_node(lp, op_add, rp);
		set(scc_isloopfun(lp));
		replace_expr_with_another(lp, assgmt_right_part(assgmt_parent(p)));
		set(scc_isloopfun(p));
		if(petit_args.ivr_debug)
		    print_program("$$$ IV recognized", assgmt_parent(p));

		/* === Add assignment to final value of IV after loop body */
		if(FirstIV) 
		    {
		    pEXPR load_vr, load_ex;
		    reset(FirstIV);
		    lp = copy_expr_tree(loop_val_finish(ind_loop));
		    rp = copy_expr_tree(loop_val_start(ind_loop));
		    lp = make_expr_node(lp, op_subtract, rp);
		    rp = make_constant(1);
		    lp = make_expr_node(lp, op_add, rp);
		    rp = make_constant(loop_step);
		    lp = make_expr_node(lp, op_ceiling_divide, rp);
		    rp = make_constant(inc_value);
		    lp = make_expr_node(lp, op_multiply, rp);
		    rp = copy_expr_tree(init_value);
		    load_ex = make_expr_node(lp, op_add, rp);
		    load_vr = make_scalar_fetch(expr_var_id(p));
		    rp = make_scalar_store(expr_var_id(p));
		    lp = make_scalar_assgmt(rp, op_assign, load_vr);
		    replace_expr_with_another(load_ex, load_vr);
		    free_expr_tree(load_vr,0);
		    place_new_stmt_after(lp, ind_loop);
		    /*
		     * Record this var=var{shadow} statement for subsequent elimination.
		     * It is NO-OP statement, but shadow expressions are used in IVR,
		     * so before everything is substituted we need these statements.
		     */
		    Assert(NoOpStmtNo<MAXNOOPSTMTNO,
			   "SccToInd: too many no-op stmts (internal problem)");
		    NoOpStmts[NoOpStmtNo++] = lp;
		    if(petit_args.ivr_debug) 
			{
			print_program("$$$ IV final value", lp);
			}
		    }
		if(expr_is_scalar_update(p)) 
		    {
		    pEXPR t = ssa_next_source(np,p,NULL);
		    free_expr_node(np,0);
		    np = t;
		    turn_update_to_store(p);
		    loc_inc -= int_const_value(assgmt_right_part(assgmt_parent(p)));
		    }

		}
	    else 
		{
		ErrAssert("SccToInd: wrong node was met while replacing IVs");
		}

	    if(!expr_is_phi(p) && !expr_is_scalar_store(p))
		free_expr_node(p,0);

	    p = np;
	    } while(p != v);

	free_expr_tree(init_value,0); 
	free_expr_node(phi,0);
	}
    }

/*
 * Replaces one expression with another.
 * If old is shadow itself, do place_expr_instead,
 * because we don't need shadow of shadows.
 */
static
void replace_expr_with_another(pEXPR new_expr, pEXPR old)
    {
    place_shadow_expr(new_expr, old);
    }


/*
 * Delete no-op assignment statements (active only with -f).
 * It's possible that no-op stmt won't be deleted, if
 * it is used in fetch, which affects dependence analysis,
 * that is which is not under phi-function, and is not Exit node.
 */
void NoOpListDelete(void)
    {
    int i;
    for(i=0; i<NoOpStmtNo; i++) 
	{
	pSTMT stmt = NoOpStmts[i];
	pEXPR store = assgmt_left_part(stmt);
	pEXPR use;
	void *wptr;
	for(use=ssa_sink(store,&wptr); use; use=ssa_next_sink(use,store,&wptr)) 
	    {
	    if(!stmt_is_exit_node(use) && !expr_is_phi(expr_parent(use))) 
		{
		if(petit_args.ivr_Assert || petit_args.ivr_debug) 
		    Warning("NoOpListDelete: no-op assignment used in non-phi fetch");
		goto NextStmt;
		}
	    }
	remove_stmt(stmt);
	  NextStmt:;
	}
  
    NoOpStmtNo=0;
    }


/*
 * =========== Dead scalar assignments elimination ============
 */

static pEXPR sc_nodes[MAXPTSTACK];

static int   noRmNodes;
static pSTMT RmNodes[MAXPTSTACK];


static void  RAAddToScc(pEXPR);
static void  RAProcessScc(pEXPR);

 
/* 
 * Eliminate dead scalar assignments, listed in SSA graph.
 * At start, all assignments are live (is_loopfun flag).
 * SCC which result is not used by live assignment, becomes dead.
 */
void ElimDeadAssgmts(void)
    {
    int i;

    if(petit_args.ivr_debug)
	fprintf(debug, "\n\n");
  
    for(i=0; i<no_ssag_nodes; i++) 
	{
	pEXPR v = ssag_nodes[i];
	if(v) 
	    {
	    scc_number(v) = 0;
	    scc_region(v) = 0;
	    reset(scc_onstack(v));
	    set(scc_isloopfun(v));
	    scc_pssag_list(v) = &ssag_nodes[i];
	    }
	}

    vertex_no = 0;
    ps_i = 0;
    sc_region = 1;
    noRmNodes = 0;

    GoDownFun  = &ssa_sink; 
    GoRightFun = &ssa_next_sink;
    AddToScc   = &RAAddToScc; 
    ProcessScc = &RAProcessScc;
    FindSccTxt = "Remove"; 

    for(i=0; i<no_ssag_nodes; i++) 
	{
	pEXPR w = ssag_nodes[i];
	if(w && scc_number(w)==0)
	    FindSCC(w);
	}

    /* remove marked dead assignment statements */
    for(i=0; i<noRmNodes; i++) 
	{
	pSTMT p = RmNodes[i];
	remove_stmt(p);
	}
    }


/*
 * Add to SCC (dead assignments)
 */
static 
void RAAddToScc(pEXPR w)
    {
    Assert(sc_vrxno<MAXPTSTACK, "RAAddToScc: sc_nodes overflow");
    sc_nodes[sc_vrxno++] = w;
    }


/*
 * Process SCC (dead assignments) -- mark dead assignments for removal.
 * If store doesn't supply values to outside of SCC ot to array reference
 * it can be removed.
 */
static 
void RAProcessScc(pEXPR /*v*/) // v unused
    {
    SCCNO i;
    for(i=0; i<sc_vrxno; i++) 
	{
	pEXPR w = sc_nodes[i];
	pEXPR p;
	void *wptr;
	if(can_not_be_dead(w))
	    return;
	for(p=ssa_sink(w,&wptr); p; p=ssa_next_sink(p,w,&wptr)) 
	    {
	    if(can_not_be_dead(p) || (scc_region(p)!=scc_region(w) && scc_isloopfun(p)))
		return;
	    }
	}

    for(i=0; i<sc_vrxno; i++) 
	{
	pEXPR w = sc_nodes[i];
	reset(scc_isloopfun(w));
	if(petit_args.ivr_debug) 
	    fprintf(debug, "---RAProcessScc: node %p marked for removal\n", w);
	if(expr_is_scalar_store(w) || expr_is_scalar_update(w)) 
	    {
	    pSTMT p = assgmt_parent(w);
	    Assert(noRmNodes<MAXPTSTACK, "RAProcessScc: RmNodes overflow");
	    if(petit_args.ivr_debug)
		fprintf(debug, "===Stmt %p marked for removal\n", p);
	    RmNodes[noRmNodes++] = p;
	    }
	}
    }


/*
 * =========== Update operations recognition ============
 */

static pEXPR     FindCmRef(pEXPR, pEXPR, EXPR_OPER, EXPR_OPER, int);
static void      MakeUpdate(pEXPR, pEXPR, EXPR_OPER, INT_CONST);


/*
 * Convert (a = a op expr) into (a op= expr) -- recognize updates.
 */
void RecUpdate(pSTMT ap) 
    {
    if(expr_op(ap)==op_assign) 
	{
	pEXPR lp = assgmt_left_part(ap);
	pEXPR rp = assgmt_right_part(ap);
	pEXPR p;
	if((p=FindCmRef(lp, rp, op_add, op_subtract, false))!=NULL) 
	    {
	    MakeUpdate(lp, p, op_add_assign, 0);
	    }
	else if((p=FindCmRef(lp, rp, op_multiply, op_divide, false))!=NULL) 
	    {
	    MakeUpdate(lp, p, op_mul_assign, 1);
	    }
	}
    }


/*
 * Find reference `ref' in tree `tree' of commutative and associative
 * operations `prim' and `sec' (reverse of prim).
 */
static
pEXPR FindCmRef(pEXPR ref, pEXPR tree, EXPR_OPER prim, EXPR_OPER sec, int isneg)
    {

    if(expr_is_memory_reference(tree)) 
	{
	if(expr_var_id(tree)==expr_var_id(ref)) 
	    {
	    sub_iterator si1, si2;
	    si1 = sub_i_for_access(tree);
	    si2 = sub_i_for_access(ref);
	    for(; !sub_i_done(si1); sub_i_next(si1)) 
		{
		if(sub_i_done(si2) || !compare_exprs(si1,si2))
		    return(NULL);
		sub_i_next(si2);
		}
	    if(sub_i_done(si2) && !isneg)
		return(tree);
	    }

	}
    else if(expr_op(tree) == prim) 
	{
	pEXPR p = FindCmRef(ref, bin_op_arg_left(tree), prim, sec, isneg);
	if(p)
	    return(p);
	p = FindCmRef(ref, bin_op_arg_right(tree), prim, sec, isneg);
	return(p);

	}
    else if(expr_op(tree) == sec) 
	{
	pEXPR p = FindCmRef(ref, bin_op_arg_left(tree), prim, sec, isneg);
	if(p)
	    return(p);
	p = FindCmRef(ref, bin_op_arg_right(tree), prim, sec, !isneg);
	return(p);
	}

    return(NULL);
    }


/*
 * Make node n to be op_update node.
 * Eliminate node p -- make it 0 or 1 constant.
 */
static
void MakeUpdate(pEXPR n, pEXPR p, EXPR_OPER op, INT_CONST value) 
    {
    turn_store_to_update(n,op,p);
    place_expr_instead(make_constant(value), p);
    free_expr_tree(p,true);
    }

} // end of namespace omega

