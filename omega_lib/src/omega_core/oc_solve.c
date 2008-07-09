#include <omega/omega_core/oc_i.h>

namespace omega {

static int solveDepth = 0;
#define maxDead maxmaxGEQs

int
Problem::
solve(int desiredResult)
    {
    assert(omegaInitialized);
    int result;

    checkVars(nVars+1);
    assert(nVars >= safeVars);
    if (desiredResult != SIMPLIFY)
	safeVars = 0;

    solveDepth++;
    if (solveDepth > 50) 
	{
	fprintf(outputFile, "Solve depth = %d, inApprox = %d, aborting\n",
		solveDepth, inApproximateMode);
	printProblem();
	fflush(outputFile);
#ifndef SPEED
	omega_core_debug = 3;
#else
	Exit(2);
#endif
	if (solveDepth > 60)
	    Exit(2);
	};

    check();
    do 
	{
	doItAgain = 0;
	check();
	if (solveEQ() == FALSE) 
	    {
	    solveDepth--;
	    return (FALSE);
	    };
	check();
	if (!nGEQs) 
	    {
	    result = TRUE;
	    nVars = safeVars;
	    break;
	    }
	else
	    result = solveGEQ(desiredResult);
	check();
	}
    while (doItAgain && desiredResult == SIMPLIFY);
    solveDepth--;

    return (result);
    }


// Supporting functions of solveGEQ
int Problem::smoothWeirdEquations()
    {
    int e1, e2, e3, p, q, k;
    coef_t alpha, alpha1, alpha2, alpha3;
    coef_t c;
    int v;
    int result = 0;

    for (e1 = nGEQs - 1; e1 >= 0; e1--)
	if (!GEQs[e1].color) 
	    {
	    coef_t g = 999999;
	    for (v = nVars; v >= 1; v--)
		if (GEQs[e1].coef[v] != 0 && abs(GEQs[e1].coef[v]) < g)
		    g = abs(GEQs[e1].coef[v]);
	    if (g > 20) 
		{

		e3 = newGEQ();  /* Create a scratch GEQ,not part of the prob.*/
		nGEQs--;
		for (v = nVars; v >= 1; v--)
		    GEQs[e3].coef[v] = int_div(6 * GEQs[e1].coef[v] + g / 2, g);
		GEQs[e3].color = 0;
		GEQs[e3].touched = 1;
		GEQs[e3].coef[0] = 9997;
		if (DBUG) 
		    {
		    fprintf(outputFile, "Checking to see if we can derive: ");
		    printGEQ(&GEQs[e3]);
		    fprintf(outputFile, "\n from: ");
		    printGEQ(&GEQs[e1]);
		    fprintf(outputFile, "\n");
		    };


		for (e2 = nGEQs - 1; e2 >= 0; e2--)
		    if (e1 != e2 && !GEQs[e2].color) 
			{
			for (p = nVars; p > 1; p--) 
			    {
			    for (q = p - 1; q > 0; q--) 
				{
				alpha = (GEQs[e1].coef[p] * GEQs[e2].coef[q] - GEQs[e2].coef[p] * GEQs[e1].coef[q]);
				if (alpha != 0)
				    goto foundPQ;
				};
			    };
			continue;

			  foundPQ:

			alpha1 = GEQs[e2].coef[q] * GEQs[e3].coef[p] - GEQs[e2].coef[p] * GEQs[e3].coef[q];
			alpha2 = -(GEQs[e1].coef[q] * GEQs[e3].coef[p] - GEQs[e1].coef[p] * GEQs[e3].coef[q]);
			alpha3 = alpha;

			if (alpha1 * alpha2 <= 0)
			    continue;
			if (alpha1 < 0) 
			    {
			    alpha1 = -alpha1;
			    alpha2 = -alpha2;
			    alpha3 = -alpha3;
			    }
			if (alpha3 > 0) 
			    {
			    /* Trying to prove e3 is redundant */

			    /* verify alpha1*v1+alpha2*v2 = alpha3*v3 */
			    for (k = nVars; k >= 1; k--)
				if (alpha3 * GEQs[e3].coef[k]
				    != alpha1 * GEQs[e1].coef[k] + alpha2 * GEQs[e2].coef[k])
				    goto nextE2;

			    c = alpha1 * GEQs[e1].coef[0] + alpha2 * GEQs[e2].coef[0];
			    if (c < alpha3 * (GEQs[e3].coef[0] + 1))
				GEQs[e3].coef[0] = int_div(c, alpha3);

			    }
			  nextE2:;
			}
		if (GEQs[e3].coef[0] < 9997) 
		    {
		    result++;
#if !defined NDEBUG
		    int e4 = 
#endif
			newGEQ();
#if !defined NDEBUG
		    assert(e3 == e4);
#endif
		    if (DBUG) 
			{
			fprintf(outputFile, "Smoothing wierd equations; adding:\n");
			printGEQ(&GEQs[e3]);
			fprintf(outputFile, "\nto:\n");
			printProblem();
			fprintf(outputFile, "\n\n");
			};
		    };
		}
	    }
    return (result);
    }



void 
Problem::
analyzeElimination(
int &v,
int &darkConstraints,
int &darkShadowFeasible,
int &unit,
coef_t &parallelSplinters,
coef_t &disjointSplinters,
coef_t &lbSplinters,
coef_t &ubSplinters,
int &parallelLB)
    {

    parallelSplinters = (posInfinity);  // was MAXINT
    disjointSplinters = 0;
    lbSplinters = 0;
    ubSplinters = 0;

    darkConstraints = 0;
    darkShadowFeasible = 1;
    coef_t maxUBc = 0;
    coef_t maxLBc = 0;
    int e,e2;
    unit = 0;
    int exact = 1;

    for (e = nGEQs - 1; e >= 0; e--) 
	{
	coef_t c = GEQs[e].coef[v];

	if (c < 0) 
	    {
	    coef_t Lc, Uc, g, diff, grey;

	    set_max(maxUBc, -c);
	    Uc = -c;
	    for (e2 = nGEQs - 1; e2 >= 0; e2--)
		if (GEQs[e2].coef[v] > 0) 
		    {
		    Lc = GEQs[e2].coef[v];
		    g = 0;
		    grey = (Lc - 1) * (Uc - 1);

		    for (int j = nVars; j >= 1; j--) 
			{
			coef_t diff = Lc * GEQs[e].coef[j] + Uc * GEQs[e2].coef[j];
			if (diff < 0) diff = -diff;
			g = gcd(g, diff);
			if (g == 1)
			    break;
			}
		    diff = Lc * GEQs[e].coef[0] + Uc * GEQs[e2].coef[0];
		    if (g == 0) 
			{
			if (diff < 0)
			    {
				/* Real shadow must be true */
				/* otherwise we would have found it during */
				/* check for opposing constraints */
			    fprintf(outputFile, "Found conflicting constraints ");
			    printGEQ(&GEQs[e]);
			    fprintf(outputFile," and ");
			    printGEQ(&GEQs[e2]);
			    fprintf(outputFile,"\nin\n");
			    printProblem();
			    assert(diff >= 0);	
			    };
			if (diff < grey) 
			    {
			    darkShadowFeasible = 0;
			    if (parallelSplinters > diff+1) 
				{
				parallelSplinters = diff + 1;
				parallelLB = e2;
				}
			    }
			else 
			    {/* dark shadow is true, don't need to worry about this constraint pair */
			    }
			}
		    else 
			{
			coef_t splinters= int_div(diff, g) - int_div(diff - grey, g);
			if (splinters) exact = 0;
			disjointSplinters += splinters;
			if (g > 1) unit++;
			darkConstraints++;
			}
		    }
	    }
	else if (c > 0) 
	    {
	    set_max(maxLBc, c);
	    } /* else
		 darkConstraints++; */
	}

    if (darkShadowFeasible) 
	{
	disjointSplinters++;
	ubSplinters++;
	lbSplinters++;
        }
    else disjointSplinters = (posInfinity);  // was MAXINT


    if (!darkShadowFeasible || !exact) 
	for (e = nGEQs - 1; e >= 0; e--) 
	    {
	    coef_t c = GEQs[e].coef[v];
	    if (c < -1) 
		{
		c = -c;
		ubSplinters += 1+(c * maxLBc - c - maxLBc) / maxLBc;
		}
	    else if (c > 1) 
		{
		lbSplinters += 1+ (c * maxUBc - c - maxUBc) / maxUBc;
		}
	    }

    if (DEBUG) 
	{
	fprintf(outputFile,"analyzing elimination of %s(%d)\n",variable(v),v);
        if (darkShadowFeasible)
	    fprintf(outputFile,"  # dark constraints = %d\n", darkConstraints);
	else 
	    fprintf(outputFile,"  dark shadow obviously unfeasible\n");

	fprintf(outputFile," " coef_fmt " LB splinters\n", lbSplinters);
	fprintf(outputFile," " coef_fmt " UB splinters\n", ubSplinters);
	if (disjointSplinters != (posInfinity))
	    fprintf(outputFile," " coef_fmt " disjoint splinters\n", disjointSplinters);
	if (parallelSplinters != (posInfinity))
	    fprintf(outputFile," " coef_fmt " parallel splinters\n", parallelSplinters);
	fprintf(outputFile, "\n");
	fprintf(outputFile," %3d unit score \n", unit);
	}
    }



	
void Problem::partialElimination() {
	    if (DBUG) {
		fprintf(outputFile, "Performing Partial elimination\n");
	        printProblem();
		}
	    int fv;
	    if (0) 
		fv = 0;
	    else
		fv = safeVars;
	    bool somethingHappened = false;
            for (int i = nVars; i > fv; i--)
		{
	        bool isDead[maxmaxGEQs];
	        int e;
	        for (e = nGEQs-1; e >= 0; e--) isDead[e] = FALSE;
		int deadEqns[maxDead];
		int numDead = 0;
		for (int e1 = nGEQs-1; e1 >= 0; e1--)
		    if (abs(GEQs[e1].coef[i]) == 1) 
			{
			bool isGood = true;
			for (int e2 = nGEQs-1; e2 >= 0; e2--)
			    if (GEQs[e2].coef[i]  * GEQs[e1].coef[i] < 0) 
				if (GEQs[e1].key != -GEQs[e2].key) 
				    {
				    coef_t Uc = abs(GEQs[e2].coef[i]);
				    for (int k = nVars; k > fv; k--)
				    if (GEQs[e2].coef[k] 
					+ check_mul(GEQs[e1].coef[k], Uc) != 0)
					isGood = false;
				    }
			if (isGood) {
	    	        somethingHappened = true;
			for (int e2 = nGEQs-1; e2 >= 0; e2--)
			    if (GEQs[e2].coef[i]  * GEQs[e1].coef[i] < 0) 
				{
				if (GEQs[e1].key != -GEQs[e2].key) 
				    {
				    coef_t Uc = abs(GEQs[e2].coef[i]);
				    int new_eqn;
				    if (numDead == 0) {
					new_eqn = newGEQ();
				    } else {
					new_eqn = deadEqns[--numDead];
				    }
				    isDead[new_eqn] = FALSE;
				    if (DBUG) {
					fprintf(outputFile,"Eliminating constraint on %s\n",
		variable(i));
					fprintf(outputFile, "e1 = %d, e2 = %d, gen = %d\n", e1, e2, new_eqn);
					printGEQ(&(GEQs[e1]));
					fprintf(outputFile, "\n");
					printGEQ(&(GEQs[e2]));
					fprintf(outputFile, "\n");
					};

				    for (int k = nVars; k >= 0; k--)
					GEQs[new_eqn].coef[k] =
					    GEQs[e2].coef[k] 
						+ check_mul(GEQs[e1].coef[k], Uc);
				    GEQs[new_eqn].touched = TRUE;
				    GEQs[new_eqn].color = GEQs[e2].color | GEQs[e1].color;
				    if (DBUG) 
					{
					fprintf(outputFile, "give ");
					printGEQ(&(GEQs[new_eqn]));
					fprintf(outputFile, "\n");
					}
				    assert(GEQs[new_eqn].coef[i] == 0);
				    };
				};
			deadEqns[numDead++] = e1;
			isDead[e1] = TRUE;
			if (DEBUG)
			    fprintf(outputFile, "Killed %d\n", e1);
			}
			};
		for (e = nGEQs - 1; e >= 0; e--) if (isDead[e]) {
			deleteGEQ(e);
			}
		};
        if (somethingHappened && DBUG) {
		fprintf(outputFile, "Result of Partial elimination\n");
	        printProblem();
		}
	}
int 
Problem:: solveGEQ(int desiredResult)
    {
    int i, j, k, e;
    int fv;
    int result;
    int coupledSubscripts;
    int eliminateAgain;
    int smoothed = 0;
    int triedEliminatingRedundant = 0;
    j = 0;

    if (desiredResult != SIMPLIFY) 
	{
	nSUBs = 0;
	nMemories = 0;
	safeVars = 0;
	varsOfInterest = 0;
	}

    
      solveGEQstart:
    while (1) 
	{
	assert(desiredResult == SIMPLIFY || nSUBs == 0);
	check_number_GEQs(nGEQs);
	if (DEBUG)
	    fprintf(outputFile, "\nSolveGEQ(%d,%d):\n",
		    desiredResult, pleaseNoEqualitiesInSimplifiedProblems);
	if (DEBUG)
	    printProblem();
	if (DEBUG)
	    fprintf(outputFile, "\n");

#ifndef NDEBUG
	for(e=0;e<nSUBs;e++)
	    for(i=safeVars+1;i<=nVars;i++)
		assert(!SUBs[e].coef[i]);
#endif

        check();

	if (nVars == 1) 
	    {
	    int uColor = black;
	    int lColor = black;
	    coef_t upperBound = posInfinity;
	    coef_t lowerBound = negInfinity;
	    for (e = nGEQs - 1; e >= 0; e--) 
		{
		coef_t a = GEQs[e].coef[1];
		coef_t c = GEQs[e].coef[0];
		/* our equation is ax + c >= 0, or ax >= -c, or c >= -ax */
		if (a == 0) 
		    {
		    if (c < 0) 
			{
			if (TRACE)
			    fprintf(outputFile, "equations have no solution (G)\n");
			return (FALSE);
			};
		    }
		else if (a > 0) 
		    {
		    if (a != 1)
			c = int_div(c, a);
		    if (lowerBound < -c ||
			(lowerBound == -c && !isRed(&GEQs[e]))) 
			{
			lowerBound = -c;
			lColor = GEQs[e].color;
			}
		    }
		else 
		    {
		    if (a != -1)
			c = int_div(c, -a);
		    if (upperBound > c ||
			(upperBound == c && !isRed(&GEQs[e]))) 
			{
			upperBound = c;
			uColor = GEQs[e].color;
			}
		    };
		};
	    if (DEBUG)
		fprintf(outputFile, "upper bound = " coef_fmt "\n", upperBound);
	    if (DEBUG)
		fprintf(outputFile, "lower bound = " coef_fmt "\n", lowerBound);
	    if (lowerBound > upperBound) 
		{
		if (TRACE)
		    fprintf(outputFile, "equations have no solution (H)\n");
		return (FALSE);
		};
	    if (desiredResult == SIMPLIFY) 
		{
		nGEQs = 0;
		if (safeVars == 1) 
		    {

		    if (lowerBound == upperBound && !uColor && !lColor) 
			{
			int e = newEQ();
			assert(e == 0);
			EQs[e].coef[0] = -lowerBound;
			EQs[e].coef[1] = 1;
			EQs[e].color = lColor | uColor;
			return (solve(desiredResult));
			}
		    else 
			{
			if (lowerBound > negInfinity) 
			    {
			    int e = newGEQ();
			    assert(e == 0);
			    GEQs[e].coef[0] = -lowerBound;
			    GEQs[e].coef[1] = 1;
			    GEQs[e].key = 1;
			    GEQs[e].color = lColor;
			    GEQs[e].touched = 0;
			    };
			if (upperBound < posInfinity) 
			    {
			    int e = newGEQ();
			    GEQs[e].coef[0] = upperBound;
			    GEQs[e].coef[1] = -1;
			    GEQs[e].key = -1;
			    GEQs[e].color = uColor;
			    GEQs[e].touched = 0;
			    };
			};
		    }
		else
		    nVars = 0;
		return (TRUE);
		};
	    if (originalProblem != noProblem && !lColor && !uColor && !conservative && lowerBound == upperBound) 
		{
		int e = newEQ();
		assert(e == 0);
		EQs[e].coef[0] = -lowerBound;
		EQs[e].coef[1] = 1;
		EQs[e].color = 0;
		addingEqualityConstraint(0);
		};
	    return (TRUE);
	    };

	if (!variablesFreed) 
	    {
	    variablesFreed = 1;
	    if (desiredResult != SIMPLIFY)
		freeEliminations(0);
	    else
		freeEliminations(safeVars);
	    if (nVars == 1)
		continue;
	    };


	switch (normalize()) 
	    {
	case normalize_false:
	    return (FALSE);
	    break;
	case normalize_coupled:
	    coupledSubscripts = TRUE;
	    break;
	case normalize_uncoupled:
	    coupledSubscripts = FALSE;
	    break;
	default:
	    coupledSubscripts = FALSE;
	    assert(0 && "impossible case in SolveGEQ");
	    }


	if ((doTrace && desiredResult == SIMPLIFY) || DBUG) 
	    {
	    fprintf(outputFile, "\nafter normalization:\n");
	    printProblem();
	    fprintf(outputFile, "\n");
	    for(e=0;e<nGEQs;e++) assert(!GEQs[e].touched);
	    fprintf(outputFile, "eliminating variable using fourier-motzkin elimination\n");
	    };

	do 
	    {
	    eliminateAgain = 0;

	    if (nEQs > 0)
		return (solve(desiredResult));

	    if (!coupledSubscripts) 
		{
		if (safeVars == 0)
		    nGEQs = 0;
		else
		    for (e = nGEQs - 1; e >= 0; e--)
			if (GEQs[e].key > safeVars || -safeVars > GEQs[e].key)
			    deleteGEQ(e);
		nVars = safeVars;
		return (TRUE);
		};

	    if (desiredResult != SIMPLIFY)
		fv = 0;
	    else
		fv = safeVars;

	    if (nVars == 0 || nGEQs == 0) 
		{
		nGEQs = 0;
		if (desiredResult == SIMPLIFY) 
		    nVars = safeVars;
		return (TRUE);
		};
	    if (desiredResult == SIMPLIFY && nVars == safeVars) 
		{
		return (TRUE);
		};


	    if (nGEQs+6 > maxGEQs || nGEQs > 2 * nVars * nVars + 4 * nVars + 10) 
		{
		if (TRACE)
		    fprintf(outputFile, "TOO MANY EQUATIONS; %d equations, %d variables, ELIMINATING REDUNDANT ONES\n", nGEQs, nVars);
		if (!quickKill(0,true))
		    return 0;
		if (nEQs > 0)
		    return (solve(desiredResult));
		if (TRACE)
		    fprintf(outputFile, "END ELIMINATION OF REDUNDANT EQUATIONS\n");
		if (DBUG) printProblem();
		};



	    {
	    int darkConstraints,
		darkShadowFeasible,
		unit,parallelLB;
	    coef_t
		parallelSplinters,
		disjointSplinters,
		lbSplinters,
		ubSplinters;
	    coef_t bestScore = (posInfinity); // was MAXINT
	    int bestVar = -1;  // make compiler shut up
	    int exact;
	    coef_t score;
	    int Ue,Le;

	    if (desiredResult != SIMPLIFY) fv = 0;
	    else fv = safeVars;

	    if (DEBUG) 
		{	
		fprintf(outputFile,"Considering elimination possibilities[ \n");
		printProblem();
		};

	    for (i = nVars; i != fv; i--) 
		{

		analyzeElimination(
		    i,
		    darkConstraints,
		    darkShadowFeasible,
		    unit,
		    parallelSplinters,
		    disjointSplinters,
		    lbSplinters,
		    ubSplinters,
		    parallelLB);

		score = min(min(parallelSplinters,disjointSplinters),
			    min(lbSplinters,ubSplinters));
		exact = score == 1;
		score = 10000 * (score-1) + darkConstraints;
		score -= 3*unit;

		if (score < bestScore) 
		    {
		    bestScore = score;
		    bestVar = i;
		    if (i > 4 && score < nGEQs) break;
		    }
		};
	    assert(bestVar>=0);
	    exact = bestScore < 10000;
	    i = bestVar;
	    assert(i<=nVars);
	    analyzeElimination(
		i,
		darkConstraints,
		darkShadowFeasible,
		unit,
		parallelSplinters,
		disjointSplinters,
		lbSplinters,
		ubSplinters,
		parallelLB);
	    if (DEBUG) 
		{	
		fprintf(outputFile,"] Choose to eliminate %s \n",variable(i));
		};
	    coef_t splinters = lbSplinters;
	    if (splinters <= parallelSplinters) 
		parallelSplinters = (MAXINT);
	    else splinters = parallelSplinters;
	    if (disjointSplinters == 1) splinters = 1;
	    exact = splinters == 1;
	    if (inApproximateMode) exact = 1;
			
	    if (!triedEliminatingRedundant &&
		darkConstraints > maxGEQs)
		{
		if (TRACE)
		    fprintf(outputFile, "Elimination will create TOO MANY EQUATIONS; %d equations, %d variables, %d new constraints, ELIMINATING REDUNDANT ONES\n", nGEQs, nVars,darkConstraints);
		if (!quickKill(0))
		    return 0;
		if (nEQs > 0)
		    return (solve(desiredResult));
		if (TRACE)
		    fprintf(outputFile, "END ELIMINATION OF REDUNDANT EQUATIONS\n");
		if (DBUG) printProblem();

		triedEliminatingRedundant = 1;
		eliminateAgain = 1;
		continue;
		};

	    if (!exact && !triedEliminatingRedundant &&
		safeVars > 0 && desiredResult==SIMPLIFY) 
		{

		if (TRACE)
			fprintf(outputFile, "Trying to produce exact elimination by finding redundant constraints [\n");
		if (!quickKill(1)) return 0;
		if (TRACE)
			fprintf(outputFile, "]\n");
		triedEliminatingRedundant = 1;
		eliminateAgain = 1;
		continue;
		}
	    triedEliminatingRedundant = 0;

	    if ( desiredResult == SIMPLIFY && !exact) 
		{
		partialElimination();
	        switch (normalize()) 
			{
		    case normalize_false:
			return (FALSE);
			break;
		    case normalize_coupled:
		    case normalize_uncoupled:
			break;
			}
		if (nEQs) return solveEQ();
		if (DBUG) fprintf(outputFile,"Stopping short due to non-exact elimination\n");
		return (TRUE);
		};

	    if ( desiredResult == SIMPLIFY && 
		darkConstraints > maxGEQs) {
		if (DBUG) fprintf(outputFile,"Stopping short due to overflow of GEQs: %d\n", darkConstraints);
		return (TRUE);
		};

	    if ((doTrace && desiredResult == SIMPLIFY) || DBUG) 
		{
		fprintf(outputFile, "going to eliminate %s, (%d)\n",
			variable(i), i);
		if (DEBUG)
		    printProblem();
		fprintf(outputFile, "score = " coef_fmt "/" coef_fmt "\n", bestScore,splinters);
		};

	    if (!exact && desiredResult == SIMPLIFY 
		&& parallelSplinters == splinters) 
		{
		return parallelSplinter(parallelLB, 
					parallelSplinters, desiredResult);
		}
	    smoothed = 0;

	    if (i != nVars) 
		{
		j = nVars;
		swapVars(i,j);

		i = j;
		}
	    else if (DEBUG) 
		{
		printVars();
		fprintf(outputFile, "No swap needed before eliminating %s(%d/%d)\n",variable(i),i,nVars);
		for(j=1;j<=i;j++) fprintf(outputFile,"var #%d = %s(%x)\n",j,variable(j),var[j]);
		printProblem();
		};
	    nVars--;

	    if (exact) 
		{
		if (nVars == 1) 
		    {
		    coef_t upperBound = posInfinity;
		    coef_t lowerBound = negInfinity;
		    int ub_color = 0;
		    int lb_color = 0;
		    coef_t constantTerm, coefficient;
		    int topEqn = nGEQs - 1;
		    coef_t Lc;
		    for (Le = topEqn; Le >= 0; Le--)
			if ((Lc = GEQs[Le].coef[i]) == 0) 
			    {
			    if (GEQs[Le].coef[1] == 1) 
				{
				constantTerm = -GEQs[Le].coef[0];
				if (constantTerm > lowerBound ||
				    (constantTerm == lowerBound &&
				     !isRed(&GEQs[Le]))) 
				    {
				    lowerBound = constantTerm;
				    lb_color = GEQs[Le].color;
				    }
				if (DEBUG) 
				    {
				    if (GEQs[Le].color == black)
					fprintf(outputFile, " :::=> %s >= " coef_fmt "\n",
						variable(1), constantTerm);
				    else
					fprintf(outputFile, " :::=> [%s >= " coef_fmt "]\n",
						variable(1), constantTerm);
				    }
				}
			    else 
				{
				constantTerm = GEQs[Le].coef[0];
				if (constantTerm < upperBound ||
				    (constantTerm == upperBound
				     && !isRed(&GEQs[Le]))) 
				    {
				    upperBound = constantTerm;
				    ub_color = GEQs[Le].color;
				    }
				if (DEBUG) 
				    {
				    if (GEQs[Le].color == black)
					fprintf(outputFile, " :::=> %s <= " coef_fmt "\n",
						variable(1), constantTerm);
				    else
					fprintf(outputFile, " :::=> [%s <= " coef_fmt "]\n",
						variable(1), constantTerm);
				    }
				};
			    }
			else if (Lc > 0) 
			    {
			    for (Ue = topEqn; Ue >= 0; Ue--)
				if (GEQs[Ue].coef[i] < 0) 
				    {
				    if (GEQs[Le].key != -GEQs[Ue].key) 
					{
					coef_t Uc = -GEQs[Ue].coef[i];
					coefficient = GEQs[Ue].coef[1] * Lc + GEQs[Le].coef[1] * Uc;
					constantTerm = GEQs[Ue].coef[0] * Lc + GEQs[Le].coef[0] * Uc;
					if (DEBUG) 
					    {
					    printGEQextra(&(GEQs[Ue]));
					    fprintf(outputFile, "\n");
					    printGEQextra(&(GEQs[Le]));
					    fprintf(outputFile, "\n");
					    };
					if (coefficient > 0) 
					    {
					    constantTerm = -(int_div(constantTerm, coefficient));
				/* assert(black == 0) */
					    if (constantTerm > lowerBound ||
						(constantTerm == lowerBound &&
						 (desiredResult != SIMPLIFY ||
						  (GEQs[Ue].color == black && GEQs[Le].color == black)))) 
						{
						lowerBound = constantTerm;
						lb_color = GEQs[Ue].color || GEQs[Le].color;
						}
					    if (DEBUG) 
						{
						if (GEQs[Ue].color || GEQs[Le].color)
						    fprintf(outputFile, " ::=> [%s >= " coef_fmt "]\n",
							    variable(1), constantTerm);
						else
						    fprintf(outputFile, " ::=> %s >= " coef_fmt "\n",
							    variable(1), constantTerm);
						}
					    }
					else if (coefficient < 0) 
					    {
					    constantTerm = (int_div(constantTerm, -coefficient));
					    if (constantTerm < upperBound ||
						(constantTerm == upperBound &&
						 GEQs[Ue].color == black && GEQs[Le].color == black)) 
						{
						upperBound = constantTerm;
						ub_color = GEQs[Ue].color || GEQs[Le].color;
						}
					    if (DEBUG) 
						{
						if (GEQs[Ue].color || GEQs[Le].color)
						    fprintf(outputFile, " ::=> [%s <= " coef_fmt "]\n",
							    variable(1), constantTerm);
						else
						    fprintf(outputFile, " ::=> %s <= " coef_fmt "\n",
							    variable(1), constantTerm);
						}
					    }
					};
				    };
			    };
		    nGEQs = 0;
		    if (DEBUG)
			fprintf(outputFile,
				" therefore, %c" coef_fmt " <= %c%s%c <= " coef_fmt "%c\n",
				lb_color ? '[' : ' ', lowerBound,
				(lb_color && !ub_color) ? ']' : ' ',
				variable(1),
				(!lb_color && ub_color) ? '[' : ' ',
				upperBound, ub_color ? ']' : ' ');
		    if (lowerBound > upperBound)
			return (FALSE);
		    
		    if (upperBound == lowerBound) 
			{
			int e = newEQ();
			assert(e == 0);
			EQs[e].coef[1] = -1;
			EQs[e].coef[0] = upperBound;
			EQs[e].color = ub_color | lb_color;
			addingEqualityConstraint(0);
			}
		    else if (safeVars == 1) 
			{
			if (upperBound != posInfinity) 
			    {
			    int e = newGEQ();
			    assert(e == 0);
			    GEQs[e].coef[1] = -1;
			    GEQs[e].coef[0] = upperBound;
			    GEQs[e].color = ub_color;
			    GEQs[e].key = -1;
			    GEQs[e].touched = 0;
			    };
			if (lowerBound != negInfinity) 
			    {
			    int e = newGEQ();
			    GEQs[e].coef[1] = 1;
			    GEQs[e].coef[0] = -lowerBound;
			    GEQs[e].color = lb_color;
			    GEQs[e].key = 1;
			    GEQs[e].touched = 0;
			    };
			};
		    if (safeVars == 0) 
			nVars = 0;
		    return (TRUE);
		    };
		eliminateAgain = 1;

		{
		int deadEqns[maxDead];
		int numDead = 0;
		int topEqn = nGEQs - 1;
		int lowerBoundCount = 0;
		for (Le = topEqn; Le >= 0; Le--)
		    if (GEQs[Le].coef[i] > 0)
			lowerBoundCount++;
		if (DEBUG)
		    fprintf(outputFile, "lower bound count = %d\n", lowerBoundCount);
		if (lowerBoundCount == 0) 
		    {
		    if (desiredResult != SIMPLIFY) fv = 0;
		    else fv = safeVars;
		    nVars++;
		    freeEliminations(fv);
		    continue;
		    };
		for (Le = topEqn; Le >= 0; Le--)
		    if (GEQs[Le].coef[i] > 0) 
			{
			coef_t Lc = GEQs[Le].coef[i];
			for (Ue = topEqn; Ue >= 0; Ue--)
			    if (GEQs[Ue].coef[i] < 0) 
				{
				if (GEQs[Le].key != -GEQs[Ue].key) 
				    {
				    coef_t Uc = -GEQs[Ue].coef[i];
				    int e2;
				    if (numDead == 0) {
				/*( Big kludge warning ) */
				/* this code is still using location nVars+1 */
				/* but newGEQ, if it reallocates, only copies*/
				/* locations up to nVars.  This fixes that.  */
					nVars++;
					e2 = newGEQ();
					nVars--;
				    } else {
					e2 = deadEqns[--numDead];
				    }
				    if (DEBUG)
					fprintf(outputFile, "Le = %d, Ue = %d, gen = %d\n", Le, Ue, e2);
				    if (DEBUG) 
					{
					printGEQextra(&(GEQs[Le]));
					fprintf(outputFile, "\n");
					printGEQextra(&(GEQs[Ue]));
					fprintf(outputFile, "\n");
					};
				    eliminateAgain = 0;
				    coef_t g = gcd(Lc,Uc);
				    coef_t Lc_over_g = Lc/g;
				    coef_t Uc_over_g = Uc/g;

				    for (k = nVars; k >= 0; k--)
					GEQs[e2].coef[k] =
					    check_mul(GEQs[Ue].coef[k], Lc_over_g) + check_mul(GEQs[Le].coef[k], Uc_over_g);
				    GEQs[e2].coef[nVars + 1] = 0;
				    GEQs[e2].touched = TRUE;
				    GEQs[e2].color = GEQs[Ue].color | GEQs[Le].color;

				    if (DEBUG) 
					{
					printGEQ(&(GEQs[e2]));
					fprintf(outputFile, "\n");
					};
				    };
				if (lowerBoundCount == 1) 
				    {
				    deadEqns[numDead++] = Ue;
				    if (DEBUG)
					fprintf(outputFile, "Killed %d\n", Ue);
				    };
				};
			lowerBoundCount--;
			deadEqns[numDead++] = Le;
			if (DEBUG)
			    fprintf(outputFile, "Killed %d\n", Le);
			};
		{
		int isDead[maxmaxGEQs];
		for (e = nGEQs - 1; e >= 0; e--)
		    isDead[e] = FALSE;
		while (numDead > 0) 
		    {
		    e = deadEqns[--numDead];
		    isDead[e] = TRUE;
		    };
		for (e = nGEQs - 1; e >= 0; e--)
		    if (isDead[e]) 
			{
			nVars++;
			deleteGEQ(e);
			nVars--;
			}
		};
		continue;
		};
		}
	    else 
		{
		Problem *rS, *iS;

		rS = new Problem;
		iS = new Problem;

		iS->nVars = rS->nVars = nVars; // do this immed.; in case of reallocation, we
                                               // need to know how much to copy
		rS->get_var_name = get_var_name;
		rS->getVarNameArgs = getVarNameArgs;
		iS->get_var_name = get_var_name;
		iS->getVarNameArgs = getVarNameArgs;

		for (e = 0; e < nGEQs; e++)
		    if (GEQs[e].coef[i] == 0) 
			{
			int re2 = rS->newGEQ();
			int ie2 = iS->newGEQ();
			eqncpy(&(rS->GEQs[re2]), &GEQs[e]);
			eqncpy(&(iS->GEQs[ie2]), &GEQs[e]);
			if (DEBUG) 
			    {
			    int t;
			    fprintf(outputFile, "Copying (%d, " coef_fmt "): ", i, GEQs[e].coef[i]);
			    printGEQextra(&GEQs[e]);
			    fprintf(outputFile, "\n");
			    for (t = 0; t <= nVars + 1; t++)
				fprintf(outputFile, coef_fmt " ", GEQs[e].coef[t]);
			    fprintf(outputFile, "\n");
			    };
			};
		for (Le = nGEQs - 1; Le >= 0; Le--)
		    if (GEQs[Le].coef[i] > 0) 
			{
			coef_t Lc = GEQs[Le].coef[i];
			for (Ue = nGEQs - 1; Ue >= 0; Ue--)
			    if (GEQs[Ue].coef[i] < 0)
				if (GEQs[Le].key != -GEQs[Ue].key) 
				    {
				    coef_t Uc = -GEQs[Ue].coef[i];
				    coef_t g = gcd(Lc,Uc);
				    coef_t Lc_over_g = Lc/g;
				    coef_t Uc_over_g = Uc/g;
				    int re2 = rS->newGEQ();
				    int ie2 = iS->newGEQ();
				    rS->GEQs[re2].touched = iS->GEQs[ie2].touched = TRUE;
				    if (DEBUG) 
					{
					fprintf(outputFile, "---\n");
					fprintf(outputFile, "Le(Lc) = %d(" coef_fmt "), Ue(Uc) = %d(" coef_fmt "), gen = %d\n", Le, Lc, Ue, Uc, ie2);
					printGEQextra(&GEQs[Le]);
					fprintf(outputFile, "\n");
					printGEQextra(&GEQs[Ue]);
					fprintf(outputFile, "\n");
					};

				    if (Uc == Lc) 
					{
					for (k = nVars; k >= 0; k--)
					    iS->GEQs[ie2].coef[k] = rS->GEQs[re2].coef[k] =
						GEQs[Ue].coef[k] + GEQs[Le].coef[k];
					iS->GEQs[ie2].coef[0] -= (Uc - 1);
					}
				    else 
					{
					for (k = nVars; k >= 0; k--)
					    iS->GEQs[ie2].coef[k] = rS->GEQs[re2].coef[k] =
						check_mul(GEQs[Ue].coef[k], Lc_over_g) + check_mul(GEQs[Le].coef[k], Uc_over_g);
					iS->GEQs[ie2].coef[0] -= (Uc_over_g - 1) * (Lc_over_g - 1);
					}

				    iS->GEQs[ie2].color = rS->GEQs[re2].color
					= GEQs[Ue].color || GEQs[Le].color;

				    if (DEBUG) 
					{
					printGEQ(&(rS->GEQs[re2]));
					fprintf(outputFile, "\n");
					};
//				    ie2 = iS->newGEQ();
//				    re2 = rS->newGEQ();
				    };

			};
		iS->variablesInitialized = rS->variablesInitialized = 1;
		iS->nEQs = rS->nEQs = 0;
		assert(desiredResult != SIMPLIFY);
		assert(nSUBs == 0);
		iS->nSUBs = rS->nSUBs = nSUBs;
		iS->safeVars = rS->safeVars = safeVars;
		int t;
		for (t = nVars; t >= 0; t--)
		    rS->var[t] = var[t];
		for (t = nVars; t >= 0; t--)
		    iS->var[t] = var[t];
		nVars++;
		if (desiredResult != TRUE) 
		    {
		    int t = trace;
		    if (TRACE)
			fprintf(outputFile, "\nreal solution(%d):\n", depth);
		    depth++;
		    trace = 0;
		    if (originalProblem == noProblem) 
			{
			originalProblem = this;
			result = rS->solveGEQ(FALSE);
			originalProblem = noProblem;
			}
		    else
			result = rS->solveGEQ(FALSE);
		    trace = t;
		    depth--;
		    if (result == FALSE) 
			{
			delete rS;
			delete iS;
			return (result);
			};

		    if (nEQs > 0) 
			{
			/* An equality constraint must have been found */
			delete rS;
			delete iS;
			return (solve(desiredResult));
			};
		    };
		if (desiredResult != FALSE) 
		    {
		    if (darkShadowFeasible) 
			{
			if (TRACE)
			    fprintf(outputFile, "\ninteger solution(%d):\n", depth);
			depth++;
			conservative++;
			result = iS->solveGEQ(desiredResult);
			conservative--;
			depth--;
			if (result != FALSE) 
			    {
			    delete rS;
			    delete iS;
			    return (result);
			    };
			};
		    if (TRACE)
			fprintf(outputFile, "have to do exact analysis\n");
		    {
		    coef_t worstLowerBoundConstant=1;
		    int lowerBounds = 0;
		    int lowerBound[maxmaxGEQs];
		    int smallest;
		    int t;
		    conservative++;
		    for (e = 0; e < nGEQs; e++)
			if (GEQs[e].coef[i] < -1) 
			    {
			    set_max(worstLowerBoundConstant,
				   -GEQs[e].coef[i]);
			    }
			else if (GEQs[e].coef[i] > 1)
			    lowerBound[lowerBounds++] = e;
		    /* sort array */
		    for (j = 0; j < lowerBounds; j++) 
			{
			smallest = j;
			for (k = j + 1; k < lowerBounds; k++)
			    if (GEQs[lowerBound[smallest]].coef[i] > GEQs[lowerBound[k]].coef[i])
				smallest = k;
			t = lowerBound[smallest];
			lowerBound[smallest] = lowerBound[j];
			lowerBound[j] = t;
			};
		    if (DEBUG) 
			{
			fprintf(outputFile, "lower bound coeeficients = ");
			for (j = 0; j < lowerBounds; j++)
			    fprintf(outputFile, " " coef_fmt, GEQs[lowerBound[j]].coef[i]);
			fprintf(outputFile, "\n");
			};


		    for (j = 0; j < lowerBounds; j++) 
			{
			coef_t maxIncr;
			coef_t c;
			e = lowerBound[j];
			maxIncr = ((GEQs[e].coef[i] - 1) * (worstLowerBoundConstant - 1) - 1)
			    / worstLowerBoundConstant;

				/* maxIncr += 2; */
			if ((doTrace && desiredResult == SIMPLIFY) || DBUG) 
			    {
			    fprintf(outputFile, "for equation ");
			    printGEQ(&GEQs[e]);
			    fprintf(outputFile, "\ntry decrements from 0 to " coef_fmt "\n", maxIncr);
			    printProblem();
			    };
			if (maxIncr > 50) 
			    {
			    if (!smoothed && smoothWeirdEquations()) 
				{
				conservative--;
				delete rS;
				delete iS;
				smoothed = 1;
				goto solveGEQstart;
				};
			    };
			int neweq = newEQ();
			assert(neweq == 0);
			eqncpy(&EQs[neweq], &GEQs[e]);
				/*
				 * if (GEQs[e].color) fprintf(outputFile,"warning: adding black equality constraint
				 * based on red inequality\n");
				 */
			EQs[neweq].color = black;
			eqnzero(&GEQs[e]);
			GEQs[e].touched = TRUE;
			for (c = maxIncr; c >= 0; c--) 
			    {
			    if (DBUG)
				fprintf(outputFile, "trying next decrement of " coef_fmt "\n", maxIncr - c);
			    if (DBUG)
				printProblem();
			    *rS = *this;
			    if (DEBUG)
				rS->printProblem();
			    result = rS->solve(desiredResult);
			    if (result == TRUE) 
				{
				delete rS;
				delete iS;
				conservative--;
				return (TRUE);
				};
			    EQs[0].coef[0]--;
			    };
			if (j + 1 < lowerBounds) 
			    {
			    nEQs = 0;
			    eqncpy(&GEQs[e], &EQs[0]);
			    GEQs[e].touched = 1;
			    GEQs[e].color = black;
			    *rS = *this;
			    if (DEBUG)
				fprintf(outputFile, "exhausted lower bound, checking if still feasible ");
			    result = rS->solve(FALSE);
			    if (result == FALSE)
				break;
			    };
			};
		    if ((doTrace && desiredResult == SIMPLIFY) || DBUG)
			fprintf(outputFile, "fall-off the end\n");
		    delete rS;
		    delete iS;

		    conservative--;
		    return (FALSE);
		    };
		    };
		delete rS;
		delete iS;
		};
	    return (UNKNOWN);
	    };
	    }
	while (eliminateAgain);
	};
    }


int
Problem::
parallelSplinter(int e, int diff, int desiredResult)
    {
    Problem *tmpProblem;
    int i;
    if (DBUG) 
	{
	fprintf(outputFile, "Using parallel splintering\n");
	printProblem();
	}
    tmpProblem = new Problem;
    int neweq = newEQ();
    assert(neweq == 0);
    eqncpy(&EQs[0], &GEQs[e]);
    for (i = 0; i <= diff; i++) 
	{
	*tmpProblem = * this;
	if (DBUG) 
	    {
	    fprintf(outputFile, "Splinter # %i\n", i);
	    printProblem();
	    }
	if (tmpProblem->solve(desiredResult)) 
	    {
	    delete tmpProblem;
	    return TRUE;
	    }
	EQs[0].coef[0]--;
	}
    delete tmpProblem;
    return FALSE;
    }

int
Problem:: verifyProblem()
    {
    int result;
    int e;
    int areRed;
    check();
    Problem tmpProblem(*this);
    tmpProblem.varsOfInterest  = 0;
    tmpProblem.safeVars = 0;
    tmpProblem.nSUBs = 0;
    tmpProblem.nMemories = 0;
    areRed = 0;
    if (mayBeRed) 
	{
	for(e=0; e<nEQs;  e++) if (EQs[e].color) areRed = 1;
	for(e=0; e<nGEQs; e++) if (GEQs[e].color) areRed = 1;
	if (areRed) tmpProblem.turnRedBlack();
	};
    originalProblem = this;
    assert(!outerColor);
    outerColor = areRed;
    if (TRACE) 
	{
        fprintf(outputFile, "verifying problem: [\n");
        printProblem();
	};
    tmpProblem.check();
    tmpProblem.freeEliminations(0);
    result = tmpProblem.solve(UNKNOWN);
    originalProblem = noProblem;
    outerColor = 0;
    if (TRACE) 
	{
        if (result)
            fprintf(outputFile, "] verified problem\n");
        else
            fprintf(outputFile, "] disproved problem\n");
        printProblem();
	};
    check();
    return result;
    }


void
Problem:: freeEliminations(int fv)
/* do free eliminations */
    {
    int tryAgain = 1;
    int i, e, e2;
    while (tryAgain) 
	{
	tryAgain = 0;
	for (i = nVars; i > fv; i--) 
	    {
	    for (e = nGEQs - 1; e >= 0; e--)
		if (GEQs[e].coef[i])
		    break;
	    if (e < 0)
		e2 = e;
	    else if (GEQs[e].coef[i] > 0) 
		{
		for (e2 = e - 1; e2 >= 0; e2--)
		    if (GEQs[e2].coef[i] < 0)
			break;
		}
	    else 
		{
		for (e2 = e - 1; e2 >= 0; e2--)
		    if (GEQs[e2].coef[i] > 0)
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
		if (DBUG)
		    fprintf(outputFile, "a free elimination of %s (%d)\n", variable(i),e);
		if (e >= 0) 
		    {
		    deleteGEQ(e);
		    for (e--; e >= 0; e--)
			if (GEQs[e].coef[i]) 
			    {
			    deleteGEQ(e);
			    };
		    tryAgain = (i < nVars);
		    };
		deleteVariable(i);


		};
	    };
	};

    if (DEBUG) 
	{
	fprintf(outputFile, "\nafter free eliminations:\n");
	printProblem();
	fprintf(outputFile, "\n");
	};
    }

void
Problem::
freeRedEliminations()
/* do free red eliminations */
    {
    int tryAgain = 1;
    int i, e, e2;
    int isRedVar[maxVars];
    int isDeadVar[maxVars];
    int isDeadGEQ[maxmaxGEQs];
    for (i = nVars; i > 0; i--) 
	{
	isRedVar[i] = 0;
	isDeadVar[i] = 0;
	};
    for (e = nGEQs - 1; e >= 0; e--) 
	{
	isDeadGEQ[e] = 0;
	if (GEQs[e].color)
	    for (i = nVars; i > 0; i--)
		if (GEQs[e].coef[i] != 0)
		    isRedVar[i] = 1;
	};

    while (tryAgain) 
	{
	tryAgain = 0;
	for (i = nVars; i > 0; i--)
	    if (!isRedVar[i] && !isDeadVar[i]) 
		{
		for (e = nGEQs - 1; e >= 0; e--)
		    if (!isDeadGEQ[e] && GEQs[e].coef[i])
			break;
		if (e < 0)
		    e2 = e;
		else if (GEQs[e].coef[i] > 0) 
		    {
		    for (e2 = e - 1; e2 >= 0; e2--)
			if (!isDeadGEQ[e2] && GEQs[e2].coef[i] < 0)
			    break;
		    }
		else 
		    {
		    for (e2 = e - 1; e2 >= 0; e2--)
			if (!isDeadGEQ[e2] && GEQs[e2].coef[i] > 0)
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
		    if (DBUG)
			fprintf(outputFile, "a free red elimination of %s\n", variable(i));
		    for (; e >= 0; e--)
			if (GEQs[e].coef[i])
			    isDeadGEQ[e] = 1;
		    tryAgain = 1;
		    isDeadVar[i] = 1;
		    };
		};
	};

    for (e = nGEQs - 1; e >= 0; e--)
	if (isDeadGEQ[e])
	    deleteGEQ(e);

    for (i = nVars; i > safeVars; i--)
	if (isDeadVar[i])
	    deleteVariable(i);


    if (DEBUG) 
	{
	fprintf(outputFile, "\nafter free red eliminations:\n");
	printProblem();
	fprintf(outputFile, "\n");
	};
    }

} // end of namespace omega
