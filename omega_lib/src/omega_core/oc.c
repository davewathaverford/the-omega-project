/* $Id: oc.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $*/

#include <basic/bool.h>
#include <omega/omega_core/oc_i.h>

namespace omega {

eqn SUBs[maxVars+1];
Memory redMemory[maxVars+1];


int
Problem::
reduceProblem()
    {
    int result;
    checkVars(nVars+1);
    assert(omegaInitialized);
    if (nVars > nEQs + 3 * safeVars)
	freeEliminations(safeVars);

    check();
    if (!mayBeRed && nSUBs == 0 && safeVars == 0) 
	{
	result = solve(UNKNOWN);
        nGEQs = 0;
	nEQs = 0;
	nSUBs = 0;
	nMemories = 0;
	if (!result) 
	    {
	    int e = newEQ();
	    assert(e == 0);
	    eqnzero(&EQs[0]);
	    EQs[0].color = black;
	    EQs[0].coef[0] = 1;
	    }
        check();
	return result;
	}
    return solve(SIMPLIFY);
    }


int
Problem::
simplifyProblem(int verify, int subs, int redundantElimination)
    {
    checkVars(nVars+1);
    assert(omegaInitialized);
    setInternals();
    check();
    if (!reduceProblem())  goto returnFalse;
    if (verify) 
	{
	addingOuterEqualities++;
	int r = verifyProblem();
	addingOuterEqualities--;
	if (!r) goto returnFalse;
	if (nEQs) { // found some equality constraints during verification
		int numRed = 0;
		if (mayBeRed) 
		        for (int e = nGEQs - 1; e >= 0; e--) if (GEQs[e].color == _red) numRed++;
		if (mayBeRed && nVars == safeVars && numRed == 1)
			nEQs = 0; // discard them
		else if (!reduceProblem()) {
			assert(0 && "Added equality constraint to verified problem generates false");
			Exit(1);
			}
		}
	};
    if (redundantElimination) 
	{
        if (redundantElimination > 1) 
	    {
	    if (!expensiveEqualityCheck()) goto returnFalse;
	    };
        if (!quickKill(0)) goto returnFalse;
        if (redundantElimination > 1) 
	    {
	    if (!expensiveKill()) goto returnFalse;
	    };
	};
    resurrectSubs();
    if (redundantElimination) 
	simplifyStrideConstraints();
    if (redundantElimination > 2 && safeVars < nVars) 
	{
        if (!quickKill(0)) goto returnFalse;
	return simplifyProblem(verify, subs, redundantElimination-2);
	};
    setExternals();
    assert(nMemories == 0);
    return (1);
returnFalse:

	    nGEQs = 0;
	    nEQs = 0;
	    resurrectSubs();
	    nGEQs = 0;
	    nEQs = 0;
	    int neweq = newEQ();
	    assert(neweq == 0);
	    eqnzero(&EQs[neweq]);
	    EQs[neweq].color = 0;
	    EQs[neweq].coef[0] = 1;
	     nMemories = 0;
	    return 0;
    }

int
Problem::
simplifyApproximate(int strides_allowed) 
    {
    int result;
    checkVars(nVars+1);
    assert(inApproximateMode  == 0);

    inApproximateMode = 1;
    inStridesAllowedMode = strides_allowed;
    if (TRACE)
	fprintf(outputFile, "Entering Approximate Mode [\n");

    assert(omegaInitialized);
    result = simplifyProblem(0,0,0);

    while (result && !strides_allowed && nVars > safeVars) 
	{
	int e;
        for (e = nGEQs - 1; e >= 0; e--) 
		if (GEQs[e].coef[nVars]) deleteGEQ(e);
        for (e = nEQs - 1; e >= 0; e--) 
		if (EQs[e].coef[nVars]) deleteEQ(e);
	nVars--;
        result = simplifyProblem(0,0,0);
	};

    if (TRACE)
	fprintf(outputFile, "] Leaving Approximate Mode\n");
    
    assert(inApproximateMode  == 1);
    inApproximateMode=0;
    inStridesAllowedMode = 0;

    assert(nMemories == 0);
    return (result);
    }





/*
 * Return 1 if red equations constrain the set of possible solutions. We assume that there are solutions to the black
 * equations by themselves, so if there is no solution to the combined problem, we return 1.
 */

#ifdef GIST_CHECK
Problem full_answer, context;
Problem redProblem;
#endif

redCheck
Problem::
redSimplifyProblem(int effort, int computeGist)
    {
    int result;
    int e;

    checkVars(nVars+1);
    assert(mayBeRed >= 0);
    mayBeRed++;

    assert(omegaInitialized);
    if (TRACE) 
	{
	fprintf(outputFile, "Checking for red equations:\n");
	printProblem();
	};
    setInternals();

#ifdef GIST_CHECK
    int r1,r2;
    if (TRACE) 
	fprintf(outputFile,"Set-up for gist invariant checking[\n");
    redProblem = *this;
    redProblem.check();
    full_answer = *this;
    full_answer.check();
    full_answer.turnRedBlack();
    full_answer.check();
    r1 = full_answer.simplifyProblem(1,0,1);
    full_answer.check();
    if (DBUG) fprintf(outputFile,"Simplifying context [\n");
    context = *this;	
    context.check();
    context.deleteRed();
    context.check();
    r2 = context.simplifyProblem(1,0,1);
    context.check();
    if (DBUG) fprintf(outputFile,"] Simplifying context\n");

    if (!r2 && TRACE) fprintf(outputFile, "WARNING: Gist context is false!\n");
    if (TRACE) 
	fprintf(outputFile,"] Set-up for gist invariant checking done\n");
#endif

    if (solveEQ() == FALSE) 
	{
	if (TRACE)
	    fprintf(outputFile, "Gist is FALSE\n");
	if (computeGist) 
	    {
	    nMemories = 0;
	    nGEQs = 0;
	    nEQs = 0;
	    resurrectSubs();
	    nGEQs = 0;
	    nEQs = 0;
	    int neweq = newEQ();
	    assert(neweq == 0);
	    eqnzero(&EQs[neweq]);
	    EQs[neweq].color = _red;
	    EQs[neweq].coef[0] = 1;
	    };
        mayBeRed--;
	return redFalse;
	}

    if (!computeGist && nMemories) 
	return redConstraints;
    if (normalize() == normalize_false) 
	{
        if (TRACE)
            fprintf(outputFile, "Gist is FALSE\n");
	if (computeGist) 
	    {
	    nGEQs = 0;
	    nEQs = 0;
	    resurrectSubs();
	    nMemories = 0;
	    nGEQs = 0;
	    nEQs = 0;
	    int neweq = newEQ();
	    assert(neweq == 0);
	    eqnzero(&EQs[neweq]);
	    EQs[neweq].color = _red;
	    EQs[neweq].coef[0] = 1;
	    };
        mayBeRed--;
        return redFalse;
	}

    result = 0;
    for (e = nGEQs - 1; e >= 0; e--) if (GEQs[e].color == _red) result = 1;
    for (e = nEQs - 1; e >= 0; e--) if (EQs[e].color == _red) result = 1;
    if (nMemories) result = 1;
    if (!result) 
	{
	if (computeGist) 
	    {
	    nGEQs = 0;
	    nEQs = 0;
	    resurrectSubs();
	    nGEQs = 0;
	    nMemories = 0;
	    nEQs = 0;
	    };
    	mayBeRed--;
	return noRed;
	};

    result = simplifyProblem(effort?1:0,1,0);
#ifdef GIST_CHECK
    if (!r1 && TRACE && result) 
	fprintf(outputFile, "Gist is False but not detected\n");
#endif
    if (!result) 
	{
	if (TRACE)
	    fprintf(outputFile, "Gist is FALSE\n");
	if (computeGist) 
	    {
	    nGEQs = 0;
	    nEQs = 0;
	    resurrectSubs();
	    nGEQs = 0;
	    nEQs = 0;
	    int neweq = newEQ();
	    assert(neweq == 0);
	    nMemories = 0;
	    eqnzero(&EQs[neweq]);
	    EQs[neweq].color = _red;
	    EQs[neweq].coef[0] = 1;
	    };
    	mayBeRed--;
	return redFalse;
	}

    freeRedEliminations();

    result = 0;
    for (e = nGEQs - 1; e >= 0; e--) if (GEQs[e].color == _red) result = 1;
    for (e = nEQs - 1; e >= 0; e--) if (EQs[e].color == _red) result = 1;
    if (nMemories) result = 1;
    if (!result) 
	{
	if (computeGist) 
	    {
	    nGEQs = 0;
	    nEQs = 0;
	    resurrectSubs();
	    nGEQs = 0;
	    nMemories = 0;
	    nEQs = 0;
	    };
    	mayBeRed--;
	return noRed;
	};

    if (effort && (computeGist || !nMemories)) 
	{
	if (TRACE)
	    fprintf(outputFile, "*** Doing potentially expensive elimination tests for red equations [\n");
	quickRedKill(computeGist);
	checkGistInvariant();
	result = nMemories;
	for (e = nGEQs - 1; e >= 0; e--) if (GEQs[e].color == _red) result++;
	for (e = nEQs - 1; e >= 0; e--) if (EQs[e].color == _red) result++;
	if (result && effort > 1 && (computeGist || !nMemories))  
	    {
	    expensiveRedKill();
	    result = nMemories;
	    for (e = nGEQs-1; e >= 0; e--) if (GEQs[e].color == _red) result++;
	    for (e = nEQs-1; e >= 0; e--) if (EQs[e].color == _red) result++;
	    };
	
	if (!result)  
	    {
	    if (TRACE)
		fprintf(outputFile, "]******************** Redudant Red Equations eliminated!!\n");
	    if (computeGist) 
		{
		nGEQs = 0;
		nEQs = 0;
		resurrectSubs();
		nGEQs = 0;
	    nMemories = 0;
		nEQs = 0;
		};
	    mayBeRed--;
	    return noRed;
	    }
	    
	if (TRACE) fprintf(outputFile, "]******************** Red Equations remain\n");
	if (DEBUG) printProblem();
	};
    if (computeGist) 
	{
	resurrectSubs();	
	cleanoutWildcards();	
	deleteBlack();
	};
    setExternals();
    mayBeRed--;
    assert(nMemories == 0);
    return redConstraints;
    }


void
Problem::
convertEQstoGEQs(int excludeStrides)
    {
    int i;
    int e;
    if (DBUG)
	fprintf(outputFile, "Converting all EQs to GEQs\n");
    simplifyProblem(0,0,0);
    for(e=0;e<nEQs;e++) 
	{
	bool isStride = 0;
	int e2 = newGEQ();
	if (excludeStrides)
	    for(i = safeVars+1; i <= nVars; i++)
		isStride = isStride || (EQs[e].coef[i] != 0);
	if (isStride) continue;
	eqncpy(&GEQs[e2], &EQs[e]);
	GEQs[e2].touched = 1;
	e2 = newGEQ();
	eqncpy(&GEQs[e2], &EQs[e]);
	GEQs[e2].touched = 1;
	for (i = 0; i <= nVars; i++)
	    GEQs[e2].coef[i] = -GEQs[e2].coef[i];
	};
    // If we have eliminated all EQs, can set nEQs to 0
    // If some strides are left, we don't know the position of them in the EQs
    // array, so decreasing nEQs might remove wrong EQs -- we just leave them
    // all in. (could sort the EQs to move strides to the front, but too hard.)
    if (!excludeStrides) nEQs=0; 
    if (DBUG)
	printProblem();
    }


void
Problem::
convertEQtoGEQs(int eq)
    {
    int i;
    if (DBUG)
	fprintf(outputFile, "Converting EQ %d to GEQs\n",eq);
    int e2 = newGEQ();
    eqncpy(&GEQs[e2], &EQs[eq]);
    GEQs[e2].touched = 1;
    e2 = newGEQ();
    eqncpy(&GEQs[e2], &EQs[eq]);
    GEQs[e2].touched = 1;
    for (i = 0; i <= nVars; i++)
	GEQs[e2].coef[i] = -GEQs[e2].coef[i];
    if (DBUG)
	printProblem();
    }



#ifdef GIST_CHECK
enum compareAnswer {apparentlyEqual, mightNotBeEqual, NotEqual};

static
compareAnswer checkEquiv(Problem *p1, Problem *p2) 
    {
    int r1,r2;

    p1->check();
    p2->check();
    p1->resurrectSubs(); 
    p2->resurrectSubs(); 
    p1->check();
    p2->check();
    p1->putVariablesInStandardOrder(); 
    p2->putVariablesInStandardOrder(); 
    p1->check();
    p2->check();
    p1->ordered_elimination(0); 
    p2->ordered_elimination(0); 
    p1->check();
    p2->check();
    r1 = p1->simplifyProblem(1,1,0);
    r2 = p2->simplifyProblem(1,1,0);
    p1->check();
    p2->check();

    if (!r1 || !r2) 
	{
	if (r1 == r2) return apparentlyEqual;
	return NotEqual;
	};
    if (p1->nVars != p2->nVars 
	|| p1->nGEQs != p2->nGEQs 
	|| p1->nSUBs != p2->nSUBs
	|| p1->checkSum()  != p2->checkSum()) 
	{
	r1 = p1->simplifyProblem(0,1,1);
	r2 = p2->simplifyProblem(0,1,1);
	assert(r1 && r2);
	p1->check();
	p2->check();
	if (p1->nVars != p2->nVars 
	    || p1->nGEQs != p2->nGEQs 
	    || p1->nSUBs != p2->nSUBs
	    || p1->checkSum()  != p2->checkSum()) 
	    {
	    r1 = p1->simplifyProblem(0,1,2);
	    r2 = p2->simplifyProblem(0,1,2);
	    p1->check();
	    p2->check();
	    assert(r1 && r2);
	    if (p1->nVars != p2->nVars 
		|| p1->nGEQs != p2->nGEQs 
		|| p1->nSUBs != p2->nSUBs
		|| p1->checkSum()  != p2->checkSum()) 
		{
		p1->check();
		p2->check();
		p1->resurrectSubs(); 
		p2->resurrectSubs(); 
		p1->check();
		p2->check();
		p1->putVariablesInStandardOrder(); 
		p2->putVariablesInStandardOrder(); 
		p1->check();
		p2->check();
		p1->ordered_elimination(0); 
		p2->ordered_elimination(0); 
		p1->check();
		p2->check();
		r1 = p1->simplifyProblem(1,1,0);
		r2 = p2->simplifyProblem(1,1,0);
		p1->check();
		p2->check();
		};
	    }
	}
    ;
    if (p1->nVars != p2->nVars 
	|| p1->nSUBs != p2->nSUBs
	|| p1->nGEQs != p2->nGEQs 
	|| p1->nSUBs != p2->nSUBs) return NotEqual;
    if (p1->checkSum()  != p2->checkSum()) return mightNotBeEqual;
    return apparentlyEqual;
    }
#endif

void Problem::checkGistInvariant() const 
    {
#ifdef GIST_CHECK
    Problem new_answer;
    int r;

    check();
    fullAnswer.check();
    context.check();

    if (safeVars < nVars) 
	{
	if (DBUG) 
	    {
	    fprintf(outputFile,"Can't check gist invariant due to wildcards\n");
	    printProblem();
	    };
	return;
	};
    if (DBUG) 
	{
	fprintf(outputFile,"Checking gist invariant on: [\n");
	printProblem();
	};

    new_answer = *this;
    new_answer->resurrectSubs();
    new_answer->cleanoutWildcards();
    if (DEBUG) 
	{
	fprintf(outputFile,"which is: \n");
	printProblem();
	};
    deleteBlack(&new_answer);
    turnRedBlack(&new_answer);
    if (DEBUG) 
	{
	fprintf(outputFile,"Black version of answer: \n");
	printProblem(&new_answer);
	};
    problem_merge(&new_answer,&context);

    r = checkEquiv(&full_answer,&new_answer);
    if (r != apparentlyEqual) 
	{
	fprintf(outputFile,"GIST INVARIANT REQUIRES MANUAL CHECK:[\n");
	fprintf(outputFile,"Original problem:\n");
	printProblem(&redProblem);

	fprintf(outputFile,"Context:\n");
	printProblem(&context);

	fprintf(outputFile,"Computed gist:\n");
	printProblem();

	fprintf(outputFile,"Combined answer:\n");
	printProblem(&full_answer);

	fprintf(outputFile,"Context && red constraints:\n");
	printProblem(&new_answer);
	fprintf(outputFile,"]\n");
	};
		
    if (DBUG) 
	{
	fprintf(outputFile,"] Done checking gist invariant on\n");
	};
#endif
    }	




} // end of namespace omega
