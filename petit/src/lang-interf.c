 /* $Id: lang-interf.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <basic/bool.h>
#include <basic/assert.h>
#include <omega/Relations.h>
#include <omega/pres_tree.h>
#include <omega/pres_conj.h>
#include <petit/lang-interf.h>
#include <petit/ivr-lang-interf.h>
#include <petit/timeTrials.h>
#include <petit/debug.h>
#include <petit/make.h>
#include <petit/affine.h>
#include <petit/pres-interf.h>
#include <petit/Exit.h>

namespace omega {

void write_ddnode_to_graph(ddnode * ddn)
{ 
    ddn->ddnextpred = ddn->ddsucc->nodeddin;
    ddn->ddnextsucc = ddn->ddpred->nodeddout;
    ddn->ddsucc->nodeddin = ddn;
    ddn->ddpred->nodeddout = ddn;
}


/*****
 * Note: this function can be used to store the dependence by several
 *   different data dependence tests. Some of them (e.g. Epsilon test)
 *   do not creation a dependence relation. In that case 
 *   d_info->dd_relation is NULL. No operations should be performed
 *   on d_info->dd_relation is that case.
 ****/


void store_dependence(ddnature nature,
			 a_access from_access,a_access to_access,
			 dir_and_diff_info *d_info) {

#if defined newTimeTrials
    if (!storeResult)
	return ;
#endif

    ddnode *ddn = new ddnode(d_info->nest);

    ddn->ddtype = nature;
    ddn->ddpred = from_access;
    ddn->ddsucc = to_access;
    ddn->ddlink = NULL;
    ddn->ddflags = d_info->flags;

    dir_and_diff_into_ddnode(d_info, ddn);

#if ! defined NDEBUG
    if (d_info->dd_relation!=NULL)  
    { 
      Relation tmp = *d_info->dd_relation;
      for (DNF_Iterator d = tmp.query_DNF(); d; d++)
	  (*d)->assert_leading_info();
    }
#endif

    if (d_info->dd_relation!=NULL) {
      ddn->dd_relation = new Relation(*(d_info->dd_relation));
      *d_info->dd_relation = Relation();
      if (!ddn->dd_relation->is_exact())
	   ddn->ddflags |= ddMemIncompFlags;

      ddn->dd_relation->compress();
    } else {
      ddn->dd_relation = d_info->dd_relation;
    }

    write_ddnode_to_graph(ddn);
}



void map_over_vars(node *expression, map_fn f, void *args)
    {
    node *nn;

    if(expression->shadow)
        expression = expression->shadow;
    
    switch( expression->nodeop )
	{
    case op_index:
	assert(var_id_is_index(loop_var_id(get_nodevalue_node(expression))));
	(*f)(loop_var_id(get_nodevalue_node(expression)), args);
 	break;
    case op_fetch:
    case op_store:
    case op_update:
	(*f)(get_ssa_scalar_id(expression), args);
	break;
    default:
	for( nn=expression->nodechild; nn != NULL; nn = nn->nodenext )
	    if (nn)
		map_over_vars( nn, f, args);
	}
    }


void map_over_vars_affine(node *expression, ar_map_fn f, void *args)
{
    node *nn;

    if (expression->nodeaffine && node_is_affine(expression)) {
       affine_expr * ae=expression->nodeaffine;
	int i;
       for (i=1; i<ae->nterms;i++) 
          (*f)(ae->terms[i].petit_var, ae->terms[i].arity, args);
       if (ae->other_branch) {
          for (i=0; i<ae->other_branch->nterms; i++)
            (*f)(ae->other_branch->terms[i].petit_var, 
                 ae->other_branch->terms[i].arity, args);
          assert (!ae->other_branch);
       }    
     }
    else if (!expression->nodeaffine)  {
	for( nn=expression->nodechild; nn != NULL; nn = nn->nodenext )
	    if (nn)
		map_over_vars_affine( nn, f, args);
     }
}

context_iterator cont_i_for_access(a_access a)
{
    node *c;

    if (a == Entry)
	return a;

    for (c = a->nodeparent;
	 !cont_i_done(c) && !cont_i_cur_is_loop(c) && !cont_i_cur_is_if(c);
	 (c) = ((c)->nodeparent))
	;
    return c;
}

context_iterator cont_i_for_assert(pASSERT a)
{
  node *c;
  for (c = a->nodeparent;
       !cont_i_done(c) && !cont_i_cur_is_loop(c) && !cont_i_cur_is_if(c);
       (c) = ((c)->nodeparent))
  ;
  return c;
}
      

rel_expr get_if_rel_expr(if_context ic)
  {
   if (ic->nodeparent->nodeop == op_if)
       return ic->nodeparent->nodechild;
   else if (ic->nodeop == op_assert)
       return ic->nodechild;
   else
       assert (0 && "wrong node operation for if_context");
       return 0;
 }

bool rel_expr_is_term(rel_expr re)
 {
   if (!is_affine(re->nodeaffine))
      return true;

   switch (re->nodeop)
     {
       case op_or:
       case op_and:
       case op_not:
           return false;
       default:
           assert (compare_op(re->nodeop) || re->nodeaffine);
           return true;
     }
 }

rel_operators rel_expr_operator(rel_expr re)
  {
   assert (! rel_expr_is_term(re));
   switch (re->nodeop) 
     {
       case op_and:
         return rel_and;
       case op_or:
         return rel_or;
       case op_not:
         return rel_not;
       default:
         assert (0 && "Wrong value for rel operator");
	 return rel_and;  // make the complier shut up
      }
  }   

bool rel_expr_term_is_ok(term t)
  {
   if (t->nodeaffine) 
     return node_is_affine(t);
   else 
     {
       assert(compare_op(t->nodeop));

       return node_is_affine(t->nodechild) && 
              node_is_affine(t->nodechild->nodenext);
     }
 }
  
    

if_compare_operators term_compare_op(term t)
    {
     if (!t->nodeaffine)
       { 
	switch (t->nodeop)
	    {
	case op_gt:
	    return greater;
	case op_ge:
	    return greater_eq;
	case op_lt:
	    return less;
	case op_le:
	    return less_eq;
	case op_eq:
	    return equal;
	case op_ne:
	    return not_equal; 
	default:
	    assert(0 && "can't get here");
	    return greater;  // make the compiler shut up
	    }
      }
     else
        return greater; 
    }

affine_expr *term_compare_left(term t)
    {
    if (t->nodeaffine)
      return t->nodeaffine;
    else {
      assert(compare_op(t->nodeop));
      assert (t->nodechild  && t->nodechild->nodeaffine); 
      return t->nodechild->nodeaffine;
        }
  }

static affine_expr ae_const_0 = { 1, { {0, 0, IRRELEVANT_ARITY} }, 0 };

affine_expr *term_compare_right(term t)
    {
     if (t->nodeaffine)
       return  &ae_const_0;
     else {
      assert(compare_op(t->nodeop));
      return t->nodechild->nodenext->nodeaffine;
     }
    }
		     

#define maxHeight 200
bool access_lexically_preceeds(node *n1, node *n2)
    {
    int i1, i2;
    node *next;
    node *parents1[maxHeight], *parents2[maxHeight];

    if (n1 == n2) return 0;

    if(n1==Entry || n2==ExitNode) return 1;

    if(n2==Entry || n1==ExitNode) return 0;

    for (i1=0; n1!=NULL; i1++,n1=n1->nodeparent)
	parents1[i1] = n1;

    assert(i1 < maxHeight);
	
    for (i2=0; n2!=NULL; i2++,n2=n2->nodeparent)
	parents2[i2] = n2;

    assert(i2 < maxHeight);

    while (parents1[--i1] == parents2[--i2]);

    if (parents1[i1+1]->nodeop == op_entry   ||
        parents1[i1+1]->nodeop == op_dolimit ||
        parents1[i1+1]->nodeop == op_then    ||
        parents1[i1+1]->nodeop == op_else)
	{
	for (next = parents1[i1]->nodenext; 
	     next != parents2[i2] && next != NULL; 
	     next = next->nodenext);
        return (next == parents2[i2]);
	}
    else
	return 0;
    } /* access_lexically_preceeds */


#if ! defined NDEBUG
/* helpful function to test the sanity of dir_and_dist_info */
void ddi_check(const dir_and_diff_info *ddi)
{
    int j;

    /* any bit thats 0 in restraint vector must be 0 in direction vector
       this assertion is overly tight - we should only care about the
       bits for the dependence directions up to "nest" */
    assert(ddi->nest == 0 ||
	   ! ((ddi->direction ^ ddi->restraint) & ddi->direction));
    
    for( j = 1; j <= (int)ddi->nest; ++j )
    {
	assert( !(ddi->differenceKnown[j] && ddi->difference[j] == 0 &&
		  ddextract1(ddi->direction,j) != ddind) );
	assert( !((!ddi->differenceKnown[j] || ddi->difference[j] != 0) &&
		  ddextract1(ddi->direction,j) == ddind) );
	assert( !(ddi->differenceKnown[j] && 
                  ddi->difference[j]*ddi->loop_dir[j] > 0 &&
		  ddextract1(ddi->direction,j) != ddfwd) );
	assert( !(ddi->differenceKnown[j] && 
                  ddi->difference[j] * ddi->loop_dir[j]< 0 &&
                  ddextract1(ddi->direction,j) != ddbck) );
    }
}
#endif

/* convert petit ddnodes into stuff our functions can handle */

void
ddnode_to_dir_and_diff(ddnode *ddn, dir_and_diff_info *ddi)
{
    int j;

    ddi->direction = ddn->dddir;
    ddi->flags = ddn->ddflags;
    ddi->restraint = ddn->ddrestraint;
    ddi->nest = ddn->ddnest;

    for(j=1; j<=(int)ddn->ddnest; j++)
	ddi->loop_dir[j] = ddn->loop_dir[j];

    for (j=1; j<=(int)ddn->ddnest; j++) {
	if ((unsigned)ddn->dddiff[j] == ddunknown) {
	    ddi->differenceKnown[j] = 0;
	} else {
	    ddi->differenceKnown[j] = 1;
	    ddi->difference[j] = ddn->dddiff[j];
	}
    }

    ddi_check(ddi);
}

/* copy info from a dir_and_diff_info into an existing ddnode */
void
dir_and_diff_into_ddnode(const dir_and_diff_info *ddi, ddnode *ddn)
{
    int j;

    ddn->ddnest = ddi->nest;
    ddn->dddir = ddi->direction;
    ddn->ddflags = ddi->flags;
    ddn->ddrestraint = ddi->restraint;

    /* ddnest == 0 <--> dddiff in NULL */
    assert( (ddn->ddnest == 0) == (ddn->dddiff == NULL) );

    ddi_check(ddi);
    
    for(j=1; j<=(int)ddi->nest; j++) {
	ddn->loop_dir[j] = ddi->loop_dir[j];
	assert(ddi->loop_dir[j]!=0);
    }

    for( j = 1; j <= (int)ddn->ddnest; ++j ){
	ddn->dddiff[j] =
	    ddi->differenceKnown[j] ?
		ddi->difference[j] : ddunknown;
	if (ddextract1(ddn->dddir,j) == ddind)
	    ddn->dddiff[j] = 0;
    }
}


int access_is_in_then_or_else_of(a_access a, context_iterator c)
{
    assert(cont_i_cur_is_if(c));
    do {
	if (a == c) return 1;
	a = a->nodeparent;
    } while (a != Entry);
    return 0;
}


/* return do_op or op_entry containing n */

node *loop_containing(node *n)
{
    node *parent;

    if(n==Entry) return(Entry);

    /* This routine is usually called during dependence analysis,
       at which time we can depend on the value of "nodecontext".
       Otherwise, just ues "nodeparent". */
    if (n->nodecontext)
    {
	for (parent = n->nodecontext;
	     parent != Entry && parent->nodeop != op_dolimit;
	     parent = parent->nodecontext)
	    assert(if_branch_op(parent->nodeop));
    } else {
	for (parent = n->nodeparent; 
	     parent != Entry && parent->nodeop != op_dolimit;
	     parent = parent->nodeparent)
	    ;
    }

    return parent == Entry ? parent : parent->nodeparent;
}

/* I'd think this would be somewhere already, but I don't see it... */

int node_depth(node *n)    
{
    node *parent = loop_containing(n);
    if( parent != Entry ) {
	assert(do_op(parent->nodeop));
	return parent->nodevalue;
    }
    else
	return 0;
}

int access_shared_depth(a_access a1, a_access a2)
{
    static a_access a1_cache = NULL;
    static a_access a2_cache = NULL;
    static int cache_result;
    a1 = loop_containing(a1);
    a2 = loop_containing(a2);

    if (a1->nodeop == op_entry || a2->nodeop == op_entry)
	return 0;

    if (a2 == a2_cache && a1 == a1_cache) return cache_result;

    a2_cache = a2;
    a1_cache = a1;
    while (a1->nodevalue > a2->nodevalue)
	a1 = loop_containing(a1);
    while (a2->nodevalue > a1->nodevalue)
	a2 = loop_containing(a2);

    while (a1 != a2) {
	a1 = loop_containing(a1);
	a2 = loop_containing(a2);
    }

    if( a1->nodeop == op_entry )
        cache_result = 0;
    else
        cache_result = a1->nodevalue;
    return cache_result;
}

// access_shared_context_at_depth finds the tightest "context"
//   within exactly "d" loops that encloses both a1 and a2
//
// it uses if_contains:

static bool it_contains(context_iterator c, node *n)
    {
    if (c == Entry)
	return true;

    for (; n != Entry; n=n->nodeparent)
	if (n == c)
	    return true;
    return false;
    }

context_iterator access_shared_context_at_depth(a_access a1, a_access a2, int d)
    {
    assert( access_shared_depth(a1,a2) >= d );

	node *ic;
    for (ic = a1; ic != Entry && node_depth(ic) >= d; ic=ic->nodeparent)
	if (node_depth(ic) == d && it_contains(ic, a2))
	    if (if_branch_op(ic->nodeop))
		return ic;

    assert(ic->nodeop == op_dolimit || ic->nodeop == op_entry);
    assert((int)cont_i_cur_depth(ic) == d);
    return ic;
    }

// return the tightest if_context that contains either a1 and a2 or a1 and a3
// as above

if_context access_inner_shared_context_at_depth(a_access a1, a_access a2, a_access a3, int d)
    {
    context_iterator ic2 = access_shared_context_at_depth(a1, a2, d);
    context_iterator ic3 = access_shared_context_at_depth(a1, a3, d);
    
    if (it_contains(ic2, ic3))
	return ic3;
    else
	{
	assert(it_contains(ic3,ic2));
	return ic2;
	}
    }

} // end of namespace omega

#if 0

OLD STUFF FOR ADDING ASSERTIONS

static node *add_symconst_to_sum(Problem *p, var_id vars[], int geq, int rv, node *sum)
    {
    node *fetch, *constant, *term, *new_sum;
    var_id v;

    if (rv == 0)
	{
	constant = make_node(op_constant);
	constant->nodevalue = abs(p->_GEQs[geq].coef[rv]);
	term = constant;
	}
    else
	{
        fetch = make_node(op_fetch);
	/* var must be a symbolic constant, so we can use its
	   original name rather than its SSA name */
	v = vars[p->_var[rv]]->original;
	assert(v && var_id_is_const(vars[p->_var[rv]]));
        fetch->nodevalue = (unsigned long int) (v->symdecl);

        if (abs(p->_GEQs[geq].coef[rv]) == 1)
	    {
            term = fetch;
	    }
        else
	    {
	    constant = make_node(op_constant);
	    constant->nodevalue = abs(p->_GEQs[geq].coef[rv]);
	    term = make_node(op_multiply);
	    term->nodechild = constant;
	    constant->nodenext = fetch;
	    constant->nodeparent = term;
	    fetch->nodeprev = constant;
	    fetch->nodeparent = term;
	    }
        }

    if (sum == NULL)
	{
        return term;
	}
    else
        {
        new_sum = make_node(op_add);
        new_sum->nodechild = sum;
        sum->nodenext = term;
        sum->nodeparent = new_sum;
        term->nodeprev = sum;
        term->nodeparent = new_sum;
        return new_sum;
        }
    } /* add_symconst_to_sum */


node *petit_GEQ_expr_on_symconsts(Problem *p, var_id vars[], int geq)
{
    int rv;
    node *top, *left_sum, *right_sum;

    assert(p->_safeVars == p->_nVars);

    left_sum = NULL;
    for (rv = 1; rv <= p->_nVars; rv++) 
	if (p->_GEQs[geq].coef[rv] > 0)
	    left_sum = add_symconst_to_sum(p, vars, geq, rv, left_sum);

    if (p->_GEQs[geq].coef[0] > 0)
	left_sum = add_symconst_to_sum(p, vars, geq, 0, left_sum);

    if (left_sum == NULL)
	{
	left_sum = make_node(op_constant);
	left_sum->nodevalue = 0;
	}

    right_sum = NULL;
    for (rv = 1; rv <= p->_nVars; rv++) 
	if (p->_GEQs[geq].coef[rv] < 0) 
	    right_sum = add_symconst_to_sum(p, vars, geq, rv, right_sum);

    if (p->_GEQs[geq].coef[0] < 0)
	right_sum = add_symconst_to_sum(p, vars, geq, 0, right_sum);

    if (right_sum == NULL)
	{
	right_sum = make_node(op_constant);
	right_sum->nodevalue = 0;
	}

    top = make_node(op_ge);
    top->nodechild = left_sum;
    left_sum->nodeparent = top;
    left_sum->nodenext = right_sum;
    right_sum->nodeparent = top;
    right_sum->nodeprev = left_sum;

    return top;
}


void add_assertion(node *expr)
{
    node *last, *new_assert;

    assert(Entry->nodenext == 0);

    new_assert = make_node(op_assert);
    new_assert->nodelink = Assertions;
    Assertions = new_assert;
    new_assert->nodechild = expr;
    expr->nodeparent = new_assert;

    for(last = Entry->nodechild; last->nodenext; last=last->nodenext)
	;
    last->nodenext = new_assert;
    new_assert->nodeparent = Entry;
    new_assert->nodeprev = last;
}


#endif
