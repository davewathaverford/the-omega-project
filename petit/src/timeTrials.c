/* $Id: timeTrials.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#if !defined(OMIT_GETRUSAGE)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <basic/bool.h>
#include <petit/Exit.h>
#include <petit/missing.h>
#include <petit/timeTrials.h>
#include <petit/print.h>
#include <petit/debug.h>
#include <petit/petit_args.h>

namespace omega {

#define MaxTimingError 0.015

bool refineTimeTrials;
bool storeResult=1;
char ddCategory[PETITBUFSIZ];

unsigned int stores;
unsigned int omegaTests;
unsigned int realOmegaTests;
unsigned int tightenTests;
unsigned int realTightenTests;
unsigned int refineTests;
unsigned int semiRealRefineTests;
unsigned int realRefineTests;
unsigned int realRefines;
unsigned int coverTests;
unsigned int realCoverTests;
unsigned int realCovers;
unsigned int terminationTests;
unsigned int realTerminationTests;
unsigned int realTerminators;
unsigned int killTests;
unsigned int killTestSuspects;
unsigned int killTestIP;
unsigned int realKills;
unsigned int zapTests;
unsigned int realZapTests;
unsigned int realZappable;

void timing1reset()
{
    stores = realCovers = realTerminators = realRefines = 0;
    tightenTests = realTightenTests = 0;
    omegaTests = realOmegaTests = 0;
    coverTests = realCoverTests = 0;
    terminationTests = realTerminationTests = 0;
}

void timing1output(ddnature ddkind, node *a1, node *a2,
		   unsigned int  omegaCount, long  omegaTime,
		   unsigned int omega2Count, long omega2Time)
{
    char from[PETITBUFSIZ], to[PETITBUFSIZ];
    const char *kind;

    /* the following is not done as an assertion because
       NDEBUG is on when doing time trials */
    if (ddkind != ddflow && ddkind != ddoutput &&
	ddkind != ddreduce && ddkind != ddanti)
    {
	fprintf(stderr, "bogus dd kind\n");
	Exit(-2);
    }
    kind = (ddkind==ddflow||ddkind==ddanti ?"F": (ddkind==ddreduce ?"R":"O"));

    print_to_buf(a1, 0, 0);
    strcpy(from, (const char *) printBuf);
    print_to_buf(a2, 0, 0);
    strcpy(to, (const char *) printBuf);

    fprintf(debug, "%s from %s at line %d to %s at line %d\n",
            kind, from, a1->nodesequence, to, a2->nodesequence);

    fprintf(debug,
	    "%s %p %p %6.3f %6.3f o(%d/%d) s(%d) c(%d/%d/%d) t(%d/%d/%d) tighten(%d/%d) %s\n",
	    kind, a1, a2, 
	    (float)omegaTime/(float)omegaCount/1000.0, 
	    (float)omega2Time/(float)omega2Count/1000.0,
	    realOmegaTests, omegaTests, stores,
	    realCovers, realCoverTests, coverTests,
	    realTerminators, realTerminationTests, terminationTests,
	    realTightenTests, tightenTests,
	    ddCategory);

    if ((omega2Time > 0) &&
	(((float)omegaTime/(float)omegaCount/1000.0 -
	  (float)omega2Time/(float)omega2Count/1000.0) > MaxTimingError))
    {
        fprintf(debug, "WARNING omega1 greater than omega2 by : %9.6f\n",
	        (float)omegaTime/(float)omegaCount/1000.0 -
	        (float)omega2Time/(float)omega2Count/1000.00);
    }
}

void timingSoutput(ddnature ddkind, node *a1, node *a2)
{
    char from[PETITBUFSIZ], to[PETITBUFSIZ];
    const char *kind;

    /* the following is not done as an assertion because
       NDEBUG is on when doing time trials */
    if (ddkind != ddflow && ddkind != ddoutput &&
	ddkind != ddreduce && ddkind != ddanti)
    {
	fprintf(stderr, "bogus dd kind\n");
	Exit(-2);
    }
    kind = (ddkind==ddflow||ddkind==ddanti ?"F": (ddkind==ddreduce ?"R":"O"));

    /* If we would have skipped this anyway, don't print it now. */
    if (petit_args.omitTopLevel && access_shared_depth(a1,a2) == 0)
	return;

    print_to_buf(a1, 0, 0);
    strcpy(from, (const char *) printBuf);
    print_to_buf(a2, 0, 0);
    strcpy(to, (const char *) printBuf);

    fprintf(debug, "%s from %s at line %d to %s at line %d\n",
            kind, from, a1->nodesequence, to, a2->nodesequence);

    fprintf(debug,
	    "%s %p %p %6.3f %6.3f o(%d/%d) s(%d) c(%d/%d/%d) t(%d/%d/%d) tighten(%d/%d) %s\n",
	    kind, a1, a2, 
	    0.0, 0.0,
	    0, 0, 0,
	    0, 0, 0,
	    0, 0, 0,
	    0, 0, "Skipped test");
}

void timingRreset()
{
    realRefines = 0;
    refineTests = semiRealRefineTests = realRefineTests = 0;
}

void timingRoutput(node *a1, node *a2,
		   unsigned int  refineCount, long  refineTime)
{
    char from[PETITBUFSIZ], to[PETITBUFSIZ];
    char *id = "R";

    print_to_buf(a1, 0, 0);
    strcpy(from, (const char *) printBuf);
    print_to_buf(a2, 0, 0);
    strcpy(to, (const char *) printBuf);

    fprintf(debug, "%s from %s at line %d to %s at line %d\n",
            id, from, a1->nodesequence, to, a2->nodesequence);

    fprintf(debug,
	    "%s %p %p %6.3f r(%d/%d/%d/%d)\n",
	    id, a1, a2, 
	    (float)refineTime/(float)refineCount/1000.0, 
	    realRefines, realRefineTests, semiRealRefineTests, refineTests);
}

void timingZreset()
{
    realZappable = 0;
    zapTests = realZapTests = 0;
}

void timingZoutput(node *a1, node *a2,
		   unsigned int  zapCount, long  zapTime)
{
    char from[PETITBUFSIZ], to[PETITBUFSIZ];
    char *id = "Z";

    print_to_buf(a1, 0, 0);
    strcpy(from, (const char *) printBuf);
    print_to_buf(a2, 0, 0);
    strcpy(to, (const char *) printBuf);

    fprintf(debug, "%s from %s at line %d to %s at line %d\n",
            id, from, a1->nodesequence, to, a2->nodesequence);

    fprintf(debug,
	    "%s %p %p %6.3f z(%d/%d/%d)\n",
	    id, a1, a2, 
	    (float)zapTime/(float)zapCount/1000.0, 
	    realZappable, realZapTests, zapTests);
}

void timingKreset()
{
    realKills = killTests = killTestIP = killTestSuspects = 0;
}

void timingKoutput(node *a1, node *a2,
		   bool quick_kill, unsigned int  killCount, long  killTime)
{
    char from[PETITBUFSIZ], to[PETITBUFSIZ];
    char *id = "K";

    print_to_buf(a1, 0, 0);
    strcpy(from, (const char *) printBuf);
    print_to_buf(a2, 0, 0);
    strcpy(to, (const char *) printBuf);

    fprintf(debug, "%s from %s at line %d to %s at line %d\n",
            id, from, a1->nodesequence, to, a2->nodesequence);

    if (quick_kill)
    {
	fprintf(debug, "%s %p %p Quick killed\n", id, a1, a2);
    } else {
	if (killCount)
	{
	    fprintf(debug, "%s %p %p %6.3f k(%d/%d/%d/%d)\n",
		    id, a1, a2, 
		    (float)killTime/(float)killCount/1000.0, 
		    realKills, killTestIP, killTestSuspects, killTests);
	} else {
	    fprintf(debug, "%s %p %p Didn't try accurate kill\n", id, a1, a2);
	}
    }
}

void eps_time_output(ddnature ddkind, node *a1, node *a2,
                   unsigned int  omegaCount, long  omegaTime)
{
    char from[PETITBUFSIZ], to[PETITBUFSIZ];
    const char *kind;

    /* the following is not done as an assertion because
       NDEBUG is on when doing time trials */

    kind = (ddkind == ddflow ? "F" : (ddkind == ddreduce ? "R" : "O"));
    print_to_buf(a1, 0, 0);
    strcpy(from, (const char *) printBuf);
    print_to_buf(a2, 0, 0);
    strcpy(to, (const char *) printBuf);

    fprintf(debug, "E from %s at line %d to %s at line %d\n",
            from, a1->nodesequence, to, a2->nodesequence);
    fprintf(debug,
            "E %p %p %6.3f \n",
            a1, a2, 
            (float)omegaTime/(float)omegaCount/1000.0);
  }

} // end namespace omega
#endif
