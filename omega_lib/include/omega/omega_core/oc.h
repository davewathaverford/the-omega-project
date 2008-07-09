/* $Id: oc.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#ifndef Already_Included_OC
#define Already_Included_OC 1

#include <basic/bool.h>

#include <stdio.h>
//#include <cstdio>
//using namespace std;

#include <basic/String.h>
#include <basic/util.h>
#include <omega/omega_core/debugging.h>

#define maxVars 56

namespace omega {


extern int maxGEQs;
extern int maxEQs;

#if !defined LONG_LONG_COEF
#define LONG_LONG_COEF 0
#endif

#if LONG_LONG_COEF
#error "using long long coefficients currently causes assertion failures"

#if defined BOGUS_LONG_DOUBLE_COEF
typedef long double coef_t;  // type of coefficients
#define coef_fmt  "%llf"
#define posInfinity (1e+24)
#define negInfinity (-1e+24)
inline coef_t
check_pos_mul(coef_t x, coef_t y) {
#ifndef NDEBUG
    if (x != 0)
        assert(((MAXDOUBLE)/4) / x > y);
#endif
    return x * y;
}

#else

typedef LONGLONG coef_t;  // type of coefficients
#define coef_fmt  "%lld"
#define posInfinity (0x7ffffffffffffffLL)
#define negInfinity (-0x7ffffffffffffffLL)

#endif

#else

typedef int coef_t;  // type of coefficients
#define coef_fmt  "%d"
#define posInfinity (0x7ffffff)
#define negInfinity (-0x7ffffff)

#endif

#if ! defined maxmaxGEQs
#define maxmaxGEQs 512
#endif
#if ! defined maxmaxEQs
#define maxmaxEQs  256
#endif


#if 0
#if ! defined Already_Included_Portable
typedef unsigned char bool;  /* what a gross thing to do */
#endif
#endif

typedef int EqnKey;

class eqn {
public:
    EqnKey  key;
    coef_t  touched;  // see oc_simple.c
    int     color;
    int     essential;
    int     varCount;
    coef_t  coef[maxVars + 1];
};

typedef eqn * Eqn;
enum redType {notRed, redEQ, redGEQ, redLEQ, redStride};
enum redCheck {noRed=0, redFalse, redConstraints};
typedef enum {
   normalize_false, normalize_uncoupled, normalize_coupled
}
normalizeReturnType;
extern char wildName[200][20];

extern FILE *outputFile; /* printProblem writes its output to this file */
#define doTrace (trace && TRACE)
#define isRed(e) (desiredResult == SIMPLIFY && (e)->color)
#define eqnncpy(e1,e2,s) {int *p00,*q00,*r00; p00 = (int *)(e1); q00 = (int *)(e2); r00 = &p00[headerWords+1+s]; while(p00 < r00) *p00++ = *q00++; }
#define eqncpy(e1,e2) eqnncpy(e1,e2,nVars)
#define eqnnzero(e,s) {int *p00,*r00; p00 = (int *)(e); r00 = &p00[headerWords+1+(s)]; while(p00 < r00) *p00++ = 0;}
#define eqnzero(e) eqnnzero(e,nVars)

extern int mayBeRed;

#ifdef SPEED
#define TRACE 0
#define DBUG 0
#define DEBUG 0
#else
#define TRACE (omega_core_debug)
#define DBUG  (omega_core_debug > 1)
#define DEBUG (omega_core_debug > 2)
#endif





class Memory {
public:
    int     length;
    coef_t  stride;
    redType kind;
    coef_t  constantTerm;
    coef_t  coef[maxVars + 1];
    int     var[maxVars + 1];
};

#define headerWords ((4*sizeof(int) + sizeof(coef_t))/sizeof(int))

void check_number_EQs(int);
void check_number_GEQs(int);
extern eqn SUBs[];
extern Memory redMemory[];

class Problem {
public:
    short     nVars, safeVars;
    short     nEQs, nGEQs,nSUBs,nMemories,allocEQs,allocGEQs;
    short varsOfInterest;
    bool variablesInitialized;
    bool variablesFreed;
    short     var[maxVars+2];
    short     forwardingAddress[maxVars+2];
    // int     variableColor[maxVars+2];
    int  mHashVersion;
    const char *(*get_var_name)(unsigned int var, void *args);
    void *getVarNameArgs;
    eqn *GEQs;
    eqn *EQs;

    Problem(int in_eqs=0, int in_geqs=0);
    Problem(const Problem &);
    ~Problem();
    Problem & operator=(const Problem &);

/* Allocation parameters and functions */

    static const int min_alloc,first_alloc_pad;
    int padEQs(int oldalloc, int newreq) {
      check_number_EQs(newreq);
      return min((newreq < 2*oldalloc ? 2*oldalloc : 2*newreq),maxmaxEQs);
    }
    int padGEQs(int oldalloc, int newreq) {
	check_number_GEQs(newreq);
	return min((newreq < 2*oldalloc ? 2*oldalloc : 2*newreq),maxmaxGEQs);
    }
    int padEQs(int newreq) {
	check_number_EQs(newreq);
	return min(max(newreq+first_alloc_pad,min_alloc), maxmaxEQs);
    }
    int padGEQs(int newreq) {
	check_number_GEQs(newreq);
	return min(max(newreq+first_alloc_pad,min_alloc), maxmaxGEQs);
    }


void zeroVariable(int i);

void putVariablesInStandardOrder();
void noteEssential(int onlyWildcards);
int findDifference(int e, int &v1, int &v2);
int chainKill(int color,int onlyWildcards);

int newGEQ();
int newEQ();
int newSUB(){
	return nSUBs++;
	}


void initializeProblem();
void initializeVariables() const;
void printProblem(int debug = 1) const;
void printSub(int v) const;
String print_sub_to_string(int v) const;
void clearSubs();
void printRedEquations() const;
int  countRedEquations() const;
int  countRedGEQs() const;
int  countRedEQs() const;
int  countRedSUBs() const;
void difficulty(int &numberNZs, coef_t &maxMinAbsCoef, coef_t &sumMinAbsCoef) const;
int  prettyPrintProblem() const;
String  prettyPrintProblemToString() const;
int  prettyPrintRedEquations() const;
int  simplifyProblem(int verify, int subs, int redundantElimination);
int  simplifyProblem();
int  simplifyAndVerifyProblem();
int  simplifyApproximate( int strides_allowed);
void coalesce();
void partialElimination();
void unprotectVariable(int var);
void negateGEQ(int);
void convertEQstoGEQs(int excludeStrides=0);
void convertEQtoGEQs(int eq);
void nameWildcard(int i);
void useWildNames();
void ordered_elimination(int symbolic);
int eliminateRedundant (bool expensive);
void eliminateRed(bool expensive);
void constrainVariableSign(int color, int var, int sign); 
void  constrainVariableValue(int color, int var, int value);
void query_difference(int v1, int v2,
        coef_t &lowerBound, coef_t &upperBound, bool &guaranteed);
int solve(int desiredResult);
String print_term_to_string(const eqn *e, int c) const;
void printTerm(const eqn * e, int c) const;
String printEqnToString(const eqn * e, int test, int extra) const;
void sprintEqn(char *str, const eqn * e, int is_geq,int extra) const;
void printEqn(const eqn * e, int is_geq, int extra) const;
void printEQ(const eqn *e) const {printEqn(e,0,0); }
String print_EQ_to_string(const eqn *e) const {return printEqnToString(e,0,0);}
String print_GEQ_to_string(const eqn *e) const {return printEqnToString(e,1,0);}
void printGEQ(const eqn *e) const {printEqn(e,1,0); }
void printGEQextra(const eqn *e) const {printEqn(e,1,1); }
void printSubstitution(int s) const;
void printVars(int debug = 1) const;
void swapVars(int i, int j);
void reverseProtectedVariables();
redCheck redSimplifyProblem(int effort, int computeGist);
int queryVariable(int i, coef_t *lowerBound, coef_t *upperBound);
int query_variable_bounds(int i, coef_t *l, coef_t *u);
void queryCoupledVariable(int i, coef_t *l, coef_t *u, int *couldBeZero, coef_t lowerBound, coef_t upperBound);
int queryVariableSigns(int i, int dd_lt, int dd_eq, int dd_gt, coef_t lowerBound, coef_t upperBound, bool *distKnown, coef_t *dist);
void addingEqualityConstraint(int e);
normalizeReturnType normalize();
void cleanoutWildcards();
void substitute(eqn *sub, int i, coef_t c);
void deleteVariable( int i);
void deleteBlack();
int addNewProtectedWildcard();
int addNewUnprotectedWildcard();
int protectWildcard( int i);
void doMod( coef_t factor, int e, int j);
void freeEliminations( int fv);
int verifyProblem();
void resurrectSubs();
int solveEQ();
int combineToTighten() ;
int quickKill(int onlyWildcards, bool desperate = false);
int expensiveEqualityCheck();
int expensiveRedKill();
int expensiveKill();
int smoothWeirdEquations();
void quickRedKill(int computeGist);
void chainUnprotect();
void freeRedEliminations();
void doElimination( int e, int i);
void analyzeElimination(
                   int &v,
                   int &darkConstraints,
                   int &darkShadowFeasible,
                   int &unit,
                   coef_t &parallelSplinters,
                   coef_t &disjointSplinters,
                   coef_t &lbSplinters,
                   coef_t &ubSplinters,
                   int &parallelLB);
int parallelSplinter(int e, int diff, int desiredResult);
int solveGEQ( int desiredResult);
void setInternals();
void setExternals();
int reduceProblem();
void problem_merge(Problem &);
void deleteRed();
void turnRedBlack();
void checkGistInvariant() const;
void check() const;
coef_t checkSum() const;
void rememberRedConstraint(eqn *e, redType type, coef_t stride);
void recallRedMemories();
void simplifyStrideConstraints();
const char * orgVariable(int i) const { 
	return ((i == 0) ?   //  cfront likes this form better
		"1" :
		((i < 0) ?
		 wildName[-i] :
		 (*get_var_name)(i,getVarNameArgs)));
	};
const char * variable(int i) const { 
	return orgVariable(var[i]) ;
	};

void deleteGEQ(int e) {
	if (DEBUG) {
		fprintf(outputFile,"Deleting %d (last:%d): ",e,nGEQs-1); 
		printGEQ(&GEQs[e]);
		fprintf(outputFile,"\n");
		}; 
	if (e < nGEQs-1) 
		eqnncpy (&GEQs[e], &GEQs[nGEQs - 1],(nVars)); 
	nGEQs--;
	};
void deleteEQ(int e) {
	if (DEBUG) {
		fprintf(outputFile,"Deleting %d (last:%d): ",e,nEQs-1); 
		printGEQ(&EQs[e]);
		fprintf(outputFile,"\n");
		}; 
	if (e < nEQs-1) 
		eqnncpy (&EQs[e], &EQs[nEQs - 1],(nVars)); 
	nEQs--;
	};

};



#define UNKNOWN 2
#define SIMPLIFY 3
#define _red 1
#define black 0

extern int print_in_code_gen_style;


void  initializeOmega(void);


/* set extra to 0 for normal use */
int singleVarGEQ(eqn *e);

void setPrintLevel(int level);

void printHeader();

void setOutputFile(FILE *file);

extern void check_number_EQs(int nEQs);
extern void check_number_GEQs(int nGEQs);
extern void checkVars(int nVars);


} // end of namespace omega

#endif
