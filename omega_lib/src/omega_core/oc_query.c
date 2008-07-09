/* $Id: oc_query.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $*/

#include <basic/bool.h>
#include <omega/omega_core/oc_i.h>

namespace omega {

void
Problem::
unprotectVariable( int v)
    {
    int e, j, i;
    coef_t t;
    i = forwardingAddress[v];
    if (i < 0) 
	{
	i = -1 - i;
	nSUBs--;
	if (i < nSUBs) 
	    {
	    eqncpy(&SUBs[i], &SUBs[nSUBs]);
	    forwardingAddress[SUBs[i].key] = -i - 1;
	    };
	}
    else 
	{
	int bringToLife[maxVars];
	int comingBack = 0;
	int e2;
	for (e = nSUBs - 1; e >= 0; e--)
	    if ((bringToLife[e] = (SUBs[e].coef[i] != 0)))
		comingBack++;

	for (e2 = nSUBs - 1; e2 >= 0; e2--)
	    if (bringToLife[e2]) 
		{

		nVars++;
		safeVars++;
		if (safeVars < nVars) 
		    {
		    for (e = nGEQs - 1; e >= 0; e--) 
			{
			GEQs[e].coef[nVars] = GEQs[e].coef[safeVars];
			GEQs[e].coef[safeVars] = 0;
			};
		    for (e = nEQs - 1; e >= 0; e--) 
			{
			EQs[e].coef[nVars] = EQs[e].coef[safeVars];
			EQs[e].coef[safeVars] = 0;
			};
		    for (e = nSUBs - 1; e >= 0; e--) 
			{
			SUBs[e].coef[nVars] = SUBs[e].coef[safeVars];
			SUBs[e].coef[safeVars] = 0;
			};
		    var[nVars] = var[safeVars];
		    forwardingAddress[var[nVars]] = nVars;
		    }
		else 
		    {
		    for (e = nGEQs - 1; e >= 0; e--) 
			{
			GEQs[e].coef[safeVars] = 0;
			};
		    for (e = nEQs - 1; e >= 0; e--) 
			{
			EQs[e].coef[safeVars] = 0;
			};
		    for (e = nSUBs - 1; e >= 0; e--) 
			{
			SUBs[e].coef[safeVars] = 0;
			};
		    };

		var[safeVars] = SUBs[e2].key;
		forwardingAddress[SUBs[e2].key] = safeVars;

		int neweq = newEQ();
		eqncpy(&(EQs[neweq]), &(SUBs[e2]));
		EQs[neweq].coef[safeVars] = -1;
		if (e2 < nSUBs - 1)
		    eqncpy(&(SUBs[e2]), &(SUBs[nSUBs - 1]));
		nSUBs--;
		};

	if (i < safeVars) 
	    {
	    j = safeVars;
	    for (e = nSUBs - 1; e >= 0; e--) 
		{
		t = SUBs[e].coef[j];
		SUBs[e].coef[j] = SUBs[e].coef[i];
		SUBs[e].coef[i] = t;
		};
	    for (e = nGEQs - 1; e >= 0; e--)
		if (GEQs[e].coef[j] != GEQs[e].coef[i]) 
		    {
		    GEQs[e].touched = TRUE;
		    t = GEQs[e].coef[j];
		    GEQs[e].coef[j] = GEQs[e].coef[i];
		    GEQs[e].coef[i] = t;
		    };
	    for (e = nEQs - 1; e >= 0; e--) 
		{
		t = EQs[e].coef[j];
		EQs[e].coef[j] = EQs[e].coef[i];
		EQs[e].coef[i] = t;
		};
	    {
	      short t;
	      t = var[j];
	      var[j] = var[i];
	      var[i] = t;
	    }
	    forwardingAddress[var[i]] = i;
	    forwardingAddress[var[j]] = j;
	    };
	safeVars--;
	};
    chainUnprotect();

    }

void
Problem::
constrainVariableSign( int color, int i, int sign)
    {
    int nV = nVars;
    int e, k, j;

    k = forwardingAddress[i];
    if (k < 0) 
	{
	k = -1 - k;

	if (sign != 0) 
	    {
	    e = newGEQ();
	    eqncpy(&GEQs[e], &SUBs[k]);
	    for (j = 0; j <= nV; j++)
		GEQs[e].coef[j] *= sign;
	    GEQs[e].coef[0]--;
	    GEQs[e].touched = 1;
	    GEQs[e].color = color;
	    }
	else 
	    {
	    e = newEQ();
	    eqncpy(&EQs[e], &SUBs[k]);
	    EQs[e].color = color;
	    };

	}
    else if (sign != 0) 
	{
	e = newGEQ();
	eqnzero(&GEQs[e]);
	GEQs[e].coef[k] = sign;
	GEQs[e].coef[0] = -1;
	GEQs[e].touched = 1;
	GEQs[e].color = color;
	}
    else 
	{
	e = newEQ();
	eqnzero(&EQs[e]);
	EQs[e].coef[k] = 1;
	EQs[e].color = color;
	};
    /*
      unprotectVariable(i);
      return (simplifyProblem(0,1,0));
      */
    }

void
Problem::
constrainVariableValue( int color, int i, int value)
    {
    int e, k;

    k = forwardingAddress[i];
    if (k < 0) 
	{
	k = -1 - k;

	e = newEQ();
	eqncpy(&EQs[e], &SUBs[k]);
	EQs[e].coef[0] -= value;

	}
    else 
	{
	e = newEQ();
	eqnzero(&EQs[e]);
	EQs[e].coef[k] = 1;
	EQs[e].coef[0] = -value;
	};
    EQs[e].color = color;
    }

// Analyze v1-v2
void Problem:: query_difference(int v1, int v2, 
coef_t &lowerBound, coef_t &upperBound, bool &guaranteed) 
    {
    int nV = nVars;
    int e,i,e2;

    coef_t lb1,ub1; 
    coef_t lb2,ub2; 
    assert(nSUBs == 0); 
    lowerBound = negInfinity;
    lb1 = lb2  = negInfinity;
    upperBound = posInfinity;
    ub1 = ub2 = posInfinity;
    guaranteed = true;
    for (e = nEQs - 1; e >= 0; e--) 
	{
	if (EQs[e].coef[v1] == 0 && EQs[e].coef[v2] == 0)  continue;
	for(i=nV;i>0;i--)  
	    if (EQs[e].coef[i] && i!=v1 && i != v2) 
		{
		break;
		};
	if (i != 0) 
	    {
	    if (i > safeVars) 
		{
		// check to see if this variable appears anywhere else
		for(e2 = nEQs-1; e2>=0;e2--) if (e != e2 && EQs[e2].coef[i]) break;
		if (e2 < 0)
		    for(e2 = nGEQs-1; e2>=0;e2--) if (e != e2 && GEQs[e2].coef[i]) break;
		if (e2 < 0)
		    for(e2 = nSUBs-1; e2>=0;e2--) if (e != e2 && SUBs[e2].coef[i]) break;
		if (e2 >= 0) guaranteed = false;
		}
	    else guaranteed = false;
	    continue;
	    }
	if (EQs[e].coef[v1]*EQs[e].coef[v2] == -1) 
	    {
	    // found exact difference
	    coef_t d = - EQs[e].coef[v1] * EQs[e].coef[0];
	    set_max(lowerBound, d);
	    set_min(upperBound, d);
	    guaranteed =true;
	    return;
	    }
	else if (EQs[e].coef[v1] == 0) 
	    lb2 = ub2 = -EQs[e].coef[0]/ EQs[e].coef[v2];
	else if (EQs[e].coef[v2] == 0) 
	    lb1 = ub1 = -EQs[e].coef[0]/ EQs[e].coef[v1];
	else guaranteed = false;
	};


    bool isDead[maxmaxGEQs];

    for (e = nGEQs - 1; e >= 0; e--) isDead[e] = false;
    int tryAgain = 1;
    while (tryAgain) 
	{
	tryAgain = 0;
	for (i = nVars; i > 0;i--) if (i!= v1 && i != v2)
	    {
	    for (e = nGEQs - 1; e >= 0; e--) 
		if (!isDead[e] && GEQs[e].coef[i])
		    break;
	    if (e < 0)
		e2 = e;
	    else if (GEQs[e].coef[i] > 0) 
		{
		for (e2 = e - 1; e2 >= 0; e2--)
		    if (!isDead[e2] && GEQs[e2].coef[i] < 0)
			break;
		}
	    else 
		{
		for (e2 = e - 1; e2 >= 0; e2--)
		    if (!isDead[e2] && GEQs[e2].coef[i] > 0)
			break;
		};
	    if (e2 < 0) 
		{
		int e3;
		for (e3 = nSUBs - 1; e3 >= 0; e3--)
		    if (SUBs[e3].coef[i])
			break;
		if (e3 >= 0)
		    continue;
		for (e3 = nEQs - 1; e3 >= 0; e3--)
		    if (EQs[e3].coef[i])
			break;
		if (e3 >= 0)
		    continue;
		if (e >= 0) 
		    {
		    isDead[e] = true;
		    for (e--; e >= 0; e--)
			if (GEQs[e].coef[i]) isDead[e] = true;
		    };


		};
	    };
	};


	
    for (e = nGEQs - 1; e >= 0; e--)  if (!isDead[e])
	{
	if (GEQs[e].coef[v1] == 0 && GEQs[e].coef[v2] == 0)  continue;
	for(i=nV;i>0;i--)  if (GEQs[e].coef[i] && i!=v1 && i != v2) break;
	if (i != 0) 
	    {
	    guaranteed = false;
	    continue;
	    }
	if (GEQs[e].coef[v1]*GEQs[e].coef[v2] == -1) 
	    {
	    // found relative difference
	    if (GEQs[e].coef[v1] == 1) 
		{ 
		// v1 - v2 + c >= 0
	        set_max(lowerBound, - GEQs[e].coef[0]);
		}
	    else 
		{
		// v2 - v1 + c >= 0
		// c >= v1-v2
		set_min(upperBound, GEQs[e].coef[0]);
		}
	    }
	else if (GEQs[e].coef[v1] == 0 && GEQs[e].coef[v2] > 0) 
	    lb2 = -GEQs[e].coef[0]/ GEQs[e].coef[v2];
	else if (GEQs[e].coef[v1] == 0 && GEQs[e].coef[v2] < 0) 
	    ub2 = -GEQs[e].coef[0]/ GEQs[e].coef[v2];
	else if (GEQs[e].coef[v2] == 0 && GEQs[e].coef[v1] > 0) 
	    lb1 = -GEQs[e].coef[0]/ GEQs[e].coef[v1];
	else if (GEQs[e].coef[v2] == 0 && GEQs[e].coef[v1] < 0) 
	    ub1 = -GEQs[e].coef[0]/ GEQs[e].coef[v1];
	else guaranteed = false;
	};

    //   ub1-lb2 >= v1-v2 >= lb1-ub2
    
    if (negInfinity < lb2 && ub1 < posInfinity) set_min(upperBound, ub1-lb2);
    if (negInfinity < lb1 && ub2 < posInfinity) set_max(lowerBound, lb1-ub2);
    if (lowerBound >= upperBound) guaranteed = 1;
    }
	

    

int
Problem::
queryVariable(int i, coef_t *lowerBound, coef_t *upperBound)
    {
    int nV = nVars;
    int e, j;
    int isSimple;
    int coupled = FALSE;
    for(j=1;j<=safeVars;j++) if (var[j] > 0) 
                assert(forwardingAddress[var[j]] == j);

    assert(i > 0);
    i = forwardingAddress[i];
    assert(i != 0);

    (*lowerBound) = negInfinity;
    (*upperBound) = posInfinity;

    if (i < 0) 
	{
	int easy = TRUE;
	i = -i - 1;
	for (j = 1; j <= nV; j++)
	    if (SUBs[i].coef[j] != 0)
		easy = FALSE;
	if (easy) 
	    {
	    *upperBound = *lowerBound = SUBs[i].coef[0];
	    return (FALSE);
	    };
	return (TRUE);
	};

    for (e = nSUBs - 1; e >= 0; e--)
	if (SUBs[e].coef[i] != 0)
	    coupled = TRUE;

    for (e = nEQs - 1; e >= 0; e--)
	if (EQs[e].coef[i] != 0) 
	    {
	    isSimple = TRUE;
	    for (j = 1; j <= nV; j++)
		if (i != j && EQs[e].coef[j] != 0) 
		    {
		    isSimple = FALSE;
		    coupled = TRUE;
		    break;
		    };
	    if (!isSimple)
		continue;
	    else 
		{
		*lowerBound = *upperBound = -EQs[e].coef[i] * EQs[e].coef[0];
		return (FALSE);
		};
	    };
    for (e = nGEQs - 1; e >= 0; e--)
	if (GEQs[e].coef[i] != 0) 
	    {
	    if (GEQs[e].key == i) 
		{
		set_max(*lowerBound, -GEQs[e].coef[0]);
		}
	    else if (GEQs[e].key == -i) 
		{
		set_min(*upperBound, GEQs[e].coef[0]);
		}
	    else
		coupled = TRUE;
	    };
    return (coupled);
    }

int
Problem::
query_variable_bounds(int i, coef_t *l, coef_t *u)
    {
    int coupled;
    *l = negInfinity;
    *u = posInfinity;
    coupled = queryVariable(i, l, u);
    if (!coupled || (nVars == 1 && forwardingAddress[i] == 1))
	return 0;
    if (abs(forwardingAddress[i]) == 1 && nVars + nSUBs == 2 && nEQs + nSUBs == 1) 
	{
	int couldBeZero;
	queryCoupledVariable(i, l, u, &couldBeZero, negInfinity, posInfinity);
	return 0;
	};
    return 1;
    }

void
Problem::
queryCoupledVariable(int i, coef_t *l, coef_t *u, int *couldBeZero, coef_t lowerBound, coef_t upperBound)
    {
    int e;
    coef_t b1, b2;
    const eqn *eqn;
    coef_t sign;
    int v;

    if (abs(forwardingAddress[i]) != 1 || nVars + nSUBs != 2 || nEQs + nSUBs != 1) 
	{
	fprintf(outputFile, "queryCoupledVariablecalled with bad parameters\n");
	printProblem();
	Exit(2);
	};

    if (forwardingAddress[i] == -1) 
	{
	eqn = &SUBs[0];
	sign = 1;
	v = 1;
	}
    else 
	{
	eqn = &EQs[0];
	sign = -eqn->coef[1];
	v = 2;
	};

    /* Variable i is defined in terms of variable v */

    for (e = nGEQs - 1; e >= 0; e--)
	if (GEQs[e].coef[v] != 0) 
	    {
	    if (GEQs[e].coef[v] == 1) 
		{
		set_max(lowerBound, -GEQs[e].coef[0]);
		}
	    else 
		{
		set_min(upperBound, GEQs[e].coef[0]);
		};
	    };
    /* lowerBound and upperBound are bounds on the value of v */

    if (lowerBound > upperBound) 
	{
	*l = posInfinity;
	*u = negInfinity;
	*couldBeZero = 0;
	return;
	};
    if (lowerBound == negInfinity) 
	{
	if (eqn->coef[v] > 0)
	    b1 = sign * negInfinity;
	else
	    b1 = -sign * negInfinity;
	}
    else
	b1 = sign * (eqn->coef[0] + eqn->coef[v] * lowerBound);
    if (upperBound == posInfinity) 
	{
	if (eqn->coef[v] > 0)
	    b2 = sign * posInfinity;
	else
	    b2 = -sign * posInfinity;
	}
    else
	b2 = sign * (eqn->coef[0] + eqn->coef[v] * upperBound);

    /* b1 and b2 are bounds on the value of i (don't know which is upper bound) */
    if (b1 <= b2) 
	{
	set_max(*l, b1);
	set_min(*u, b2);
	}
    else 
	{
	set_max(*l, b2);
	set_min(*u, b1);
	};
    coef_t barb = abs(eqn->coef[v]);
    *couldBeZero = *l <= 0 && 0 <= *u && int_mod(eqn->coef[0], barb) == 0;
    }



int
Problem::
queryVariableSigns(int i, int dd_lt, int dd_eq, int dd_gt, coef_t lowerBound, coef_t upperBound, bool *distKnown, coef_t *dist)
    {
    int result;
    coef_t l, u;
    int couldBeZero;

    l = negInfinity;
    u = posInfinity;

    queryVariable(i, &l, &u);
    queryCoupledVariable(i, &l, &u, &couldBeZero, lowerBound, upperBound);
    result = 0;
    if (l < 0)
	result |= dd_gt;
    if (u > 0)
	result |= dd_lt;
    if (couldBeZero)
	result |= dd_eq;
    if (l == u) 
	{
	*distKnown = 1;
	*dist = l;
	}
    else 
	{
	*distKnown = 0;
	};
    return (result);
    }

} // end of namespace omega
