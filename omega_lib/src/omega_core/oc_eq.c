/* $Id: oc_eq.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <omega/omega_core/oc_i.h>

namespace omega {


void Problem::simplifyStrideConstraints() 
    {
    int e,e2,i;
    if (DBUG) fprintf(outputFile,"Checking for stride constraints\n");
    for(i=safeVars+1;i<=nVars;i++) 
	{
	if (DBUG) fprintf(outputFile,"checking %s\n",variable(i));
	for(e=0;e<nGEQs;e++) if (GEQs[e].coef[i]) break;
	if (e >= nGEQs) 
	    {
	    if (DBUG) fprintf(outputFile,"%s passed GEQ test\n",variable(i));
	    e2 = -1;
	    for(e=0;e<nEQs;e++) if (EQs[e].coef[i]) 
		{
		if (e2 == -1) e2 = e;
		else 
		    {
		    e2 = -1;
		    break;
		    };
		};
	    if (e2 >= 0) 
		{
		if (DBUG) 
		    {
		    fprintf(outputFile,"Found stride constraint: ");
		    printEQ(&EQs[e2]);
		    fprintf(outputFile,"\n");
		    };
		/* Is a stride constraint */
		coef_t g = abs(EQs[e2].coef[i]);
		assert(g>0);
		int j;
		for(j=0;j<=nVars;j++) if (i != j)
		    EQs[e2].coef[j] 
			= int_mod_hat( EQs[e2].coef[j], g);
		};
	    }
	};
    }
		
	  
  
void Problem::doMod(coef_t factor, int e, int j)
/* Solve e = factor alpha for x_j and substitute */
    {
    int k;
    eqn eq;
    coef_t nFactor;

    
    int alpha;

    // if (j > safeVars) alpha = j;
    // else
    if (EQs[e].color) {
	rememberRedConstraint(&EQs[e],redEQ,0);
	EQs[e].color = 0;
	}
    alpha = addNewUnprotectedWildcard();
    eqncpy(&eq, &EQs[e]);
    newVar = alpha;

    if (DEBUG) 
	{
	fprintf(outputFile, "doing moding: ");
	fprintf(outputFile,"Solve ");
	printTerm(&eq,1);
	fprintf(outputFile," = " coef_fmt " %s for %s and substitute\n",
		factor,variable(alpha),variable(j));
	};
    for (k = nVars; k >= 0; k--) 
	eq.coef[k] = int_mod_hat(eq.coef[k], factor);
    nFactor = eq.coef[j];
    assert(nFactor == 1 || nFactor == -1);
    eq.coef[alpha] = factor/nFactor;
    if (DEBUG) 
	{
	fprintf(outputFile, "adjusted: ");
	fprintf(outputFile,"Solve ");
	printTerm(&eq,1);
	fprintf(outputFile," = 0 for %s and substitute\n",
		variable(j));
	};

    eq.coef[j] = 0;
    substitute( &eq, j, nFactor);
    newVar = -1;
    deleteVariable( j);
    for (k = nVars; k >= 0; k--) 
	{
	assert(EQs[e].coef[k] % factor == 0);
	EQs[e].coef[k] = EQs[e].coef[k] / factor;
	};
    if (DEBUG) 
	{
	fprintf(outputFile, "Mod-ing and normalizing produces:\n");
	printProblem();
	};
    }

void Problem:: substitute(eqn *sub, int i, coef_t c)
    {
    int e, j;
    coef_t k;
    int recordSubstitution = 
	i <= safeVars && var[i] >= 0;

    redType clr;
    if (sub->color)  clr = redEQ;
    else clr = notRed;

    assert(c == 1 || c == -1);

    if (DBUG || doTrace) 
	{
	if (sub->color) fprintf(outputFile, "RED SUBSTITUTION\n");
	fprintf(outputFile, "substituting using %s := ", variable(i));
	printTerm(sub, -c);
	fprintf(outputFile, "\n");
	printVars();
	};
#ifndef NDEBUG
    if (i > safeVars && clr)
	{
	bool unsafeSub = false;
	for (e = nEQs - 1; e >= 0; e--) 
	    if (!(EQs[e].color || !EQs[e].coef[i]))
			unsafeSub = true;
	for (e = nGEQs - 1; e >= 0; e--) 
	    if (!(GEQs[e].color || !GEQs[e].coef[i]))
			unsafeSub = true;
	for (e = nSUBs - 1; e >= 0; e--) 
	    if (SUBs[e].coef[i])
			unsafeSub = true;
	if (unsafeSub) {
		fprintf(outputFile, "UNSAFE RED SUBSTITUTION\n");
		fprintf(outputFile, "substituting using %s := ", variable(i));
		printTerm(sub, -c);
		fprintf(outputFile, "\n");
		printProblem();
		assert(0 && "UNSAFE RED SUBSTITUTION");
		}
	};
#endif

    for (e = nEQs - 1; e >= 0; e--) 

	{
	eqn *eq;
	eq = &(EQs[e]);
	k = eq->coef[i];
	if (k != 0) 
	    {
	    k = check_mul(k,c); // Should be k = k/c, but same effect since abs(c) == 1
	    eq->coef[i] = 0;
	    for (j = nVars; j >= 0; j--) 
		{
		eq->coef[j] -= check_mul(sub->coef[j],k);
		};
	    };
	if (DEBUG) 
	    {
	    printEQ(eq);
	    fprintf(outputFile, "\n");
	    };
	};
    for (e = nGEQs - 1; e >= 0; e--) 
	{
	int zero;
	eqn *eq;
	eq = &(GEQs[e]);
	k = eq->coef[i];
	if (k != 0) 
	    {
	    k = check_mul(k,c); // Should be k = k/c, but same effect since abs(c) == 1
	    eq->touched = TRUE;
	    eq->coef[i] = 0;
	    zero = 1;
	    for (j = nVars; j >= 0; j--) 
		{
		eq->coef[j] -= check_mul(sub->coef[j],k);
		if (j > 0 && eq->coef[j]) zero = 0;
		};
	    if (zero && clr != notRed && !eq->color) 
		{
                coef_t barb = abs(k);
		coef_t z = int_div(eq->coef[0],barb);
		if (DBUG || doTrace) 
		    {
		    fprintf(outputFile,"Black inequality matches red substitution\n");
		    if (z < 0) fprintf(outputFile,"System is infeasible\n");
		    else if (z > 0) fprintf(outputFile,"Black inequality is redundant\n");
		    else 
			{
			fprintf(outputFile,"Black constraint partially implies red equality\n");
			if (k < 0)   
			    {
			    fprintf(outputFile, "Black constraints tell us ");
			    assert(sub->coef[i] == 0);
			    sub->coef[i] = c;
			    printTerm(sub, 1);
			    sub->coef[i] = 0;
			    fprintf(outputFile, "<= 0\n");
			    }
			else 
			    {
			    fprintf(outputFile, "Black constraints tell us ");
			    assert(sub->coef[i] == 0);
			    sub->coef[i] = c;
			    printTerm(sub, 1);
			    sub->coef[i] = 0;
			    fprintf(outputFile, " >= 0\n");
			    }
			}
		    }
		if (z == 0) 
		    {
		    if (k < 0) 
			{
			if (clr == redEQ) clr = redGEQ;
			else if (clr == redLEQ) clr = notRed;
			}
		    else 
			{
			if (clr == redEQ) clr = redLEQ;
			else if (clr == redGEQ) clr = notRed;
			};
		    }

		}
	    }
	if (DEBUG) 
	    {
	    printGEQ(eq);
	    fprintf(outputFile, "\n");
	    };
	};
    if (i <= safeVars && clr) 
	{
	assert(sub->coef[i] == 0);
	sub->coef[i] = c;
	rememberRedConstraint(sub, clr, 0);
	sub->coef[i] = 0;
	};

    if (recordSubstitution) 
	{
	int s = nSUBs++;
	int kk;
	eqn *eq = &(SUBs[s]);
	for (kk = nVars; kk >= 0; kk--)
	    eq->coef[kk] = check_mul(-c,(sub->coef[kk]));
	eq->key = var[i];
	if (DEBUG) 
	    {
	    fprintf(outputFile, "Recording substition as: ");
	    printSubstitution(s);
	    fprintf(outputFile, "\n");
	    }
	};
    if (DEBUG) 
	{
	fprintf(outputFile, "Ready to update subs\n");
	if (sub->color) fprintf(outputFile, "RED SUBSTITUTION\n");
	fprintf(outputFile, "substituting using %s := ", variable(i));
	printTerm(sub, -c);
	fprintf(outputFile, "\n");
	printVars();
	};

    for (e = nSUBs - 1; e >= 0; e--) 
	{
	eqn *eq = &(SUBs[e]);
	k = eq->coef[i];
	if (k!= 0) 
	    {
	    k = check_mul(k,c); // Should be k = k/c, but same effect since abs(c) == 1
	    eq->coef[i] = 0;
	    for (j = nVars; j >= 0; j--) 
		{
		eq->coef[j] -= check_mul(sub->coef[j],k);
		};
	    };
	if (DEBUG) 
	    {
	    fprintf(outputFile, "updated sub (" coef_fmt "): ",c);
	    printSubstitution(e);
	    fprintf(outputFile, "\n");
	    };
	};

    if (DEBUG) 
	{
	fprintf(outputFile, "---\n\n");
	printProblem();
	fprintf(outputFile, "===\n\n");
	};
    };


void Problem::doElimination(int e, int i)
    {
    eqn sub;
    coef_t c;
    if (DBUG || doTrace)
	fprintf(outputFile, "eliminating variable %s\n", variable(i));
    eqncpy(&sub, &EQs[e]);
    c = sub.coef[i];
    sub.coef[i] = 0;
    if (c == 1 || c == -1) 
	{
	substitute(&sub, i, c);
	} 
    else 
	{
	coef_t a = abs(c);
	if (TRACE)
	    fprintf(outputFile, "performing non-exact elimination, c = " coef_fmt "\n", c);
	if (DBUG) 
	    printProblem();
	assert(inApproximateMode);
	for (e = nEQs - 1; e >= 0; e--)
	    if (EQs[e].coef[i]) 
		{
		eqn *eq = &(EQs[e]);
		int j;
		coef_t k;
		for (j = nVars; j >= 0; j--)
		    eq->coef[j] = check_mul(eq->coef[j],a);
		k = eq->coef[i];
		eq->coef[i] = 0;
		eq->color |= sub.color;
		for (j = nVars; j >= 0; j--)
		    eq->coef[j] -= check_mul(sub.coef[j], k / c);
		};
	for (e = nGEQs - 1; e >= 0; e--)
	    if (GEQs[e].coef[i]) 
		{
		eqn *eq = &(GEQs[e]);
		int j;
		coef_t k;
		for (j = nVars; j >= 0; j--)
		    eq->coef[j] = check_mul(eq->coef[j],a);
		k = eq->coef[i];
		eq->coef[i] = 0;
		eq->color |= sub.color;
		for (j = nVars; j >= 0; j--)
		    eq->coef[j] -= check_mul(sub.coef[j],k / c);
		eq->touched = 1;
		};
	for (e = nSUBs - 1; e >= 0; e--)
	    if (SUBs[e].coef[i]) 
		{
		eqn *eq = &(SUBs[e]);
		int j;
		coef_t k;
		assert(0); 
		// We can't handle this since we can't multiply 
		// the coefficient of the left-hand side
		assert(!sub.color);
		for (j = nVars; j >= 0; j--)
		    eq->coef[j] = check_mul(eq->coef[j],a);
		k = eq->coef[i];
		eq->coef[i] = 0;
		for (j = nVars; j >= 0; j--)
		    eq->coef[j] -= check_mul(sub.coef[j], k / c);
		};
	};
    deleteVariable(i);
    }

int
Problem::solveEQ()
    {
    int i, j, e,k;
    coef_t g, g2;
    g = 0;


    if (DBUG && nEQs > 0) 
	{
	fprintf(outputFile, "\nSolveEQ(%d)\n", mayBeRed);
	printProblem();
	fprintf(outputFile, "\n");
	};

    check();

    int delay[maxmaxEQs];

    for(e=0;e<nEQs;e++) 
	{
	delay[e] = 0;
	if (EQs[e].color) delay[e]+= 8;
        int nonunitWildCards = 0;
        int unitWildCards = 0;
	for (i = nVars; i > safeVars; i--) if (EQs[e].coef[i]) 
	    {
	    if (EQs[e].coef[i] == 1 || EQs[e].coef[i] == -1)
		unitWildCards++;
	    else nonunitWildCards++;
	    };
        int unit = 0;
        int nonUnit = 0;
	for (; i > 0; i--) if (EQs[e].coef[i]) 
	    {
	    if (EQs[e].coef[i] == 1 || EQs[e].coef[i] == -1)
		unit++;
	    else nonUnit++;
	    };
	if (unitWildCards == 1 && nonunitWildCards == 0)
	    delay[e] += 0;
	else if (unitWildCards >= 1 && nonunitWildCards == 0)
	    delay[e] += 1;
	else if (inApproximateMode && nonunitWildCards > 0)
	    delay[e] += 2;
	else if (unit == 1 && nonUnit  == 0 &&  nonunitWildCards == 0)
	    delay[e] += 3;
	else if (unit > 1 && nonUnit  == 0 &&  nonunitWildCards == 0)
	    delay[e] += 4;
	else if (unit >= 1 && nonunitWildCards <= 1)
	    delay[e] += 5;
	else delay[e] += 6;
	};
	
    for(e=0;e<nEQs;e++) 
	{
	int e2,slowest;
	slowest = e;
	for(e2 = e+1;e2<nEQs;e2++) if (delay[e2]>delay[slowest]) slowest = e2;
	if (slowest != e) 
	    {
	    int tmp = delay[slowest];
	    delay[slowest] = delay[e];
	    delay[e] = tmp;
	    eqn eq;
	    eqncpy(&eq, &EQs[slowest]);
	    eqncpy(&EQs[slowest], &EQs[e]);
	    eqncpy(&EQs[e], &eq);
	    };
	};

    if (DBUG && nEQs > 0) 
	{
	fprintf(outputFile, "\nSolveEQ(%d)\n", mayBeRed);
	printProblem();
	fprintf(outputFile, "\n");
	};
	
    /* Eliminate all EQ equations */
    for (e = nEQs - 1; e >= 0; e--) 
	{
	eqn *eq = &(EQs[e]);
	int sv;

	assert(mayBeRed || !eq->color);

	check();

	if (DEBUG) fprintf(outputFile, "considering: ");
	if (DEBUG) printEQ(eq);
	if (DEBUG) fprintf(outputFile, "\n");

	g = 0;
	for (i = nVars; i >= 1; i--) {
          coef_t barb = abs(eq->coef[i]);
          g = gcd(barb,g);
        }

	if (g == 0) 
	    {
	    if (eq->coef[0] != 0) 
		{
		if (DBUG) printEQ(eq);
		if (DBUG) fprintf(outputFile, "\nequations have no solution (B)\n");
		return (FALSE);
		};
	    nEQs--;
	    continue;
	    };

          if (inApproximateMode) {
            coef_t barb = abs(eq->coef[0]);
            g = gcd(barb,g);
          }

        if (eq->coef[0] % g != 0) 
	    {
	    if (DBUG) printEQ(eq);
	    if (DBUG) fprintf(outputFile, "\nequations have no solution (A)\n");
	    return (FALSE);
	    };

	if (g != 1) for (i = nVars; i >= 0; i--) eq->coef[i] /= g;

	if (!eq->color) 
	    {
	    bool involvesWildCards = false;
	    for (i = nVars; i > safeVars; i--) {
		if (eq->coef[i]!= 0) involvesWildCards = true;
		if (abs(eq->coef[i]) == 1) break;
		}
	    if (!involvesWildCards || i > safeVars) {
	      if (!involvesWildCards) 
		for (; i > 0; i--) 
		  if (abs(eq->coef[i]) == 1) break;
	      if (i > 0) 
		{
		nEQs--;
		doElimination(e, i);
		continue;
		};
	    };
	   };

	for (i = nVars; i > 0; i--) if ((g = eq->coef[i]) != 0) break;
	for (j = i - 1; j > 0; j--) if (eq->coef[j]) break;
	if (j < 0) {
	    fprintf(outputFile, "\nAssertion failure; i = %d, j = %d, g = " coef_fmt "\n",
				i,j,g);
	    printEQ(eq);
	    assert(j >= 0);
	    }	
	assert(j >= 0);
	/* i == position of last non-zero coef */
	/* g == coef of i */
	/* j == position of next non-zero coef */

	if (g < 0)
	    g = -g;
	if (g == 1) 
	    {
	    nEQs--;
	    // exact elimination, remember if i protected
	    doElimination(e, i);
	    } 
	else 
	    {
	    k = j;
	    if (k > safeVars) 
		{
                  for (; g != 1 && k > safeVars; k--) {
                    coef_t barb = abs(eq->coef[k]);
		    g = gcd(barb, g);
                  }
		g2 = g;
		}
	    else if (i > safeVars)
		g2 = g;
	    else
		g2 = 0;

	    // Now, gcd of all coefficients is 1
	    // Now, gcd of unprotected coefficients is g2
	    if (g2 > 1) 
		{
		int e2;
		for (e2 = e - 1; e2 >= 0; e2--) if (EQs[e2].coef[i]) break;
		if (e2 == -1)
		    for (e2 = nGEQs - 1; e2 >= 0; e2--) if (GEQs[e2].coef[i]) break;
		if (e2 == -1)
		    for (e2 = nSUBs - 1; e2 >= 0; e2--) if (SUBs[e2].coef[i]) break;
		if (e2 == -1) 
		    {
		    bool change = 0;
		    if (DBUG) 
			fprintf(outputFile, "Is a stride constraint, with %s as wildcard\n",variable(i));
		    if (DBUG) printEQ(eq);
		    if (DBUG) fprintf(outputFile, " \n");

		    g = eq->coef[i];
		    g = abs(g);
		    for (e2 = e - 1; e2 >= 0; e2--) {
			for (j = nVars; j >= 0; j--) 
			   if (i != j 
				&& EQs[e].coef[j]-EQs[e2].coef[j] % g == 0) break;
			if (j < 0) break;
			}
		    if (0 && e2 >= 0) {
			// Implied by e2
			if (DBUG) {
			  fprintf(outputFile, "Implied by ");
			  printEQ(&EQs[e2]);
			  fprintf(outputFile, " \n");
			  }
		        nEQs--;
			continue;
			};


		    for (k = i - 1; k >= 0; k--) if (abs(eq->coef[k]) != 1) 
			{
			coef_t t = int_mod_hat(eq->coef[k], g);
			if (t != eq->coef[k]) 
			    {
			    eq->coef[k] = t;
			    change = 1;
			    };
			};
		    if (!change) 
			{
			if (DBUG) fprintf(outputFile, "So what?\n");
			}
		    else 
			{
			nameWildcard(i);
			if (DEBUG) printEQ(eq);
			if (DEBUG) fprintf(outputFile, " \n");
			e++;  		/* go back and try this equation again */
			continue;
			};
		    };
		}

	    if (g2 > 1 && !(inApproximateMode && !inStridesAllowedMode)) 
		{
		if (DEBUG) 
		    {
		    fprintf(outputFile, "generating stride constraint due to gcd of " coef_fmt " of unprotected coefficients\n",g2);
		    printEQ(eq);
		    fprintf(outputFile, "\n----\n");
		    printProblem();
		    fprintf(outputFile, "\n----\n");
		    fprintf(outputFile, "\n----\n");
		    };
		// Found stride constraint, remember it if red

		for(k = j; k >0 ; k--) if (abs(EQs[e].coef[k])==1) break;
	
		if (DEBUG)	   
		    fprintf(outputFile, "quick stride: %d,%d,%d," coef_fmt "\n",
			    k,j,safeVars,EQs[e].coef[k]);
		if (j <= safeVars && 1 <= k) 
		    {
		    if (EQs[e].color)  
			{
			coef_t tmp = EQs[e].coef[i];
			EQs[e].coef[i] = 0;
			rememberRedConstraint(&EQs[e],  redStride, g2);
			EQs[e].coef[i] = tmp;
			EQs[e].color = 0;
			};
		    protectWildcard(i);
		    nEQs--;
		    doElimination(e, k);
		    if (DEBUG) printProblem();
		    continue;
		    }
		else 
		    {
		    i = addNewProtectedWildcard();
		    int neweq = newEQ();
		    assert(neweq == e+1); // we were working on highest-numbered EQ
		    eqnzero(&EQs[neweq]);
		    eqnncpy(&EQs[neweq], eq, safeVars);

		    for (j = nVars; j >= 0; j--) 
			{
			EQs[neweq].coef[j] = int_mod_hat(EQs[neweq].coef[j], g2);
			};
		    if (EQs[e].color) 
			rememberRedConstraint(&EQs[neweq],  redStride, g2);
		    EQs[neweq].coef[i] = g2;
		    EQs[neweq].color = 0;
		    e += 2;  		/* go back and start on new equation */
		    if (DEBUG) printProblem();
		    continue;
		    };
		};

	    sv = safeVars;
	    if (g2 == 0) // there are no non-zero coefficients for unprotected variables
		sv = 0;

	    /* find variable to eliminate */
	    if (inApproximateMode && nVars > sv) 
		{
		if (TRACE) 
		    {
		    fprintf(outputFile, "looking for non-exact elimination: ");
		    printEQ(eq);
		    fprintf(outputFile, "\n");
		    printProblem();
		    };
		for (i = nVars; i > sv; i--)
		    if (EQs[e].coef[i] != 0) {
			g2 = EQs[e].coef[i];
			break;
			}	
		}
	    else
		for (i = nVars; i > sv; i--)
		    if (EQs[e].coef[i] == 1 || EQs[e].coef[i] == -1) break;

	    // we picked i
	    if (i > sv && (!inApproximateMode || i > safeVars))
		{
		if (DBUG) 
		    {
		    fprintf(outputFile, "About to do elimination; i = %d, sv = %d, g2 = " coef_fmt "\n",
			i,sv,g2);
		    }
		nEQs--;
	        // exact elimination, remember if i protected
		doElimination(e, i);
		if (TRACE) 
		    {
		    assert(inApproximateMode);
		    fprintf(outputFile, "result of non-exact elimination:\n");
		    printProblem();
		    fflush(outputFile);
		    };
		}
	    else if (inApproximateMode) {
		    if (TRACE) {
			fprintf(outputFile, "Dropping equation on the floor");
		        printEQ(eq);
			fprintf(outputFile, "\n");
			}
		    }
	    else
		{
		coef_t factor = (posInfinity);  // was MAXINT
		j = 0;

		for (i = nVars; i > sv; i--)
		    if (EQs[e].coef[i] != 0 && factor > abs(EQs[e].coef[i]) + 1) 
			{
			factor = abs(EQs[e].coef[i]) + 1;
			j = i;
			};
		assert(j > sv);
		doMod(factor, e, j);
		/* go back and try this equation again */
		e++;
		};
	    };


	};
    nEQs = 0;
    return (UNKNOWN);
    }


} // end of namespace omega

