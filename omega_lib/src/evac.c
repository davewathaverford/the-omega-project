int evac_debug = 0;

#if defined STUDY_EVACUATIONS

#include <basic/bool.h>
#include <omega/Relations.h>
#include <omega/pres_conj.h>
#include <omega/evac.h>
#include <omega/omega_core/debugging.h>

#include <omega/omega_core/oc_i.h>

namespace omega {

char *evac_names[] = { "trivial",
		       "offset",
		       "subseq",
		       "off_sub",
//		       "perm.",
		       "affine",
		       "nasty" };
 
int single_evacs[evac_nasty+1];
int double_evacs[evac_nasty+1][evac_nasty+1];

/*
 * We're going to try to describe the equalities among a set of variables
 * We want to perform some substitutions to ensure that we don't miss
 *   v_1 = v_2 due to its expression as v_1 = v_3 && v_2 = v_3
 * We therefore try to substitute out all variables that we don't care
 *   about (e.g., v_3 in the above example).
 */

static bool try_to_sub(Problem *p, int col)
    {
    int e, i;

    if (!p->variablesInitialized) 
	{
	p->initializeVariables();
	}

    assert(col <= p->nVars);
    assert(!inApproximateMode);

    for(e=0;e<p->nEQs;e++)
	if (p->EQs[e].coef[col] == 1 || p->EQs[e].coef[col] == -1)  
	    {
	    int var = p->var[col];
	    p->doElimination(e, col);
	    if (col != p->nVars + 1)
		p->forwardingAddress[p->var[p->nVars+1]] = col;
	    assert(p->SUBs[p->nSUBs-1].key = var);
	    p->forwardingAddress[var] = -p->nSUBs;
	    break;
	    }
 
    if (e == p->nEQs)
	return false;

    for (int c=0;c<=p->nVars;c++)
	{
	assert(p->EQs[e].coef[c] == 0);
	}

    p->nEQs--;
    if (e < p->nEQs) eqnncpy(&p->EQs[e], &p->EQs[p->nEQs], p->nVars);

    for (i = 0; i < p->nSUBs; i++)
	{
	assert(p->forwardingAddress[p->SUBs[i].key] == -i - 1);
	}

    return true;
    }


// should be static, but must be a friend
bool check_subseq_n(Conjunct *c, Sequence<Variable_ID> &evac_from, Sequence<Variable_ID> &evac_to, int n_from, int n_to, int max_arity, int n, bool allow_offset)
    {
    // check each position v to see if from[v] == to[v+n] (+ offset)
    
    assert(max_arity + n <= n_to);

    for (int v = 1; v <= max_arity; v++)
	{
	// first, get rid of possible interlopers:

	int col;
	Conjunct *d = c->copy_conj_same_relation();
	for (int tv = 1; tv <= n_to; tv++) if (tv != v+n)
	    if ((col = d->find_column(evac_to[tv])) > 0)
		try_to_sub(d->problem, col);
	for (int fv = 1; fv <= n_from; fv++) if (fv != v)
	    if ((col = d->find_column(evac_from[fv])) > 0)
		try_to_sub(d->problem, col);

	int c_to = d->find_column(evac_to[v+n]);
	int c_from = d->find_column(evac_from[v]);
	assert(c_to > 0);
	assert(c_from > 0);
	assert(c_to != c_from);

	// now, just look for an equality c_to = c_from + offset

	bool found_needed_eq = false;

	for (int e = 0; e < d->problem->nEQs; e++)
	    {
	    if (d->problem->EQs[e].coef[c_from] != 0)
		{
		for (int k = allow_offset?1:0; k < d->problem->nVars; k++)
		    if (k!=c_to && k!=c_from && d->problem->EQs[e].coef[k]!=0)
			break;  // this EQ is not what we need
		if (k == d->problem->nVars) // this EQ is what we need
		    {
		    found_needed_eq = true;
		    break;
		    }
		}
	    }

	delete d;

	if (!found_needed_eq)
	    return false;  // no EQ did what we need
	}

    return true;
    }

void assert_subbed_syms(Conjunct *c)
    {
    int v, col;

    // where possible, symbolic constants must have been subbed out
    for (v = 1; v <= c->relation()->global_decls()->length(); v++)
	if ((col = c->find_column((*c->relation()->global_decls())[v]))>0)
	    assert(!try_to_sub(c->problem, col));
    }


static bool check_offset(Conjunct *c, Sequence<Variable_ID> &evac_from, Sequence<Variable_ID> &evac_to, int n_from, int n_to, int max_arity)
    {
    assert_subbed_syms(c);

    return check_subseq_n(c,evac_from,evac_to,n_from,n_to,max_arity,0,true);
    }

static bool check_subseq(Conjunct *c, Sequence<Variable_ID> &evac_from, Sequence<Variable_ID> &evac_to, int n_from, int n_to, int max_arity)
    {
    assert_subbed_syms(c);

    for (int i = 0; i <= n_to - max_arity; i++)
	if (check_subseq_n(c,evac_from,evac_to,n_from,n_to,max_arity,i,false))
	    return true;

    return false;
    }

static bool check_offset_subseq(Conjunct *c, Sequence<Variable_ID> &evac_from, Sequence<Variable_ID> &evac_to, int n_from, int n_to, int max_arity)
    {
    assert_subbed_syms(c);

    for (int i = 0; i <= n_to - max_arity; i++)
	if (check_subseq_n(c,evac_from,evac_to,n_from,n_to,max_arity,i,true))
	    return true;

    return false;
    }

bool check_affine(Conjunct *d, Sequence<Variable_ID> &evac_from, Sequence<Variable_ID> &evac_to, int n_from, int n_to, int max_arity)
    {
    int v, col;
    Conjunct *c = d->copy_conj_same_relation();
    assert_subbed_syms(c);

    // try to find substitutions for all evac_to variables
    for (v = 1; v <= max_arity; v++)
	if ((col = c->find_column(evac_to[v])) > 0)
	    try_to_sub(c->problem, col);
    
    // any that didn't have substitutions, aren't affine
    for (v = 1; v <= max_arity; v++)
	if (c->find_column(evac_to[v]) >= 0)
	    {
	    delete c;
	    return false;
	    }

    // FERD - disallow symbolic constants?
    delete c;
    return true;
    }


evac study(Conjunct *C, Sequence<Variable_ID> &evac_from, Sequence<Variable_ID> &evac_to, int n_from, int n_to, int max_arity)
{
    assert(max_arity > 0);
    assert(max_arity <= C->relation()->n_inp());
    assert(max_arity <= C->relation()->n_out());

    assert((&evac_from == &input_vars && &evac_to == &output_vars) ||
	   (&evac_from == &output_vars && &evac_to == &input_vars));

    evac ret = evac_nasty;

    if (C->query_guaranteed_leading_0s() >= max_arity)
	ret = evac_trivial;
    else
	{
	Conjunct *c = C->copy_conj_same_relation();
	assert(c->relation() == C->relation());

	if (evac_debug >= 3)
	    {
	    fprintf(DebugFile, "About to study %s evacuation for conjunct\n",
		    &evac_from == &input_vars ? "In-->Out" : "Out-->In");
	    use_ugly_names++;
	    C->prefix_print(DebugFile);
	    use_ugly_names--;
	    }

	bool sat = simplify_conj(c, true, 4, black);
	assert(sat);  // else c is deleted

	int v, col;

	// Substitute out all possible symbolic constants
	assert(c->problem->nSUBs == 0);
	for (v = 1; v <= c->relation()->global_decls()->length(); v++)
	    if ((col = c->find_column((*c->relation()->global_decls())[v]))>0)
		try_to_sub(c->problem, col);

	if (check_offset(c, evac_from, evac_to, n_from, n_to, max_arity))
	    ret = evac_offset;
	else if (check_subseq(c, evac_from, evac_to, n_from, n_to, max_arity))
	    ret = evac_subseq;
	else if (check_offset_subseq(c, evac_from, evac_to, n_from, n_to, max_arity))
	    ret = evac_offset_subseq;
	else if (check_affine(c, evac_from, evac_to, n_from, n_to, max_arity))
	    ret = evac_affine;

	delete c;
    	}

    if (evac_debug >= 2)
	{
	if ((evac_debug == 2 && ret != evac_trivial && ret != evac_nasty))
	    {
	    fprintf(DebugFile, "Studied %s evacuation for conjunct\n",
		    &evac_from == &input_vars ? "In-->Out" : "Out-->In");
	    use_ugly_names++;
	    C->prefix_print(DebugFile);
	    use_ugly_names--;
	    }

	fprintf(DebugFile, "Saw evacuation type %s\n", evac_names[ret]);
	}

    return ret;
}


void study_evacuation(Conjunct *C, which_way dir, int max_arity)
    {
    if (evac_debug > 0)
	{
	assert(max_arity >= 0);

	if (max_arity > 0)
	    if (dir == in_to_out)
		{
		assert(max_arity <= C->relation()->n_inp());
		if (max_arity <= C->relation()->n_out())
		    single_evacs[study(C, input_vars, output_vars,
				       C->relation()->n_inp(),
				       C->relation()->n_out(),
				       max_arity)]++;
		}
	    else
		{
		assert(max_arity <= C->relation()->n_out());
		if (max_arity <= C->relation()->n_inp())
		    single_evacs[study(C, output_vars, input_vars,
				       C->relation()->n_out(),
				       C->relation()->n_inp(),
				       max_arity)]++;
		}
	}
    }

void study_evacuation(Conjunct *C1, Conjunct *C2, int max_arity)
    {
    if (evac_debug > 0)
	{
	assert(max_arity >= 0);
	assert(max_arity <= C1->relation()->n_inp());
	assert(C2->relation()->n_out() == C1->relation()->n_inp());

	if (max_arity > 0)
	    if (max_arity <= C1->relation()->n_out() &&
		max_arity <= C2->relation()->n_inp())
		{
		double_evacs[study(C1, input_vars, output_vars, 
				   C1->relation()->n_inp(),
				   C1->relation()->n_out(),
				   max_arity)]
		    [study(C2, output_vars, input_vars,
			   C2->relation()->n_out(),
			   C2->relation()->n_inp(),
			   max_arity)]++;
		}
	    else if (max_arity <= C1->relation()->n_out())
		{
		single_evacs[study(C1, input_vars, output_vars,
				   C1->relation()->n_inp(),
				   C1->relation()->n_out(),
				   max_arity)]++;
		}
	    else if (max_arity <= C2->relation()->n_inp())
		{
		single_evacs[study(C2, output_vars, input_vars,
				   C2->relation()->n_out(),
				   C2->relation()->n_inp(),
				   max_arity)]++;
		}
	}
    }

class Evac_info_printer {
public:
    ~Evac_info_printer();
};

Evac_info_printer::~Evac_info_printer()
{
    if (evac_debug > 0)
	{
	int i, j;

	fprintf(DebugFile, "\n");

	fprintf(DebugFile, "SINGLE");
	for (i = 0; i <= evac_nasty; i++)
	    fprintf(DebugFile, "\t%s", evac_names[i]);
	fprintf(DebugFile, "\n");
	
	for (i = 0; i <= evac_nasty; i++)
	    fprintf(DebugFile, "\t%d", single_evacs[i]);
	fprintf(DebugFile, "\n\n");
	
	
	fprintf(DebugFile, "DOUBLE");
	for (i = 0; i <= evac_nasty; i++)
	    fprintf(DebugFile, "\t%s", evac_names[i]);
	fprintf(DebugFile, "\n");
	
	for (i = 0; i <= evac_nasty; i++)
	    {
		fprintf(DebugFile, "%s\t", evac_names[i]);
		for (j = 0; j <= evac_nasty; j++)
		    fprintf(DebugFile, "%d\t", double_evacs[i][j]);
		fprintf(DebugFile, "\n");
	    }
	}
}

static Evac_info_printer print_stats_at_exit;

} // end of namespace omega

#endif

