/* $Id: affine.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <basic/bool.h>
#include <basic/assert.h>
#include <petit/pres-interf.h>
#include <petit/omega2flags.h>
#include <petit/debug.h>
#include <petit/affine.h>
#include <petit/find_affine.h>
#include <petit/tree.h>
#include <petit/vutil.h>
#include <petit/petity.h>
#include <petit/ssa.h>
#include <petit/ivr-lang-interf.h>
#include <petit/Exit.h>
#include <petit/lang-interf.h>
#include <petit/petit_args.h>

namespace omega {

#if ! defined dont_merge_uses_of_a_readonly_array
#define dont_merge_uses_of_a_readonly_array 0
#endif

/*

	NOTES TO THOSE TRYING TO ADAPT THIS FOR OTHER COMPILERS:

THIS COMMENT IS PROBABLY SOMEWHAT OUT OF DATE, BUT MAY BE USEFUL ANYWAY?

We associate a pointer of type affine_expr * with each subscript, loop
bound, or conditional (``if'' or ``assert'').  Our goal is to set this
pointer to a special value if we can not handle the expression, or
point the pointer to an affine_expr structure if we can.  We can
handle affine functions of the loop indices and symbolic constants,
plus min and max operations in some cases, and the comparison
operations >, >=, <, and <= in conditionals.  An expression is an
affine function of variables v1, v2, ... vN if it can be expressed as
the sum of a known constants times each of these variables plus a
constant, that is, as c0 + c1*v1 + c2*v2 + ... + cN*vN, for known
integers c0...cN.

We start the process of finding affine expressions by calling
r_find_affine_exprs with the root of our parse tree.  This function
traverses the parse tree, calling ``find_affine_expr'' on subscripts
and loop indices, and ``find_puny_affine_conditions'' on conditionals.

We use a different function for conditionals because we want to allow
a comparison such as > between two affine expressions.  As its name
suggests, this function could be made more powerful - it could allow
conjunctions of tests, for example.  If you're not worried about
conditionals, you can omit this stuff and simply indicate to the omega
test that all conditionals can not be analysed - more on communicating
with the omega test later.

The functions ``find_affine_expr'' and ``find_puny_affine_conditions''
both rely on ``m_linearity'' and ``find_coef'' to determine whether
the function is affine, and if so, determine the set of variables
v1...vN, and the constants c0...cN.  I will start with a simplified
description of these functions, and then elaborate:

m_linearity tags each of the variables used in the expression, and
checks for non-linear terms such as arrays, function calls, or
multiplication by a non-constant.  The tags ensure that we enter each
variable into the affine_expr structure only once, establishing the
set of variables v1...vN.  If we come across something non-affine, we
return a number greater than 1, and the calling function will store a
pointer to the special value ``not_affine'' to show the node is not
affine.

find_coef takes a variable "petit_var", and evaluates the expression
assuming that "petit_var" has the value 1 and all other variables have
the value 0.  This tells us the sum of the co-efficient of that
variable and the constant c0.  We determine constant c0 by calling
find_coef with NULL instead of a real variable, in which case it
treats all variables as if they had the value 0.

Thus, m_linearity and find_coef find v1...vN, and c0...cN.  After we
are done calling them, we reset the tags of the variables tagged in
m_linearity.

The sitation is somewhat complicated by the fact that we can handle
lower loop bounds with max expressions, and upper loop bounds with min
expressions.  For example, given a loop with lower bound max(A,B), we
will add two constraints to our system: i >= A and i >= B.  To let us
do this, want to associate two affine_expression structures with the
lower loop bound.  We do this by including a pointer ``other_branch''
in the affine structure, and using this to link together a list of
affine_exprs.

We currently only build a list of up to 2 affine_exprs.  If there is
an appropriate min or max operation, m_linearity sets
N_min_or_max_args to the number of operands, in which case we build
that many affine_expr structures: we do this by calling find_coef with
the number of the operand to be examined.  This system could (should)
be extended to handle more than one min or max expression.  Note that
a negated min is equivalent to a max and vice versa.

Once the affine_expr structures have been associated with the nodes in
the parse tree, our dependence tester can use them to build sets of
constraints on integer expressions.  To do this, it must relies on
macros from lang-interf.h, such as loop_start and loop_end, which find
the affine expressions associated with the start and end bounds of the
loop.  Our system currently only works with loops having POSITIVE
KNOWN STEPS, so the start is the lower bound, and the end the upper
bound.

To access the affine_exprs from an if, we need to know if we are in
the then or the else block, what the comparison is, and what
affine_exprs are being compared.  This information should be given by
the macros if_else_branch, if_compare_op, if_compare_left, and
if_compare_right, for any if we can handle.  Ifs we could not handle
should cause if_condition_ok to return false.
*/


#define USE_PUFS_FOR_ANALYSIS (!petit_args.skipping_omega5)
// this controls what we do with non-affine terms.
// if true, we create Omega Library "Global_Var"s to represent
//	    the function that produces their value in any given iteration
//    else  we tag the root of any expression containing non-affine terms
//	    as non-affine

bool nodes_subs_are_affine(a_access A)
{
    sub_iterator subs = sub_i_for_access(A);

    while ( !sub_i_done(subs) )
    {
	if (!sub_i_cur_is_affine(subs))
	    return 0;
	sub_i_next(subs);
    }
    return 1;
}


// this seemed to belong with m_linearity
// Find the depth at which the expression "expr" is invariant
// "expr" may be an "if"

unsigned int expr_invariant_at_depth(node *expr)
    {
    if(expr->shadow)
	expr = expr->shadow;

    switch(expr->nodeop)
	{
	
    case op_constant:
	{
	return 0;
	}

    case op_index :
	{
	assert(get_nodevalue_node(expr)->nodeop == op_dolimit);
	int which_loop = get_nodevalue_node(expr)->nodeparent->nodevalue;
	return which_loop;
	}

    case op_fetch :
	{
	/* fetch's value is pointer to declaration */
	assert(get_nodevalue_node(expr)->nodeop == op_declare);
	Var_Id sym = get_ssa_scalar_id(expr);
	return access_shared_depth(var_id_def_node(sym),expr);
	}

    case op_gt:
    case op_ge:
    case op_lt:
    case op_le:
    case op_eq:
    case op_ne:
	{
	assert(expr->nodeparent->nodeop == op_if ||
	       expr->nodeparent->nodeop == op_assert ||
               expr->nodeparent->nodeop == op_and ||
               expr->nodeparent->nodeop == op_or ||
               expr->nodeparent->nodeop == op_not);
	}

    case op_add:
    case op_subtract :
    case op_multiply :
    case op_min :
    case op_max :
    case op_divide:
    case op_call :
    case op_and :
    case op_or : 
    case op_not :
	{
	unsigned int max_operand_depth = 0;

	for (node *child = expr->nodechild;
	     child != NULL;
	     child = child->nodenext )
	    {
	    unsigned int d = expr_invariant_at_depth(child);
	    assert((int)d <= node_depth(expr));
	    if (d > max_operand_depth) max_operand_depth = d;
	    }

	return max_operand_depth;
	}

    case op_fetch_array :
	{
	return node_depth(expr);  // should check for const arrays here
	}

    case op_if:  // nonlinear if functions point back to op_if
	{
	// the first child is the conditional expression
//	return expr_invariant_at_depth(expr->nodechild);
        assert (0 && "Why do we ask expr_invar_at_depth for op_if???");
	// THIS was for old system that attached affine-expr's to "if"
	// rather than somewhere sane.
	}

    default :
	{
	// assert(0 && "Unexpected case in UFS arity code");
	// could be something like op_sqrt, in which case we
	// really should check for invariance of the arguments,
	// but that would require us knowing which functions
	// are referentially transparent, which I'd rather
	// not hard-code here.
	return node_depth(expr);
	}
	}
    }




/* access to tag info */

/* We currently use a somewhat anachronistic system.
   It should be possible to make it handle multiple mins or maxes,
   */

affine_expr not_affine = { -1, { {0, 0} }, 0 };

static bool HaveNVars(int n)
    {
    return (n < aeMaxVars);
    }

/* a Min_or_max is true if not "none,"
   and can be negated to turn a posmax into a posmin and vice versa
 */
typedef enum { posmax = -1, none = 0, posmin = 1 } Min_or_max;




void setup_function(node *n)
    {
    if (!n->function)
	{
	symtabentry *var;
	char var_name[256];
	
	if (n->nodeop == op_declare) {
		symtabentry *sym = (symtabentry*)n->nodevalue;
		sprintf(var_name, "%s", sym->symname);
		}
	else sprintf(var_name, "<line-%d>", n->nodesequence);

	var = add_symbol(var_name, 1, symfunction, exprrefint, 0);
	var->def_node = n;

	n->function = var;  // it is important to assign function before
                            // creating omega_var (at the moment)
        
   	add_omega_var(var);
	}
    }


/* We want to use a unique UFS for all fetches from an array,
   whenever this is valid. The simplest case occurs when
   the array is never changed in the program. In such a case
   the same UFS can be used for all fetches from an array where
   all the subscripts of the array can be expressed as elements
   of the input or output tuple, i.e. they are just index variables
   of the surrounding loops.  Furthermore, it would be nice
   to do it in the lazy fasion, i.e. to create an UFS for the read-only
   array only if it is going to be used in the subscribt, assert or 
   if expression. But let for a moment do it for any array.
 */

static void setup_array_functions(void)
{
 nametabentry *n;
 node * decl;

 assert (USE_PUFS_FOR_ANALYSIS);

 if (dont_merge_uses_of_a_readonly_array)
     return;

 for (n = NameHead; n != NULL; n = n->namenext)
   {
   decl = n->namest.symdecl;
   if (decl != NULL && decl->nodelink != NULL && 
       n->namest.symclass != CLBUILTIN && n->namest.symdims !=0)
     {
      // this is really array name. Now check if it is read-only
      bool read_only = true;
      for (node * l=decl; l!=NULL; l=l->nodelink)
	{
         read_only = true;
         // because this is not a scalar we never should get phi access
         assert (!is_phi_access(l));
         if (store_op(l->nodeop))
	   {
            read_only=false;
            break;
          }
       }
      if (read_only)   
         setup_function(decl);
     } //end if
   }// endfor
}   
  

/* check that each subscript for the array fetch consists
   of the single index variable that corresponds to the
   proper surrounding loop
 */

static bool access_has_index_subscripts(node * a)
{
 assert(access_is_fetch(a));
 node * subscr;
 // find the last subscript
 for (subscr=a->nodechild; subscr->nodenext!=NULL; subscr=subscr->nodenext);
 if (subscr->nodeop != op_index)
    return false;
 context_iterator c;
 c=cont_i_for_access(a);

 // find loop corresponding to the last subscript
 while (!cont_i_done(c) && get_nodevalue_node(subscr) != c)
    cont_i_next(c);
 if (cont_i_done(c))
   return false;

 while (subscr !=NULL) 
   {
    if (subscr->nodeop != op_index)
      return false;
    while (!cont_i_cur_is_loop(c) && !cont_i_done(c))
      cont_i_next(c);
    if (cont_i_done(c))
      return false;
    if (get_nodevalue_node(subscr) != c)
      return false;
    cont_i_next(c);
    subscr=subscr->nodeprev;
  }
 if (!cont_i_done(c))
   return false;
 return true;
}
    

/* Ensure that the expression expr is an affine function of
   the variables in ae->terms[1...ae->nterms-1].petit_var.

   if USE_PUFS_FOR_ANALYSIS is non-zero,
      setup_function for each expression node that is to be
      treated as an atomic non-linear term.
      Theoretically, no other nodes should need "function" values,
      but they may get added *BELOW* such a node anyway


   return 0 for constant,
          1 for linear
          more if we failed to build "ae":
	    if we had too many variables
	    or a nasty expression but not USE_PUFS_FOR_ANALYSIS

     if return is >= 2 and dep_analysis_debug >= 2, a message will have
     been printed to debug before the return

   When this function is called, ae should have nterms set to 1 (the const)


   SOME ANACHRONISTIC, PRIMITIVE STUFF:

   the expression may also contain either
    (a) one "min" expression in a positive context or
        one "max" expression in a negative context (if min_or_max == posmin)
    (b) one "max" expression in a positive context or
        one "min" expression in a negative context (if min_or_max == posmax)
    (c) neither, if min_or_max = none
   if a or b, then N_min_or_max_args must have been set to 0 before the call,
    and it will be 0 on return if there were no mins or maxes,
        or >=1 or return if there was exactly one of the appropriate type.
        If there was more than one min or max, linearity returns > 1.

   It should be OK to use one set of variables for either side of the
   min & max, as the two sides will end up in the same omega equation 
   anyway eventually.  Thus the only inefficiency caused by having extra
   copies of the variables used on only one side, comes only in the
   omega-build code.
 */

static int N_min_or_max_args;

static unsigned int
m_linearity(affine_expr *ae, Min_or_max min_or_max, node *expr)
    {
    node *ch;
    int lin, l, lmax;
    bool function_is_set = false;

#if ! defined NDEBUG
    lin = -1;
#endif

    if(expr->shadow)
	expr = expr->shadow;

    switch(expr->nodeop)
	{

	/* THESE CASES MUST MATCH THOSE IN FIND_COEF */

    case op_constant:
	{
	/* constant term */
	lin = 0;
	break;
	}

    case op_fetch :
    case op_index :
	{
	/* fetch's value is pointer to declaration */
	/* index's value is pointer to dolimit */
	assert(get_nodevalue_node(expr)->nodeop == op_dolimit ||
	       get_nodevalue_node(expr)->nodeop == op_declare);
	Var_Id sym = get_ssa_scalar_id(expr);

	if (sym->symtag == UNTAGGED) 
	    {
	    if (!USE_PUFS_FOR_ANALYSIS &&
		!(var_id_is_const(sym) || var_id_is_index(sym)))
		{
		if (petit_args.dep_analysis_debug >= 2 )
		    fprintf(debug,
			    "Nonlinear term due to scalar that has"
			    " not been shown to be linear by IVR\n");
		lin = 2;
		}
	    else if (!HaveNVars(ae->nterms+1))
		{
		if (petit_args.dep_analysis_debug >= 1)
		    fprintf(debug,
			    "Ran out of variables in struct affine_expr\n");
		lin = 4;  /* too many vars to be linear */
		}
	    else 
		{
		assert(ae->nterms > 0);
		sym->symtag = (void *) ae->nterms;
		if (var_id_is_const(sym) || var_id_is_index(sym))
		    ae->terms[(ae->nterms)].arity = IRRELEVANT_ARITY;
		else 
		    { 
		    ae->terms[(ae->nterms)].arity =
			access_shared_depth(var_id_def_node(sym), expr);
		    add_omega_var(sym, ae->terms[(ae->nterms)].arity);
		    }
		ae->terms[(ae->nterms)++].petit_var = sym;
		lin = 1;	    /* linear */
		}
	    }
	else 
	    {
	    //assert(ae->terms[(int) sym->symtag].petit_var == sym);
	    assert(ae->terms[ae->nterms].petit_var == sym);
	    lin = 1;	    /* linear */
	    }
	break;
	}

    case op_subtract :
	{
	assert(expr->nodechild->nodenext->nodenext == NULL);

	lin = 0;
	lin = m_linearity(ae, min_or_max, expr->nodechild);
	l = m_linearity(ae, (Min_or_max) -min_or_max,
			expr->nodechild->nodenext);
	if( l > lin ) lin = l;

	break;
	}

    case op_add :
	{
	/* max of linearity of addends */
	lin = 0;
	for( ch = expr->nodechild; ch != NULL; ch = ch->nodenext )
	    {
	    l = m_linearity(ae, min_or_max, ch);
	    if( l > lin ) lin = l;
	    }

	break;
	}

    case op_multiply :
	{
	/* two more more indices multiplied is nonlinear */
	/* lin keeps track of linearity.
	   lmax is maximum subexpression linearity */
	lin = lmax = 0;
	for( ch = expr->nodechild; ch != NULL; ch = ch->nodenext )
	    {
	    unsigned int next_lin;
	    next_lin = m_linearity(ae, min_or_max, ch);
	    lin += next_lin;
	    if( (int)next_lin > lmax ) lmax = next_lin;
	    }

	if (lin > 1 && petit_args.dep_analysis_debug >= 2)
	    fprintf(debug, "Nonlinear term due to multiplication\n");

	break;
	}

    case op_min :
    case op_max :
	{
	// THESE SHOULD BE RE-DONE

	if ((min_or_max == posmin && expr->nodeop == op_min) || 
	    (min_or_max == posmax && expr->nodeop == op_max))
	    {
	    lin = 0;
	    for( ch = expr->nodechild; ch != NULL; ch = ch->nodenext )
		{
		N_min_or_max_args++;
		l = m_linearity(ae, none, ch);
		if( l > lin ) lin = l;
		}
	    }
	else 
	    {
	    if (petit_args.dep_analysis_debug >= 2 )
		fprintf(debug, "Nonlinear term due to min/max\n");

	    lin = 3;
	    }

	break;
	}

    case op_gt:
    case op_ge:
    case op_lt:
    case op_le:
    case op_eq:
    case op_ne:
	{
        //we should never get here if the operands are integers
         assert (!integer_expr(expr->nodechild->exptype) || 
                 !integer_expr(expr->nodechild->nodenext->exptype)); 
        if (petit_args.dep_analysis_debug >=2 )
           fprintf(debug, "Nonlinear term due to the comparing of reals\n");
        lin = 2;
        break;
	}

    case op_fetch_array :
	{
        lin=2;
        if ( get_nodevalue_node(expr)->function !=NULL) 
	  {
           assert (USE_PUFS_FOR_ANALYSIS);
           if (access_has_index_subscripts(expr))
             {
              function_is_set = true;
              expr->function = get_nodevalue_node(expr)->function;
              if (petit_args.dep_analysis_debug >=2)
                fprintf(debug, "Nonlinear term due to array fetch - use predefined UFS\n");
            }
          }           
        break;
	}

    case op_divide:

    default :
	{
	if (petit_args.dep_analysis_debug >= 2 )
	    fprintf(debug, "Nonlinear term due to unhandled operation\n");

	lin = 2;
	}
	}

    if (lin > 1 && USE_PUFS_FOR_ANALYSIS)
	{
        if (!function_is_set) 
   	  setup_function(expr);
	if (!HaveNVars(ae->nterms+1))
	    {
	    if (petit_args.dep_analysis_debug >= 1)
		fprintf(debug,
			"Ran out of variables in affine_expr"
			"while adding a nonlinear term\n");
	    return 4;  /* too many vars to be linear */
	    }
	assert(ae->nterms > 0);
	assert(expr->function);
	ae->terms[ae->nterms].arity = expr_invariant_at_depth(expr);
	add_omega_var(expr->function, ae->terms[(ae->nterms)].arity);
	ae->terms[ae->nterms++].petit_var = expr->function;
	
	lin = 1;  // just like a variable reference
	}

    return lin;

    }/* m_linearity */


/*
  The function below controls when we "throw out" some excessively
  complex yet still linear expressions by simply reporting their
  linearity as "2" (nonlinear) rather than 1 (or 0).

  If this function returns NULL, leave linearity alone;
  otherwise, treat it as 2.

  The exact details of what we return depend on what we want to throw out.
  Currently we're experimenting.
 */

static bool pretend_not_affine(const affine_expr *ae, node *n)
{
    char *why_not = 0;

    if (petit_args.dep_analysis_debug >= 3)
	{
	fprintf(debug,
		"Checking affine expresssion %s (node at %p at line %d)"
		" to see if we want to pretend it's not affine.\n",
		print_rep(ae), n, n->nodesequence);
	}

    assert(ae);
    assert(ae->terms[0].petit_var == 0);

    // currently restrict to LI(2)-unit:

    int nvars = 0;
    bool nonunit = false;

    for (int v = 1; v<ae->nterms; v++)
	{
	assert(ae->terms[v].petit_var);

	// The following is featured on "America's Funniest Switch Statemests"
	switch (ae->terms[v].coefficient)
	    {
	case 0:
	    break;
	default:
	    nonunit=true;
	case 1:
	case -1:
	    nvars++;
	    }
	}

    if (nvars > 1 && nonunit)
	why_not = "it's not LI(2) and has nonunit coefficients";
    else if (nvars > 1)
	why_not = "it's not LI(2)";
    else if (nonunit)
	why_not = "it has nonunit coefficients";
    else
	why_not = 0;

    if (why_not && petit_args.dep_analysis_debug >= 2)
	fprintf(debug,
		"Pretending affine expresssion %s (node at %p at line %d) is"
		" not affine because %s.\n",
		print_rep(ae), n, n->nodesequence,
		why_not);

    return why_not != 0;
}


/* find constant factor of the multiplication operation.
   F.e. for (i+j) * 2 * (3 * i) constant factor is 6
 */

static int find_multiply_const(node * expr) {
  assert(expr->nodeop == op_multiply);

  int factor=1;

  node * child;
  for (child=expr->nodechild; child!=NULL; child=child->nodenext)
    {
     node * c=child->shadow ? child->shadow  : child;
     if (c->nodeop == op_constant)
            factor *=c->nodevalue;
     else if (c->nodeop == op_multiply)
            factor *= find_multiply_const(c);
    }
  return factor;
}

/* if petit_var points to a node for an index or id used in the expression,
         return (+ the coefficient of that index/id, the constant term)
   else  return the constant term

   expr MUST point to a node whose linearity is 1 or 0, and which
   has at most one min or max.  If it has one, then l_or_r tells
   which part of the min or max should be considered.  If there is
   no min or max, l_or_r is ignored
 */

static int find_coef(node *expr, int mom_arg, Var_Id petit_var)
    {
    node *ch;
    int val, mpy;

    if(expr->shadow)
	expr = expr->shadow;

    if (expr->function)
	{
         if (petit_var == expr->function) 
	   {
            if (expr->nodeop == op_multiply)
              return find_multiply_const(expr);
            else
              return 1;
           } 
         else
           return 0;
	}


    switch(expr->nodeop)
	{

	/* THESE CASES MUST MATCH THOSE IN M_LINEARITY */

    case op_constant:
	{
	/* constant term */
	return expr->nodevalue;
	}

    case op_fetch :  /* nodevalue points to declaration */
    case op_index :  /* nodevalue points to dolimit */
	{
	assert((expr->nodeop == op_fetch
		&& get_nodevalue_node(expr)->nodeop == op_declare)
		|| get_nodevalue_node(expr)->nodeop == op_dolimit);
	/* return 1 if this is the var of interest, else zero */

	return get_ssa_scalar_id(expr) == petit_var;
	}

    case op_add :
	{
	/* sum up */
	val = 0;
	for( ch = expr->nodechild; ch != NULL; ch = ch->nodenext )
	    {
	    val = val + find_coef(ch, mom_arg, petit_var);
	    }
	return val;
	}

    case op_subtract :
	{
	/* sum up */
	val = 0;
	mpy = 1;
	for( ch = expr->nodechild; ch != NULL; ch = ch->nodenext )
	    {
	    val = val + mpy*find_coef(ch, mom_arg, petit_var);
	    mpy = -1; /* negate other (second) operands */
	    }
	return val;
	}

    case op_multiply :
	{
	val = 1;
	for( ch = expr->nodechild; ch != NULL; ch = ch->nodenext )
	    {
	    val = val * find_coef(ch, mom_arg, petit_var);
	    }
	return val;
	}

    case op_min :
    case op_max :

	{
	node *arg = expr->nodechild;
	int i;

	assert(mom_arg > 0);

	for (i=1 ; i<mom_arg; i++)
	    {
	    assert(arg->nodenext);
	    arg = arg->nodenext;
	    }

	return find_coef(arg, -1, petit_var);
	}

    case op_divide:
    case op_fetch_array:
    default :
	{
	/* anything else should not happen */

	assert(0 && "inconsistency between find_coef and m_linearity");
	}
	}
    return 0;
    }/* find_coef */


/* Attach an "affine_expr" structure to n->nodeaffine. */

static void find_affine_expr(node *n, Min_or_max mm)
    {
    affine_expr *ae;
    int k, t;

    N_min_or_max_args = 0;

    if (n->nodeaffine)
	free((char *)n->nodeaffine);

    n->nodeaffine = ae = (affine_expr *)calloc(1, sizeof(affine_expr));
    if (ae == 0) out_of_memory();

    ae->other_branch = 0;
    ae->terms[0].petit_var = 0;
    ae->terms[0].arity = IRRELEVANT_ARITY;   
    ae->nterms = 1;

#if ! defined NDEBUG
    {
    nametabentry *n;
    for( n = NameHead; n != NULL; n = n->namenext )
	assert(n->namest.symtag == UNTAGGED);
    }
#endif

    if (m_linearity(n->nodeaffine, mm, n) <= 1)
	{
	int mom_arg;
	affine_expr *other = ae;
	
	mom_arg = 1;

	other->terms[0].coefficient = k = find_coef(n, mom_arg, 0);
	for (t = 1; t<other->nterms; t++)
	    other->terms[t].coefficient =
		find_coef(n, mom_arg, other->terms[t].petit_var)-k;
	
	if (!USE_PUFS_FOR_ANALYSIS && petit_args.li2_only &&
	    pretend_not_affine(other, n))
	    {
	    for (t = 1; t<ae->nterms; t++)
		ae->terms[t].petit_var->symtag = UNTAGGED;
	    FreeAffineExpr(n->nodeaffine);
	    n->nodeaffine = &not_affine;
	    return;
	    }
	  
	assert(other->terms[0].petit_var == NULL);
	assert(other->terms[other->nterms].petit_var == NULL);
	
	for (mom_arg = 1 ; mom_arg <= N_min_or_max_args ; mom_arg++) 
	    {
	    affine_expr *tmp;

	    tmp = (affine_expr *) calloc(1, sizeof(affine_expr));
	    if (tmp == 0) out_of_memory();
	    other->other_branch = tmp;
	    other = tmp;

	    other->nterms = ae->nterms;
	    for (t = 1; t<ae->nterms; t++) {
		other->terms[t].petit_var = ae->terms[t].petit_var;
                other->terms[t].arity = ae->terms[t].arity;
            } 
	    
	    other->terms[0].coefficient = k = find_coef(n, mom_arg, 0);
	    for (t = 1; t<ae->nterms; t++)
		other->terms[t].coefficient =
		    find_coef(n, mom_arg, other->terms[t].petit_var) - k;
	    
	    assert(other->terms[0].petit_var == NULL);
	    assert(other->terms[ae->nterms].petit_var == NULL);

	    if (!USE_PUFS_FOR_ANALYSIS && petit_args.li2_only &&
		pretend_not_affine(other, n))
		{
		for (t = 1; t<ae->nterms; t++)
		    ae->terms[t].petit_var->symtag = UNTAGGED;
		FreeAffineExpr(n->nodeaffine);
		n->nodeaffine = &not_affine;
		return;
		}

	    }
	other->other_branch = NULL;
        for (t = 1; t<ae->nterms; t++)
	  ae->terms[t].petit_var->symtag = UNTAGGED;
	}
    else
	{
	assert(!USE_PUFS_FOR_ANALYSIS);
        for (t = 1; t<ae->nterms; t++)
	  ae->terms[t].petit_var->symtag = UNTAGGED;
	free((char *)n->nodeaffine);
	  n->nodeaffine = &not_affine;
	}


#if ! defined NDEBUG
    {
    nametabentry *n;
    for( n = NameHead; n != NULL; n = n->namenext )
	assert(n->namest.symtag == UNTAGGED);
    }
#endif
    }


void propagate_non_affinity(node * n)
{
 assert (!USE_PUFS_FOR_ANALYSIS);
 switch (n->nodeop) {
 case op_and:
 case op_or:
      if (!is_affine(n->nodechild->nodeaffine) &&
          !is_affine(n->nodechild->nodenext->nodeaffine))
	  {
            n->nodeaffine=&not_affine;
            n->nodechild->nodeaffine = 0;
            n->nodechild->nodenext->nodeaffine = 0;
	  }
      break;
 case op_not:
      if (!is_affine(n->nodechild->nodeaffine))
	{
          n->nodeaffine = & not_affine;
          n->nodechild->nodeaffine = 0;
	}
      break;
 default:
      assert (0 && "propagate_non_affinity is  called for non-logical operation");
  }
}   



/*
   This code sets things up to be consistent with lang-interf.[hc]:
   For all operations we care to handle, we attach their operands'
           affine expressions (left then right) to the if node.
   For other cases, we set the if node's "function" to a new UFS.
 */

static void subtree_find_affine_exprs(node *);

static void r_find_affine_exprs(node *nn)
{
    for( ; nn != NULL; nn = nn->nodenext )
       subtree_find_affine_exprs(nn);       
}


static void subtree_find_affine_exprs(node *nn)
    {
    node *sub;
    switch( nn->nodeop )
	{
    CASE_ARRAY_REF:
	for (sub = nn->nodechild; sub != NULL; sub=sub->nodenext) 
	    {
	    find_affine_expr(sub, none);

	    /* if subscript not affine, it may itself be an array ref */
	    if(is_array_ref(sub) && petit_args.printIxArrays)
		{
		fprintf(stderr, "== Index Array at stmt %d ==\n",
			nn->nodesequence);
		if (petit_args.dep_analysis_debug >= 2) 
		    {
		    fprintf(debug, "== Index Array at stmt %d ==\n",
			    nn->nodesequence);
		    }
		}
	    r_find_affine_exprs(sub);
	    }

    CASE_SCALAR_REF:
	break;
	  
    case op_dolimit:
	    
	sub = nn->nodenext;  /* lower bound */
	find_affine_expr(sub, posmax);
	r_find_affine_exprs(sub);

	sub = sub->nodenext;  /* upper bound */
	find_affine_expr(sub, posmin);
	r_find_affine_exprs(sub);

	sub = sub->nodenext;  /* step */
	if (sub != NULL)
	    {
	    find_affine_expr(sub, none);
	    r_find_affine_exprs(sub);
	    }

	r_find_affine_exprs( nn->nodechild );  /* stmts in loop */

	return;

    case op_if:

	assert(nn->nodechild->nodenext->nodeop == op_then);
	assert(nn->nodechild->nodenext->nodenext == NULL ||
	       (nn->nodechild->nodenext->nodenext->nodeop == op_else &&
		nn->nodechild->nodenext->nodenext->nodenext == NULL));

	if (!logic_op(nn->nodechild->nodeop) && 
	    !compare_op(nn->nodechild->nodeop))
	    {
	    find_affine_expr(nn->nodechild,none);
	    subtree_find_affine_exprs(nn->nodechild);
	    }
	else
	    subtree_find_affine_exprs(nn->nodechild);
	assert (nn->nodechild->nodenext->nodeop == op_then);
	r_find_affine_exprs(nn->nodechild->nodenext);
	break;
       

    case op_and:
    case op_or:
	    // check second operand first
  
            if (!logic_op(nn->nodechild->nodenext->nodeop) && 
                !compare_op(nn->nodechild->nodenext->nodeop))
		{
		find_affine_expr(nn->nodechild->nodenext,none);
		subtree_find_affine_exprs(nn->nodechild->nodenext);
		}
            else
		r_find_affine_exprs(nn->nodechild->nodenext);
          
	//now just fall through to the unary operations where the
	// first operand is done 
    case op_assert:
    case op_not:
            if (!logic_op(nn->nodechild->nodeop) && 
                !compare_op(nn->nodechild->nodeop))
		{
		find_affine_expr(nn->nodechild, none);
		subtree_find_affine_exprs( nn->nodechild );
		}
            else
		subtree_find_affine_exprs(nn->nodechild); 
               
            if (!USE_PUFS_FOR_ANALYSIS && nn->nodeop!=op_assert)
                propagate_non_affinity(nn);
                  
	break;

    case op_eq:
    case op_ne:
    case op_gt:
    case op_ge:
    case op_lt:
    case op_le:
       if (!integer_expr(nn->nodechild->exptype) || 
           !integer_expr(nn->nodechild->nodenext->exptype)) 
        { 
          find_affine_expr(nn, none);
        }
       else       
	{
	  find_affine_expr(nn->nodechild, none);
  	  find_affine_expr(nn->nodechild->nodenext, none);
        }
	subtree_find_affine_exprs(nn->nodechild);
	subtree_find_affine_exprs(nn->nodechild->nodenext);
	break;
    default:
	r_find_affine_exprs( nn->nodechild );
	}
}


#define tested(node)  (!USE_PUFS_FOR_ANALYSIS || is_affine(node))

static void r_check_affine_exprs(node *n)
    {
    for( ; n != NULL; n = n->nodenext )
	{
	switch (n->nodeop)
	    {
	CASE_ARRAY_REF:
	    {
	    for (node *sub = n->nodechild; sub != NULL; sub=sub->nodenext)
		{
		assert(sub->nodeaffine && tested(sub->nodeaffine));
		}
	    break;
	    }

	case op_dolimit:
	    {
	    // lower bound
	    assert(n->nodenext->nodeaffine &&
		   tested(n->nodenext->nodeaffine));
	    // upper bound
	    assert(n->nodenext->nodenext->nodeaffine &&
		   tested(n->nodenext->nodenext->nodeaffine));
	    // step
	    assert(!n->nodenext->nodenext->nodenext ||
		   (n->nodenext->nodenext->nodenext->nodeaffine &&
		    tested(n->nodenext->nodenext->nodenext->nodeaffine)));
	    break;
	    }
	default:
	    ;
	    }
	r_check_affine_exprs(n->nodechild);
	}
    }


/* is ae made of only symbolic constants? */
static int no_indices(affine_expr *ae)
{
    int i;
    for (i=1; i<ae->nterms; i++) {
	if (!var_id_is_const(ae->terms[i].petit_var) &&
	    !var_id_is_var(ae->terms[i].petit_var))
	    return 0;
    }
    return ae->other_branch == NULL ? 1 : no_indices(ae->other_branch);
}


void find_affine_exprs(void)
    {
// NOT REENTRANT, DUE TO TAG FIELD
#if ! defined NDEBUG
    static int recursive = 0;

    assert(recursive++ == 0);
#endif

    nametabentry *n;
    for( n = NameHead; n != NULL; n = n->namenext ){
	assert(n->namest.symtag == UNTAGGED);
    }

    if (USE_PUFS_FOR_ANALYSIS)  
      setup_array_functions(); 
    r_find_affine_exprs(Entry);

#if ! defined NDEBUG
    r_check_affine_exprs(Entry);
#endif


    for( n = NameHead; n != NULL; n = n->namenext ){
	assert(n->namest.symtag == UNTAGGED);
    }

#if ! defined NDEBUG
    assert(--recursive == 0);
#endif
}

void find_affine_exprs_in(node *inwhat)
{
    nametabentry *n;
    for( n = NameHead; n != NULL; n = n->namenext ){
	assert(n->namest.symtag == UNTAGGED);
    }

    r_find_affine_exprs(inwhat);

    for( n = NameHead; n != NULL; n = n->namenext ){
	assert(n->namest.symtag == UNTAGGED);
    }
}

/* zap all functions in the program */

static void zap_functions(void) {
   nametabentry **pname;

   for (pname=&NameHead; *pname!=NULL; ) {
   if ((*pname)->namest.symtype == symfunction) 
     {
      symtabentry *var = &((*pname)->namest);
      assert (var->def_node->function == var);
      var->def_node->function=NULL;
      nametabentry *n = *pname;
      *pname = (*pname)->namenext;
      if (n->namest.symname)
        delete n->namest.symname;
      delete n;
    }
   else {
     pname = &(*pname)->namenext;
  }
 }
}
     
  

void r_zap_affine_exprs(node *n)
{
    node *nn;
    for( nn=n; nn != NULL; nn = nn->nodenext ){
        FreeAffineExpr(nn->nodeaffine);
	nn->nodeaffine = NULL;
	r_zap_affine_exprs( nn->nodechild );
    }
}

static void unaffinify_assertions(void)
{
    node *ass;

    for (ass = Assertions; ass; ass = ass->nodelink) {
	assert(node_is_affine(ass));
#if 0
	this has been done
	r_zap_affine_exprs(ass);
#endif
    }
}

void zap_affine_exprs(void)
{
    r_zap_affine_exprs(Entry);
    zap_functions();

    unaffinify_assertions();
}

/*
 * Compare two affine expressions
 */
int CmpAffineExprs(affine_expr *e1, affine_expr *e2) {
  int i1, i2;
  if(e1->nterms!=e2->nterms ||
     e1->terms[0].coefficient!=e2->terms[0].coefficient)
    return(1);
  for(i1=1; i1<e1->nterms; i1++) {
    affine_term *t1 = &e1->terms[i1];
    for(i2=1; i2<e2->nterms; i2++) {
      affine_term *t2 = &e2->terms[i2];
      if(t2->petit_var == t1->petit_var) {
	if(t2->coefficient != t1->coefficient)
	  return(1);
	else if (t2->arity != t2->arity)
          return(1);
        else
	  goto NextI1;
      }
    }
    return(1);
    NextI1:;
  }
  return(0);
}

/*
 * Copy affine expression stuff
 */
affine_expr *CopyAffineExpr(affine_expr *ae) {
  if(ae != NULL && ae != &not_affine) {
    affine_expr *new_expr = new affine_expr;
    memcpy((char*)new_expr, (char*)ae, sizeof(affine_expr));
    new_expr->other_branch = CopyAffineExpr(ae->other_branch);
    return(new_expr);
  } else {
    return(ae);
  }
}


/*
 * Free affine expression stuff
 */
void FreeAffineExpr(affine_expr *ae) {
  if(ae != NULL && ae != &not_affine) {
    FreeAffineExpr(ae->other_branch);
    free((char *)ae);
  }
}

static void append (char *where, int &remains, const char *what)
{
    if (remains-3 > 0)
    {
	int len = strlen(what);
	if (len > remains-3)
	    {
	    strcat(where, "...");
	    remains = 0;
	    }
	else
	    {
	    strncat(where, what, remains);
	    remains -= len;
	    }
    }
}

static void append (char *where, int &remains, int num)
{
    const int sz=64;
    char buf[sz];
    for (int i=0; i<sz; i++) buf[i]='\0';  // paranioa
    sprintf(buf, "%d", num);
    append(where, remains, buf);
}

char *print_rep(const affine_expr *ae)
{
    const int sz = 64*aeMaxVars+256;
    static char buf[sz];
    buf[0]=0;
    int remains = sz-1;

    for (int i = 1; i < ae->nterms; i++)
	{
	if (i>1)
	    append(buf, remains, "+");
	if (ae->terms[i].coefficient != 1)
	    append(buf, remains, ae->terms[i].coefficient);
	append(buf, remains, var_id_name(ae->terms[i].petit_var));
	}
    if (ae->terms[0].coefficient != 0)
	{
	if (ae->nterms > 1)
	    append(buf, remains, "+");
	append(buf, remains, ae->terms[0].coefficient);
	}

    return buf;
}

} // end of namespace omega
