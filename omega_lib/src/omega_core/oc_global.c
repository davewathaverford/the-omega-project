/* $Id: oc_global.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#include <omega/omega_core/oc_i.h>

namespace omega {

const int Problem::min_alloc = 10;
const int Problem::first_alloc_pad = 5;

int omega_core_debug=0;

int maxEQs  = 35;
int maxGEQs = 70;

int newVar = -1;
int findingImplicitEqualities = 0;
int firstCheckForRedundantEquations = 0;
int doItAgain;
int conservative = 0;
FILE *outputFile;	/* printProblem writes its output to this file */
int nextKey;
char wildName[200][20];
int nextWildcard = 0;
int trace = 1;
int depth = 0;
int packing[maxVars];
int headerLevel;
int inApproximateMode = 0;
int inStridesAllowedMode = 0;
int addingOuterEqualities = 0;
int outerColor = 0;
class eqn hashMaster[hashTableSize];
int reduceWithSubs = 1;
int pleaseNoEqualitiesInSimplifiedProblems = 0;
int hashVersion = 0;
Problem *originalProblem = noProblem;
int omegaInitialized = 0;

int mayBeRed = 0;



} // end of namespace omega
