/* $Id: Relations.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <omega/Relation.h>
#include <omega/Rel_map.h>
#include <omega/pres_tree.h>
#include <omega/pres_dnf.h>
#include <omega/pres_conj.h>
#include <omega/hull.h>
#include <basic/assert.h>
#include <basic/Exit.h>
#include <basic/Tuple.h>
#include <basic/Map.h>
#include <basic/util.h>
#include <omega/omega_i.h>
#if defined STUDY_EVACUATIONS
#include <omega/evac.h>
#endif

#define CHECK_MAYBE_SUBSET 1

namespace omega {

int relation_debug=0;

static int leave_pufs_untouched = 0;
Variable_ID_Tuple exists_ids; 
List<int> exists_numbers;
F_And * and_below_exists;

#define Assert(c,t) if(!(c)) PresErrAssert(t)
void PresErrAssert(char *t);


/* The following allows us to avoid warnings about passing 
   temporaries as non-const references.  This is useful but 
   has suddenly become illegal.  */

Relation consume_and_regurgitate(NOT_CONST Relation &R) {
  if(!R.is_null())
    ((Relation &) R).finalize();
  Relation S = (Relation &) R;
  (Relation &) R = Relation::Null();
  return S;
}


//
//      r1 Union r2.
// 		align the input tuples (if any) for F and G
// 		align the output tuples (if any) for F and G
// 		match named variables in F and G
// 		formula is f | g
//
Relation Union(NOT_CONST Relation &input_r1, 
	       NOT_CONST Relation &input_r2) {
  Relation r1 = consume_and_regurgitate(input_r1);
  Relation r2 = consume_and_regurgitate(input_r2);
  skip_set_checks++;
  assert(r1.n_inp() == r2.n_inp());
  assert(r1.n_out() == r2.n_out());
  assert(!r1.is_null() && !r2.is_null());
  int in = r1.n_inp(), out = r1.n_out();
  skip_set_checks--;

  return MapAndCombineRel2(r1, r2, Mapping::Identity(in, out),
			   Mapping::Identity(in,out), Comb_Or);
}


//
// 	F intersection G
// 		align the input tuples (if any) for F and G
// 		align the output tuples (if any) for F and G
// 		match named variables in F and G
// 		formula is f & g
//
Relation Intersection(NOT_CONST Relation &input_r1,
		      NOT_CONST Relation &input_r2) {
  Relation r1 = consume_and_regurgitate(input_r1);
  Relation r2 = consume_and_regurgitate(input_r2);
  skip_set_checks++;
  assert(r1.n_inp() == r2.n_inp());
  assert(r1.n_out() == r2.n_out());
  assert(!r1.is_null() && !r2.is_null());
  int in = r1.n_inp(), out = r1.n_out();
  skip_set_checks--;

  return MapAndCombineRel2(r1, r2, Mapping::Identity(in,out),
			   Mapping::Identity(in,out), Comb_And);
}


//
// 	F \ G (the relation F restricted to domain G)
// 		align the input tuples for F and G
// 		match named variables in F and G
// 		formula is f & g
//
Relation Restrict_Domain(NOT_CONST Relation &input_r1,
			 NOT_CONST Relation &input_r2) {
  Relation r1 = consume_and_regurgitate(input_r1);
  Relation r2 = consume_and_regurgitate(input_r2);

  assert(!r1.is_null() && !r2.is_null());
  skip_set_checks++;
  assert(r1.n_inp() == r2.n_set());
  assert(r2.is_set());
  int in = r1.n_inp(), out = r1.n_out();
  skip_set_checks--;

  int i;
  Mapping m2(r2.n_set());
  for(i=1; i<=r2.n_set(); i++) m2.set_map_set(i, Input_Var,i);

  skip_set_checks++;
  assert(r2.query_guaranteed_leading_0s() == -1 &&
	 r2.query_possible_leading_0s() == -1);
  skip_set_checks--;

  Relation result = MapAndCombineRel2(r1, r2, Mapping::Identity(in,out),
				      m2, Comb_And);
  // FERD -- update leading 0's - the may close up?
  //result.invalidate_leading_info();  // could do better
  return result;
}

// 
// 	
// 	F / G (the relation F restricted to range G)
// 		align the output tuples for F and G
// 		match named variables in F and G
// 		formula is f & g
//
Relation Restrict_Range(NOT_CONST Relation &input_r1,
			NOT_CONST Relation &input_r2) {
  Relation r1 = consume_and_regurgitate(input_r1);
  Relation r2 = consume_and_regurgitate(input_r2);

  skip_set_checks++;
  assert(r1.n_out() == r2.n_set());
  assert(r2.is_set());
  assert(!r1.is_null() && !r2.is_null());
  int in = r1.n_inp(), out = r1.n_out();
  skip_set_checks--;

  int i;
  Mapping m2(r2.n_set());
  for(i=1; i<=r2.n_set(); i++) m2.set_map_set(i, Output_Var,i);

  skip_set_checks++;
  assert(r2.query_guaranteed_leading_0s() == -1 &&
	 r2.query_possible_leading_0s() == -1);
  skip_set_checks--;

  Relation result = MapAndCombineRel2(r1, r2, Mapping::Identity(in, out),
				      m2, Comb_And);
  // FERD -- update leading 0's - the may close up?
  // result.invalidate_leading_info();  // could do better
  return result;
}


//
// Add input variable to relation.
//
Relation Extend_Domain(NOT_CONST Relation &S) {
  Relation R = consume_and_regurgitate(S);
  assert(!R.is_null() && (skip_set_checks || !R.is_set()));
  Rel_Body *r = R.split();
  r->In_Names.append(Const_String());
  r->number_input++;
  assert(!r->is_null());

  if (r->number_input <= r->number_output)
	  R.invalidate_leading_info(r->number_input);

  return R;
}

//
// Add more input variables to relation.
//
Relation Extend_Domain(NOT_CONST Relation &S, int more) {
  Relation R = consume_and_regurgitate(S);
  assert(!R.is_null());
  R.split();
  for (int i=1; i<=more; i++) R = Extend_Domain(R);
  return R;
}


//
// Add output variable to relation.
//
Relation Extend_Range(NOT_CONST Relation &S) {
  Relation R = consume_and_regurgitate(S);
  assert(!R.is_null() && !R.is_set());
  Rel_Body *r = R.split();
  r->Out_Names.append(Const_String());
  r->number_output++;
  assert(!r->is_null());

  if (r->number_output <= r->number_input)
	  R.invalidate_leading_info(r->number_output);

  return R;
}

//
// Add more output variables to relation.
//
Relation Extend_Range(NOT_CONST Relation &S, int more) {
  Relation R = consume_and_regurgitate(S);
  assert(!R.is_null());
  R.split();
  for (int i=1; i<=more; i++) R = Extend_Range(R);
  return R;
}


//
// Add set variable to set.
//
Relation Extend_Set(NOT_CONST Relation &S) {
  Relation R = consume_and_regurgitate(S);
  assert(!R.is_null() && R.is_set());
  Rel_Body *r = R.split();
  r->In_Names.append(Const_String());
  r->number_input++;
  assert(!r->is_null());
  return R;
}

//
// Add more variables to set
//
Relation Extend_Set(NOT_CONST Relation &S, int more) {
  Relation R = consume_and_regurgitate(S);
  R.split();
  for (int i=1; i<=more; i++) R = Extend_Set(R);
  return R;
}



//
// Domain and Range.
// Make output (input) variables wildcards and simplify.
// Move all UFS's to have have the remaining tuple as an argument,
//   and maprel will move them to the set tuple
// RESET all leading 0's
//
Relation Domain(NOT_CONST Relation &S)
    {
    assert(!S.is_null());
    assert(!S.is_set());
    Relation r = consume_and_regurgitate(S);
    skip_set_checks++;
    int i;
    Mapping m1(r.n_inp(), r.n_out());
    for(i=1; i<=r.n_inp(); i++) m1.set_map_in (i, Set_Var,i);
    for(i=1; i<=r.n_out(); i++) m1.set_map_out(i, Exists_Var,i);
    skip_set_checks--;

    int a = r.max_ufs_arity_of_out();
    if (a > 0)
	{
	// UFS's must evacuate from the output tuple

	Variable_ID_Tuple remapped;

	r.simplify();
	DNF *d = r.split()->DNFize();
	d->count_leading_0s();
	// Any conjucts with leading_0s == -1 must have >= "a" leading 0s
	// What a gross way to do this. Ferd

	for (DNF_Iterator conj(d); conj; conj++)
	    {
#if defined STUDY_EVACUATIONS
	    study_evacuation(*conj, out_to_in, a);
#endif

	    int cL0 = (*conj)->guaranteed_leading_0s;

	    for (Variable_ID_Iterator func((*conj)->mappedVars); func; func++)
		if ((*func)->kind() == Global_Var)
		    {
		    Global_Var_ID f = (*func)->get_global_var();
		    if (f->arity() > 0 && (*func)->function_of()==Output_Tuple)
			{
			if (cL0 >= f->arity())
			    {
			    (*func)->remap = r.get_local(f, Input_Tuple);
			    }
			else
			    {
			    (*func)->remap = (*conj)->declare();
			    (*conj)->make_inexact();
			    }
			remapped.append(*func);
			}
		    }
	    (*conj)->remap();
	    reset_remap_field(remapped);
	    remapped.clear();
	    
	    (*conj)->guaranteed_leading_0s =  
		(*conj)->possible_leading_0s = -1;
	    (*conj)->leading_dir = 0;
	    }
	}

    MapRel1(r, m1, Comb_Id); //  this invalidates leading0s
    assert(r.is_set() || m1.n_in() == 0);  // MapRel can't tell to make a set
    r.markAsSet();                         // if there were no inputs.      

    skip_set_checks++;
    assert(r.query_guaranteed_leading_0s() == -1 &&
	   r.query_possible_leading_0s() == -1);
    skip_set_checks--;

    return r;
    }


Relation Range(NOT_CONST Relation &S)
    {
    Relation r = consume_and_regurgitate(S);
    assert(!r.is_null());
    skip_set_checks++;

    int i;
    Mapping m1(r.n_inp(), r.n_out());
    for(i=1; i<=r.n_inp(); i++) m1.set_map_in (i, Exists_Var,i);
    for(i=1; i<=r.n_out(); i++) m1.set_map_out(i, Set_Var,i);
    skip_set_checks--;

    int a = r.max_ufs_arity_of_in();
    if (a > 0)
	{
	// UFS's must evacuate from the input tuple

	Variable_ID_Tuple remapped;

	r.simplify();
	DNF *d = r.split()->DNFize();
	d->count_leading_0s();
	// Any conjucts with leading_0s == -1 must have >= "a" leading 0s
	// What a gross way to do this. Ferd

	for (DNF_Iterator conj(d); conj; conj++)
	    {
#if defined STUDY_EVACUATIONS
	    study_evacuation(*conj, in_to_out, a);
#endif

	    int cL0 = (*conj)->guaranteed_leading_0s;
	    for (Variable_ID_Iterator func((*conj)->mappedVars); func; func++)
		if ((*func)->kind() == Global_Var)
		    {
		    Global_Var_ID f = (*func)->get_global_var();
		    if (f->arity() > 0 && (*func)->function_of()==Input_Tuple)
			{
			if (cL0 >= f->arity())
			    {
			    (*func)->remap = r.get_local(f, Output_Tuple);
			    }
			else
			    {
			    (*func)->remap = (*conj)->declare();
			    (*conj)->make_inexact();
			    }
			remapped.append(*func);
			}
		    }
	    (*conj)->remap();
	    reset_remap_field(remapped);
	    remapped.clear();

	    (*conj)->guaranteed_leading_0s =  
		(*conj)->possible_leading_0s = -1;
	    (*conj)->leading_dir = 0;
	    }
	}

    MapRel1(r, m1, Comb_Id); // this invalidates leading0s
    assert(r.is_set() || m1.n_out() == 0); // MapRel can't tell to make a set
    r.markAsSet();                        // if there were no outputs.

    skip_set_checks++;
    assert(r.query_guaranteed_leading_0s() == -1 &&
	   r.query_possible_leading_0s() == -1);
    skip_set_checks--;

    return r;

    }


// Cross Product.  Give two sets, A and B, create a relation whose
// domain is A and whose range is B.
Relation Cross_Product(NOT_CONST Relation &input_A,
		       NOT_CONST Relation &input_B) {
  Relation A = consume_and_regurgitate(input_A);
  Relation B = consume_and_regurgitate(input_B);

    assert(A.is_set());
    assert(B.is_set());

    skip_set_checks++;
    assert(A.query_guaranteed_leading_0s() == -1 &&
	   A.query_possible_leading_0s() == -1);
    assert(B.query_guaranteed_leading_0s() == -1 &&
	   B.query_possible_leading_0s() == -1);
    skip_set_checks--;

    Mapping mA(A.n_set());
    Mapping mB(B.n_set()); 
    int i;
    for(i = 1; i <= B.n_set(); i++) mB.set_map_set(i, Output_Var,i);
    for(i = 1; i <= A.n_set(); i++) mA.set_map_set(i, Input_Var,i);
    return MapAndCombineRel2(A, B, mA, mB, Comb_And);
}


//
// 	inverse F
// 		reverse the input and output tuples
//
Relation Inverse(NOT_CONST Relation &S) {
  Relation r = consume_and_regurgitate(S);
  assert(!r.is_null());
  assert(!r.is_set());
  int i;

  Mapping m1(r.n_inp(), r.n_out());
  for(i=1; i<=r.n_inp(); i++) m1.set_map_in (i, Output_Var,i);
  for(i=1; i<=r.n_out(); i++) m1.set_map_out(i, Input_Var,i);

  MapRel1(r, m1, Comb_Id, -1, -1, false);

  r.reverse_leading_dir_info();

  return r;
}

Relation After(NOT_CONST Relation &input_S,
	       int carried_by, int new_output,int dir) {
  Relation S = consume_and_regurgitate(input_S);
  assert(!S.is_null());
  assert(!S.is_set());
  int i;
  Relation r(*S.split(),42);

    int a = r.max_ufs_arity_of_out();
    int preserved_positions = min(carried_by-1,new_output);
    if (a >= preserved_positions)
	{
	// UFS's must evacuate from the output tuple

	Variable_ID_Tuple remapped;

	r.simplify();
	DNF *d = r.split()->DNFize();
	d->count_leading_0s();
	// Any conjucts with leading_0s == -1 must have >= "a" leading 0s
	// What a gross way to do this. Ferd

	for (DNF_Iterator conj(d); conj; conj++)
	    {

	    int cL0 = (*conj)->guaranteed_leading_0s;

	    for (Variable_ID_Iterator func((*conj)->mappedVars); func; func++)
		if ((*func)->kind() == Global_Var)
		    {
		    Global_Var_ID f = (*func)->get_global_var();
		    if (f->arity() > preserved_positions 
			&& (*func)->function_of()==Output_Tuple)
			{
			if (cL0 >= f->arity())
			    {
			    (*func)->remap = r.get_local(f, Input_Tuple);
			    }
			else
			    {
			    (*func)->remap = (*conj)->declare();
			    (*conj)->make_inexact();
			    }
			remapped.append(*func);
			}
		    }
	    (*conj)->remap();
	    reset_remap_field(remapped);
	    remapped.clear();
	    
	    (*conj)->guaranteed_leading_0s =  
		(*conj)->possible_leading_0s = -1;
	    (*conj)->leading_dir = 0;
	    }
	}

  Mapping m1(r.n_inp(), r.n_out());
  for(i=1; i<=r.n_inp(); i++) m1.set_map_in (i, Input_Var,i);
  if (carried_by > new_output) {
	  int preserve = min(new_output,r.n_out());
	  for(i=1; i<=preserve; i++) m1.set_map_out(i, Output_Var,i);
	  for(i=preserve+1; i<=r.n_out(); i++) m1.set_map_out(i, Exists_Var,-1);
          MapRel1(r, m1, Comb_Id, -1, -1, true);
	  if (new_output > preserve)
		  r = Extend_Range(r,new_output-r.n_out());
	  return r;
	}

  for(i=1; i<carried_by; i++) m1.set_map_out(i, Output_Var,i);
  m1.set_map_out(carried_by, Exists_Var,1);
  for(i=carried_by+1; i<=r.n_out(); i++) m1.set_map_out(i, Exists_Var,-1);

  MapRel1(r, m1, Comb_Id, -1, -1, true,false);

  Rel_Body *body = r.split();
  body->Out_Names.append(Const_String());
  body->number_output++;
  assert(body->n_out() <= input_vars.size());


  GEQ_Handle h = and_below_exists->add_GEQ(0);
  assert(carried_by < 128);
  h.update_coef(exists_ids[1],-dir);
  h.update_coef(r.output_var(carried_by),dir);
  h.update_const(-1);
  h.finalize();
  r.finalize();
  if (new_output > r.n_out())
	  r = Extend_Range(r,new_output-r.n_out());
  return r;
}

//
// Identity.
//
Relation Identity(int n_inp) {
  Relation rr(n_inp, n_inp);
  F_And *f = rr.add_and();
  for(int i=1; i<=n_inp; i++) {
    EQ_Handle e = f->add_EQ();
    e.update_coef(rr.input_var(i), -1);
    e.update_coef(rr.output_var(i), 1);
    e.finalize();
  }
  rr.finalize();
  assert(!rr.is_null());
  return rr;
}

Relation Identity(NOT_CONST Relation &input_r) {
  Relation   r = consume_and_regurgitate(input_r);
  return Restrict_Domain(Identity(r.n_set()),r);
}

//
// Deltas(F)
//   Return a set such that the ith variable is old Out_i - In_i
//   Delta variables are created as input variables.
//   Then input and output variables are projected out.
//
Relation Deltas(NOT_CONST Relation &S) {
  Relation R = consume_and_regurgitate(S);
  assert(!R.is_null());
  skip_set_checks++;
  assert(R.n_inp()==R.n_out());
  int in = R.n_inp();
  skip_set_checks--;
  return Deltas(R,in);
}

Relation Deltas(NOT_CONST Relation &S, int eq_no) {
  Relation R = consume_and_regurgitate(S);
  skip_set_checks++;
  assert(!R.is_null());
  assert(eq_no<=R.n_inp());
  assert(eq_no<=R.n_out());
  R.split();

  int no_inp = R.n_inp();
  int no_out = R.n_out();

  if(relation_debug) {
        fprintf(DebugFile,"Computing Deltas:\n");
        R.prefix_print(DebugFile);
        };
  int a = R.max_ufs_arity();
  if (a > 0)
      {
      Variable_ID_Tuple remapped;
      
      // UFS's must evacuate from all tuples - we need to go to DNF
      // to enumerate the variables, I think...
      R.simplify();
	  if(relation_debug) {
		fprintf(DebugFile,"Relation simplified:\n");
		R.prefix_print(DebugFile);
		};
      DNF *d = R.split()->DNFize();

      for (DNF_Iterator conj(d); conj; conj++)
	  {
	  for (Variable_ID_Iterator func((*conj)->mappedVars); func; func++)
	      if ((*func)->kind() == Global_Var)
		  {
		  Global_Var_ID f = (*func)->get_global_var();
		  if (f->arity() > 0)
			{
			(*func)->remap = (*conj)->declare();
			(*conj)->make_inexact();
			remapped.append(*func);
			}
		  }
	  (*conj)->remap();
	  reset_remap_field(remapped);
	  remapped.clear();
	  }
      }

  R = Extend_Domain(R, eq_no);                // add eq_no Delta vars
  Mapping M(no_inp+eq_no, no_out);
  int i;
  for(i=1; i<=eq_no; i++) {          // Set up Deltas equalities
    EQ_Handle E = R.and_with_EQ();
    /* delta_i - w_i + r_i = 0 */
    E.update_coef(R.input_var(i), 1);
    E.update_coef(R.output_var(i), -1);
    E.update_coef(R.input_var(no_inp+i), 1);
    E.finalize();
    M.set_map(Input_Var, no_inp+i, Set_Var, i);  // Result will be a set
  }
  for(i=1; i<=no_inp; i++) {	         // project out input variables
    M.set_map(Input_Var, i, Exists_Var, i);
  }
  for(i=1; i<=no_out; i++) {	         // project out output variables
    M.set_map(Output_Var, i, Exists_Var, no_inp+i);
  }
  MapRel1(R, M, Comb_Id, eq_no, 0);

  if(relation_debug) {
	fprintf(DebugFile,"Computing deltas:\n");
	R.prefix_print(DebugFile);
	};
  R.finalize();
  assert(R.is_set());  // Should be since we map things to Set_Var
  assert(R.n_set() == eq_no);
  skip_set_checks--;
  return R;
}




Relation DeltasToRelation(NOT_CONST Relation &D, int n_inputs, int n_outputs) {
  Relation R = consume_and_regurgitate(D);
  skip_set_checks++;
  assert(!R.is_null());
  R.markAsRelation();
  int common = R.n_inp();
  assert(common <= n_inputs);
  assert(common <= n_outputs);
  R.split();

  if (R.max_ufs_arity() > 0)
      {
      assert(R.max_ufs_arity() == 0 &&
             "'Deltas' not ready for UFS yet"); // FERD
      fprintf(stderr, "'Deltas' not ready for UFS yet");
      Exit(1);
      }

  R = Extend_Domain(R, n_inputs);
  R = Extend_Range(R, n_outputs);
  Mapping M(common+n_inputs, n_outputs);
  int i;
  for(i=1; i<=common; i++) {          // Set up Deltas equalities
    EQ_Handle E = R.and_with_EQ();
    /* delta_i - w_i + r_i = 0 */
    E.update_coef(R.input_var(i), 1);
    E.update_coef(R.output_var(i), -1);
    E.update_coef(R.input_var(common+i), 1);
    E.finalize();
    M.set_map(Input_Var, i, Exists_Var, i);  // Result will be a set
  }
  for(i=1; i<=n_inputs; i++) {           // project out input variables
    M.set_map(Input_Var, common+i, Input_Var, i);
  }
  for(i=1; i<=n_outputs; i++) {          // project out output variables
    M.set_map(Output_Var, i, Output_Var, i);
  }
  MapRel1(R, M, Comb_Id, n_inputs, n_outputs);

  if(relation_debug) {
        fprintf(DebugFile,"Computed DeltasToRelation:\n");
        R.prefix_print(DebugFile);
        };
  R.finalize();
  assert(!R.is_set());
  skip_set_checks--;
  return R;
}



Relation Join(NOT_CONST Relation &G, NOT_CONST Relation &F)
{
    return Composition(F, G);
}

bool prepare_relations_for_composition(Relation &r1,
				       Relation &r2)
    {
    assert(!r2.is_null() && !r1.is_null());

    if(r2.is_set())
	{
	int a1 = r1.max_ufs_arity_of_in(), a2 = r2.max_ufs_arity_of_set();

	if (a1 == 0 && a2 == 0)
	    return true;
	else
	    {
	    assert(0 &&
		   "Can't compose relation and set with function symbols");
	    fprintf(stderr,
		    "Can't compose relation and set with function symbols");
	    Exit(1);
	    return false;  // make compiler shut up
	    }
	}

    assert(r2.n_out() == r1.n_inp());


    int zeros = max(r1.query_guaranteed_leading_0s(),
			r2.query_guaranteed_leading_0s());
    return (zeros >= r1.max_ufs_arity_of_in()
		&& zeros >= r2.max_ufs_arity_of_out());
    }

//
// Composition(F, G) = F o G, where F o G (x) = F(G(x))
//	  That is, if F = { [i] -> [j] : ... }
//		  and G = { [x] -> [y] : ... }
//	         then Composition(F, G) = { [x] -> [j] : ... }
//
// 	align the output tuple for G and the input tuple for F,
// 		these become existensially quantified variables
// 	use the output tuple from F and the input tuple from G for the result
// 	match named variables in G and F
// 	formula is g & f
//
// If there are function symbols of arity > 0, we call special case
// code to handle them.  This is not set up for the r2.is_set case yet.
//

Relation Composition(NOT_CONST Relation &input_r1,
		     NOT_CONST Relation &input_r2)
    {
    Relation r1 = consume_and_regurgitate(input_r1);
    Relation r2 = consume_and_regurgitate(input_r2);
    assert(!r2.is_null() && !r1.is_null());

    if(r2.is_set())
	{
        int a1 = r1.max_ufs_arity_of_in(), a2 = r2.max_ufs_arity_of_set();
        if (r2.n_set() != r1.n_inp()) {
		fprintf(stderr,"Illegal composition/application, arities don't match\n");
		fprintf(stderr,"Trying to compute r1(r2)\n");
		fprintf(stderr,"arity of r2 must match input arity of r1\n");
		fprintf(stderr,"r1: ");
		r1.print_with_subs(stderr);
		fprintf(stderr,"r2: ");
		r2.print_with_subs(stderr);
		fprintf(stderr,"\n");
	        assert(r2.n_set() == r1.n_inp());
		Exit(1);
		}
	skip_set_checks++;
	int i;
	if (a1 == 0 && a2 == 0)
	    {
	    int x = r1.n_out();
	    Mapping m1(r1.n_inp(), r1.n_out());
	    for(i=1; i<=r1.n_out(); i++) m1.set_map_out(i, Set_Var,i);
	    for(i=1; i<=r1.n_inp(); i++) m1.set_map_in (i, Exists_Var,i);
	    Mapping m2(r2.n_set());
	    for(i=1; i<=r2.n_set(); i++) m2.set_map_set(i, Exists_Var,i);
	    Relation R3 = MapAndCombineRel2(r2, r1, m2, m1, Comb_And);
	    skip_set_checks--;
            if (x == 0)
		R3.markAsSet();
	    return R3;
	    }
	else
	    {
	    assert(0 &&
		   "Can't compose relation and set with function symbols");
	    fprintf(stderr,
		    "Can't compose relation and set with function symbols");
	    Exit(1);
	    return Identity(0);  // make compiler shut up
	    }
	}

    if (r2.n_out() != r1.n_inp()) {
		fprintf(stderr,"Illegal composition, arities don't match\n");
		fprintf(stderr,"Trying to compute r1 compose r2\n");
		fprintf(stderr,"Output arity of r2 must match input arity of r1\n");
		fprintf(stderr,"r1: ");
		r1.print_with_subs(stderr);
		fprintf(stderr,"r2: ");
		r2.print_with_subs(stderr);
		fprintf(stderr,"\n");
	        assert(r2.n_out() == r1.n_inp());
		Exit(1);
		}

    int a1 = r1.max_ufs_arity_of_in(), a2 = r2.max_ufs_arity_of_out();

    if (a1 == 0 && a2 == 0 && 0 /* FERD - leading 0's go wrong here */ )
	{
	// If no real UFS's, we can just use the general code:
	int i;
	Mapping m1(r1.n_inp(), r1.n_out());
	for(i=1; i<=r1.n_inp(); i++) m1.set_map_in (i, Exists_Var,i);
	for(i=1; i<=r1.n_out(); i++) m1.set_map_out(i, Output_Var,i);
	Mapping m2(r2.n_inp(), r2.n_out());
	for(i=1; i<=r2.n_inp(); i++) m2.set_map_in (i, Input_Var,i);
	for(i=1; i<=r2.n_out(); i++) m2.set_map_out(i, Exists_Var,i);

	return MapAndCombineRel2(r2, r1, m2, m1, Comb_And);
	}
    else
	{
	Relation result(r2.n_inp(), r1.n_out());
	int mid_size = r2.n_out();
        int i;
	for(i =1; i<=r2.n_inp(); i++)
		result.name_input_var(i,r2.input_var(i)->base_name);
	for(i =1; i<=r1.n_out(); i++)
		result.name_output_var(i,r1.output_var(i)->base_name);

	r1.simplify();
	r2.simplify();

	Rel_Body *b1 = r1.split(), *b2 = r2.split();

	if (b1 == b2)
	    {
	    assert(0 && "Compose: not ready to handle b1 == b2 yet.");
	    fprintf(stderr, "Compose: not ready to handle b1 == b2 yet.\n");
	    exit(1);
	    }

	DNF *d1 = b1->DNFize();
	DNF *d2 = b2->DNFize();
      
	d1->count_leading_0s();
	d2->count_leading_0s();
	// Any conjucts with leading_0s == -1 must have >= max_arity leading 0s
	// What a gross way to do this.  Ferd

	F_Exists *exists = result.add_exists();
	Section<Variable_ID> middle_tuple = exists->declare_tuple(mid_size);
	Map<Global_Var_ID, Variable_ID> lost_functions((Variable_ID)0);

	F_Or *result_conjs = exists->add_or();

	for (DNF_Iterator conj1(d1); conj1; conj1++)
	    for (DNF_Iterator conj2(d2); conj2; conj2++)
		{
		// combine conj1 and conj2:
		//   conj2's in becomes result's in; conj1's out becomes out
		//   conj2's out and conj1's in get merged and exist. quant.
		//   conj2's f(in) and conj1's f(out) become f(in) and f(out)
		//   conj2's f(out) and conj1's f(in) get merged, evacuate:
		//     if conj1 has f.arity leading 0s, they become f(out),
		//     if conj2 has f.arity leading 0s, they become f(in)
		//     if neither has enough 0s, they become a wildcard
		//                               and the result is inexact
		//   old wildcards stay wildcards

#if defined STUDY_EVACUATIONS
		study_evacuation(*conj1, *conj2, max(a1, a2));
#endif

		Conjunct *copy1, *copy2;
		copy2 = (*conj2)->copy_conj_same_relation();
		copy1 = (*conj1)->copy_conj_same_relation();

		Variable_ID_Tuple remapped;

		int c1L0 = copy1->guaranteed_leading_0s;
		int c2L0 = copy2->guaranteed_leading_0s;

		int inexact = 0;

		// get rid of conj2's f(out)
		{
		for (Variable_ID_Iterator func(copy2->mappedVars);
		     func;
		     func++)
		    if ((*func)->kind() == Global_Var)
			{
			Global_Var_ID f = (*func)->get_global_var();
			if (f->arity() > 0 &&
			    (*func)->function_of()==Output_Tuple)
			    {
			    if (c2L0 >= f->arity())
				{
				(*func)->remap = r2.get_local(f, Input_Tuple);
				remapped.append(*func);
				}
			    else if (c1L0 >= f->arity())
				{
				// f->remap = copy1->get_local(f, Output_Tuple);
				// this should work with the current impl.
				// SHOULD BE A NO-OP?
				assert((*func)==r1.get_local(f,Output_Tuple));
				}
			    else
				{
				Variable_ID f_quantified = lost_functions[f];
				if (!f_quantified)
				    {
				    f_quantified = exists->declare();
				    lost_functions[f] = f_quantified;
				    }
				inexact = 1;
				(*func)->remap = f_quantified;
				remapped.append(*func);
				}
			    }
			}	    
		}

		// remap copy2's out
		for (i=1; i<=mid_size; i++)
		    {
		    r2.output_var(i)->remap = middle_tuple[i];
		    }

		// do remapping for conj2, then reset everything so
		// we can go on with conj1

		copy2->remap();
		reset_remap_field(remapped);
		reset_remap_field(output_vars,mid_size);


		remapped.clear();

		// get rid of conj1's f(in)
		{
		for (Variable_ID_Iterator func(copy1->mappedVars);
		     func;
		     func++)
		    if ((*func)->kind() == Global_Var)
			{
			Global_Var_ID f = (*func)->get_global_var();
			if (f->arity() > 0 &&
			    (*func)->function_of()==Input_Tuple)
			    {
			    if (c1L0 >= f->arity())
				{
				(*func)->remap = r1.get_local(f,Output_Tuple);
				remapped.append(*func);
				}
			    else if (c2L0 >= f->arity())
				{
				// f->remap = copy2->get_local(f, Input_Tuple);
				// this should work with the current impl.
				// SHOULD BE A NO-OP?
				assert((*func)==r2.get_local(f,Input_Tuple));
				}
			    else
				{
				Variable_ID f_quantified = lost_functions[f];
				if (!f_quantified)
				    {
				    f_quantified = exists->declare();
				    lost_functions[f] = f_quantified;
				    }
				inexact = 1;
				(*func)->remap = f_quantified;
				remapped.append(*func);
				}
			    }
			}		    
		}

		// merge copy1's in with the already remapped copy2's out
		for (i=1; i<=mid_size; i++)
		    {
		    r1.input_var(i)->remap = middle_tuple[i];
		    }

		copy1->remap();
		reset_remap_field(remapped);
		reset_remap_field(input_vars,mid_size);

		Conjunct *conj3 = merge_conjs(copy1, copy2, MERGE_COMPOSE, exists->relation());
		result_conjs->add_child(conj3);
		delete copy1;
		delete copy2;

		// make sure all variables used in the conjunct
		// are listed in the "result" relation

		for (Variable_ID_Iterator func(conj3->mappedVars);
		     func;
		     func++)
		    if ((*func)->kind() == Global_Var)
			{
			Global_Var_ID f = (*func)->get_global_var();
			if (f->arity() > 0)
			    result.get_local(f, (*func)->function_of());
			else
			    result.get_local(f);
			}

		if (inexact)
		    conj3->make_inexact();
		}

	result.finalize();
        r1 = r2 = Relation();
	return result;
	}
    }



bool Is_Obvious_Subset(NOT_CONST Relation &input_r1,
		 NOT_CONST Relation &input_r2) 
    {
    Relation r1 = consume_and_regurgitate(input_r1);
    Relation r2 = consume_and_regurgitate(input_r2);

    assert(!r1.is_null() && !r2.is_null());
    Rel_Body *rr1 = r1.split();
    Rel_Body *rr2 = r2.split();
    rr1->simplify();
    rr2->simplify();
    use_ugly_names++;

    remap_DNF_vars(rr2, rr1);

    for(DNF_Iterator pd1(rr1->query_DNF()); pd1.live(); pd1.next()) 
        {
        Conjunct *conj1 = pd1.curr();
        int found = false;
        for(DNF_Iterator pd2(rr2->query_DNF()); pd2.live(); pd2.next()) 
            {
            Conjunct *conj2 = pd2.curr();
            if (!conj2->is_exact()) continue;

    
            Conjunct *cgist = merge_conjs(conj1, conj2, MERGE_GIST, conj2->relation());
#ifndef NDEBUG
            cgist->setup_names();
#endif
            if (cgist->redSimplifyProblem(2, 0) == noRed) 
                {
                delete cgist;
                found = true;
                break;
                }
            delete cgist;
            }
        if (! found)  {
            use_ugly_names--;
	    r1 = r2 = Relation();
            return false;
	    };
        }
    use_ugly_names--;
    r1 = r2 = Relation();
    return true;
    } /* Is_Obvious_Subset */


bool do_subset_check(NOT_CONST Relation &input_r1,
			    NOT_CONST Relation &input_r2);

// do_subset_check really implements Must_Be_Subset anyway (due to 
// correct handling of inexactness in the negation code), but
// still take upper and lower bounds here
bool Must_Be_Subset(NOT_CONST Relation &r1, NOT_CONST Relation &r2) {
  Relation s1 = Upper_Bound(consume_and_regurgitate(r1));
  Relation s2 = Lower_Bound(consume_and_regurgitate(r2));
  return do_subset_check(s1,s2);
}

bool Might_Be_Subset(NOT_CONST Relation &r1, NOT_CONST Relation &r2) {
  Relation s1 = Lower_Bound(consume_and_regurgitate(r1));
  Relation s2 = Upper_Bound(consume_and_regurgitate(r2));
  return do_subset_check(s1,s2);
}

bool May_Be_Subset(NOT_CONST Relation &r1, NOT_CONST Relation &r2){
  return Might_Be_Subset(r1,r2);
}




//
// F Must_Be_Subset G
// Test that (f => g) === (~f | g) is a Tautology
// or that (f & ~g) is unsatisfiable:
// 	align the input tuples (if any) for F and G
// 	align the output tuples (if any) for F and G
// Special case: if r2 has a single conjunct then use HasRedQeuations.
// 

bool do_subset_check(NOT_CONST Relation &input_r1,
		     NOT_CONST Relation &input_r2) {
  Relation r1 = consume_and_regurgitate(input_r1);
  Relation r2 = consume_and_regurgitate(input_r2);

  assert(!r1.is_null() && !r2.is_null());
  skip_set_checks++;
  assert(r1.n_inp() == r2.n_inp());
  assert(r1.n_out() == r2.n_out());
  skip_set_checks--;
  r1.simplify(1,0);
  r2.simplify(2,2);
  Rel_Body *rr1 = r1.split();

  if(relation_debug) {
    fprintf(DebugFile, "\n$$$ Must_Be_Subset IN $$$\n");
  }

  bool c;

  c = true;

  // Check each conjunct separately
  for(DNF_Iterator pd(rr1->query_DNF()); c &&  pd.live(); ) {
      Relation tmp(r1,pd.curr());
      pd.next();
#ifndef CHECK_MAYBE_SUBSET
      if (pd.live())
         c = !Difference(tmp,copy(r2)).is_upper_bound_satisfiable();
      else 
         c = !Difference(tmp,r2).is_upper_bound_satisfiable();
#else
      Relation d=Difference(copy(tmp), copy(r2));
      c=!d.is_upper_bound_satisfiable();
      if (!c && !d.is_exact()) // negation-induced inexactness
	  {
	  static int OMEGA_WHINGE = -1;
	  if (OMEGA_WHINGE < 0)
	      {
	      OMEGA_WHINGE = getenv("OMEGA_WHINGE") ? atoi(getenv("OMEGA_WHINGE")) : 0;
	      }
	  if (OMEGA_WHINGE)
	      {
	      fprintf(DebugFile,"\n===== r1 is maybe a Must_Be_Subset of r2 ========\n");
	      fprintf(DebugFile,"-------> r1:\n");
	      tmp.print_with_subs(DebugFile);
	      fprintf(DebugFile,"-------> r2:\n");
	      r2.print_with_subs(DebugFile);
	      fprintf(DebugFile,"-------> r1-r2:\n");
	      d.print_with_subs(DebugFile);
	      }
	  }
#endif         
      }

  if(relation_debug) {
    fprintf(DebugFile, "$$$ Must_Be_Subset OUT $$$\n");
  }
  r1 = r2 = Relation();
  return c;
}


//
// 	F minus G
//
Relation Difference(NOT_CONST Relation &input_r1,
		    NOT_CONST Relation &input_r2) {

  Relation r1 = consume_and_regurgitate(input_r1);
  Relation r2 = consume_and_regurgitate(input_r2);

  assert(!r1.is_null() && !r2.is_null());
  skip_set_checks++;
  assert(r1.n_inp() == r2.n_inp());
  assert(r1.n_out() == r2.n_out());

  int i;
  Mapping m1(r1.n_inp(), r1.n_out());
  for(i=1; i<=r1.n_inp(); i++) m1.set_map_in (i, Input_Var,i);
  for(i=1; i<=r1.n_out(); i++) m1.set_map_out(i, Output_Var,i);
  Mapping m2(r2.n_inp(), r2.n_out());
  for(i=1; i<=r2.n_inp(); i++) m2.set_map_in (i, Input_Var,i);
  for(i=1; i<=r2.n_out(); i++) m2.set_map_out(i, Output_Var,i);
  skip_set_checks--;

  return MapAndCombineRel2(r1, r2, m1, m2, Comb_AndNot);
}

//
// 	complement F
// 		not F
//
Relation Complement(NOT_CONST Relation &S) {
  Relation r = consume_and_regurgitate(S);
  assert(!r.is_null());
  skip_set_checks++;  
  int i;
  Mapping m(r.n_inp(), r.n_out());
  for(i=1; i<=r.n_inp(); i++) m.set_map_in (i, Input_Var,i);
  for(i=1; i<=r.n_out(); i++) m.set_map_out(i, Output_Var,i);
  skip_set_checks--;

  MapRel1(r, m, Comb_AndNot, -1, -1, false);
  return r;
}


//
// Compute (gist r1 given r2).
// Currently we assume that r2 has only one conjunct.
// r2 may have zero input and output OR may have # in/out vars equal to r1.
//
Relation GistSingleConjunct(NOT_CONST Relation &input_R1,
			    NOT_CONST Relation &input_R2, int effort) {
  Relation R1 = consume_and_regurgitate(input_R1);
  Relation R2 = consume_and_regurgitate(input_R2);

  skip_set_checks++;
  assert(!R1.is_null() && !R2.is_null());
  assert((R1.n_inp() == R2.n_inp() && R1.n_out() == R2.n_out()) ||
	 (R2.n_inp() == 0 && R2.n_out() == 0));
  R1.simplify();
  R2.simplify();
  Rel_Body *r1 = R1.split();
  Rel_Body *r2 = R2.split();

  if(relation_debug) {
    fprintf(DebugFile, "\n### GIST computation start ### [\n");
    R1.prefix_print(DebugFile);
    R2.prefix_print(DebugFile);
    fprintf(DebugFile, "### ###\n");
  }


//  The merged conjunct has to have the variables of either r1 or r2, but
//  not both. Use r1's, since it'll be cheaper to remap r2's single conj.
  remap_DNF_vars(r2, r1);
  Assert(r2->is_upper_bound_satisfiable(), "Gist: second operand is FALSE");
  skip_set_checks--;

  Conjunct *known = r2->single_conjunct();
  Assert(known!=NULL, "Gist: second operand has more than 1 conjunct");

  DNF *new_dnf = new DNF();
  for(DNF_Iterator pd(r1->simplified_DNF); pd.live(); pd.next()) {
    Conjunct *conj = pd.curr();
    Conjunct *cgist = merge_conjs(known, conj, MERGE_GIST, conj->relation()); // Uses r1's vars
    cgist->set_relation(r1);   // Thinks it's part of r1 now, for var. purposes
    if(simplify_conj(cgist, true, effort+1, _red)) {
      /* Throw out black constraints, turn red constraints into black */
      cgist->rm_color_constrs();
      if(cgist->is_true()) {
	delete new_dnf;
	delete cgist;
	skip_set_checks++;
	Relation retval = Relation::True(r1->n_inp(), r2->n_out());
	retval.finalize();
	if(R1.is_set() && R2.is_set()) retval.markAsSet();
        skip_set_checks--;
	return retval;
      } else {
	new_dnf->add_conjunct(cgist);
      }
    }
  }
  delete r1->simplified_DNF;
  r1->simplified_DNF = new_dnf;
  assert(!r1->is_null());
  R1.finalize();
  if(relation_debug) {
    fprintf(DebugFile, "] ### GIST computation end ###\n");
    R1.prefix_print(DebugFile);
    fprintf(DebugFile, "### ###\n");
  }
  return(R1);
}


// Compute gist r1 given r2. r2 can have multiple conjuncts

Relation Gist(NOT_CONST Relation &input_R1,
	      NOT_CONST Relation &input_R2, int effort) {
  Relation R1 = consume_and_regurgitate(input_R1);
  Relation R2 = consume_and_regurgitate(input_R2);

  skip_set_checks++;
  assert(!R1.is_null() && !R2.is_null());
  assert((R1.n_inp() == R2.n_inp() && R1.n_out() == R2.n_out()) ||
	 (R2.n_inp() == 0 && R2.n_out() == 0));
  skip_set_checks--;
  R2.simplify();

  if(relation_debug) {
    fprintf(DebugFile, "\n### multi-GIST computation start ### [\n");
    R1.prefix_print(DebugFile);
    R2.prefix_print(DebugFile);
    fprintf(DebugFile, "### ###\n");
  }

  if (!R2.is_upper_bound_satisfiable())
      return Relation::True(R1);
  if (R2.is_obvious_tautology()) {
      R1.simplify();
      return R1;
      }

  if (!Intersection(copy(R1), copy(R2)).is_upper_bound_satisfiable())
      return Relation::False(R1);

  int nconj1=0;
  for (DNF_Iterator di(R1.simplified_DNF()); di.live(); di.next()) 
      nconj1++;
  int nconj2=0;
  for (DNF_Iterator di2(R2.simplified_DNF()); di2.live(); di2.next()) 
      nconj2++;

  {  
  static int OMEGA_WHINGE = -1;
  if (OMEGA_WHINGE < 0)
      {
      OMEGA_WHINGE = getenv("OMEGA_WHINGE") ? atoi(getenv("OMEGA_WHINGE")) : 0;
      }
  if (OMEGA_WHINGE && (nconj1 + nconj2 > 50))
      {
      fprintf(DebugFile,"WOW!!!! - Gist (%d conjuncts, %d conjuncts)!!!\n",
	      nconj1,nconj2);
      fprintf(DebugFile,"Base:\n");
      R1.prefix_print(DebugFile);
      fprintf(DebugFile,"Context:\n");
      R2.prefix_print(DebugFile);
      }
  }

  if (nconj2==1)
    return GistSingleConjunct(R1,R2, effort);
  else { 
    R1.simplify(0,1);
    R2.simplify(0,1);
    Relation G = Relation::True(R1); 
    for (DNF_Iterator di2(R2.simplified_DNF()); di2.live(); di2.next()) {
      Conjunct * c2 = di2.curr();
      Relation G2 = Relation::False(R1); 
      for (DNF_Iterator di1(R1.simplified_DNF()); di1.live(); di1.next()) {
        Conjunct * c1 = di1.curr();
       Relation G1=GistSingleConjunct(Relation(R1,c1), Relation(R2,c2),effort);
	
       if (G1.is_obvious_tautology()) {
		G2 = G1;
		break;
		}
	else if (!G1.is_upper_bound_satisfiable() || !G1.is_exact()) {
		if(relation_debug) {
		  fprintf(DebugFile, "gist A given B is unsatisfiable\n");
		  fprintf(DebugFile, "A:\n");
		  Relation(R1,c1).prefix_print(DebugFile);
		  fprintf(DebugFile, "B:\n");
		  Relation(R2,c2).prefix_print(DebugFile);
		  fprintf(DebugFile, "\n");
		  }
		//G1 = Relation(R1,c1);
		return R1;
		}
	else if(0 && G1.is_exact() && !Must_Be_Subset(Relation(R1,c1),copy(G1))) {
		fprintf(DebugFile,"Unexpected non-Must_Be_Subset gist result!\n");
		fprintf(DebugFile,"base: \n");
		Relation(R1,c1).prefix_print(DebugFile);
		fprintf(DebugFile,"context: \n");
		Relation(R2,c2).prefix_print(DebugFile);
		fprintf(DebugFile,"result: \n");
		G1.prefix_print(DebugFile);
		fprintf(DebugFile,"base not subseteq result: \n");
		assert(!G1.is_exact() || Must_Be_Subset(Relation(R1,c1),copy(G1)));
		}
        G2=Union(G2,G1); 
	}
	G2.simplify(0,1);
	G = Intersection(G,G2);
	G.simplify(0,1);
       if(relation_debug) {
	    fprintf(DebugFile, "result so far is:\n");
	    G.prefix_print(DebugFile);
	    }
      } 
		      
  if(relation_debug) {
    fprintf(DebugFile, "\n### end multi-GIST computation ### ]\n");
    fprintf(DebugFile, "G is:\n");
    G.prefix_print(DebugFile);
    fprintf(DebugFile, "### ###\n");
  }
#if ! defined NDEBUG
     Relation S1 = Intersection(copy(R1), copy(R2));
     Relation S2 = Intersection(copy(G), copy(R2));


     if(relation_debug) {
       fprintf(DebugFile, "\n---->[Checking validity of the GIST result\n");
       fprintf(DebugFile, "for G=gist R1 given R2:\n");
       fprintf(DebugFile, "R1 intersect R2 is:\n");
       S1.print_with_subs(DebugFile);
       fprintf(DebugFile, "\nG intersect R2 is:\n");
       S2.print_with_subs(DebugFile);
       fprintf(DebugFile, "---->]\n");
      }
     assert (!S1.is_exact() || !S2.is_exact() || Must_Be_Subset(copy(S1),copy(S2)) && Must_Be_Subset(copy(S2),copy(S1)));
#endif   
    return G;
  }
}  


// Project an input, output or set variable, leaving a variable in that position
// with no constraints.
Relation Project(NOT_CONST Relation &S, int pos, Var_Kind vkind) {
    Relation R = consume_and_regurgitate(S);
    Variable_ID v = 0; // shut the compiler up
    switch (vkind)
	{
	case Input_Var:
	    v = input_vars[pos];
	    break;
	case Output_Var:
	    v = output_vars[pos];
	    break;
	case Set_Var:
	    v = set_vars[pos];
	    break;
        default:
	    assert(0);
        }
    
    skip_finalization_check++;

    Rel_Body *r = R.split();
    r->DNF_to_formula();
    Formula *f = r->rm_formula();
    F_Exists *ex = r->add_exists();
    ex->add_child(f);

    v->remap = ex->declare(v->base_name);
    f->remap();
    v->remap = v;

    skip_finalization_check--;
    
    R.finalize();
    return R;
}

// Project away all input and output variables.
Relation Project_On_Sym(NOT_CONST Relation &S,
			NOT_CONST Relation &input_context) {
  Relation R = consume_and_regurgitate(S);
  Relation context = consume_and_regurgitate(input_context);
  int i;

  skip_set_checks++;
  leave_pufs_untouched++;
  int  in_arity = R.max_ufs_arity_of_in();
  int  out_arity = R.max_ufs_arity_of_out();
  assert(!R.is_null());
  R.split();

  int no_inp = R.n_inp();
  int no_out = R.n_out();
  Mapping M(no_inp, no_out);

  for(i=1; i<=no_inp; i++) {	         // project out input variables
    M.set_map(Input_Var, i, Exists_Var, i);
  }
  for(i=1; i<=no_out; i++) {	         // project out output variables
    M.set_map(Output_Var, i, Exists_Var, no_inp+i);
  }
  MapRel1(R, M, Comb_Id, 0, 0);

  R.finalize();
  if (in_arity) R = Extend_Domain(R,in_arity);
  if (out_arity) R = Extend_Range(R,out_arity);

  int d = min(in_arity,out_arity);
  if (d && !context.is_null()) {
	int g = min(d,context.query_guaranteed_leading_0s());
	int p = min(d,context.query_possible_leading_0s());
	int dir = context.query_leading_dir();
	R.enforce_leading_info(g,p,dir);
	}

  leave_pufs_untouched--;
  skip_set_checks--;
  if(relation_debug) {
	fprintf(DebugFile,"\nProjecting onto symbolic (%d,%d):\n",in_arity,out_arity);
	R.prefix_print(DebugFile);
	};
  return R;
}

//
// Project out global variable g from relation r
//
Relation Project(NOT_CONST Relation &S, Global_Var_ID g)
    {
    Relation R = consume_and_regurgitate(S);
    assert(!R.is_null());

    skip_finalization_check++;

    Rel_Body *r = R.split();
    r->DNF_to_formula();
    Formula *f = r->rm_formula();
    F_Exists *ex = r->add_exists();
    ex->add_child(f);
    
    if (g->arity() == 0)
	{
	Assert(R.has_local(g), 
	       "Project: Relation doesn't contain variable to be projected");
	Variable_ID v = R.get_local(g);
	
	bool rmd = rm_variable(r->Symbolic,v);
	Assert(rmd, "Project: Variable to be projected doesn't exist");
	
	v->remap = ex->declare(v->base_name);
	f->remap();
	v->remap = v;
	}
    else
	{
	Assert(R.has_local(g, Input_Tuple) || R.has_local(g, Output_Tuple),
	       "Project: Relation doesn't contain variable to be projected");

	if (R.has_local(g, Input_Tuple))
	    {
	    Variable_ID v = R.get_local(g, Input_Tuple);
	
	    bool rmd = rm_variable(r->Symbolic,v);
	    Assert(rmd, "Project: Variable to be projected doesn't exist");
	
	    v->remap = ex->declare(v->base_name);
	    f->remap();
	    v->remap = v;
	    }
	if (R.has_local(g, Output_Tuple))
	    {
	    Variable_ID v = R.get_local(g, Output_Tuple);
	
	    bool rmd = rm_variable(r->Symbolic,v);
	    Assert(rmd, "Project: Variable to be projected doesn't exist");
	
	    v->remap = ex->declare(v->base_name);
	    f->remap();
	    v->remap = v;
	    }
	}

    skip_finalization_check--;
    
    R.finalize();
    return R;
}


//
// Project all symbolic variables from relation r
//
Relation Project_Sym(NOT_CONST Relation &S) {
  Relation R = consume_and_regurgitate(S);
  assert(!R.is_null());

  Rel_Body *r = R.split();
  r->DNF_to_formula();

  Formula *f = r->rm_formula();

  skip_finalization_check++;
  F_Exists *ex = r->add_exists();
  for(Variable_ID_Iterator R_Sym(r->Symbolic); R_Sym; R_Sym++) {
    Variable_ID v = *R_Sym;
    v->remap = ex->declare(v->base_name);
  }
  ex->add_child(f);
  skip_finalization_check--;

  f->remap();

  reset_remap_field(r->Symbolic);
  r->Symbolic.clear();

  R.finalize();
  return R;
}

Relation  Project(NOT_CONST Relation &S, Sequence<Variable_ID> &s){
    // This is difficult to do with mappings.  This cheats, since it is
    // much easier and more straightforward.

    Relation R = consume_and_regurgitate(S);
    assert(!R.is_null());

    Rel_Body *r = R.split();
    r->DNF_to_formula();

    Formula *f = r->rm_formula();

    skip_finalization_check++;
    F_Exists *ex = r->add_exists();
    int i;
    for(i = 1; i <= s.size(); i++) s[i]->remap = ex->declare(s[i]->base_name);
    ex->add_child(f);
    skip_finalization_check--;

    f->remap();

    reset_remap_field(s);
//    r->Symbolic.clear();

    R.finalize();
    return R;
}

//
// Variables in DNF of map_rel reference declarations of map_rel (or not).
// remap_DNF_vars makes them to reference declarations of ref_rel.
// Ref_rel can get new global variable declarations in the process.
//
void remap_DNF_vars(Rel_Body *map_rel, Rel_Body *ref_rel) {
  skip_set_checks++;
  assert (map_rel->simplified_DNF);
  assert (ref_rel->simplified_DNF);

  skip_set_checks++;

  for(DNF_Iterator pd(map_rel->simplified_DNF); pd.live(); pd.next()) {
    Conjunct *cc = pd.curr();
    Variable_ID_Tuple &mvars = cc->mappedVars;
    for(Variable_Iterator mvarsIter=mvars; mvarsIter; mvarsIter++) {
      Variable_ID v = *mvarsIter;
      switch(v->kind()) {
      case Input_Var:
	assert(ref_rel->n_inp() >= v->get_position());
	break;
      case Output_Var:
	assert(ref_rel->n_out() >= v->get_position());
	break;
      case Global_Var:
	  // The assignment is a noop, but tells ref_rel that the global may be
          // used inside it, which is required.
	*mvarsIter = ref_rel->get_local(v->get_global_var(),v->function_of());
	break;
      case Wildcard_Var:
	break;
      default:
	PresErrAssert("bad variable kind");
      }
    }
  }
  skip_set_checks--;
}


// Determine if a mapping requires an f_exists node
static bool has_existentials(const Mapping &m) 
{ 
    for(int i=1;i<=m.n_in();  i++) 
	if (m.get_map_in_kind(i) == Exists_Var) return true;
    for(int j=1;j<=m.n_out(); j++) 
	if (m.get_map_out_kind(j) == Exists_Var) return true;
    return false;
}


static
void get_relation_arity_from_one_mapping(const Mapping &m1,
					 int &in_req, int &out_req)
{
    int j, i;
    in_req = 0; out_req = 0;
    for(i = 1;  i <= m1.n_in();  i++) {
      j = m1.get_map_in_pos(i);
      switch(m1.get_map_in_kind(i)) {
      case Input_Var:  in_req = max(in_req, j);   break;
      case Set_Var:  in_req = max(in_req, j);   break;
      case Output_Var: out_req = max(out_req, j); break;
      default: break;
      }
    }
    for(i = 1;  i <= m1.n_out();  i++) {
      j = m1.get_map_out_pos(i);
      switch(m1.get_map_out_kind(i)) {
      case Input_Var:  in_req = max(in_req, j);   break;
      case Set_Var:  in_req = max(in_req, j);   break;
      case Output_Var: out_req = max(out_req, j); break;
      default: break;
      }
    }
}


// Scan mappings to see how many input and output variables they 
// require. 
static
void get_relation_arity_from_mappings(const Mapping &m1,
				      const Mapping &m2,
				      int &in_req, int &out_req)
{
   int inreq1, inreq2, outreq1, outreq2;
   get_relation_arity_from_one_mapping(m1, inreq1, outreq1);
   get_relation_arity_from_one_mapping(m2, inreq2, outreq2);
   in_req = max(inreq1, inreq2);
   out_req = max(outreq1, outreq2);
}


void align(Rel_Body *originalr, Rel_Body *newr, F_Exists *fe,
	   Formula *f, const Mapping &mapping, bool &newrIsSet,
	   List<int> &seen_exists, Variable_ID_Tuple &seen_exists_ids);


void MapRel1(Relation &R, const Mapping &map, Combine_Type ctype,
	     int number_input, int number_output,
	     bool invalidate_resulting_leading_info,
	     bool finalize) {
#if defined(INCLUDE_COMPRESSION)
    assert(!R.is_compressed());
#endif
    assert(!R.is_null());
   
    Relation inputRel = R; 
    R = Relation();
    Rel_Body *inputRelBody = inputRel.split();

    int in_req=0, out_req=0;
    get_relation_arity_from_one_mapping(map, in_req, out_req);

    R = Relation(number_input == -1 ? in_req : number_input,
	       number_output == -1 ? out_req : number_output);

    Rel_Body *outputRelBody = R.split();

    inputRelBody->DNF_to_formula();
    Formula *f1 = inputRelBody->rm_formula();

    F_Exists *fe;
    Formula *f;
    if (has_existentials(map)) {
      f = fe = outputRelBody->add_exists();
    } else {
      fe = NULL;
      f = outputRelBody;
    }
    and_below_exists = NULL;
    if (finalize) and_below_exists = NULL;
    else f = and_below_exists = f->add_and();
    if(ctype == Comb_AndNot) {
      f = f->add_not();
    }
    f->add_child(f1);

    exists_ids.clear();
    exists_numbers.clear();

    bool returnAsSet=false;
    align(inputRelBody, outputRelBody, fe, f1, map, returnAsSet,
	  exists_numbers, exists_ids);

    if (returnAsSet || 
	(inputRelBody->is_set() && outputRelBody->n_out() == 0)) {
	R.markAsSet();
	R.invalidate_leading_info(); // nonsensical for a set
    }


    if (finalize) R.finalize();
    inputRel = Relation();
    if (invalidate_resulting_leading_info)
	R.invalidate_leading_info();
}



Relation MapAndCombineRel2(Relation &R1, Relation &R2, const Mapping &mapping1,
			   const Mapping &mapping2, Combine_Type ctype,
			   int number_input, int number_output)
{
#if defined(INCLUDE_COMPRESSION)
    assert(!R1.is_compressed());
    assert(!R2.is_compressed());
#endif
    assert(!R1.is_null() && !R2.is_null());
    Rel_Body *r1 = R1.split();
    Rel_Body *r2 = R2.split();
    
    int in_req, out_req;      // Create the new relation
    get_relation_arity_from_mappings(mapping1, mapping2, in_req, out_req);
    Relation R3(number_input == -1 ? in_req : number_input,
		number_output == -1 ? out_req : number_output);
    Rel_Body *r3 = R3.split();  // This is just to get the pointer, it's cheap

    /* permit the add_{exists,and} below, reset after they are done.*/
    skip_finalization_check++;
    
    F_Exists *fe = NULL;
    Formula *f;
    if (has_existentials(mapping1) || has_existentials(mapping2)) {
      fe = r3->add_exists();
      f = fe;
    } else {
      f = r3;
    }	

    r1->DNF_to_formula();
    Formula *f1 = r1->rm_formula();
    r2->DNF_to_formula();
    Formula *f2 = r2->rm_formula();

    // align: change r1 vars to r3 vars in formula f1 via map mapping1,
    //        declaring needed exists vars in F_Exists *fe
    // Also maps symbolic variables appropriately, sets relation ptrs in f1.
    // In order to map variables of both relations to the same variables,
    // we keep a list of new existentially quantified vars between calls.
    // returnAsSet means mark r3 as set before return.  Don't mark it yet,
    // because internally we need to refer to "input_vars" of a set, and that
    // would blow assertions.

    bool returnAsSet=false;
    exists_ids.clear();
    exists_numbers.clear();
    align(r1, r3, fe, f1, mapping1, returnAsSet, exists_numbers, exists_ids);
    // align: change r2 vars to r3 vars in formula f2 via map mapping2
    align(r2, r3, fe, f2, mapping2, returnAsSet, exists_numbers, exists_ids);

    switch (ctype) {
    case Comb_Or:
      if(f1->node_type() == Op_Or) {
	f->add_child(f1); 
	f = f1;
      } else {
	f = f->add_or();
	f->add_child(f1); 
      }
      break;
    case Comb_And:
    case Comb_AndNot:
      if(f1->node_type() == Op_And) {
	f->add_child(f1); 
	f = f1;
      } else {
	f = f->add_and();
	f->add_child(f1); 
      }
      break;
    default:
      assert(0 && "Invalid combine type in MapAndCombineRel2");
    }

    Formula *c2;
    if (ctype==Comb_AndNot) {
      c2 = f->add_not();
    } else {
      c2 = f;
    }
    c2->add_child(f2);

    skip_finalization_check--;     /* Set this back for return */
    R3.finalize(); 

    if (returnAsSet ||
	(R1.is_set() && R2.is_set() && R3.n_inp() >= 0 && R3.n_out() == 0)){
      R3.markAsSet();
      R3.invalidate_leading_info();
    }
    R1 = Relation();
    R2 = Relation();
    return R3;
}


// Build lists of variables that need to be replaced in the given 
// Formula.  Declare globals in new relation.  Then call
// map_vars to do the replacements.

// Obnoxiously many arguments here:
// Relation arguments contain declarations of symbolic and in/out vars.
// F_Exists argument is where needed existentially quant. vars can be decl.

// Mapping specifies how in/out vars are mapped
// Two lists are required to be able to map in/out variables from the first
// and second relations to the same existentially quantified variable.

void align(Rel_Body *originalr, Rel_Body *newr, F_Exists *fe,
	   Formula *f, const Mapping &mapping, bool &newrIsSet,
	   List<int> &seen_exists, Variable_ID_Tuple &seen_exists_ids)
{
    int i, cur_ex = 0;  // initialize cur_ex to shut up the compiler

    f->set_relation(newr);  // Might not need to do this anymore, if bugs were fixed
    int input_remapped = 0;
    int output_remapped = 0;
    int sym_remapped = 0;
    skip_set_checks++;

    Variable_ID new_var;
    Const_String new_name;
    int new_pos;

    // MAP old input variables by setting their remap fields 
    for(i = 1; i <= originalr->n_inp(); i++)
    { 
	Variable_ID this_var = originalr->input_var(i), New_E;
	Const_String this_name = originalr->In_Names[i];

	switch (mapping.get_map_in_kind(i)) {

	case Input_Var:
	case Set_Var:
	    if (mapping.get_map_in_kind(i) == Set_Var)
		newrIsSet = true;  // Don't mark it just yet; we still need to 
	                           // refer to its "input" vars internally

	    assert((newrIsSet && mapping.get_map_in_kind(i) == Set_Var)
		   || ((!newrIsSet &&mapping.get_map_in_kind(i) == Input_Var)));

	    new_pos = mapping.get_map_in_pos(i);
	    new_var = newr->input_var(new_pos);
	    if (this_var != new_var) {
	      input_remapped = 1;
	      this_var->remap = new_var;
	      };
	    new_name = newr->In_Names[new_pos];
	    if (!this_name.null())                 // should we name this?
		if (!new_name.null()) {            // already named, anonymize
	            if (new_name != this_name)
		        newr->name_input_var(new_pos, Const_String());
		} else
		    newr->name_input_var(new_pos, this_name);
	    break;

	case Output_Var:
	    assert(!newr->is_set());
	    input_remapped = 1;
	    new_pos = mapping.get_map_in_pos(i);
	    this_var->remap = new_var = newr->output_var(new_pos);
	    new_name = newr->Out_Names[new_pos];
	    if (!this_name.null()) 
		if (!new_name.null()) {             // already named, anonymize
		    if (new_name != this_name)
		        newr->name_output_var(new_pos, Const_String());
		} else
		    newr->name_output_var(new_pos, this_name);
	    break;

	case Exists_Var:
	    input_remapped = 1;
	    // check if we have declared it, use that if so.
	    // create it if not.  
            if (mapping.get_map_in_pos(i) <= 0 ||
		(cur_ex = seen_exists.index(mapping.get_map_in_pos(i))) == 0){
		if (!this_name.null())
		    New_E = fe->declare(this_name);
		else
		    New_E = fe->declare();
	        this_var->remap = New_E;
		if (mapping.get_map_in_pos(i) > 0)
		    {
		    seen_exists.append(mapping.get_map_in_pos(i));
		    seen_exists_ids.append(New_E);
		    }
	    } else {
		this_var->remap = new_var = seen_exists_ids[cur_ex];
		if (!this_name.null())  // Have we already assigned a name?
		    if (!new_var->base_name.null()) {
			if (new_var->base_name != this_name)
			    new_var->base_name = Const_String();
		    } else {
			new_var->base_name = this_name;
			assert(!this_name.null());
		    }
	    }
	    break;
	default:
	    PresErrAssert("Unsupported var type in MapRel2");
	    break;
	}
    }

    //  MAP old output variables.
    for(i = 1; i <= originalr->n_out(); i++)
    {   
	Variable_ID this_var = originalr->output_var(i), New_E;
	Const_String this_name = originalr->Out_Names[i];

	switch (mapping.get_map_out_kind(i)) {
	case Input_Var:
	case Set_Var:
	    if (mapping.get_map_out_kind(i) == Set_Var)
		newrIsSet = true;  // Don't mark it just yet; we still need to 
	                           // refer to its "input" vars internally
		
	    assert((newrIsSet && mapping.get_map_out_kind(i) == Set_Var)
		   ||((!newrIsSet &&mapping.get_map_out_kind(i) == Input_Var)));

	    output_remapped = 1;
	    new_pos = mapping.get_map_out_pos(i);
	    this_var->remap = new_var = newr->input_var(new_pos);
	    new_name = newr->In_Names[new_pos];
	    if (!this_name.null())
		if (!new_name.null()) {    // already named, anonymize
		    if (new_name != this_name)
			newr->name_input_var(new_pos, Const_String());
		} else
		    newr->name_input_var(new_pos, this_name);
	    break;

	case Output_Var:
	    assert(!newr->is_set());
	    new_pos = mapping.get_map_out_pos(i);
	    new_var = newr->output_var(new_pos);
	    if (new_var != this_var) {
	      output_remapped = 1;
	      this_var->remap = new_var;
	      };
	    new_name = newr->Out_Names[new_pos];
	    if (!this_name.null())
		if (!new_name.null()) {    // already named, anonymize
			if (new_name != this_name)
			    newr->name_output_var(new_pos, Const_String());
		    } else
			newr->name_output_var(new_pos, this_name);
	    break;

	case Exists_Var:
	   // check if we have declared it, create it if not.  
	    output_remapped = 1;
	    if (mapping.get_map_out_pos(i) <= 0 ||
		(cur_ex = seen_exists.index(mapping.get_map_out_pos(i))) == 0)
	    {   // Declare it.
	        New_E = fe->declare(this_name);
		this_var->remap = New_E;
		if (mapping.get_map_out_pos(i) > 0)
		    {
		    seen_exists.append(mapping.get_map_out_pos(i));
		    seen_exists_ids.append(New_E);
		    }
	    } else {
		this_var->remap = new_var = seen_exists_ids[cur_ex];
		if (!this_name.null())
		    if (!new_var->base_name.null()) {
			if (new_var->base_name != this_name)
			    new_var->base_name = Const_String(); 
		    } else {
			new_var->base_name = this_name;
		    }
	    }
	    break;
	default:
	    PresErrAssert("Unsupported var type in MapRel2");
	    break;
	}
    }

    Variable_ID_Tuple *oldSym = originalr->global_decls();
    for(i=1; i<=(*oldSym).size(); i++)
	{
	Variable_ID v = (*oldSym)[i];
	assert(v->kind()==Global_Var);
	if (v->get_global_var()->arity() > 0)
	    {
	    Argument_Tuple new_of = v->function_of();
	    if (!leave_pufs_untouched) 
		new_of = mapping.get_tuple_fate(new_of, v->get_global_var()->arity());
	    if (new_of == Unknown_Tuple)
		{
		// hopefully v is not really used
		// if we get here, f should have been in DNF,
		//                 now an OR node with conjuncts below
		// we just need to check that no conjunct uses v
#if ! defined NDEBUG
		if (f->node_type() == Op_Conjunct)
		    {
		    assert(f->really_conjunct()->mappedVars.index(v)==0
			   && "v unused");
		    }
#if 0
		else
		    {
		    // assert(f->node_type() == Op_Or);
		    for (List_Iterator<Formula *> conj(f->children());
			 conj;
			 conj++)
			{
			assert((*conj)->really_conjunct()->mappedVars.index(v)==0
			       && "v unused");
			}
		    }
#endif
#endif
		// since its not really used, don't bother adding it to
		// the the global_vars list of the new relation

		continue;

		}
	    if (v->function_of() != new_of)
		{
		Variable_ID new_v=newr->get_local(v->get_global_var(),new_of);
		assert(v != new_v);
		v->remap = new_v;
		sym_remapped = 1;
		}
	    else
		{
		// add symbolic to symbolic list
#if ! defined NDEBUG
		Variable_ID new_v =
#endif
		    newr->get_local(v->get_global_var(), v->function_of());
#if ! defined NDEBUG
		assert(v == new_v);
#endif
		}
	    }
	else
	    {
	    // add symbolic to symbolic list
#if ! defined NDEBUG
	    Variable_ID new_v =
#endif
		newr->get_local(v->get_global_var());
#if ! defined NDEBUG
	    assert(v == new_v);
#endif
	    }
	}

    if (sym_remapped || input_remapped || output_remapped) {
      f->remap();

      // If 2 vars mapped to same variable, combine them
      //There's a column to combine only when there are two equal remap fields.
      Tuple<Variable_ID> vt(0);
      bool combine = false;
      Tuple_Iterator<Variable_ID> t(input_vars);
      for(i=1; !combine && i<=originalr->n_inp(); t++, i++)
	  if (vt.index((*t)->remap))
	      combine = true;
	  else
	      vt.append((*t)->remap);
      Tuple_Iterator<Variable_ID> t2(output_vars);
      for(i=1; !combine && i <= originalr->n_out(); t2++, i++)
	  if (vt.index((*t2)->remap))
	      combine = true;
	  else
	      vt.append((*t2)->remap);
      if (combine) f->combine_columns(); 

      if (sym_remapped) 
    	reset_remap_field(originalr->Symbolic);
      if (input_remapped) 
	reset_remap_field(input_vars,originalr->n_inp());
      if (output_remapped) 
	reset_remap_field(output_vars,originalr->n_out());
      };

      skip_set_checks--;

#ifndef NDEBUG
    if (fe)
      foreach(v,Variable_ID,fe->myLocals,assert(v == v->remap));
#endif
}


Relation projectOntoJust(Relation R, Variable_ID v) {

    skip_set_checks++;

    
    int ivars = R.n_inp(), ovars = R.n_out();
    int ex_ivars= 0, ex_ovars = 0;
    
    assert(v->kind() == Input_Var || v->kind() == Output_Var);
    if (v->kind() == Input_Var) {
	ex_ivars = 1;
	R = Extend_Domain(R,1);
    } else {
	ex_ovars = 1;
	R = Extend_Range(R,1);
    }

    // Project everything except v
    Mapping m(ivars+ex_ivars,ovars+ex_ovars);
    int j;
    for(j = 1; j <=ivars+ex_ivars; j++) m.set_map_in(j, Exists_Var, j);
    for(j = 1; j <=ovars+ex_ovars; j++) m.set_map_out(j, Exists_Var, j+ivars+ex_ivars);
    m.set_map(v->kind(), v->get_position(), v->kind(), v->get_position());

    MapRel1(R, m, Comb_Id,-1,-1);
    R.finalize();
    skip_set_checks--;
    return R;
}

//static 
//void copyEQtoGEQ(GEQ_Handle &g, const EQ_Handle &e, bool negate) {
//extern void copy_constraint(Constraint_Handle H, Constraint_Handle initial);
//    copy_constraint(g, e);
//}


Relation EQs_to_GEQs(NOT_CONST Relation &S, bool ) {
    Relation R = consume_and_regurgitate(S);
    assert(R.is_simplified());
    use_ugly_names++;
    for (DNF_Iterator s(R.query_DNF()); s.live(); s.next())
       s.curr()->convertEQstoGEQs();
    use_ugly_names--;
    return R;
}


void Conjunct::calculate_dimensions(Relation &R, int &ndim_all, int &ndim_domain) {

   Conjunct * c = this;
   Relation rc=Relation(R, c);

   if(relation_debug) {
       fprintf(DebugFile,"{{{\nIn Conjunct::calculate_dimensions:\n");
       rc.prefix_print(DebugFile);
   }

   rc=Approximate(rc);
   Relation rd=rc;

   if(relation_debug) {
       fprintf(DebugFile,"Conjunct::calculate_dimensions: Approximated as:\n");
       rc.prefix_print(DebugFile);
   }

   skip_set_checks++;

   Conjunct * rc_conj=rc.single_conjunct();
   ndim_all=rc.n_inp()+rc.n_out();
   ndim_all-=rc_conj->n_EQs();

   rc = Project_On_Sym(rc);
   rc.simplify();

   if(relation_debug) {
       fprintf(DebugFile,
	       "Conjunct::calculate_dimensions: after project_on_sym\n");
       rc.prefix_print(DebugFile);
   }

   int n_eq_sym = 1000;
   for (DNF_Iterator s(rc.query_DNF()); s.live(); s.next()) 
       n_eq_sym = min(n_eq_sym, s.curr()->n_EQs());
   ndim_all+=n_eq_sym;
   skip_set_checks--;

   if (R.is_set()) 
       ndim_domain = ndim_all;
   else 
       {
       /* get dimensions for the domain (broadcasting) */

       rd=Domain(rd);
       rd.simplify();

       if(relation_debug) {
	   fprintf(DebugFile,"Domain is:\n");
	   rd.prefix_print(DebugFile);
       }

       rc_conj=rd.single_conjunct();
       ndim_domain=rd.n_set()-rc_conj->n_EQs()+n_eq_sym;
       }

   if(relation_debug) {
      fprintf(DebugFile,"n_eq_sym=%d \n",n_eq_sym);
      fprintf(DebugFile,"Dimensions: all=%d domain=%d\n}}}\n",
	      ndim_all,ndim_domain);
   }

}


void Relation::dimensions(int & ndim_all, int &ndim_domain) {
	ndim_all = ndim_domain = 0;
    int a,d;
    simplify(2,2);
    for (DNF_Iterator s(query_DNF()); s.live(); s.next()) {
        s.curr()->calculate_dimensions(*this, a, d);
	if (a > ndim_all) ndim_all = a;
	if (d > ndim_domain) ndim_domain = d;
	}
    }
//
// Make a set: assert that it had only input or output variables, make it
// it have only input, set a flag.  Called from domain, range, and difference,
// as well as functions that require a set as input.

void Relation::makeSet() {
  assert(!is_null());
  // Assert that it is a set...
  assert((n_inp() == 0 && n_out() >= 0) || n_inp() >= 0 && n_out() == 0);

  if ((n_inp() == 0 && n_out() != 0) || !is_set()) split();  // split if we'll modify this
  if (n_inp() == 0 && n_out() != 0)  //Inverse the relation
      Inverse(*this);  // Modifies "this"; also returns this but we ignore it
  rel_body->_is_set = true;
}

// Tuple to find values for is input+output
Relation Symbolic_Solution(NOT_CONST Relation &R){
    Relation S = consume_and_regurgitate(R);
    Tuple<Variable_ID> vee;
    skip_set_checks++;
    int i;
    for(i = 1; i <= S.n_inp(); i++) vee.append(input_var(i));
    for(i = 1; i <= S.n_out(); i++) vee.append(output_var(i));
    skip_set_checks--;

    return Solution(S, vee);
}

// Tuple to find values for is given as arg, plus input and output
Relation Symbolic_Solution(NOT_CONST Relation &R, Sequence<Variable_ID> &for_these){
    Relation S = consume_and_regurgitate(R);
    Tuple<Variable_ID> vee;
    skip_set_checks++;
    int i;
    for(Any_Iterator<Variable_ID> it(for_these); it; it++)
	vee.append(*it);
    for(i = 1; i <= S.n_inp(); i++) vee.append(input_var(i));
    for(i = 1; i <= S.n_out(); i++) vee.append(output_var(i));
    skip_set_checks--;

    return Solution(S, vee);
}


// Tuple to find values for is input+output+global_decls
Relation Sample_Solution(NOT_CONST Relation &R) {
    Relation S = consume_and_regurgitate(R);

    Tuple<Variable_ID> vee;

    skip_set_checks++;
    int i;
    for(i = 1; i <= S.global_decls()->size(); i++)
	vee.append((*S.global_decls())[i]);
    for(i = 1; i <= S.n_inp(); i++) vee.append(input_var(i));
    for(i = 1; i <= S.n_out(); i++) vee.append(output_var(i));
    skip_set_checks--;

    return Solution(S,vee);
}





Relation Solution(NOT_CONST Relation &S, Sequence<Variable_ID> &for_these ){
// Tuple to find values for is given as arg

    Relation R = consume_and_regurgitate(S);
    assert(!R.is_null());

    if(!R.is_upper_bound_satisfiable()) {
	return Relation::False(R);
    }
    
    bool inexactAnswer=false;
    if(R.is_inexact()) {
      if(R.is_lower_bound_satisfiable())
	R = Lower_Bound(R); // a solution to LB is a solution to the relation
      else {
	// A solution to the UB may not be a solution to the relation:
	// There may be a solution which satisfies all known constraints, but
	// we have no way of knowing if it satisifies the unknown constraints.
	inexactAnswer = true;
	R = Upper_Bound(R);
      }
    }

    Conjunct *c = 0;
    DNF_Iterator d = R.query_DNF();
    c = *d;

    Sequence<Variable_ID> &vee = for_these;
    Relation current(R, c);
    int i;
    for(i = vee.size()-1; i >= 0; i--) {
	bool some_constraints = false, one_stride = false;
	
	int current_var = vee.size()-i;
	Section<Variable_ID> s(&vee,current_var+1,i); 

	// Query variable in vee[current_var]
	Relation projected = Project(copy(current), s);
	assert(projected.has_single_conjunct());
	DNF_Iterator one = projected.query_DNF();

	EQ_Iterator ei(*one);
	GEQ_Iterator gi(*one);
	EQ_Handle stride;

        // Look for candidate EQ's
	for(; ei; ei++) {
	    if((*ei).get_coef(vee[current_var]) != 0) {
	        if(!Constr_Vars_Iter(*ei,true).live()) { // no wildcards
		    some_constraints = true;
		    // Add this constraint to the current as an EQ
		    current.and_with_EQ(*ei);
		    break;
		} else {
		    one_stride = !one_stride && !some_constraints;
		    stride = *ei;
		}
	    }
	}
	if(ei) 
	    continue; // Found an EQ, skip to next variable
	else if (one_stride && !some_constraints) { 
	    // if unconstrained except for a stride, pick stride as value
	    Constr_Vars_Iter cvi(stride,true);
	    assert(cvi.live());
	    cvi++;
	    if(!cvi) {  // Just one existentially quantified variable
		Relation current_copy = current;
		EQ_Handle eh = current_copy.and_with_EQ();
		for(Constr_Vars_Iter si = stride; si; si++)
		    if((*si).var->kind() != Wildcard_Var){
                        // pick "0" for wildcard, don't set its coef
			eh.update_coef((*si).var, (*si).coef);
		    }
		eh.update_const(stride.get_const());
		if(current_copy.is_upper_bound_satisfiable()){
		    current = current_copy;
		    continue; // skip to next var
		}
	    }
	    some_constraints = true; // count the stride as a constraint
	}

	// Can we convert a GEQ?
	for(; gi; gi++) {
	    if((*gi).get_coef(vee[current_var]) != 0) {
		some_constraints = true;
	        if(!Constr_Vars_Iter(*gi,true).live()) { // no wildcards
		    Relation current_copy = current;
		    // Add this constraint to the current as an EQ & test
		    current_copy.and_with_EQ(*gi);
		    if (current_copy.is_upper_bound_satisfiable()) {
			current = current_copy;
			break;
		    }
		}
	    }
	}
	if (gi) continue; // Turned a GEQ into EQ, skip to next

	// Remove wildcards, try try again
	Relation approx = Approximate(copy(projected));
	assert(approx.has_single_conjunct());
	DNF_Iterator d2 = approx.query_DNF();
	GEQ_Iterator gi2(*d2);
	EQ_Iterator ei2(*d2);

	for(; ei2; ei2++) {
	    if((*ei2).get_coef(vee[current_var]) != 0) {
		some_constraints = true;
	        assert(!Constr_Vars_Iter(*ei2,true).live()); // no wildcards
		Relation current_copy = current;
		// Add this constraint to the current as an EQ & test
		current_copy.and_with_EQ(*ei2);
		if (current_copy.is_upper_bound_satisfiable()) {
		    current = current_copy;
		    break;
		}
	    }
	}
	if(ei2) continue; // Found an EQ, skip to next variable

	for(; gi2; gi2++) {
	    if((*gi2).get_coef(vee[current_var]) != 0) {
		some_constraints = true;
	        assert(!Constr_Vars_Iter(*gi2,true).live()); // no wildcards
		Relation current_copy = current;
		// Add this constraint to the current as an EQ & test
		current_copy.and_with_EQ(*gi2);
		if (current_copy.is_upper_bound_satisfiable()) {
		    current = current_copy;
		    break;
		}
	    }
	}
	if(gi2) continue;
	
	if(!some_constraints) { // No constraints on this variable were found.
	    EQ_Handle e = current.and_with_EQ();
	    e.update_const(-42);  // Be creative!
	    e.update_coef(vee[current_var], 1);
	    continue;
	}

	// If we get here, we failed to find a suitable constraint
	// Can either bail, or look for another conjunct.  For now
	// we bail out.

	fprintf(stderr,"Couldn't find suitable constraint for variable %s:\n",
		vee[current_var]->char_name());
	projected.prefix_print(stderr);
	return Relation::Unknown(R);
    }

    if(inexactAnswer) current.and_with_and()->add_unknown();

    current.finalize();
    return current;
}


Relation Approximate(NOT_CONST Relation &R) {
	return Approximate(R,0);
	};

Relation Approximate(NOT_CONST Relation &input_R, int strides_allowed) {

  Relation R = consume_and_regurgitate(input_R);
  assert(!R.is_null());
  Rel_Body *r = R.split();
  r->simplify(-1,-1);

  if (pres_debug) {
	fprintf(DebugFile,"Computing approximation ");
	if (strides_allowed) fprintf(DebugFile,"with strides allowed ");
	fprintf(DebugFile,"[ \n");
	r->prefix_print(DebugFile);
	};

  use_ugly_names++; 
  for (DNF_Iterator pd(r->simplified_DNF); pd.live(); ) {
    Conjunct *C = pd.curr();
    pd.next();

    int i;
    for(i=0; i<C->problem->nGEQs; i++) 
      C->problem->GEQs[i].touched = 1;

    C->reorder();
    if(C->problem->simplifyApproximate(strides_allowed)==0) 
      r->simplified_DNF->rm_conjunct(C);
    else {
       C->simplifyProblem(1,0,1);
    
       free_var_decls(C->myLocals);  C->myLocals.clear();
   
       Problem *p = C->problem;
       Variable_ID_Tuple new_mapped(0);  // This is expanded by "append"
       for (i=1; i<=p->safeVars; i++) {
         // what is now in column i used to be in column p->var[i]
         Variable_ID v = C->mappedVars[p->var[i]];
         assert (v->kind() != Wildcard_Var);
         new_mapped.append(v);
       }
       assert(strides_allowed || C->problem->nVars == C->problem->safeVars);
       C->mappedVars = new_mapped;  
       for (i = p->safeVars+1; i<=p->nVars; i++) {
	    Variable_ID v = C->declare();
	    C->mappedVars.append(v);
	  }

   
       // reset var and forwarding address if desired.
       p->variablesInitialized = 0;
       for(i=1; i<C->problem->nVars; i++)
         C->problem->var[i] = C->problem->forwardingAddress[i] = i;
     }
  }
 
  if (pres_debug) 
	fprintf(DebugFile,"] done Computing approximation\n");
  use_ugly_names--; 
  return R;
}


Relation Lower_Bound(NOT_CONST Relation &r){
  Relation s = consume_and_regurgitate(r);
  s.interpret_unknown_as_false();
  return s;
}

Relation Upper_Bound(NOT_CONST Relation &r) {
  Relation s = consume_and_regurgitate(r);
  s.interpret_unknown_as_true();
  return s;
}
} // end of namespace omeganamespace omega {
