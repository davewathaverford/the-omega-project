/* $Id: oc_print.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $*/

#include <omega/omega_core/oc_i.h>

namespace omega {

int print_in_code_gen_style = 0;

void
Problem::
initializeVariables() const
    {
    Problem *p = (Problem *)this;
    int i;
    assert(!p->variablesInitialized);
    for (i = p->nVars; i >= 0; i--)
        p->forwardingAddress[i] = p->var[i] = i;
    p->variablesInitialized = 1;
    }


String
Problem:: print_term_to_string(const eqn * e, int c) const
    {
    String s="";
    int i;
    int first;
    int n = nVars;
    int wentFirst = -1;
    first = 1;
    for (i = 1; i <= n; i++)
	if (c * e->coef[i] > 0) 
	    {

	    first = 0;
	    wentFirst = i;

	    if (c * e->coef[i] == 1)
		s+= variable(i);
	    else
		{
 		s +=  itoS(c * e->coef[i]);
		if (print_in_code_gen_style) s += String("*");
		s += variable(i);
		}
	    break;
	    };
    for (i = 1; i <= n; i++)
	if (i != wentFirst && c * e->coef[i] != 0) 
	    {
	    if (!first && c * e->coef[i] > 0)
		s += "+";

	    first = 0;

	    if (c * e->coef[i] == 1)
		s += variable(i);
	    else if (c * e->coef[i] == -1) 
                {
		s +=  "-"; s += variable(i);
	        }
	    else
		{
		s += itoS(c * e->coef[i]); 
		if (print_in_code_gen_style) s += String("*");
		s += variable(i);
		}
	    };
    if (!first && c * e->coef[0] > 0)
	s += "+";
    if (first || c * e->coef[0] != 0)
	s += itoS(c * e->coef[0]);
    return s;
    }

void
Problem::
printTerm(const eqn * e, int c) const
    {
    String s = print_term_to_string(e, c);
    fprintf(outputFile, (const char *) s);
    }

void Problem::printSub(int v) const
    {
    String s = print_sub_to_string(v);
    fprintf(outputFile, (const char *) s);
    }

String Problem::print_sub_to_string(int v) const 
    {
    String s;

    if (v > 0) 
	s = variable(v);
    else 
	s = print_term_to_string(&SUBs[-v-1], 1);
    return s;
    }


void Problem:: clearSubs() 
    {
    nSUBs = 0;
    nMemories = 0;
    }

void
Problem::
printEqn(const eqn * e, int test, int extra) const
    {
    char buf[maxVars * 12 + 80];

    sprintEqn(buf, e, test, extra);
    fprintf(outputFile, "%s", buf);
    }

String Problem::printEqnToString(const eqn * e, int test, int extra) const
    {
    char buf[maxVars * 12 + 80];
    sprintEqn(buf, e, test, extra);
    return String(buf);
    }

void
Problem::
sprintEqn(char *str, const eqn * e, int test, int extra) const
    {
    int i;
    int first;
    int n = nVars + extra;
    int isLT;

    isLT = test && e->coef[0] == -1;
    if (isLT)
	isLT = 1;
#if 0
    if (test) 
	{
	if (DEBUG && e->touched) 
	    {
	    sprintf(str, "!");
	    while (*str)
		str++;
	    }
	else if (DBUG && !e->touched && e->key != 0) 
	    {
	    sprintf(str, "%d: ", e->key);
	    while (*str)
		str++;
	    }
	}
#endif
    if (e->color) 
	{
	sprintf(str, "[");
	while (*str)
	    str++;
	}
    first = 1;
    for (i = isLT; i <= n; i++)
	if (e->coef[i] < 0) 
	    {
	    if (!first) 
		{
		sprintf(str, "+");
		while (*str)
		    str++;
		}
	    else
		first = 0;
	    if (i == 0) 
		{
		sprintf(str, coef_fmt, -e->coef[i]);
		while (*str)
		    str++;
		}
	    else if (e->coef[i] == -1) 
		{
		sprintf(str, "%s", variable(i));
		while (*str)
		    str++;
		}
	    else 
		{
		if (print_in_code_gen_style) 
			sprintf(str, coef_fmt "*%s", -e->coef[i], variable(i));
		else sprintf(str, coef_fmt "%s", -e->coef[i], variable(i));
		while (*str)
		    str++;
		}
	    };
    if (first) 
	{
	if (isLT) 
	    {
	    sprintf(str, "1");
	    isLT = 0;
	    }
	else
	    sprintf(str, "0");
	while (*str)
	    str++;
	}
    if (test == 0) 
	{
	if (print_in_code_gen_style) sprintf(str, " == ");
	else sprintf(str, " = ");
	while (*str)
	    str++;
	}
    else 
	{
	if (isLT)
	    sprintf(str, " < ");
	else
	    sprintf(str, " <= ");
	while (*str)
	    str++;
	}

    first = 1;
    for (i = 0; i <= n; i++)
	if (e->coef[i] > 0) 
	    {
	    if (!first) 
		{
		sprintf(str, "+");
		while (*str)
		    str++;
		}
	    else
		first = 0;
	    if (i == 0) 
		{
		sprintf(str,  coef_fmt , e->coef[i]);
		while (*str)
		    str++;
		}
	    else if (e->coef[i] == 1) 
		{
		sprintf(str, "%s", variable(i));
		while (*str)
		    str++;
		}
	    else 
		{
		if (print_in_code_gen_style) sprintf(str, coef_fmt "*%s", e->coef[i], variable(i));
		else sprintf(str, coef_fmt "%s", e->coef[i], variable(i));
		while (*str)
		    str++;
		}
	    }
    if (first) 
	{
	sprintf(str, "0");
	while (*str)
	    str++;
	}
    if (e->color) 
	{
	sprintf(str, "]");
	while (*str)
	    str++;
	}
    }

void Problem::printSubstitution(int s) const 
    {
    const eqn * eq = &(SUBs[s]);
    assert(eq->key > 0);
    fprintf(outputFile, "%s := ", orgVariable(eq->key));
    printTerm(eq, 1);
    }

void
Problem::
printVars(int /*debug*/) const
    {
    int i;
    fprintf(outputFile, "variables = ");
    if (safeVars > 0)
	fprintf(outputFile, "(");
    for (i = 1; i <= nVars; i++) 
	{
	fprintf(outputFile, "%s", variable(i));
	if (i == safeVars)
	    fprintf(outputFile, ")");
	if (i < nVars)
	    fprintf(outputFile, ", ");
	};
    fprintf(outputFile, "\n");
    /*
    fprintf(outputFile, "forward addresses = ");
    if (safeVars > 0)
	fprintf(outputFile, "(");
    for (i = 1; i <= nVars; i++) 
	{
	int v = forwardingAddress[i];
	if (v > 0) fprintf(outputFile, "%s", variable(i));
	else fprintf(outputFile, "*");
	if (i == safeVars)
	    fprintf(outputFile, ")");
	if (i < nVars)
	    fprintf(outputFile, ", ");
	};
    fprintf(outputFile, "\n");
    */
    }


void printHeader() 
    {
    int i;
    for(i=0; i<headerLevel; i++) 
	{
	fprintf(outputFile, ". ");
	}
    }

void
Problem::
printProblem(int debug) const
    {
    int e;

    if (!variablesInitialized)
	initializeVariables();
    if (debug)
	{
        printHeader();
        fprintf(outputFile, "#vars = %d, #EQ's = %d, #GEQ's = %d, # SUB's = %d, ofInterest = %d\n",
	        nVars,nEQs,nGEQs,nSUBs,varsOfInterest);
        printHeader();
        printVars(debug);
        }
    for (e = 0; e < nEQs; e++) 
	{
	printHeader();
	printEQ(&EQs[e]);
	fprintf(outputFile, "\n");
	};
    for (e = 0; e < nGEQs; e++) 
	{
	printHeader();
	printGEQ(&GEQs[e]);
	fprintf(outputFile, "\n");
	};
    for (e = 0; e < nSUBs; e++) 
	{
	printHeader();
	printSubstitution(e);
	fprintf(outputFile, "\n");
	};

    for (e = 0; e < nMemories; e++) 
	{
	int i;
	printHeader();
	switch(redMemory[e].kind) 
	    {
	case notRed: fprintf(outputFile,"notRed: ");
	    break;
	case redGEQ: fprintf(outputFile,"Red: 0 <= ");
	    break;
	case redLEQ: fprintf(outputFile,"Red ??: 0 >= ");
	    break;
	case redEQ: fprintf(outputFile,"Red: 0 == ");
	    break;
	case redStride: fprintf(outputFile,"Red stride " coef_fmt ": ",
				redMemory[e].stride);
	    break;
	    };
	fprintf(outputFile," " coef_fmt, redMemory[e].constantTerm);
	for(i=0;i< redMemory[e].length; i++)
	    if(redMemory[e].coef[i] >= 0)
		fprintf(outputFile,"+" coef_fmt "%s", redMemory[e].coef[i],
			orgVariable(redMemory[e].var[i]));
	    else fprintf(outputFile,"-" coef_fmt "%s", -redMemory[e].coef[i],
			 orgVariable(redMemory[e].var[i]));
	fprintf(outputFile, "\n");
	};
    fflush(outputFile);

    CHECK_FOR_DUPLICATE_VARIABLE_NAMES;
    }


void
Problem::
printRedEquations() const
    {
    int e;


    if (!variablesInitialized)
	initializeVariables();
    printVars(1);
    for (e = 0; e < nEQs; e++) 
	{
	if (EQs[e].color == _red) 
	    {
	    printEQ(&EQs[e]);
	    fprintf(outputFile, "\n");
	    }
	};
    for (e = 0; e < nGEQs; e++) 
	{
	if (GEQs[e].color == _red) 
	    {
	    printGEQ(&GEQs[e]);
	    fprintf(outputFile, "\n");
	    }
	};
    for (e = 0; e < nSUBs; e++) 
	{
	if (SUBs[e].color) 
	    {
	    printSubstitution(e);
	    fprintf(outputFile, "\n");
	    }
	};
    fflush(outputFile);
    }



int
Problem::prettyPrintProblem() const 
    {
    String s = prettyPrintProblemToString();
    fprintf(outputFile, (const char *) s);
    fflush(outputFile);
    return 0;
    }


String
Problem::
prettyPrintProblemToString() const
    {
    String s="";
    int e;
    int v;
    int live[maxmaxGEQs];
    int v1, v2, v3;
    int t, change;
    int stuffPrinted = 0;
    char *connector = CONNECTOR;

    typedef enum {
	none, le, lt
    } partialOrderType;

    partialOrderType po[maxVars][maxVars];
    int poE[maxVars][maxVars];
    int lastLinks[maxVars];
    int firstLinks[maxVars];
    int chainLength[maxVars];
    int chain[maxVars];
    int varCount[maxVars];
    int i, m, multiprint;


    if (!variablesInitialized)
	initializeVariables();

    if (nVars > 0) 
	{

	for (e = 0; e < nEQs; e++) 
	    {
	    if (stuffPrinted)
		s += connector;
	    stuffPrinted = 1;
	    s += print_EQ_to_string(&EQs[e]);
	    };

	for (e = 0; e < nGEQs; e++)
	    {
	    live[e] = TRUE;
	    varCount[e] = 0;
	    for (v = 1; v <= nVars; v++)  if (GEQs[e].coef[v]) varCount[e]++;
	    }

	if (!print_in_code_gen_style)
	while (1) 
	    {
	    for (v = 1; v <= nVars; v++) 
		{
		lastLinks[v] = firstLinks[v] = 0;
		chainLength[v] = 0;
		for (v2 = 1; v2 <= nVars; v2++)
		    po[v][v2] = none;
		};

	    for (e = 0; e < nGEQs; e++)
		if (live[e] && varCount[e] <= 2) 
		    {
		    for (v = 1; v <= nVars; v++) 
			{
			if (GEQs[e].coef[v] == 1)
			    firstLinks[v]++;
			else if (GEQs[e].coef[v] == -1)
			    lastLinks[v]++;
			};

		    v1 = nVars;
		    while (v1 > 0 && GEQs[e].coef[v1] == 0)
			v1--;
		    v2 = v1 - 1;
		    while (v2 > 0 && GEQs[e].coef[v2] == 0)
			v2--;
		    v3 = v2 - 1;
		    while (v3 > 0 && GEQs[e].coef[v3] == 0)
			v3--;

		    if (GEQs[e].coef[0] > 0 || GEQs[e].coef[0] < -1
			|| v2 <= 0 || v3 > 0
			|| GEQs[e].coef[v1] * GEQs[e].coef[v2] != -1) 
			{
			/* Not a partial order relation */

			}
		    else 
			{
			if (GEQs[e].coef[v1] == 1) 
			    {
			    v3 = v2;
			    v2 = v1;
			    v1 = v3;
			    };
			/* relation is v1 <= v2 or v1 < v2 */
			po[v1][v2] = ((GEQs[e].coef[0] == 0) ? le : lt);
			poE[v1][v2] = e;
			};
		    }
	    for (v = 1; v <= nVars; v++)
		chainLength[v] = lastLinks[v];


	    /*
	     * printf("\n\nPartial order:\n"); printf("       	"); for (v1 = 1; v1 <= nVars; v1++)
	     * printf("%7s",variable(v1)); printf("\n"); for (v1 = 1; v1 <= nVars; v1++) { printf("%6s:
	     * ",variable(v1)); for (v2 = 1; v2 <= nVars; v2++) switch (po[v1][v2]) { case none: printf("       ");
	     * break; case le:   printf("    <= "); break; case lt:   printf("    <  "); break; } printf("\n"); }
	     */



	    /* Just in case nVars <= 0 */
	    change = FALSE;
	    for (t = 0; t < nVars; t++) 
		{
		change = FALSE;
		for (v1 = 1; v1 <= nVars; v1++)
		    for (v2 = 1; v2 <= nVars; v2++)
			if (po[v1][v2] != none &&
			    chainLength[v1] <= chainLength[v2]) 
			    {
			    chainLength[v1] = chainLength[v2] + 1;
			    change = TRUE;
			    }
		};

	    if (change) 
		{
		/* caught in cycle */
	   
#if 0 
	        printf("\n\nPartial order:\n"); printf("       	"); 
		for (v1 = 1; v1 <= nVars; v1++) printf("%7s",variable(v1)); printf("\n"); 
		for (v1 = 1; v1 <= nVars; v1++) { 
			printf("%6s: ",variable(v1)); 
			for (v2 = 1; v2 <= nVars; v2++) switch (po[v1][v2]) { 
				case none: printf("       "); break; 
				case le:   printf("    <= "); break; 
				case lt:   printf("    <  "); break; 
				} 
			printf("\n"); 
			}

		printProblem(1);
#endif
		break;
		};

	    for (v1 = 1; v1 <= nVars; v1++)
		if (chainLength[v1] == 0)
		    firstLinks[v1] = 0;

	    v = 1;
	    for (v1 = 2; v1 <= nVars; v1++)
		if (chainLength[v1] + firstLinks[v1] > chainLength[v] + firstLinks[v])
		    v = v1;

	    if (chainLength[v] + firstLinks[v] == 0)
		break;

	    if (stuffPrinted)
		s += connector;
	    stuffPrinted = 1;
	    /* chain starts at v */
	    /* print firstLinks */
	    {
	    coef_t tmp;
	    int first;
	    first = 1;
	    for (e = 0; e < nGEQs; e++)
		if (live[e] && GEQs[e].coef[v] == 1 && varCount[e] <= 2) 
		    {
		    if (!first)
			s += ", ";
		    tmp = GEQs[e].coef[v];
		    ((Problem *)this)->
			GEQs[e].coef[v] = 0;
		    s += print_term_to_string(&GEQs[e], -1);
		    ((Problem *)this)->
			GEQs[e].coef[v] = tmp;
		    live[e] = FALSE;
		    first = 0;
		    };
	    if (!first)
		s += " <= ";
	    };


	    /* find chain */
	    chain[0] = v;
	    m = 1;
	    while (1) 
		{
		/* print chain */
		for (v2 = 1; v2 <= nVars; v2++)
		    if (po[v][v2] && chainLength[v] == 1 + chainLength[v2])
			break;
		if (v2 > nVars)
		    break;
		chain[m++] = v2;
		v = v2;
		}

	    s += variable(chain[0]);

	    multiprint = 0;
	    for (i = 1; i < m; i++) 
		{
		v = chain[i - 1];
		v2 = chain[i];
		if (po[v][v2] == le)
		    s += " <= ";
		else
		    s += " < ";
		s +=  variable(v2);
		live[poE[v][v2]] = FALSE;
		if (!multiprint && i < m - 1)
		    for (v3 = 1; v3 <= nVars; v3++) 
			{
			if (v == v3 || v2 == v3)
			    continue;
			if (po[v][v2] != po[v][v3])
			    continue;
			if (po[v2][chain[i + 1]] != po[v3][chain[i + 1]])
			    continue;
			s += ","; s += variable(v3);
			live[poE[v][v3]] = FALSE;
			live[poE[v3][chain[i + 1]]] = FALSE;
			multiprint = 1;
			}
		else
		    multiprint = 0;
		};

	    v = chain[m - 1];
	    /* print lastLinks */
	    {
	    coef_t tmp;
	    int  first;
	    first = 1;
	    for (e = 0; e < nGEQs; e++)
		if (live[e] && GEQs[e].coef[v] == -1 && varCount[e] <= 2) 
		    {
		    if (!first)
			s += ", ";
		    else
			s += " <= ";
		    tmp = GEQs[e].coef[v];
		    ((Problem *)this)->
			GEQs[e].coef[v] = 0;
		    s += print_term_to_string(&GEQs[e], 1);
		    ((Problem *)this)->
			GEQs[e].coef[v] = tmp;
		    live[e] = FALSE;
		    first = 0;
		    };
	    };
	    };


	for (e = 0; e < nGEQs; e++)
	    if (live[e]) 
		{
		if (stuffPrinted)
		    s += connector;
		stuffPrinted = 1;
		s += print_GEQ_to_string(&GEQs[e]);
		};

	for (e = 0; e < nSUBs; e++) 
	    {
	    const eqn * eq = &SUBs[e];
	    if (stuffPrinted)
		s += connector;
	    stuffPrinted = 1;
	    if (eq->key > 0)
                {
		s += orgVariable(eq->key); s += " := ";
                }
	    else
		{
		s += "#"; s += itoS(eq->key); s += " := ";
		}
	    s += print_term_to_string(eq, 1);
	    };
	};
    return s;
    }



int
Problem::
prettyPrintRedEquations() const
    {
    int e, stuffPrinted = 0;
    char * connector = CONNECTOR;


    if (!variablesInitialized)
	initializeVariables();

    for (e = 0; e < nEQs; e++) 
	{
	if (EQs[e].color == _red)
	    {
	    if (stuffPrinted)
		fprintf(outputFile, connector);
	    stuffPrinted = 1;
	    ((Problem *)this)->
		EQs[e].color = black;
	    printEQ(&EQs[e]);
	    ((Problem *)this)->
		EQs[e].color = _red;
	    }
	}
    for (e = 0; e < nGEQs; e++) 
	{
	if (GEQs[e].color == _red)
	    {
	    if (stuffPrinted)
		fprintf(outputFile, connector);
	    stuffPrinted = 1;
	    ((Problem *)this)->
		GEQs[e].color = black;
	    printGEQ(&GEQs[e]);
	    ((Problem *)this)->
		GEQs[e].color = _red;
	    }
	}
    for (e = 0; e < nSUBs; e++) 
	{
	if (SUBs[e].color) 
	    {
	    if (stuffPrinted)
		fprintf(outputFile, connector);
	    stuffPrinted = 1;
	    printSubstitution(e);
	    }
	}
    fflush(outputFile);

    return 0;
    }

} // end of namespace omega
