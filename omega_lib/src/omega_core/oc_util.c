
/* $Id: oc_util.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <omega/omega_core/oc_i.h>


namespace omega {

void Problem:: problem_merge(Problem &p2)
    {
    int newLocation[maxVars];
    int i,e2;

    resurrectSubs();
    p2.resurrectSubs();
    setExternals();
    p2.setExternals();

    assert(safeVars == p2.safeVars);
    if(DBUG) 
	{
	fprintf(outputFile,"Merging:\n");
	printProblem();
	fprintf(outputFile,"and\n");
	p2.printProblem();
	};
    for(i=1; i<= p2.safeVars; i++)  
	{
	assert(p2.var[i] > 0) ;
	newLocation[i] = forwardingAddress[p2.var[i]];
	};
    for(; i<= p2.nVars; i++) 
	{
	int j = ++(nVars);
	newLocation[i] = j;
	zeroVariable(j);
	var[j] = -1;
	};
    newLocation[0] = 0;
	    
    for (e2 = p2.nEQs - 1; e2 >= 0; e2--)  
	{
	int e1 = newEQ();
        eqnnzero(&(EQs[e1]), nVars);
	for(i=0;i<=p2.nVars;i++) 
	    EQs[e1].coef[newLocation[i]] = p2.EQs[e2].coef[i];
	};
    for (e2 = p2.nGEQs - 1; e2 >= 0; e2--)  
	{
	int e1 = newGEQ();
        eqnnzero(&(GEQs[e1]), nVars);
	GEQs[e1].touched = 1;
	for(i=0;i<=p2.nVars;i++) 
	    GEQs[e1].coef[newLocation[i]] = p2.GEQs[e2].coef[i]; 
        }; 
    int w = -1; 
    for (i = 1; i <= nVars; i++) 
	if (var[i] < 0)  var[i] = w--; 
    if(DBUG) 
	{
	fprintf(outputFile,"to get:\n");
	printProblem();
	};
    }



void
Problem::
chainUnprotect()
    {
    int i, e;
    int unprotect[maxVars];
    int any = 0;
    for (i = 1; i <= safeVars; i++) 
	{
	unprotect[i] = (var[i] < 0);
	for (e = nSUBs - 1; e >= 0; e--)
	    if (SUBs[e].coef[i])
		unprotect[i] = 0;
	};
    for (i = 1; i <= safeVars; i++) if (unprotect[i]) any=1;
    if (!any) return;

    if (DBUG) 
	{
	fprintf(outputFile, "Doing chain reaction unprotection\n");
	printProblem();
	for (i = 1; i <= safeVars; i++)
	    if (unprotect[i])
		fprintf(outputFile, "unprotecting %s\n", variable(i));
	};
    for (i = 1; i <= safeVars; i++)
	if (unprotect[i]) 
	    {
	    /* wild card */
	    if (i < safeVars) 
		{
		int j = safeVars;
		swap(var[i], var[j]);
		for (e = nGEQs - 1; e >= 0; e--) 
		    {
		    GEQs[e].touched = 1;
		    swap(GEQs[e].coef[i], GEQs[e].coef[j]);
		    };
		for (e = nEQs - 1; e >= 0; e--)
		    swap(EQs[e].coef[i], EQs[e].coef[j]);
		for (e = nSUBs - 1; e >= 0; e--)
		    swap(SUBs[e].coef[i], SUBs[e].coef[j]);
		swap(unprotect[i], unprotect[j]);
		i--;
		};
	    safeVars--;
	    };
    if (DBUG) 
	{
	fprintf(outputFile, "After chain reactions\n");
	printProblem();
	};
    }

void
Problem::
resurrectSubs()
    {
    if (nSUBs > 0 && !pleaseNoEqualitiesInSimplifiedProblems) 
	{
	int i, e, n, m,mbr;
	mbr = 0;
        for (e = nGEQs - 1; e >= 0; e--) if (GEQs[e].color) mbr=1;
        for (e = nEQs - 1; e >= 0; e--) if (EQs[e].color) mbr=1;
	if (nMemories) mbr = 1;

        assert(!mbr || mayBeRed);
	if (DBUG) 
	    {
	    fprintf(outputFile,
		    "problem reduced, bringing variables back to life\n");
	    if(mbr && !mayBeRed) fprintf(outputFile,
					 "Red equations we don't expect\n");
	    printProblem();
	    };
	if (DBUG && nEQs > 0)
	    fprintf(outputFile,"This is wierd: problem has equalities\n");	

	for (i = 1; i <= safeVars; i++)
	    if (var[i] < 0) 
		{
		/* wild card */
		if (i < safeVars) 
		    {
		    int j = safeVars;
		    swap(var[i], var[j]);
		    for (e = nGEQs - 1; e >= 0; e--) 
			{
			GEQs[e].touched = 1;
			swap(GEQs[e].coef[i], GEQs[e].coef[j]);
			};
		    for (e = nEQs - 1; e >= 0; e--)
			swap(EQs[e].coef[i], EQs[e].coef[j]);
		    for (e = nSUBs - 1; e >= 0; e--)
			swap(SUBs[e].coef[i], SUBs[e].coef[j]);
		    i--;
		    };
		safeVars--;
		};

	m = nSUBs;
	n = nVars;
 	if (n < safeVars + m)
 	   n = safeVars + m;
	for (e = nGEQs - 1; e >= 0; e--) 
	    {
	    if (singleVarGEQ(&GEQs[e])) 
		{
		i = abs(GEQs[e].key);
		if (i >= safeVars + 1)
		    GEQs[e].key += (GEQs[e].key > 0 ? m : -m);
		}
	    else 
		{
		GEQs[e].touched = TRUE;
		GEQs[e].key = 0;
		}
	    };
	for (i = nVars; i >= safeVars + 1; i--) 
	    {
	    var[i + m] = var[i];
	    for (e = nGEQs - 1; e >= 0; e--)
		GEQs[e].coef[i + m] = GEQs[e].coef[i];
	    for (e = nEQs - 1; e >= 0; e--)
		EQs[e].coef[i + m] = EQs[e].coef[i];
	    for (e = nSUBs - 1; e >= 0; e--)
		SUBs[e].coef[i + m] = SUBs[e].coef[i];
	    };
	for (i = safeVars + m;
	     i >= safeVars + 1;
	     i--) 
	    {
	    for (e = nGEQs - 1; e >= 0; e--) GEQs[e].coef[i] = 0;
	    for (e = nEQs - 1; e >= 0; e--) EQs[e].coef[i] = 0;
	    for (e = nSUBs - 1; e >= 0; e--) SUBs[e].coef[i] = 0;
	    };
	nVars += m;
	safeVars += m;
	for (e = nSUBs - 1; e >= 0; e--)  
	    var[safeVars -m + 1 + e] = SUBs[e].key;
	for (i = 1; i <= safeVars; i++)
	    forwardingAddress[var[i]] = i;
	if (DEBUG) 
	    {
	    fprintf(outputFile,"Ready to wake substitutions\n");
	    printProblem();
	    };
	for (e = nSUBs - 1; e >= 0; e--)  
	    {
	    int neweq = newEQ();
	    eqncpy(&(EQs[neweq]), &(SUBs[e]));
	    EQs[neweq].coef[safeVars -m + 1 + e] = -1;
	    EQs[neweq].color = 0;
	    if (DBUG) 
		{
		fprintf(outputFile, "brought back: ");
		printEQ(&EQs[neweq]);
		fprintf(outputFile, "\n");
		};
	    };
	nSUBs = 0;

	if (DBUG) 
	    {
	    fprintf(outputFile, "variables brought back to life\n");
	    printProblem();
	    }

	}
    coalesce();
    recallRedMemories();
    cleanoutWildcards();
    }


void 
Problem::
reverseProtectedVariables() 
    {
    int v1,v2,e,i;
    coef_t t;
    for (v1 = 1; v1 <= safeVars; v1++) 
	{
	v2 = safeVars+1-v1;
	if (v2>=v1) break;
	for(e=0;e<nEQs;e++)  
	    {
	    t = EQs[e].coef[v1];
	    EQs[e].coef[v1] = EQs[e].coef[v2];
	    EQs[e].coef[v2] = t;
	    };
	for(e=0;e<nGEQs;e++)  
	    {
	    t = GEQs[e].coef[v1];
	    GEQs[e].coef[v1] = GEQs[e].coef[v2];
	    GEQs[e].coef[v2] = t;
	    GEQs[e].touched = 1;
	    };
	for(e=0;e<nSUBs;e++)  
	    {
	    t = SUBs[e].coef[v1];
	    SUBs[e].coef[v1] = SUBs[e].coef[v2];
	    SUBs[e].coef[v2] = t;
	    };
	};

    for (i = 1; i <= safeVars; i++)
	forwardingAddress[var[i]] = i;
    for (i = 0; i < nSUBs; i++)
	forwardingAddress[SUBs[i].key] = -i - 1;
    };

void Problem::ordered_elimination(int symbolic) 
    {
    int i,j,e;
    int isDead[maxmaxGEQs];
    for(e=0;e<nEQs;e++) isDead[e] = 0;

    if (!variablesInitialized) 
	{
	initializeVariables();
	}

    for(i=nVars;i>symbolic;i--) 
	{
	for(e=0;e<nEQs;e++) if (EQs[e].coef[i] == 1 || EQs[e].coef[i] == -1)  
	    {
	    for(j=nVars;j>i;j--) if (EQs[e].coef[j]) break;
	    if (i==j) 
		{
		doElimination(e, i);
		isDead[e] = 1;
		break;
		};
	    };
	};
    for(e=nEQs-1;e>=0;e--) if (isDead[e])  
	{
	nEQs--;
	if (e < nEQs) eqncpy(&EQs[e], &EQs[nEQs]);
	}

    for (i = 1; i <= safeVars; i++)
	forwardingAddress[var[i]] = i;
    for (i = 0; i < nSUBs; i++)
	forwardingAddress[SUBs[i].key] = -i - 1;

    }


coef_t Problem::checkSum()  const
    {
    coef_t cs;
    int e;
    cs = 0;
    for(e=0;e<nGEQs;e++) 
	{
	coef_t c = GEQs[e].coef[0];
	cs += c*c*c;
	}
    return cs;
    }



void
Problem::coalesce()
    {
    int e, e2, colors;
    int isDead[maxmaxGEQs];
    int foundSomething = 0;


    colors = 0;
    for (e = 0; e < nGEQs; e++)
	if (GEQs[e].color)
	    colors++;
    if (colors < 2)
	return;
    for (e = 0; e < nGEQs; e++)
	isDead[e] = 0;
    for (e = 0; e < nGEQs; e++)
	if (!GEQs[e].touched)
	    for (e2 = e + 1; e2 < nGEQs; e2++)
		if (!GEQs[e2].touched && GEQs[e].key == -GEQs[e2].key
		    && GEQs[e].coef[0] == -GEQs[e2].coef[0]) 
		    {
		    int neweq = newEQ();
		    eqncpy(&EQs[neweq], &GEQs[e]);
		    EQs[neweq].color = GEQs[e].color || GEQs[e2].color;
		    foundSomething++;
		    isDead[e] = 1;
		    isDead[e2] = 1;
		    };
    for (e = nGEQs - 1; e >= 0; e--)
	if (isDead[e]) 
	    {
	    deleteGEQ(e);
	    }
    if (DEBUG && foundSomething) 
	{
	fprintf(outputFile, "Coalesced GEQs into %d EQ's:\n", foundSomething);
	printProblem();
	};
    }


} // end of namespace omega
