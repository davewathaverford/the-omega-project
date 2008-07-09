#include <omega/omega_core/oc_i.h>


namespace omega {


int
checkIfSingleVar(eqn* e, int i)
    {
    for (; i > 0; i--)
        if (e->coef[i]) 
	    {
            i--;
            break;
	    };
    for (; i > 0; i--)
        if (e->coef[i])
            break;
    return (i == 0);
    }

int singleVarGEQ(eqn* e) 
    {
    return  !e->touched && e->key != 0 && -maxVars <= e->key && e->key <= maxVars;
    }

void checkVars(int nVars)
    {
    if (nVars > maxVars)
	{
	fprintf(stderr, "\nERROR:\n");
	fprintf(stderr, "An attempt was made to create a conjunction with %d variables.\n", nVars);
	fprintf(stderr, "The current limit on variables in a single conjunction is %d.\n", maxVars);
	fprintf(stderr, "This limit can be changed by changing the #define of maxVars in oc.h.\n\n");
	Exit(2);
	}
    } /* checkVars */


void
Problem::
difficulty(int &numberNZs, coef_t &maxMinAbsCoef, coef_t &sumMinAbsCoef) const {
    int i,e = 0;
    numberNZs=0;
    maxMinAbsCoef=0;
    sumMinAbsCoef=0;
    for (e = 0; e < nGEQs; e++) {
      coef_t maxCoef = 0;
      for(i=1;i <= nVars;i++) 
        if (GEQs[e].coef[i]!=0) {
		coef_t a = abs(GEQs[e].coef[i]);
      		maxCoef = omega::max(maxCoef,a);
		numberNZs++;
		}
      coef_t nextCoef = 0;
      for(i=1;i <= nVars;i++) 
        if (GEQs[e].coef[i]!=0) {
		coef_t a = abs(GEQs[e].coef[i]);
		if (a < maxCoef) nextCoef = omega::max(nextCoef,a);
		else if (a == maxCoef) maxCoef = 0x7fffffff;
		}
      maxMinAbsCoef = omega::max(maxMinAbsCoef,nextCoef);
      sumMinAbsCoef += nextCoef;
      }



    for (e = 0; e < nEQs; e++) {
      coef_t maxCoef = 0;
      for(i=1;i <= nVars;i++) 
        if (EQs[e].coef[i]!=0) {
		coef_t a = abs(EQs[e].coef[i]);
      		maxCoef = omega::max(maxCoef,a);
		numberNZs++;
		}
      coef_t nextCoef = 0;
      for(i=1;i <= nVars;i++) 
        if (EQs[e].coef[i]!=0) {
		coef_t a = abs(EQs[e].coef[i]);
		if (a < maxCoef) nextCoef = omega::max(nextCoef,a);
		else if (a == maxCoef) maxCoef = 0x7fffffff;
		}
      maxMinAbsCoef = omega::max(maxMinAbsCoef,nextCoef);
      sumMinAbsCoef += nextCoef;
      }
}

int
Problem::
countRedGEQs() const
    {
    int e, result = 0;
    for (e = 0; e < nGEQs; e++)
        if (GEQs[e].color == _red) result++;
    return result;
    }

int
Problem::
countRedEQs() const
    {
    int e, result = 0;
    for (e = 0; e < nEQs; e++)
        if (EQs[e].color == _red) result++;
    return result;
    }

int
Problem::
countRedEquations() const
    {
    int e, i;
    int result = 0;
    for (e = 0; e < nEQs; e++)
        if (EQs[e].color == _red) 
	    {
	    for (i = nVars; i > 0; i--) if (EQs[e].coef[i]) break;
	    if (i == 0 && EQs[e].coef[0] != 0) return 0;
	    else result+=2;
	    };
    for (e = 0; e < nGEQs; e++)
        if (GEQs[e].color == _red) result+=1;
    for (e = 0; e < nMemories; e++)
        switch(redMemory[e].kind ) 
	    {
	case redEQ:
	case redStride:
	    e++;
	case redLEQ:
	case redGEQ:
	    e++;
	case notRed:
	    ;    /* avoid warning about notRed not handled */
	    };
    return result;
    }

void
Problem::
deleteBlack()
    {
    int e;
    int RedVar[maxVars];
    int i;
    for(i=safeVars+1;i <= nVars;i++) RedVar[i] = 0;

    assert(nSUBs == 0);

    for (e = nEQs-1; e >=0; e--)
        if (EQs[e].color != _red) 
	    {
	    eqncpy(&EQs[e],&EQs[nEQs-1]);
	    nEQs--;
	    }
        else for(i=safeVars+1;i <= nVars;i++)
	    if (EQs[e].coef[i]) RedVar[i] = 1;

    for (e = nGEQs-1; e >=0; e--)
        if (GEQs[e].color != _red) 
	    {
	    eqncpy(&GEQs[e],&GEQs[nGEQs-1]);
	    nGEQs--;
	    }
        else for(i=safeVars+1;i <= nVars;i++)
	    if (GEQs[e].coef[i]) RedVar[i] = 1;

    assert(nSUBs == 0);

    for(i=nVars; i> safeVars;i--) 
	{
	if (!RedVar[i]) deleteVariable(i);
	}
    };

void
Problem::
deleteRed()
    {
    int e;
    int BlackVar[maxVars];
    int i;
    for(i=safeVars+1;i <= nVars;i++) BlackVar[i] = 0;

    assert(nSUBs == 0);
    for (e = nEQs-1; e >=0; e--)
        if (EQs[e].color) 
	    {
	    eqncpy(&EQs[e],&EQs[nEQs-1]);
	    nEQs--;
	    }
        else for(i=safeVars+1;i <= nVars;i++)
	    if (EQs[e].coef[i]) BlackVar[i] = 1;

    for (e = nGEQs-1; e >=0; e--)
        if (GEQs[e].color) 
	    {
	    eqncpy(&GEQs[e],&GEQs[nGEQs-1]);
	    nGEQs--;
	    }
        else for(i=safeVars+1;i <= nVars;i++)
	    if (GEQs[e].coef[i]) BlackVar[i] = 1;

    assert(nSUBs == 0);

    for(i=nVars; i> safeVars;i--) 
	{
	if (!BlackVar[i]) deleteVariable(i);
	}
    };


void
Problem::
turnRedBlack()
    {
    int e;

    for (e = nEQs-1; e >=0; e--) EQs[e].color = 0;
    for (e = nGEQs-1; e >=0; e--) GEQs[e].color = 0;
    };


void
Problem::
useWildNames()
    {
    int i;
    for(i=safeVars+1; i<= nVars; i++) nameWildcard(i);
    }


void
negateCoefficients(eqn* eqn, int nVars)
    {
    int i;
    for (i = nVars; i >= 0; i--)
        eqn-> coef[i] = -eqn->coef[i];
    eqn->touched = TRUE;
    }


void
Problem::
negateGEQ(int e)
    {
    negateCoefficients(&GEQs[e],nVars);
    GEQs[e].coef[0]--;
    }


void
Problem:: deleteVariable(int i)
    {
    int e;
    if (i < safeVars) 
	{
	int j = safeVars;
	for (e = nGEQs - 1; e >= 0; e--) 
	    {
	    GEQs[e].touched = TRUE;
	    GEQs[e].coef[i] = GEQs[e].coef[j];
	    GEQs[e].coef[j] = GEQs[e].coef[nVars];
	    };
	for (e = nEQs - 1; e >= 0; e--) 
	    {
	    EQs[e].coef[i] = EQs[e].coef[j];
	    EQs[e].coef[j] = EQs[e].coef[nVars];
	    };
	for (e = nSUBs - 1; e >= 0; e--) 
	    {
	    SUBs[e].coef[i] = SUBs[e].coef[j];
	    SUBs[e].coef[j] = SUBs[e].coef[nVars];
	    };
	var[i] = var[j];
	var[j] = var[nVars];
	}
    else if (i < nVars) 
	{
	for (e = nGEQs - 1; e >= 0; e--)
	    if (GEQs[e].coef[nVars]) 
		{
		GEQs[e].coef[i] = GEQs[e].coef[nVars];
		GEQs[e].touched = TRUE;
		};
	for (e = nEQs - 1; e >= 0; e--)
	    EQs[e].coef[i] = EQs[e].coef[nVars];
	for (e = nSUBs - 1; e >= 0; e--)
	    SUBs[e].coef[i] = SUBs[e].coef[nVars];
	var[i] = var[nVars];
	};
    if (i <= safeVars)
	safeVars--;
    nVars--;
    }

void 
Problem::
setInternals() 
    {
    int i;

    if (!variablesInitialized) 
	{
        initializeVariables();
	};

    var[0] = 0;
    nextWildcard = 0;
    for(i=1;i<=nVars;i++) if (var[i] < 0) 
	var[i] = --nextWildcard;
    assert(nextWildcard >= -maxWildcards);

    CHECK_FOR_DUPLICATE_VARIABLE_NAMES;

    int v = nSUBs;
    for(i=1;i<=safeVars;i++) if (var[i] > 0) v++;
    varsOfInterest = v;

    if (nextKey * 3 > maxKeys) 
	{
        int e;
	    hashVersion++;
        nextKey = maxVars + 1;
        for (e = nGEQs - 1; e >= 0; e--)
            GEQs[e].touched = TRUE;
        for (i = 0; i < hashTableSize; i++)
            hashMaster[i].touched = -1;
        mHashVersion = hashVersion;
	}
    else if (mHashVersion != hashVersion) 
	{
        int e;
        for (e = nGEQs - 1; e >= 0; e--)
            GEQs[e].touched = TRUE;
        mHashVersion = hashVersion;
	};
    }

void Problem::setExternals() 
    {
    int i;
    for (i = 1; i <= safeVars; i++)
	forwardingAddress[var[i]] = i;
    for (i = 0; i < nSUBs; i++)
	forwardingAddress[SUBs[i].key] = -i - 1;
    }

void
setOutputFile(FILE * file)
    {
    /* sets the file to which printProblem should send its output to "file" */

    outputFile = file;

    }                               /* end setOutputFile(FILE *file) */


void
setPrintLevel(int level) 
    {
    /* Sets the nber of points printed before constraints in printProblem */
    headerLevel = level;
    }


void Problem::putVariablesInStandardOrder() 
    {
    int i,j,b;

    for(i=1;i<=safeVars;i++)  
	{
	b = i;
	for(j=i+1;j<=safeVars;j++) 
	    {
	    if (var[b] < var[j]) b = j;
	    };
	if (b != i) swapVars(i,b);
	};
    };
	

void
Problem::
nameWildcard(int i)
    {
    int j;
    do 
	{
        --nextWildcard;
        if (nextWildcard < -maxWildcards)
            nextWildcard = -1;
        var[i] = nextWildcard;
	for(j=nVars; j > 0;j--) if (i!=j && var[j] == nextWildcard) break;
	}
    while (j != 0);
	
    }

int Problem::
protectWildcard(int i) 
    {
    assert (i > safeVars);
    if (i != safeVars+1) swapVars(i,safeVars+1);
    safeVars++;
    nameWildcard(safeVars);
    return safeVars;
    };

int
Problem::
addNewProtectedWildcard()
    {
    int e;
    int i = ++safeVars;
    nVars++;
    if (nVars != i) 
	{
        for (e = nGEQs - 1; e >= 0; e--) 
	    {
            if (GEQs[e].coef[i] != 0)
                GEQs[e].touched = TRUE;
            GEQs[e].coef[nVars] = GEQs[e].coef[i];

	    };
        for (e = nEQs - 1; e >= 0; e--) 
	    {
            EQs[e].coef[nVars] = EQs[e].coef[i];
	    };
        for (e = nSUBs - 1; e >= 0; e--) 
	    {
            SUBs[e].coef[nVars] = SUBs[e].coef[i];
	    };
        var[nVars] = var[i];
	};
    for (e = nGEQs - 1; e >= 0; e--)
        GEQs[e].coef[i] = 0;
    for (e = nEQs - 1; e >= 0; e--)
        EQs[e].coef[i] = 0;
    for (e = nSUBs - 1; e >= 0; e--)
        SUBs[e].coef[i] = 0;
    nameWildcard(i);
    return (i);
    }

int
Problem::
addNewUnprotectedWildcard()
    {
    int e;
    int i = ++nVars;
    for (e = nGEQs - 1; e >= 0; e--) GEQs[e].coef[i] = 0;
    for (e = nEQs - 1; e >= 0; e--) EQs[e].coef[i] = 0;
    for (e = nSUBs - 1; e >= 0; e--) SUBs[e].coef[i] = 0;
    nameWildcard(i);
    return i;
    }

void
Problem::
cleanoutWildcards()
    {
    int e, e2, i, j;
    coef_t a, c, barb;
    coef_t g;
    int renormalize = 0;

    if (DEBUG) fprintf(outputFile,"Trying to cleanout wildcards\n");
    eqn *sub;
    for (e = nEQs - 1; e >= 0; e--) 
	{
	for (i = nVars; i >= safeVars + 1; i--)
	    if (EQs[e].coef[i]) 
		{
		for (j = i - 1; j >= safeVars + 1; j--)
		    if (EQs[e].coef[j])
			break;
		if (j >= safeVars + 1) 
		    {
		    /* a multi wild card equality*/
		    if (inApproximateMode) 
			{
			if (e < nEQs-1) eqnncpy (&EQs[e], &EQs[nEQs - 1],(nVars));
			nEQs--;
			};
		    continue;
		    };
		/* Found a single wild card equality */
		if (DBUG) 
		    {
		    fprintf(outputFile, "Found a single wild card equality: ");
		    printEQ(&EQs[e]);
		    fprintf(outputFile, "\n");
		    printProblem();
		    }
		c = EQs[e].coef[i];
		a = abs(c);
		sub = &(EQs[e]);
		bool preserveThisConstraint = true;

		for (e2 = nEQs - 1; e2 >= 0; e2--)
		    if (e != e2 && EQs[e2].coef[i]
			&& EQs[e2].color >= EQs[e].color ) 
			{
			eqn* eqn = &(EQs[e2]);
			int j;
			coef_t k;
                        barb = abs(eqn->coef[i]);
			preserveThisConstraint  =
			  preserveThisConstraint && gcd(a,barb) != 1;
			for (j = nVars; j >= 0; j--)
			    eqn->coef[j] *= a;
			k = eqn->coef[i];
			for (j = nVars; j >= 0; j--)
			    eqn->coef[j] -= sub->coef[j] * k / c;
			eqn->coef[i] = 0;
			g = 0;
			for (j = nVars; j >= 0; j--) {
                            barb = abs(eqn->coef[j]);
			    g = gcd(barb, g);
                        }
			if (g)
			    for (j = nVars; j >= 0; j--)
				eqn->coef[j] = eqn->coef[j] / g;

			};

		for (e2 = nGEQs - 1; e2 >= 0; e2--)
		    if (GEQs[e2].coef[i]
			&& GEQs[e2].color >= EQs[e].color) 
			{
			eqn* eqn = &(GEQs[e2]);
			int j;
			coef_t k;
			for (j = nVars; j >= 0; j--)
			    eqn->coef[j] *= a;
			k = eqn->coef[i];
			for (j = nVars; j >= 0; j--)
			    eqn->coef[j] -= sub->coef[j] * k / c;
			eqn->coef[i] = 0;
			eqn->touched = 1;
			renormalize = 1;
			};
		for (e2 = nSUBs - 1; e2 >= 0; e2--)
		    if (SUBs[e2].coef[i]
			&& SUBs[e2].color >= EQs[e].color ) 
			{
			eqn* eqn = &(SUBs[e2]);
			int j;
			coef_t k;
			for (j = nVars; j >= 0; j--)
			    eqn->coef[j] *= a;
			k = eqn->coef[i];
			for (j = nVars; j >= 0; j--)
			    eqn->coef[j] -= sub->coef[j] * k / c;
			eqn->coef[i] = 0;
			g = 0;
			for (j = nVars; j >= 0; j--) {
                            barb = abs(eqn->coef[j]);
			    g = gcd(barb, g);
                        }
			if (g)
			    for (j = nVars; j >= 0; j--)
				eqn->coef[j] = eqn->coef[j] / g;
			};

		if (!preserveThisConstraint) 
			{
			if (e < nEQs-1) eqnncpy (&EQs[e], &EQs[nEQs - 1],(nVars));
			nEQs--;
			deleteVariable(i);
			};
		if (DBUG) 
		    {
		    fprintf(outputFile, "cleaned-out wildcard: ");
		    printProblem();
		    }
		break;
		}
	}
    if (renormalize)
	normalize();
    }




void 
Problem:: check() const 
    {
#ifndef NDEBUG
    int v = nSUBs;
    int i;
    int e;
    checkVars(nVars+1);
    for(i=1;i<=safeVars;i++) if (var[i] > 0) v++;
    assert(v == varsOfInterest);
    for(e=0;e<nGEQs;e++) assert(GEQs[e].touched || GEQs[e].key != 0);
    if(!mayBeRed) 
	{
	for(e=0;e<nEQs;e++) assert(!EQs[e].color);
	for(e=0;e<nGEQs;e++) assert(!GEQs[e].color);
	}
    else 
	for(i=safeVars+1; i <= nVars;i++) 
	    {
	    int isBlack = 0;
	    int isRed = 0;
	    for(e=0;e<nEQs;e++) if (EQs[e].coef[i]) 
		{
		if (EQs[e].color) isRed = 1;
		else isBlack = 1;
		}
	    for(e=0;e<nGEQs;e++) if (GEQs[e].coef[i]) 
		{
		if (GEQs[e].color) isRed = 1;
		else isBlack = 1;
		}
	    if (isBlack && isRed && 0) 
		{
		fprintf(outputFile,"Mixed Red and Black variable:\n");
		printProblem();
		};
	
	    };
#endif 
    }

void 
Problem::
rememberRedConstraint(eqn* e, redType type, coef_t stride) 
    {

    int i,j,k;
    coef_t l;

    if (type == redEQ && newVar == nVars && e->coef[newVar]) 
	{
	/* this is really a stride constraint */
	type = redStride;
	stride = e->coef[newVar];
	}
    else for(i=safeVars+1; i<=nVars;i++) assert(!e->coef[i]);

    assert(type != notRed);
    assert(type == redStride || stride == 0);

    if (TRACE) 
	{
	fprintf(outputFile,"being asked to remember red constraint:\n");
	switch(type) 
	    {
	case notRed: fprintf(outputFile,"notRed: ");
	    break;
	case redGEQ: fprintf(outputFile,"Red: 0 <= ");
	    break;
	case redLEQ: fprintf(outputFile,"Red: 0 >= ");
	    break;
	case redEQ: fprintf(outputFile,"Red: 0 == ");
	    break;
	case redStride: fprintf(outputFile,"Red stride " coef_fmt ": ",stride);
	    break;
	    };
	printTerm(e,1);
	fprintf(outputFile,"\n");
	printProblem();
	fprintf(outputFile,"----\n");
	};

    eqn mem;
    eqncpy(&mem,e);
    e = &mem;
    int m = nMemories++;
    redMemory[m].length = 0;
    if (type == redLEQ) 
	{
        for(i=0; i<=safeVars;i++) e->coef[i] = -e->coef[i];
        type = redGEQ;
        };
    int backSub[maxVars+2];
    int anyWild = 0;
    for(i=1; i<=safeVars;i++) if (var[i] < 0) anyWild = 1;
 
    if (anyWild) 
	{
	for(i=1; i<=nVars;i++) backSub[i] = -1;
	for(i=0; i<nSUBs;i++) 
	    {
	    k = -1;
	    for(j=1; j<=safeVars;j++) if (var[j] < 0 && SUBs[i].coef[j]) 
		{
		if (k == -1) k = j;
		else 
		    {
		    k = -1; break;
		    }
		;
		};
	    if (k>=0) 
		{
		backSub[k] = i;
		if (DEBUG) 
		    {
		    fprintf(outputFile,"Wildcard %s handled via sub:\n",variable(k));
		    printSubstitution(i);
		    fprintf(outputFile,"\n");
		    };
		};
	    };
	l = 1;
        for(i=1; i<=safeVars;i++) if (var[i] < 0 && e->coef[i]) 
	    {
	    coef_t k;
	    if (backSub[i] < 0) 
		{
		fprintf(outputFile,"sophisticated back substitutions not handled\n");
		printProblem();
		assert(0);
		exit(1);
		};
	    j = backSub[i];
	    k = abs(SUBs[j].coef[i]);
            
            coef_t barb = abs(e->coef[i]);
	    k = k / gcd(k,barb);
	    l = lcm(l,k);
	    };
	for(i=0; i<=safeVars;i++) e->coef[i] *= l;
	stride *= l;
	if (DEBUG) 
	    {
	    fprintf(outputFile,"Expression scaled by " coef_fmt ": ", l);
	    printTerm(e,1);
	    fprintf(outputFile,"\n");
	    };
	}

    coef_t coef[maxVars+2];

    for(i=1; i<=maxVars;i++) coef[i] = 0;
    redMemory[m].kind = type;

    coef[0] = e->coef[0];
    for(i=1; i<=safeVars;i++)  if(e->coef[i]) 
	{
	if (DEBUG) fprintf(outputFile,"Handling %s:\n", variable(i));
	if (var[i] > 0) coef[var[i]] += e->coef[i];
	else 
	    {
	    coef_t m = e->coef[i];
	    coef_t s = SUBs[backSub[i]].coef[i];
	    if (DEBUG) fprintf(outputFile,"m = " coef_fmt ", s = " coef_fmt "\n",m,s);
	    assert(m%s == 0);
	    m = m/s;

	    for(j=1; j<=safeVars;j++) if (var[j] > 0 && SUBs[backSub[i]].coef[j]) 
		{
		coef[var[j]] -= m * SUBs[backSub[i]].coef[j];
		if (DEBUG) fprintf(outputFile,
				   "Subtracting " coef_fmt " * " coef_fmt "  * %s\n",
				   m, SUBs[backSub[i]].coef[j],
				   variable(j));
		};
	    if (SUBs[backSub[i]].coef[0]) 
		{
		coef[0] -= m * SUBs[backSub[i]].coef[0];
		if (DEBUG) fprintf(outputFile,
				   "Subtracting " coef_fmt " * " coef_fmt "\n",
				   m, SUBs[backSub[i]].coef[0]);
		};
		
	    coef[SUBs[backSub[i]].key] += m;
	    if (DEBUG) fprintf(outputFile,
			       "Adding " coef_fmt " * %s\n",
			       m, orgVariable(SUBs[backSub[i]].key));
		
	    };
	if (DEBUG) 
	    {
	    int j,k;
	    fprintf(outputFile,"Coef = " coef_fmt ,coef[0]);
	    for(k=maxVars; k > 0; k--) if (coef[k]) break;
	    for(j=1; j<=k;j++)  fprintf(outputFile," " coef_fmt,coef[j]);
	    fprintf(outputFile,"\n");
            if (type == redStride) fprintf(outputFile,"Stride = " coef_fmt "\n",stride);
	    };
	}

    redMemory[m].constantTerm = coef[0];
	
    for(i=1; i<=maxVars;i++)  if (coef[i]) 
	{
        int j = redMemory[m].length++;
        redMemory[m].coef[j] = coef[i];
        redMemory[m].var[j] = i;
        };
    if (type == redStride) redMemory[m].stride = stride;
    if (DBUG) 
	{
	fprintf(outputFile,"Red constraint remembered\n");
	printProblem();
	};
    };

void 
Problem::
recallRedMemories() 
    {
    int i;
    if (nMemories) 
	{
	if (TRACE) 
	    {
	    fprintf(outputFile,"Recalling red memories\n");
	    printProblem();
	    };

	eqn* e = 0;

	for(int m = 0; m < nMemories; m++) 
	    {
	    int temporary_eqn;  
	    switch(redMemory[m].kind) 
		{
		int i;
            case notRed:
            case redLEQ:
		    assert(0);
                exit(1);
                break;
            case redGEQ:
	        temporary_eqn = newGEQ();
		e = &GEQs[temporary_eqn];
		eqnnzero(e, nVars);
		e->touched = 1;
                break;
            case redEQ:
	        temporary_eqn = newEQ();
		e = &EQs[temporary_eqn];
		eqnnzero(e, nVars);
                break;
            case redStride:
	        temporary_eqn = newEQ();
		e = &EQs[temporary_eqn];
		eqnnzero(e, nVars);
                i = addNewUnprotectedWildcard();
                e->coef[i] = -redMemory[m].stride;
                break;
		}
	    e->color = _red;
	    e->coef[0] = redMemory[m].constantTerm;
	    for(i = 0; i < redMemory[m].length; i++)  
		{
		int v = redMemory[m].var[i];
		assert(var[forwardingAddress[v]] == v);
		e->coef[forwardingAddress[v]] =
		    redMemory[m].coef[i];
		}
	    };
        
	nMemories = 0;
	if (TRACE) 
	    {
	    fprintf(outputFile,"Red memories recalled\n");
	    printProblem();
	    };
	}
    }

void Problem::swapVars(int i, int j) 
    {
    coef_t t;
    int e;
    if (DEBUG) 
	{
	use_ugly_names++;
        fprintf(outputFile, "Swapping %d and %d\n", i, j);
        printProblem();
	use_ugly_names--;
	};
    swap(var[i], var[j]);
    for (e = nGEQs - 1; e >= 0; e--)
        if (GEQs[e].coef[i] != GEQs[e].coef[j]) 
	    {
            GEQs[e].touched = TRUE;
            t = GEQs[e].coef[i];
            GEQs[e].coef[i] = GEQs[e].coef[j];
            GEQs[e].coef[j] = t;
	    };
    for (e = nEQs - 1; e >= 0; e--)
        if (EQs[e].coef[i] != EQs[e].coef[j]) 
	    {
            t = EQs[e].coef[i];
            EQs[e].coef[i] = EQs[e].coef[j];
            EQs[e].coef[j] = t;
	    };
    for (e = nSUBs - 1; e >= 0; e--)
        if (SUBs[e].coef[i] != SUBs[e].coef[j]) 
	    {
            t = SUBs[e].coef[i];
            SUBs[e].coef[i] = SUBs[e].coef[j];
            SUBs[e].coef[j] = t;
	    };
    if (DEBUG) 
	{
	use_ugly_names++;
        fprintf(outputFile, "Swapping complete \n");
        printProblem();
        fprintf(outputFile, "\n");
	use_ugly_names--;
	};
    };

void Problem::addingEqualityConstraint(int e)
    {
    int e2, i, j;

    if (addingOuterEqualities 
	&& originalProblem != noProblem 
	&& originalProblem != this 
	&& !conservative) 
	{
	e2 = originalProblem->newEQ();
	if (TRACE)
	    fprintf(outputFile,
		    "adding equality constraint %d to outer problem\n",
		    e2);
	eqnnzero(&originalProblem->EQs[e2], originalProblem->nVars);
	for (i = nVars; i >= 1; i--) 
	    {
	    for (j = originalProblem->nVars; j >= 1; j--)
		if (originalProblem->var[j] == var[i])
		    break;
	    if (j <= 0 || (outerColor && j > originalProblem->safeVars))
		{
		if (DBUG)
		    fprintf(outputFile, "retracting\n");
		originalProblem->nEQs--;
		return;
		};
	    originalProblem->EQs[e2].coef[j] = EQs[e].coef[i];
	    };
	originalProblem->EQs[e2].coef[0] = EQs[e].coef[0];
	
	originalProblem->EQs[e2].color = outerColor;
	if (DBUG)
	    originalProblem->printProblem();
	};
    }


int fastLookup[maxKeys * 2];

normalizeReturnType
Problem::
normalize()

    {
    int e, i, j, k;
    int coupledSubscripts = 0;

    check();

    for (e = 0; e < nGEQs; e++) 
	{
	if (!GEQs[e].touched) 
	    {
	    if (!singleVarGEQ(&GEQs[e]))
		coupledSubscripts = 1;
	    }
	else 
	    {
	    coef_t g;
	    int topVar;
	    int i0;
	    coef_t hashCode;

	    if (DEBUG) 
		{
		fprintf(outputFile,"Normalizing: ");
		printGEQ(&GEQs[e]);
		fprintf(outputFile,"\n");
		};

	    {
	    int *p = &packing[0];
	    for (k = 1; k <= nVars; k++)
		if (GEQs[e].coef[k]) 
		    {
		    *(p++) = k;
		    };
	    topVar = (p - &packing[0]) - 1;
	    };

	    if (topVar == -1) 
		{
		if (GEQs[e].coef[0] < 0) 
		    {
		    if (DBUG)
			printGEQ(&GEQs[e]);
		    if (DBUG)
			fprintf(outputFile, "\nequations have no solution (D)\n");
		    return (normalize_false);
		    };
		deleteGEQ(e);
		e--;
		continue;
		}
	    else if (topVar == 0) 
		{
		int singleVar = packing[0];
		g = GEQs[e].coef[singleVar];
		if (g > 0) 
		    {
		    GEQs[e].coef[singleVar] = 1;
		    GEQs[e].key = singleVar;
		    }
		else 
		    {
		    g = -g;
		    GEQs[e].coef[singleVar] = -1;
		    GEQs[e].key = -singleVar;
		    };
		if (g > 1)
		    GEQs[e].coef[0] = int_div(GEQs[e].coef[0], g);
		}
	    else 
		{

		coupledSubscripts = 1;
		i0 = topVar;
		i = packing[i0--];
		g = GEQs[e].coef[i];
		hashCode = g * (i + 3);
		if (g < 0)
		    g = -g;
		for (; i0 >= 0; i0--) 
		    {
		    coef_t x;
		    i = packing[i0];
		    x = GEQs[e].coef[i];
		    hashCode = hashCode * keyMult * (i + 3) + x;
		    if (x < 0)
			x = -x;
		    if (x == 1) 
			{
			g = 1;
			i0--;
			break;
			}
		    else
			g = gcd(x, g);
		    };
		for (; i0 >= 0; i0--) 
		    {
		    coef_t x;
		    i = packing[i0];
		    x = GEQs[e].coef[i];
		    hashCode = hashCode * keyMult * (i + 3) + x;
		    };
		if (g > 1) 
		    {
		    GEQs[e].coef[0] = int_div(GEQs[e].coef[0], g);
		    i0 = topVar;
		    i = packing[i0--];
		    GEQs[e].coef[i] = GEQs[e].coef[i] / g;
		    hashCode = GEQs[e].coef[i] * (i + 3);
		    for (; i0 >= 0; i0--) 
			{
			i = packing[i0];
			GEQs[e].coef[i] = GEQs[e].coef[i] / g;
			hashCode = hashCode * keyMult * (i + 3) + GEQs[e].coef[i];
			};
		    };

		{
		coef_t g2 = abs(hashCode);
		if (DEBUG) 
		    {
		    fprintf(outputFile, "Hash code = " coef_fmt ", eqn = ", hashCode);
		    printGEQ(&GEQs[e]);
		    fprintf(outputFile, "\n");
		    };
		j = (int) (g2 % (coef_t) hashTableSize);
		assert (g2 % (coef_t) hashTableSize == j);
		while (1) 
		    {
		    eqn *proto = &(hashMaster[j]);
		    if (proto->touched == g2) 
			{
			if (proto->coef[0] == topVar) 
			    {
			    if (hashCode >= 0)
				for (i0 = topVar; i0 >= 0; i0--) 
				    {
				    i = packing[i0];
				    if (GEQs[e].coef[i] != proto->coef[i])
					break;
				    }
			    else
				for (i0 = topVar; i0 >= 0; i0--) 
				    {
				    i = packing[i0];
				    if (GEQs[e].coef[i] != -proto->coef[i])
					break;
				    };
			    if (i0 < 0) 
				{
				if (hashCode >= 0)
				    GEQs[e].key = proto->key;
				else
				    GEQs[e].key = -proto->key;
				break;
				};
			    };
			}
		    else if (proto->touched < 0) 
			{
			eqnzero(proto);
			if (hashCode >= 0)
			    for (i0 = topVar; i0 >= 0; i0--) 
				{
				i = packing[i0];
				proto->coef[i] = GEQs[e].coef[i];
				}
			else
			    for (i0 = topVar; i0 >= 0; i0--) 
				{
				i = packing[i0];
				proto->coef[i] = -GEQs[e].coef[i];
				}
			proto->coef[0] = topVar;
			proto->touched = g2;
			if (DEBUG)
			    fprintf(outputFile, " constraint key = %d\n", nextKey);
			proto->key = nextKey++;
			if (proto->key > maxKeys) 
			    {
			    fprintf(outputFile, "too many hash keys generated \n");
			    fflush(outputFile);
			    assert(0 && "too many hash keys generated");
			    Exit(2);
			    };
			if (hashCode >= 0)
			    GEQs[e].key = proto->key;
			else
			    GEQs[e].key = -proto->key;
			break;
			};
		    j = (j + 1) % hashTableSize;
		    };
		};
		};

	    };

	GEQs[e].touched = FALSE;

	{
	int eKey = GEQs[e].key;
	int e2;
	if (e > 0) 
	    {
	    e2 = fastLookup[maxKeys - eKey];
	    if (e2 < e && GEQs[e2].key == -eKey) 
		{
		if (GEQs[e2].coef[0] < -GEQs[e].coef[0]) 
		    {
		    if (DBUG) 
			{
			printGEQ(&GEQs[e]);
			fprintf(outputFile, "\n");
			printGEQ(&GEQs[e2]);
			fprintf(outputFile, "\nequations have no solution (E)\n");
			};
		    return (normalize_false);
		    };
		if (GEQs[e2].coef[0] == -GEQs[e].coef[0]) 
		    {
		    int neweq = newEQ();
		    eqncpy(&EQs[neweq], &GEQs[e]);
		    EQs[neweq].color = GEQs[e].color || GEQs[e2].color;
		    addingEqualityConstraint(neweq);
		    };
		};

	    e2 = fastLookup[maxKeys + eKey];
	    if (e2 < e && GEQs[e2].key == eKey) 
		{
		if (GEQs[e2].coef[0] > GEQs[e].coef[0] ||
		    GEQs[e2].coef[0] == GEQs[e].coef[0] && GEQs[e2].color) 
		    {
		    /* e2 is redundant */
		    if (DEBUG) 
			{
			fprintf(outputFile, "Removing Redudant Equation: ");
			printGEQ(&(GEQs[e2]));
			fprintf(outputFile, "\n");
			fprintf(outputFile, "[a]      Made Redundant by: ");
			printGEQ(&(GEQs[e]));
			fprintf(outputFile, "\n");
			}
		    GEQs[e2].coef[0] = GEQs[e].coef[0];
		    GEQs[e2].color =  GEQs[e].color;
		    deleteGEQ(e);
		    e--;
		    continue;
		    }
		else 
		    {
		    /* e is redundant */
		    if (DEBUG) 
			{
			fprintf(outputFile, "Removing Redudant Equation: ");
			printGEQ(&(GEQs[e]));
			fprintf(outputFile, "\n");
			fprintf(outputFile, "[b]      Made Redundant by: ");
			printGEQ(&(GEQs[e2]));
			fprintf(outputFile, "\n");
			}
		    deleteGEQ(e);
		    e--;
		    continue;
		    }
		}
	    };
	fastLookup[maxKeys + eKey] = e;
	};
	};
    return coupledSubscripts ? normalize_coupled : normalize_uncoupled;
    }

void
initializeOmega(void)
    {
    int i;
    if (omegaInitialized)
        return;

    assert(sizeof(eqn)==sizeof(int)*(headerWords)+sizeof(coef_t)*(1+maxVars));
    nextWildcard = 0;
    nextKey = maxVars + 1;
    for (i = 0; i < hashTableSize; i++)
        hashMaster[i].touched = -1;

    sprintf(wildName[1], "__alpha");
    sprintf(wildName[2], "__beta");
    sprintf(wildName[3], "__gamma");
    sprintf(wildName[4], "__delta");
    sprintf(wildName[5], "__tau");
    sprintf(wildName[6], "__sigma");
    sprintf(wildName[7], "__chi");
    sprintf(wildName[8], "__omega");
    sprintf(wildName[9], "__pi");
    sprintf(wildName[10], "__ni");
    sprintf(wildName[11], "__Alpha");
    sprintf(wildName[12], "__Beta");
    sprintf(wildName[13], "__Gamma");
    sprintf(wildName[14], "__Delta");
    sprintf(wildName[15], "__Tau");
    sprintf(wildName[16], "__Sigma");
    sprintf(wildName[17], "__Chi");
    sprintf(wildName[18], "__Omega");
    sprintf(wildName[19], "__Pi");

    omegaInitialized = 1;
    }

//
// This is experimental (I would say, clinical) fact:
// If the code below is removed then simplifyProblem cycles.
//
class brainDammage {
public:
    brainDammage();
};
 
brainDammage::brainDammage()
    {
    initializeOmega();
    }
 
static brainDammage Podgorny;

} // end of namespace omega
