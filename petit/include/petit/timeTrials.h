/* $Id: timeTrials.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_TimeTrials
#define Already_Included_TimeTrials

#include <basic/bool.h>
#include <petit/lang-interf.h>

namespace omega {

#if defined miniTimeTrials
#define timePeriod	100000
#define timeMinReps	5
#define timeMaxReps	500
#else
#define timePeriod	10000000
#define timeMinReps	10
#define timeMaxReps	8500
#endif


extern bool storeResult;
extern unsigned int stores;

extern unsigned int omegaTests;
extern unsigned int realOmegaTests;
extern unsigned int tightenTests;
extern unsigned int realTightenTests;
extern unsigned int refineTests;
extern unsigned int semiRealRefineTests;
extern unsigned int realRefineTests;
extern unsigned int realRefines;
extern unsigned int coverTests;
extern unsigned int realCoverTests;
extern unsigned int realCovers;
extern unsigned int terminationTests;
extern unsigned int realTerminationTests;
extern unsigned int realTerminators;
extern unsigned int killTests;
extern unsigned int killTestSuspects;
extern unsigned int killTestIP;
extern unsigned int realKills;
extern unsigned int zapTests;
extern unsigned int realZapTests;
extern unsigned int realZappable;

extern char ddCategory[];

/* reset & print stats for dd test (includes cover & term. tests) */
void timing1reset( void );
void timing1output(ddnature ddkind, pEXPR a1, pEXPR a2,
		   unsigned int  omegaCount, long  omegaTime,
		   unsigned int omega2Count, long omega2Time);

/* print message about skipped dd test */
void timingSoutput(ddnature ddkind, pEXPR a1, pEXPR a2);

/* reset & print stats for refine test */
void timingRreset( void );
void timingRoutput(pEXPR a1, pEXPR a2, unsigned int  refineCount, long  refineTime);

/* reset & print stats for zappable test */
void timingZreset( void );
void timingZoutput(pEXPR a1, pEXPR a2, unsigned int  zapCount, long  zapTime);

/* reset & print stats for kill test */
void timingKreset( void );
void timingKoutput(a_access a1, a_access a2,
		   bool quick_kill, unsigned int  killCount, long  killTime);

void eps_time_output(ddnature ddkind, a_access a1, a_access a2,
                   unsigned int  omegaCount, long  omegaTime);

}

#endif

