/* $Id: oc_i.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#if !defined(Already_included_oc_i)
#define Already_included_oc_i
#include <basic/assert.h>
#include <stdlib.h>
#include <basic/String.h>
#include <basic/Exit.h>
#include <basic/util.h>
#include <omega/omega_core/oc.h>

#define Omega3    1
#define CONNECTOR " && "
#define maxWildcards 18

#ifndef APROX
#define APROX 0
#endif

#define keyMult 31
#define TRUE 1
#define FALSE 0


namespace omega {

extern int findingImplicitEqualities;
extern int firstCheckForRedundantEquations;
extern int use_ugly_names;
extern int doItAgain;
extern int newVar;
extern int conservative;
extern FILE *outputFile;	/* printProblem writes its output to this file */
extern int nextKey;
extern int nextWildcard;
extern int trace;
extern int depth;
extern int packing[maxVars];
extern int headerLevel;
extern int inApproximateMode;
extern int inStridesAllowedMode;
extern int addingOuterEqualities;
extern int outerColor;

const int hashTableSize =5*maxmaxGEQs;
const int maxKeys = 8*maxmaxGEQs;

extern eqn hashMaster[hashTableSize];
extern int reduceWithSubs;
extern int pleaseNoEqualitiesInSimplifiedProblems;
extern int hashVersion;

#define noProblem ((Problem *) 0)

extern Problem *originalProblem;
int checkIfSingleVar(eqn *e, int i);
/* Solve e = factor alpha for x_j and substitute */

void negateCoefficients(eqn * eqn, int nV);

extern int omegaInitialized;
extern Problem full_answer, context,redProblem;

#if defined BRAIN_DAMAGED_FREE
static inline void free(const Problem *p)
{
   free((char *)p);
}
#endif

#if defined NDEBUG 
#define CHECK_FOR_DUPLICATE_VARIABLE_NAMES 
#else
#define CHECK_FOR_DUPLICATE_VARIABLE_NAMES \
{ \
    char name[maxVars][80]; \
    int i; \
    for(i=1; i<=nVars; i++) { \
	strcpy(name[i],variable(i)); \
	assert(name[i][0] != '\0'); \
        for(int j=1;j<i; j++) assert(strcmp(name[i],name[j]) != 0); \
    } \
}
#endif

} // end of namespace omega

#endif
