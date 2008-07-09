#include <basic/bool.h>
#include <omega/pres_conj.h>
#include <omega/RelBody.h>
#include <omega/omega_i.h>


namespace omega {


//
// Copy column fr_col of problem fp
// to   column to_col of problem tp.
// Displacement for constraints in tp are start_EQ and start_GEQ.
//
void copy_column(Problem *tp,  int to_col,
		 Problem *fp,  int fr_col,
		 int start_EQ, int start_GEQ) {
  checkVars(to_col);
  assert(start_EQ + fp->nEQs  <= tp->allocEQs);
  assert(start_GEQ + fp->nGEQs  <= tp->allocGEQs);

  int i;
  for(i=0; i<fp->nEQs; i++) {
    tp->EQs[i+start_EQ].coef[to_col] = fp->EQs[i].coef[fr_col];
  }
  for(i=0; i<fp->nGEQs; i++) {
    tp->GEQs[i+start_GEQ].coef[to_col] = fp->GEQs[i].coef[fr_col];
  }
}

//
// Zero column to_col of problem tp.
// Displacement for constraints in to_conj are start_EQ and start_GEQ.
// Number of constraints to zero are no_EQ and no_GEQ.
//
void zero_column(Problem *tp,  int to_col,
		 int start_EQ, int start_GEQ,
	         int no_EQs,   int no_GEQs) {
  assert(start_EQ + no_EQs <= tp->allocEQs);
  assert(start_GEQ + no_GEQs <= tp->allocGEQs);

  int i;
  for(i=0; i<no_EQs; i++) {
    tp->EQs[i+start_EQ].coef[to_col] = 0;
  }
  for(i=0; i<no_GEQs; i++) {
    tp->GEQs[i+start_GEQ].coef[to_col] = 0;
  }
}
//
// return column for D in conjunct
//
int Conjunct::get_column(Variable_ID D)
    {
    int col = find_column(D);
    if (col == 0)    // if it does not already have a column assigned
	col = map_to_column(D);
    assert(col > 0); // Not substituted
    return col;
    }

//
// Find column in conjunct.
//  
int Conjunct::find_column(Variable_ID D) {
  assert(D != 0);
  int column = mappedVars.index(D);
  
  // If it has been through the omega core (variablesInitialized), 
  // and it exists in the problem, check to see if it has been forwarded.
  // This will likely only be the case if substitutions have been done;
  // that won't arise in user code, only in print_with_subs and the
  // Substitutions class.
  if (problem->variablesInitialized && column > 0) {
    assert(problem->forwardingAddress[column] != 0); 
    column = problem->forwardingAddress[column];
  }
  assert (column <= problem->nVars);
  return column;
}
  
//
// Create new column in conjunct.
//  
int Conjunct::map_to_column(Variable_ID D) {
  assert(D != 0);
  // This heavy-duty assertion says that if you are trying to use a global
  // var's local representative in a relation, that you have first told the 
  // relation that you are using it here.  PUFS requires that we know
  // all the function symbols that might be used in a relation.
  // If one wanted to be less strict, one could just tell the relation 
  // that the global variable was being used.
  assert(D->kind() != Global_Var ||
	 (relation()->has_local(D->get_global_var(), D->function_of()) 
	  && "Attempt to update global var without a local variable ID"));

  cols_ordered = false; // extremely important
  checkVars(problem->nVars+2);
  int col = ++problem->nVars;
  mappedVars.append(D);
  problem->forwardingAddress[col] = col;
  problem->var[col] = col;
  zero_column(problem, col, 0, 0, problem->nEQs, problem->nGEQs);
  return col;
}

} // end of namespace omega

