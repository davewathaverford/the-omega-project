/* $Id: farkas.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <basic/Bag.h>
#include <basic/Map.h>
#include <omega.h>
#include <omega/farkas.h>

#ifdef WIN32
#define ONELONGLONG 1
#else
#define ONELONGLONG 1LL
#endif

namespace omega {

static Global_Var_Decl constant_term("constantTerm");
Global_Var_ID coefficient_of_constant_term = &constant_term;

int farkas_debug = 0;

coef_t farkasDifficulty;

//*****************************************************************************
//
// forall x1,..,xn s.t. a10 + a11 x1 + ... + a1n xn >= 0 and
//                                     ...
//                      am0 + am1 x1 + ... + amn xn >= 0
//
// b0 + b1 x1 + ... + bn xn >= 0 
//
//            iff
//
// exists lambda_0,...,lambda_m >= 0 s.t.
//     forall x1,..,xn 
//       lambda_0 +
//       lambda_1 ( a10 + a11 x1 + ... + a1n xn) +
//                                ...
//       lambda_m ( am0 + am1 x1 + ... + amn xn) =
//
//                   b0 +  b1 x1 + ... +  bn xn
//
//            iff
//
// exists lambda_0,...,lambda_m >= 0 s.t.
//   lambda_0 + sum_i ( lambda_i a_i0 ) = b_0
//   for j in 1..n
//       sum_i ( a_ij lambda_i ) = b_j
//
//            iff
//
// exists lambda0,...,lambda_m s.t.
//        lambda1,...,lambda_m >= 0 
//        lambda0 >= 0 
//   lambda_0 = b_0 - sum_i ( lambda_i a_i0 )
//   for j in 1..n
//       sum_i ( a_ij lambda_i ) = b_j
//            iff
//
// exists lambda1,...,lambda_m s.t.
//        lambda1,...,lambda_m >= 0 
//   b_0 - sum_i ( lambda_i a_i0 ) >= 0
//   for j in 1..n
//       sum_i ( a_ij lambda_i ) = b_j
//
// a_ij come from relation rel
//
// x_1,...,x_n are input and output variables from rel.
//
// b_0,...,b_m are input and output arrays of coef_vars
//
//*****************************************************************************


// Given a Relation/Set R
// Compute A,B,C such that
// Ax+By + C >= 0 is true for all x,y in R
// iff [A,B] : constantTerm=C is in AffineClosure(R)
// Note: constantTerm is a special global variable
// If constantTerm appears in the incoming relation
// then set it's coefficient to be 1 in the result


// # For example, given
// R := {[i,j] : 1 <= i <= 10 && 1 <= j <= n};
// # the farkas closure of R is:
// # ac := approximate {[i,j] : exists (lambda0, lambda1,lambda2,lambda3,lambda4 :
// #       0 <= lambda1,lambda2,lambda3,lambda4
// #       && constantTerm - (-lambda1+ 10 lambda2 - lambda3) >= 0
// #       && i = lambda1-lambda2
// #       && j = lambda3-lambda4
// #       && n = lambda4)};
// # 
// # ac;
// 
// {[i,j]: 0 <= n && 0 <= n+constantTerm+i+j 
//			&& 0 <= n+constantTerm+10i+j && 0 <= n+j}
// 
// The ConvexCombination of ac is:
//# 
//# approximate {[i,j] : exists (lambda1,lambda2,lambda3,lambda4 :
//#       0 <= lambda1,lambda2,lambda3,lambda4
//#       && 1 = lambda2+lambda3
//#       && i = lambda2+10lambda3
//#       && j = lambda2+lambda3+lambda4
//#       && n = lambda1+lambda2+lambda3+lambda4
//#       )};
//
//{[i,j]: 1 <= i <= 10 && 1 <= j <= n}
//

typedef Map<GEQ_Handle,Variable_ID> GEQmap; 
typedef Map<EQ_Handle,Variable_ID> EQmap;

static void handleVariable(Relation &farkas, Conjunct * conj,
			   F_And* and_node, 
			   GEQmap &gMap,
			   EQmap &eMap,
			   Variable_ID v) {
  use_ugly_names++;
  if (farkas_debug > 1) {
    fprintf(DebugFile,"Building equality for %s\n",
	    (const char *)(v)->name());
  }

  EQ_Handle e = and_node->add_EQ();

  if ((v)->kind() == Global_Var 
      && (v)->get_global_var() == coefficient_of_constant_term) 
    e.update_const(-1);
  else e.update_coef(farkas.get_local(v),-1);

  for (GEQ_Iterator g = conj->GEQs(); g.live(); g.next())
    if (gMap(*g) != (Variable_ID) 0)
      e.update_coef(gMap(*g),(*g).get_coef(v));
	   
  for (EQ_Iterator eq = conj->EQs(); eq.live(); eq.next())
    if (eMap(*eq) != (Variable_ID) 0)
      e.update_coef(eMap(*eq),(*eq).get_coef(v));

  e.finalize();
  if (farkas_debug > 1) {
    fprintf(DebugFile,"Constraint is %s\n",
	    (const char *)e.print_to_string());
  }
  use_ugly_names--;
}



typedef Set<Variable_ID> Variable_ID_Set;


Relation Farkas(NOT_CONST Relation &input_R, Farkas_Type op)
{
  assert(!input_R.is_null());
  use_ugly_names++;
  farkasDifficulty = 0;

  Relation R = consume_and_regurgitate(input_R);

  R = Approximate(R,false);
  R.simplify(0,2);

  Relation result = Relation::False(R);

  if (farkas_debug) {
    fprintf(DebugFile,"Computing farka of: [\n");
    R.prefix_print(DebugFile);
  }

  Variable_ID_Tuple vars;
  for (Variable_ID_Iterator v(*R.global_decls()); v; v++) vars.append(*v);
  if (R.is_set()) 
    for(int i=1; i <= R.n_set(); i++) vars.append(R.set_var(i));
  else {
    int i;
    for(i=1; i <= R.n_inp(); i++) vars.append(R.input_var(i));
    for(i=1; i <= R.n_out(); i++) vars.append(R.output_var(i));
  }
 
  Variable_ID_Set empty;
  Variable_ID_Tuple owners;
  Map<Variable_ID,Variable_ID_Set> connectedVariables(empty);

  if (op == Decoupled_Farkas) {
    for (Variable_ID_Iterator v(*R.global_decls()); v; v++) 
      if ((*v)->kind() == Global_Var) {
	Global_Var_ID g = (*v)->get_global_var();
	if (g->arity() > 0) {
	  if (R.is_set()) 
	    for(int i=1; i <= g->arity(); i++) 
	      (*v)->UF_union(R.set_var(i));
	  else if ((*v)->function_of() == Input_Tuple)
	    for(int i=1; i <= g->arity(); i++) 
	      (*v)->UF_union(R.input_var(i));
	  else
	    for(int i=1; i <= g->arity(); i++) 
	      (*v)->UF_union(R.output_var(i));
	}
      }
		
	

    for (DNF_Iterator s(R.query_DNF()); s.live(); s.next()) {
      for (Variable_ID_Iterator v1(*(*s)->variables()); v1; v1++) {
	for (EQ_Iterator eq = (*s)->EQs(); eq.live(); eq.next()) 
	  if ((*eq).get_coef(*v1)) 
	    for (Variable_ID_Iterator v2(*(*s)->variables()); v2; v2++) 
	      if ((*eq).get_coef(*v2)) 
		(*v1)->UF_union(*v2);
	for (GEQ_Iterator g = (*s)->GEQs(); g.live(); g.next()) 
	  if ((*g).get_coef(*v1)) 
	    for (Variable_ID_Iterator v2(*(*s)->variables()); v2; v2++) 
	      if ((*g).get_coef(*v2)) 
		(*v1)->UF_union(*v2);
      }
    }
    for (Variable_ID_Iterator v3(vars); v3.live(); v3.next()) 
      connectedVariables[(*v3)->UF_owner()] |= *v3;

    foreach_map(v,Variable_ID,s,Variable_ID_Set,connectedVariables,
		owners.append(v);
		if (farkas_debug) {
		  fprintf(DebugFile,"%s:",v->char_name());
		  foreach(v2,Variable_ID,s,
			  fprintf(DebugFile," %s",v2->char_name());
			  );
		  fprintf(DebugFile,"\n");
		}
		);

  }


  Variable_ID_Iterator varGroup(owners);
  int lambda_cnt = 1;
    
  Relation partialResult;


  bool firstGroup = true;

  while (op == Decoupled_Farkas && varGroup.live()
	 || op != Decoupled_Farkas && firstGroup) {

    if (farkas_debug && op == Decoupled_Farkas) {
	fprintf(DebugFile,"[Computing decoupled farkas for:");
	foreach(v2,Variable_ID,connectedVariables(varGroup.curr()),
			  fprintf(DebugFile," %s",v2->char_name());
			  );
	fprintf(DebugFile,"\n");
	}
    firstGroup = false;
    partialResult = Relation::True(R);
    coef_t difficulty = 0;
    for (DNF_Iterator s(R.query_DNF()); s.live(); s.next()) {
      int nz;
      coef_t m,sum;
      (*s)->difficulty(nz,m,sum);
      difficulty = max((coef_t) nz,2*nz+2*m+sum);
      if (farkas_debug) {
	fprintf(DebugFile,"Computing farka of conjunct: \n");
	(*s)->prefix_print(DebugFile);
	fprintf(DebugFile,"Difficulty is " coef_fmt "(%d," coef_fmt "," coef_fmt ")\n",
		difficulty,nz,m,sum);
      }
      if (difficulty >= 500)  {
	farkasDifficulty = difficulty;
        if (farkas_debug) {
	   fprintf(DebugFile,"Too ugly, returning dull result\n");
	   }
        use_ugly_names--;
  	if (op == Basic_Farkas || op == Decoupled_Farkas) 
		return Relation::False(partialResult);
	else return Relation::True(partialResult);
	}
      Relation farkas = Relation::Empty(R);
      farkas.copy_names(R);
      F_Exists* exist = farkas.add_exists();
      F_And* and_node = exist->add_and();
      GEQmap gMap((Variable_ID)0);
      EQmap eMap((Variable_ID)0);
      for (EQ_Iterator eq = (*s)->EQs(); eq.live(); eq.next()) {
	if (op == Decoupled_Farkas) {
	  bool ShouldConsider = true;
	  for (Variable_ID_Iterator v(*(*s)->variables()); v; v++) {
	    if ((*eq).get_coef(*v) != 0 
		&& (*v)->UF_owner() != varGroup.curr())  {
	      ShouldConsider = false;
	      break;
	    }
	  }
	  if (!ShouldConsider) continue;
	}
	char s[10];
	sprintf(s, "lambda%d", lambda_cnt++);
	eMap[*eq] = exist->declare(s);
	assert(op == Basic_Farkas || op == Decoupled_Farkas 
	       || (*eq).get_const() == 0);
      }
      for (GEQ_Iterator g = (*s)->GEQs(); g.live(); g.next()) {
	if (op == Decoupled_Farkas) {
	  bool ShouldConsider = true;
	  for (Variable_ID_Iterator v(*(*s)->variables()); v; v++) {
	    if ((*g).get_coef(*v) != 0 
		&& (*v)->UF_owner() != varGroup.curr())  {
	      ShouldConsider = false;
	      break;
	    }
	  }
	  if (!ShouldConsider) continue;
	}
	char s[10];
	sprintf(s, "lambda%d", lambda_cnt++);
	Variable_ID lambda = exist->declare(s);
	GEQ_Handle positive;
	switch(op) {
	case Positive_Combination_Farkas:
	case Convex_Combination_Farkas:
	case Basic_Farkas: 
	case Decoupled_Farkas: 
	  positive = and_node->add_GEQ();
	  positive.update_coef(lambda, 1);
	  positive.finalize();
	  break;
	case Linear_Combination_Farkas:
	case Affine_Combination_Farkas: 
	  break;
	}
	gMap[*g] = lambda;
	assert(op == Basic_Farkas || op == Decoupled_Farkas 
	       || (*g).get_const() == 0);
      }

      for (Variable_ID_Iterator v(vars); v; v++) {
	assert((*v)->kind() != Wildcard_Var);
	if ((*v)->kind() == Global_Var 
	    && (*v)->get_global_var() == coefficient_of_constant_term)  {
	  assert(op != Basic_Farkas && op != Decoupled_Farkas);
	  if (op == Linear_Combination_Farkas) continue;
	  if (op == Positive_Combination_Farkas) continue;
	}
        if (op == Decoupled_Farkas && (*v)->UF_owner() != varGroup.curr()) {
	  EQ_Handle e = and_node->add_EQ();
	  e.update_coef(farkas.get_local(*v),-1);
	  continue;
	}
	handleVariable(farkas, *s, and_node, gMap,eMap, *v);
      }

      if (op == Basic_Farkas ||
	  op == Decoupled_Farkas) {
	GEQ_Handle e = and_node->add_GEQ();
	e.update_coef(farkas.get_local(coefficient_of_constant_term),1);
	for (GEQ_Iterator g = s.curr()->GEQs(); g.live(); g.next()) 
	  if (gMap(*g) != (Variable_ID) 0)
	    e.update_coef( gMap(*g),-(*g).get_const());
	for (EQ_Iterator eq = s.curr()->EQs(); eq.live(); eq.next()) 
	  if (eMap(*eq) != (Variable_ID) 0)
	    e.update_coef(eMap(*eq),-(*eq).get_const());
	e.finalize();
      }
      farkas.simplify(-ONELONGLONG,-ONELONGLONG);
      farkas.single_conjunct()->difficulty(nz,m,sum);
      difficulty = max((coef_t) nz,2*nz+2*m+sum);
      if (farkas_debug) {
	fprintf(DebugFile,"farka has difficulty " coef_fmt "(%d," coef_fmt "," coef_fmt "):\n",
		difficulty,nz,m,sum);
	farkas.prefix_print(DebugFile);
      }
      if (difficulty >= 500)  {
        farkasDifficulty = difficulty;
        if (farkas_debug) {
	   fprintf(DebugFile,"Too ugly, returning dull result\n");
	   }
        use_ugly_names--;
  	if (op == Basic_Farkas || op == Decoupled_Farkas) 
		return Relation::False(partialResult);
	else return Relation::True(partialResult);
	}
      farkas = Approximate(farkas);
      if (farkas_debug) {
	fprintf(DebugFile,"simplified:\n");
	farkas.prefix_print(DebugFile);
      }
      partialResult = Approximate(Intersection(partialResult,farkas));
      if (farkas_debug) {
	fprintf(DebugFile,"combined:\n");
	partialResult.prefix_print(DebugFile);
      }
    if (partialResult.has_single_conjunct())
      {
	partialResult.single_conjunct()->difficulty(nz,m,sum);
	difficulty = max((coef_t) nz,2*nz+2*m+sum);
      }
    else 
      difficulty = 1000;
    if (difficulty >= 500)  {
	farkasDifficulty = difficulty;
        if (farkas_debug) {
	   fprintf(DebugFile,"Too ugly, returning dull result\n");
	   }
        use_ugly_names--;
  	if (op == Basic_Farkas || op == Decoupled_Farkas) 
		return Relation::False(partialResult);
	else return Relation::True(partialResult);
	}
    }
    farkasDifficulty += difficulty;

	
    if (farkas_debug) {
      fprintf(DebugFile,"] done computing farkas\n");
      partialResult.prefix_print(DebugFile);
    }

    if (op == Decoupled_Farkas) {
      result = Union(result,partialResult);
      varGroup.next();
    }
  } // while loop

  if (1 || op == Decoupled_Farkas)  {
    foreach(v,Variable_ID,vars, reset_remap_field(v));
  }
  use_ugly_names--;
  if (op == Decoupled_Farkas)  {
    if (farkas_debug) {
      fprintf(DebugFile,"] decoupled result:\n");
      result.prefix_print(DebugFile);
    }
    return result;
  }
  return partialResult;
} /* farkas_lemma */


} // end of namespace omega
