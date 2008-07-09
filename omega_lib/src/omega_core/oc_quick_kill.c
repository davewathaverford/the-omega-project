

#include <omega/omega_core/oc_i.h>

namespace omega {

int Problem::combineToTighten() 
    {
    int e,e2,i;
    coef_t diff;

    int effort = min(12+5*(nVars-safeVars),23);

    if (DBUG) 
	{
	fprintf(outputFile, "\nin combineToTighten (%d,%d):\n",effort,nGEQs);
	printProblem();
	fprintf(outputFile, "\n");
	};
    if (nGEQs >effort)  {
        if (TRACE) 
	    {
	    fprintf(outputFile, "too complicated to tighten\n");
	    }
	return 1;
	}
    for(e=0; e < nGEQs; e++) 
	{
	for(e2=0; e2 < e; e2++) 
	    {
	    coef_t g = 0;
	    for(i=nVars; g!= 1 && i>=1;i--) 
		{
		diff = GEQs[e].coef[i] + GEQs[e2].coef[i];
		if (diff < 0) diff = -diff;
		g = gcd(g,diff);
		};
	    diff = GEQs[e].coef[0] + GEQs[e2].coef[0];
	    if (g > 1 && diff % g != 0) 
		{
		int e3 = newGEQ();
		for(i=nVars; i>=1;i--) 
		    {
		    GEQs[e3].coef[i] = (GEQs[e].coef[i] + GEQs[e2].coef[i])/g;
		    }
		GEQs[e3].coef[0] = int_div(GEQs[e].coef[0] + GEQs[e2].coef[0],g);
		GEQs[e3].color = GEQs[e].color || GEQs[e2].color;
		GEQs[e3].touched = 1;
		if (DBUG) 
		    {
		    fprintf(outputFile,  "Combined     ");
		    printGEQ(&GEQs[e]);
		    fprintf(outputFile,"\n         and ");
		    printGEQ(&GEQs[e2]);
		    fprintf(outputFile,"\n to get #%2d: ",e3);
		    printGEQ(&GEQs[e3]);
		    fprintf(outputFile,"\n\n");
		    };

		if (nGEQs > effort+5 || nGEQs > maxmaxGEQs-10) goto doneCombining;
		}
	    }
	}
      doneCombining:
    if (normalize() == normalize_false) return 0;
    while (nEQs) 
	{
	if (!solveEQ()) return 0;
	if (normalize() == normalize_false) return 0;
	};
    return 1;
    };


void Problem::noteEssential(int onlyWildcards) 
    {
    int e,e2,i;
    for (e = nGEQs - 1; e >= 0; e--) 
	{
	GEQs[e].essential = 0;
	GEQs[e].varCount = 0;
	};
    if (onlyWildcards) {
        for (e = nGEQs - 1; e >= 0; e--)  {
	  GEQs[e].essential = 1;
          for (i = nVars; i > safeVars; i--) 
			if (GEQs[e].coef[i] < -1 || GEQs[e].coef[i] > 1) {
		 GEQs[e].essential = 0;
		 break;
		 }
	  }
	}
    for (i = nVars; i >= 1; i--) 
	{
	int onlyLB = -1;
	int onlyUB = -1;
	for (e = nGEQs - 1; e >= 0; e--) 
	    if (GEQs[e].coef[i] > 0) 
		{
		GEQs[e].varCount ++;
		if (onlyLB == -1) onlyLB = e;
		else onlyLB = -2;
		}
	    else if (GEQs[e].coef[i] < 0) 
		{
		GEQs[e].varCount ++;
		if (onlyUB == -1) onlyUB = e;
		else onlyUB = -2;
		}
	if (onlyUB >= 0) 
	    {
	    if (DBUG) 
		{
		fprintf(outputFile,"only UB: ");
		printGEQ(&GEQs[onlyUB]);
		fprintf(outputFile,"\n");
		};
	    GEQs[onlyUB].essential = 1;
	    };
	if (onlyLB >= 0) 
	    {
	    if (DBUG) 
		{
		fprintf(outputFile,"only LB: ");
		printGEQ(&GEQs[onlyLB]);
		fprintf(outputFile,"\n");
		};
	    GEQs[onlyLB].essential = 1;
	    };
	};
    for (e = nGEQs - 1; e >= 0; e--) 
	if (!GEQs[e].essential && GEQs[e].varCount > 1) 
	    {
	    int i1,i2,i3;
	    for (i1 = nVars; i1 >= 1; i1--)  if (GEQs[e].coef[i1]) break;
	    for (i2 = i1-1; i2 >= 1; i2--)  if (GEQs[e].coef[i2]) break;
	    for (i3 = i2-1; i3 >= 1; i3--)  if (GEQs[e].coef[i3]) break;
	    assert(i2 >= 1);
	    for (e2 = nGEQs - 1; e2 >= 0; e2--) if (e!=e2) 
		{
		coef_t crossProduct;
		crossProduct = GEQs[e].coef[i1]*GEQs[e2].coef[i1];
		crossProduct += GEQs[e].coef[i2]*GEQs[e2].coef[i2];
		for (i = i3; i >= 1; i--)  if (GEQs[e2].coef[i])
		    crossProduct += GEQs[e].coef[i]*GEQs[e2].coef[i];
		if (crossProduct > 0) 
		    {
		    if (DBUG) fprintf(outputFile,"Cross product of %d and %d is " coef_fmt "\n",
				      e,e2,crossProduct);
		    break;
		    };
		};
	    if (e2 < 0) GEQs[e].essential = 1;
	    };
    if (DBUG) 
	{
	fprintf(outputFile,"Computed essential equations\n");
	fprintf(outputFile,"essential equations:\n");
	for (e = 0; e < nGEQs; e++) if (GEQs[e].essential) 
	    {
	    printGEQ(&GEQs[e]);
	    fprintf(outputFile,"\n");
	    };
	fprintf(outputFile,"potentially redundant equations:\n");
	for (e = 0; e < nGEQs; e++) if (!GEQs[e].essential) 
	    {
	    printGEQ(&GEQs[e]);
	    fprintf(outputFile,"\n");
	    };
	};
    };




int Problem::findDifference(int e, int &v1, int &v2) 
    {
    // if 1 returned, eqn E is of form v1 -coef >= v2 
    for(v1=1;v1<=nVars;v1++)
	if (GEQs[e].coef[v1]) break;
    for(v2=v1+1;v2<=nVars;v2++)
	if (GEQs[e].coef[v2]) break;
    if (v2 > nVars) 
	{
	if (GEQs[e].coef[v1] == -1) 
	    {
	    v2 = v1;
	    v1 = 0;	
	    return 1;
	    };
	if (GEQs[e].coef[v1] == 1) 
	    {
	    v2 = 0;
	    return 1;
	    };
	return 0;
	};
    if (GEQs[e].coef[v1] * GEQs[e].coef[v2] != -1)  return 0;
    if (GEQs[e].coef[v1] < 0) swap(v1,v2);
    return 1;
    };

struct succListStruct {
    int num;
    int notEssential;
    int var[maxVars];
    coef_t diff[maxVars];
    int eqn[maxVars];
} succ[maxVars];
   
int Problem::chainKill(int color, int onlyWildcards) 
    {
    int v1,v2,e;
    int essentialPred[maxVars];
    int redundant[maxmaxGEQs];
    int inChain[maxVars];
    int goodStartingPoint[maxVars];
    int tryToEliminate[maxmaxGEQs];
    int triedDoubleKill = 0;


      restart:

    int anyToKill = 0;
    int anyKilled = 0;
    int canHandle = 0;
   
    for(v1=0;v1<=nVars;v1++) 
	{
	succ[v1].num = 0;
	succ[v1].notEssential = 0;
	goodStartingPoint[v1] = 0;
	inChain[v1] = -1;
	essentialPred[v1] = 0;
	};

    int essentialEquations = 0;
    for (e = 0; e < nGEQs; e++) 
	{
	redundant[e] = 0;
	tryToEliminate[e] = !GEQs[e].essential;
	if (GEQs[e].essential) essentialEquations++;
	if (color && !GEQs[e].color) tryToEliminate[e] = 0;
	};
    if (essentialEquations == nGEQs) return 0;
    if (2*essentialEquations < nVars) return 1;

      
    for (e = 0; e < nGEQs; e++) 
	if (tryToEliminate[e] && 
	    GEQs[e].varCount <= 2 &&
	    findDifference(e,v1,v2))  
	    {
	    assert(v1 == 0 || GEQs[e].coef[v1] == 1);
	    assert(v2 == 0 || GEQs[e].coef[v2] == -1);
	    succ[v2].notEssential++;
	    int s = succ[v2].num++;
	    succ[v2].eqn[s] = e;
	    succ[v2].var[s] = v1;
	    succ[v2].diff[s] = -GEQs[e].coef[0];
	    goodStartingPoint[v2] = 1;
	    anyToKill++;
	    canHandle++;
	    };
    if (!anyToKill) 
	{
	return canHandle < nGEQs;
	};
    for (e = 0; e < nGEQs; e++) 
	if (!tryToEliminate[e] && 
	    GEQs[e].varCount <= 2 &&
	    findDifference(e,v1,v2))  
	    {
	    assert(v1 == 0 || GEQs[e].coef[v1] == 1);
	    assert(v2 == 0 || GEQs[e].coef[v2] == -1);
	    int s = succ[v2].num++;
	    essentialPred[v1]++;
	    succ[v2].eqn[s] = e;
	    succ[v2].var[s] = v1;
	    succ[v2].diff[s] = -GEQs[e].coef[0];
	    canHandle++;
	    };


    if (DBUG) 
	{
	int s;
	fprintf(outputFile,"In chainkill: [\n");
        for(v1 = 0;v1<=nVars;v1++) 
	    {
	    fprintf(outputFile,"#%d <=   %s: ",essentialPred[v1],variable(v1));
	    for(s=0;s<succ[v1].notEssential;s++) 
		fprintf(outputFile," %s(" coef_fmt ") ",variable(succ[v1].var[s]),
			succ[v1].diff[s]);
	    for(;s<succ[v1].num;s++) 
		fprintf(outputFile," %s[" coef_fmt "] ",variable(succ[v1].var[s]),
			succ[v1].diff[s]);
	    fprintf(outputFile,"\n");
	    };
	};

    for(;v1<=nVars;v1++) if (succ[v1].num == 1 && succ[v1].notEssential == 1) 
	{
	succ[v1].notEssential--;
	essentialPred[succ[v1].var[succ[v1].notEssential]]++;
	};

    if (DBUG) fprintf(outputFile,"Trying quick double kill:\n");
    int s1a,s1b,s2;
    int v3;
    for(v1 = 0;v1<=nVars;v1++) 
	for(s1a=0;s1a<succ[v1].notEssential;s1a++) 
	    {
	    v3 = succ[v1].var[s1a];
	    for(s1b=0;s1b<succ[v1].num;s1b++) if (s1a != s1b) 
		{
		v2 = succ[v1].var[s1b];
		for(s2=0;s2<succ[v2].num;s2++) if (succ[v2].var[s2] == v3
						   && succ[v1].diff[s1b] + succ[v2].diff[s2] >=
						   succ[v1].diff[s1a]) 
		    {
		    if (DBUG) 
			{
			fprintf(outputFile,"quick double kill: "); 
			printGEQ(&GEQs[succ[v1].eqn[s1a]]);
			fprintf(outputFile,"\n"); 
			};
		    redundant[succ[v1].eqn[s1a]] = 1;
		    anyKilled++;
		    anyToKill--;
		    goto nextVictim;
		    };
		}
	      nextVictim: v1 = v1;
	    };
    if (anyKilled) 
	{
	for (e = nGEQs-1; e >= 0;e--) if (redundant[e])  
	    {
	    if (DBUG) 
		{
		fprintf(outputFile,"Deleting ");
		printGEQ(&GEQs[e]);
		fprintf(outputFile,"\n");
		};
	    deleteGEQ(e);
	    };

	if (!anyToKill) return canHandle < nGEQs;
	noteEssential(onlyWildcards);
	triedDoubleKill = 1;
	goto restart;
	};

    for(v1 = 0;v1<=nVars;v1++) if (succ[v1].num == succ[v1].notEssential && succ[v1].notEssential > 0) 
	{
	succ[v1].notEssential--;
	essentialPred[succ[v1].var[succ[v1].notEssential]]++;
	};

    while (1) 
	{
	int chainLength;
	int chain[maxVars];
	coef_t distance[maxVars];
	// pick a place to start
	for(v1 = 0;v1<=nVars;v1++) if (essentialPred[v1] == 0
				       && succ[v1].num > succ[v1].notEssential) break;
	if (v1 > nVars) 
	    for(v1 = 0;v1<=nVars;v1++) if (goodStartingPoint[v1]  &&
					   succ[v1].num > succ[v1].notEssential) break;
	if (v1 > nVars) break;

	chainLength = 1;
	chain[0] = v1;
	distance[0] = 0;
	inChain[v1] = 0;
	int s;
   
	while (succ[v1].num > succ[v1].notEssential) 
	    {
	    s = succ[v1].num-1;
	    if (inChain[succ[v1].var[s]] >= 0) 
		{
		// Found cycle, don't do anything with them yet
		break;
		};
	    succ[v1].num = s;

	    distance[chainLength]= 
		distance[chainLength-1]  +
		succ[v1].diff[s];
	    v1 = chain[chainLength] = succ[v1].var[s];
	    essentialPred[v1]--;
	    assert(essentialPred[v1] >= 0);
	    inChain[v1] = chainLength;
	    chainLength++;
	    };


	int c;
	if (DBUG) 
	    {
	    fprintf(outputFile,"Found chain: \n");
	    for (c = 0; c < chainLength; c++) 
		fprintf(outputFile,"%s:" coef_fmt "  ",variable(chain[c]),
			distance[c]);
	    fprintf(outputFile,"\n");
	    };
	
	
	for (c = 0; c < chainLength; c++) 
	    {
	    v1 = chain[c];
	    for(s=0;s<succ[v1].notEssential;s++) 
		{
		if (DBUG)
		    fprintf(outputFile,"checking for %s + " coef_fmt " <= %s \n",
			    variable(v1),succ[v1].diff[s],
			    variable(succ[v1].var[s]));
		if (inChain[succ[v1].var[s]] > c+1) 
		    {
		    if (DBUG)
			fprintf(outputFile,"%s + " coef_fmt " <= %s is in chain\n",
				variable(v1), 
				distance[inChain[succ[v1].var[s]]]- distance[c],
				variable(succ[v1].var[s]));
		    if ( distance[inChain[succ[v1].var[s]]]- distance[c]
			 >= succ[v1].diff[s]) 
			{
			if (DBUG) 
			    fprintf(outputFile,"%s + " coef_fmt " <= %s is redundant\n",
				    variable(v1),succ[v1].diff[s],
				    variable(succ[v1].var[s]));
			redundant[succ[v1].eqn[s]] = 1;
			}
		    };
		}
	    }	
	for (c = 0; c < chainLength; c++) 
	    inChain[chain[c]] = -1;

	};
    for (e = nGEQs-1; e >= 0;e--) if (redundant[e])  
	{
	if (DBUG) 
	    {
	    fprintf(outputFile,"Deleting ");
	    printGEQ(&GEQs[e]);
	    fprintf(outputFile,"\n");
	    };
	deleteGEQ(e);
	anyKilled = 1;
	};

    if (anyKilled) noteEssential(onlyWildcards);

    if (anyKilled && DBUG) 
	{
	fprintf(outputFile,"\nResult:\n");
	printProblem();
	};
    if (DBUG)  
	{
	fprintf(outputFile,"] end chainkill\n");
	printProblem();
	};
    return canHandle < nGEQs;
    };


int Problem::quickKill(int onlyWildcards, bool desperate)
    {
    int e, e1, e2, e3, p, q, i, k;
    coef_t alpha, alpha1, alpha2, alpha3;
    int e3L,e3U,e3I;
    coef_t c;
    int isDead[maxmaxGEQs];
    int involvesWildcard[maxmaxGEQs];
    int killOrder[maxmaxGEQs];
    unsigned int P[maxmaxGEQs], Z[maxmaxGEQs], N[maxmaxGEQs];
    unsigned int PP, PZ, PN;	/* possible Positives, possible zeros & possible negatives */
    int anyKilled = 0;

    int maxVarCount = 0;
    if (!onlyWildcards) if (!combineToTighten()) return 0;
    noteEssential(onlyWildcards);

    int moreToDo = chainKill(0,onlyWildcards);

#ifdef NDEBUG
    if (!moreToDo) return 1;
#endif

    if (!desperate && nGEQs > 60) {
        if (TRACE) 
	    {
	    fprintf(outputFile, "too complicated to quick kill\n");
	    }
	return 1;
	}


    if (DBUG) 
	{
	fprintf(outputFile, "in eliminate Redudant:\n");
	printProblem();
	};

    for (e = nGEQs - 1; e >= 0; e--) 
	{
	int tmp = 1;
	isDead[e] = 0;
	involvesWildcard[e] = 0;
	P[e] = Z[e] = N[e] = 0;
	if (maxVarCount < GEQs[e].varCount)
	    maxVarCount = GEQs[e].varCount;
	for (i = nVars; i >= 1; i--) 
	    {
	    if (GEQs[e].coef[i] == 0)
		Z[e] |= tmp;
	    else 
		{
		if (i > safeVars) involvesWildcard[e] = 1;
		if (GEQs[e].coef[i] < 0)
		    N[e] |= tmp;
		else
		    P[e] |= tmp;
		};
	    tmp <<= 1;
	    }
	}

    int ordered = 0;
    for (i = 1; ordered < nGEQs && i<=maxVarCount;i++) 
	for (e = 0; e < nGEQs; e++) if ((!GEQs[e].essential || involvesWildcard[e])
					&& GEQs[e].varCount ==i) 
	    {
	    for(e2=0;e2<ordered;e2++) assert(killOrder[e2] != e);
	    killOrder[ordered++] = e;
	    };

    if (DEBUG) 
	{
	fprintf(outputFile,"Prefered kill order:\n");
	for (e3I = ordered - 1; e3I >= 0; e3I--) 
	    {
	    fprintf(outputFile,"%2d: ",nGEQs-1-e3I);
	    printGEQ(&GEQs[killOrder[e3I]]);
	    fprintf(outputFile,"\n");
	    }
	};

    e3U = ordered-1;
    while(e3U >= 0) 
	{
	for(e3L = e3U;e3L > 0 && GEQs[killOrder[e3L-1]].varCount ==
			  GEQs[killOrder[e3U]].varCount;e3L--);
	if(DBUG) 
	    fprintf(outputFile,"Trying to kill %d..%d\n",e3U,e3L);
	for (e1 = 0; e1 <nGEQs; e1++)
	    if (!isDead[e1])
		for (e2 = 0; e2 <nGEQs; e2++)
		    if (e1 != e2 && !isDead[e2]) 
			{
			for (p = nVars; p > 1; p--) 
			if (abs(GEQs[e1].coef[p]) <= 0x7fff && 
				abs(GEQs[e2].coef[p]) <= 0x7fff)
			    {
			    for (q = p - 1; q > 0; q--) 
				if (abs(GEQs[e1].coef[q]) <= 0x7fff && 
					abs(GEQs[e2].coef[q]) <= 0x7fff)
				{
				alpha = 
	check_mul(GEQs[e1].coef[p], GEQs[e2].coef[q]) 
	- check_mul(GEQs[e2].coef[p], GEQs[e1].coef[q]);
				if (alpha != 0 && abs(alpha) <= 0x7fff)
				    goto foundPQ;
				};
			    };
			continue;

			  foundPQ:
			PZ = (Z[e1] & Z[e2]) | (P[e1] & N[e2]) | (N[e1] & P[e2]);
			PP = P[e1] | P[e2];
			PN = N[e1] | N[e2];
			if (DEBUG) 
			    {
			    fprintf(outputFile,"Considering combination of ");
			    printGEQ(&(GEQs[e1]));
			    fprintf(outputFile," and  ");
			    printGEQ(&(GEQs[e2]));
			    fprintf(outputFile,"\n");
			    };

			for (e3I = e3U; e3I >= e3L; e3I--)
			    if (!isDead[e3=killOrder[e3I]]
				&& e3 != e1 && e3 != e2) 
				{

				if (!implies(Z[e3], PZ))
				    goto nextE3;

				alpha1 = 
        //GEQs[e2].coef[q] * GEQs[e3].coef[p] 
	//- GEQs[e2].coef[p] * GEQs[e3].coef[q];
	check_mul(GEQs[e2].coef[q], GEQs[e3].coef[p]) 
	- check_mul(GEQs[e2].coef[p], GEQs[e3].coef[q]);
				alpha2 = 
		//-(GEQs[e1].coef[q] * GEQs[e3].coef[p] 
		//- GEQs[e1].coef[p] * GEQs[e3].coef[q]);
	-(check_mul(GEQs[e1].coef[q], GEQs[e3].coef[p]) 
	  - check_mul(GEQs[e1].coef[p], GEQs[e3].coef[q]));
				alpha3 = alpha;

				if (abs(alpha1) > 0x7fff 
					|| abs(alpha2) > 0x7fff
					|| alpha1 * alpha2 <= 0)
				    goto nextE3;
				if (alpha1 < 0) 
				    {
				    alpha1 = -alpha1;
				    alpha2 = -alpha2;
				    alpha3 = -alpha3;
				    }
				{
				coef_t g = gcd(gcd(abs(alpha1),abs(alpha2)),abs(alpha3));
				alpha1 /= g;
				alpha2 /= g;
				alpha3 /= g;
			        }
				if (DEBUG) 
				    {
				      fprintf(outputFile, coef_fmt "e1 + " coef_fmt "e2 = " coef_fmt "e3: ",alpha1,alpha2,alpha3);
				    printGEQ(&(GEQs[e3]));
				    fprintf(outputFile,"\n");
				    };

				if (alpha3 > 0) 
				    {
				/* Trying to prove e3 is redundant */
				    if (!implies(P[e3], PP) | !implies(N[e3], PN))
					goto nextE3;
				    if (!GEQs[e3].color && (GEQs[e1].color || GEQs[e2].color))
					goto nextE3;

				/* verify alpha1*v1+alpha2*v2 = alpha3*v3 */
				    for (k = nVars; k >= 1; k--)
					if (
				check_mul(alpha3,  GEQs[e3].coef[k])
					    != 
				check_mul(alpha1, GEQs[e1].coef[k]) 
				+ check_mul(alpha2, GEQs[e2].coef[k]))
					    goto nextE3;

				    c = alpha1 * GEQs[e1].coef[0] + alpha2 * GEQs[e2].coef[0];
				    if (c < alpha3 * (GEQs[e3].coef[0] + 1)) 
					{
					if (DBUG) 
					    {
					    fprintf(outputFile, "found redundant inequality\n");
					    fprintf(outputFile, "alpha1, alpha2, alpha3 = " coef_fmt "," coef_fmt "," coef_fmt "\n",
						    alpha1, alpha2, alpha3);

					    printGEQ(&(GEQs[e1]));
					    fprintf(outputFile, "\n");
					    printGEQ(&(GEQs[e2]));
					    fprintf(outputFile, "\n=> ");
					    printGEQ(&(GEQs[e3]));
					    fprintf(outputFile, "\n\n");
					    assert(moreToDo);
					    };

					anyKilled = 1;
					isDead[e3] = 1;
					}
				    } 
				else 
				    {
				/*
				 * trying to prove e3 <= 0 and therefore e3 = 0, or trying to prove e3 < 0, and
				 * therefore the problem has no solutions
				 * 
				 */
				    if (!implies(P[e3], PN) | !implies(N[e3], PP))
					goto nextE3;

				    alpha3 = alpha3;
				/* verify alpha1*v1+alpha2*v2 = alpha3*v3 */
				    for (k = nVars; k >= 1; k--)
					if (alpha3 * GEQs[e3].coef[k]
					    != alpha1 * GEQs[e1].coef[k] + alpha2 * GEQs[e2].coef[k])
					    goto nextE3;

				    if (DEBUG) 
					{
					fprintf(outputFile,"All but constant term checked\n");
					};
				    c = alpha1 * GEQs[e1].coef[0] + alpha2 * GEQs[e2].coef[0];
				    if (DEBUG) 
					{
					fprintf(outputFile,"All but constant term checked\n");
					fprintf(outputFile,"Constant term is " coef_fmt " vs " coef_fmt "\n",
						alpha3*GEQs[e3].coef[0],
						alpha3*(GEQs[e3].coef[0]-1));
					};
				    if (c < alpha3 * (GEQs[e3].coef[0])) 
					{

				/* We just proved e3 < 0, so no solutions exist */
					if (DBUG) 
					    {
					    fprintf(outputFile, "found implied over tight inequality\n");
					    fprintf(outputFile, "alpha1, alpha2, alpha3 = " coef_fmt "," coef_fmt "," coef_fmt "\n",
						    alpha1, alpha2, -alpha3);
					    printGEQ(&(GEQs[e1]));
					    fprintf(outputFile, "\n");
					    printGEQ(&(GEQs[e2]));
					    fprintf(outputFile, "\n=> not ");
					    printGEQ(&(GEQs[e3]));
					    fprintf(outputFile, "\n\n");
					    };
					return (0);

					}
				    else if (c < alpha3 * (GEQs[e3].coef[0] - 1)) 
					{

				/* We just proved that e3 <=0, so e3 = 0 */
					if (DBUG) 
					    {
					    fprintf(outputFile, "found implied tight inequality\n");
					    fprintf(outputFile, "alpha1, alpha2, alpha3 = " coef_fmt "," coef_fmt "," coef_fmt "\n",
						    alpha1, alpha2, -alpha3);
					    printGEQ(&(GEQs[e1]));
					    fprintf(outputFile, "\n");
					    printGEQ(&(GEQs[e2]));
					    fprintf(outputFile, "\n=> inverse ");
					    printGEQ(&(GEQs[e3]));
					    fprintf(outputFile, "\n\n");
					    };
					int neweq = newEQ();
					eqncpy(&EQs[neweq], &GEQs[e3]);
					EQs[neweq].color = GEQs[e1].color || GEQs[e2].color || GEQs[e3].color;
					addingEqualityConstraint(neweq);
					isDead[e3] = 1;

					}
				    }
				  nextE3:;
				}
			}
	e3U = e3L-1;
	};

    for (e = nGEQs - 1; e >= 0; e--)
	if (isDead[e])
	    deleteGEQ(e);

    if (anyKilled && DBUG) 
	{
	fprintf(outputFile,"\nResult:\n");
	printProblem();
	};
    return 1;
    }

} // end of namespace omega
