#include <omega/omega_core/oc_i.h> 

namespace omega {

Problem::~Problem()
    {
    delete [] EQs;
    delete [] GEQs;
    }



void
check_number_EQs(int n) {
    if (n < 0) {
	fprintf(stderr,"ERROR: nEQs < 0??\n");
	Exit(1);
    }

    if (n > maxmaxEQs) {
	fprintf(stderr, "\nERROR:\n");
	fprintf(stderr, "An attempt was made to set the number of available equality constraints to %d.\n", n);
	fprintf(stderr, "The maximum number of equality constraints in a conjunction is %d.\n", maxmaxEQs);
	fprintf(stderr, "This limit can be changed by redefining maxmaxEQs in oc.h and recompiling.\n\n");
	Exit(2);
    }
}

void
check_number_GEQs(int n) {
    if (n < 0) {
	fprintf(stderr,"ERROR: nGEQs < 0??\n");
	Exit(1);
    }

    if (n > maxmaxGEQs) {
	fprintf(stderr, "\nERROR:\n");
	fprintf(stderr, "An attempt was made to set the number of available inequality constraints to %d.\n", n);
	fprintf(stderr, "The maximum number of inequality constraints in a conjunction is %d.\n", maxmaxGEQs);
	fprintf(stderr, "This limit can be changed by redefining maxmaxGEQs in oc.h and recompiling.\n\n");
	Exit(2);
    }
}


void check_number_EQs_GEQs(int e, int g) {
    check_number_EQs(e);
    check_number_GEQs(g);
}


Problem::Problem(int in_eqs, int in_geqs)
    {
    check_number_EQs_GEQs(in_eqs, in_geqs);
    allocEQs = padEQs(in_eqs);
    allocGEQs = padGEQs(in_geqs);
    EQs = new eqn [allocEQs];
    GEQs = new eqn [allocGEQs];
    nVars = 0;
    mHashVersion = hashVersion;
    variablesInitialized = 0;
    variablesFreed = 0;
    varsOfInterest = 0;
    safeVars = 0;
    nEQs = 0;
    nGEQs = 0;
    nSUBs = 0;
    nMemories = 0;
    }

Problem::Problem(const Problem & p2)
    {
    allocEQs = padEQs(p2.nEQs); // Don't over-allocate; p2 might have too many!
    allocGEQs = padGEQs(p2.nGEQs);
    EQs = new eqn [allocEQs];
    GEQs = new eqn [allocGEQs];
    int e, i;
    nVars = p2.nVars;
    mHashVersion = p2.mHashVersion;
    variablesInitialized = p2.variablesInitialized;
    variablesFreed = p2.variablesFreed;
    varsOfInterest = p2.varsOfInterest;
    safeVars = p2.safeVars;
    nEQs = p2.nEQs;
    //nSUBs = 0;
    for (e = p2.nEQs - 1; e >= 0; e--)
        eqnncpy(&(EQs[e]), &(p2.EQs[e]), p2.nVars);
    nGEQs = p2.nGEQs;
    for (e = p2.nGEQs - 1; e >= 0; e--)
        eqnncpy(&(GEQs[e]), &(p2.GEQs[e]), p2.nVars);
    for (i = 0; i <= p2.nVars; i++)
        var[i] = p2.var[i];
    for (i = 0; i <= maxVars; i++)
        forwardingAddress[i] = p2.forwardingAddress[i];
    //nMemories = 0;
    get_var_name = p2.get_var_name;
    getVarNameArgs = p2.getVarNameArgs;
    }

Problem & Problem::operator=(const Problem & p2)
    {
    if (this != &p2) {
      if(allocEQs < p2.nEQs) {
	  delete [] EQs;
	  allocEQs = padEQs(p2.nEQs);
	  EQs = new eqn[allocEQs];
      }
      if(allocGEQs < p2.nGEQs) {
	  delete [] GEQs;
	  allocGEQs = padGEQs(p2.nGEQs);
	  GEQs = new eqn[allocGEQs];
      }
      int e, i;
      nVars = p2.nVars;
      mHashVersion = p2.mHashVersion;
      variablesInitialized = p2.variablesInitialized;
      variablesFreed = p2.variablesFreed;
      varsOfInterest = p2.varsOfInterest;
      safeVars = p2.safeVars;
      nEQs = p2.nEQs;
      //nSUBs = 0;
      for (e = p2.nEQs - 1; e >= 0; e--)
          eqnncpy(&(EQs[e]), &(p2.EQs[e]), p2.nVars);
      nGEQs = p2.nGEQs;
      for (e = p2.nGEQs - 1; e >= 0; e--)
          eqnncpy(&(GEQs[e]), &(p2.GEQs[e]), p2.nVars);
      for (i = 0; i <= p2.nVars; i++)
          var[i] = p2.var[i];
      for (i = 0; i <= maxVars; i++)
          forwardingAddress[i] = p2.forwardingAddress[i];
      //nMemories = 0;
      get_var_name = p2.get_var_name;
      getVarNameArgs = p2.getVarNameArgs;
      }
    return *this;
    }


void Problem::zeroVariable(int i) 
    {
    int e;
    for (e = nGEQs - 1; e >= 0; e--) GEQs[e].coef[i] = 0;
    for (e = nEQs - 1; e >= 0; e--) EQs[e].coef[i] = 0;
    for (e = nSUBs - 1; e >= 0; e--) SUBs[e].coef[i] = 0;
    }
	
/* Functions for allocating EQ's and GEQ's */


int Problem::newGEQ() {
    if (++nGEQs > allocGEQs) {
  	check_number_GEQs(nGEQs);
	allocGEQs = padGEQs(allocGEQs, nGEQs);
	assert(allocGEQs >= nGEQs);
	eqn * new_geqs = new eqn[allocGEQs];
	for (int e = nGEQs - 2; e >= 0; e--)
	    eqnncpy(&(new_geqs[e]), &(GEQs[e]), nVars);
	delete [] GEQs; 
	GEQs = new_geqs;
    }
//    problem->GEQs[nGEQs-1].color = black;
//    eqnnzero(&problem->GEQs[nGEQs-1],problem->nVars);
    return nGEQs-1;
}

int Problem::newEQ(){
    if (++nEQs > allocEQs) {
	check_number_EQs(nEQs);
	allocEQs = padEQs(allocEQs, nEQs);
	assert(allocEQs >= nEQs);
	eqn * new_eqs = new eqn[allocEQs];
	for (int e = nEQs - 2; e >= 0; e--)
	    eqnncpy(&(new_eqs[e]), &(EQs[e]), nVars);
	delete [] EQs; 
	EQs = new_eqs;
    }
// Could do this here, but some calls to newEQ do a copy instead of a zero;
//    problem->EQs[nEQs-1].color = black;
//    eqnnzero(&problem->EQs[nEQs-1],problem->nVars);
    return nEQs-1;
}

} // end of namespace omega
