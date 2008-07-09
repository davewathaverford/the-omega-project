/* Code generation output routines contributed by Fortran D group.
   Modified slightly to move out of the omega core and presburger, using
   new Substitutions interface. */

#include <omega.h>
#include <code_gen/stmt_builder.h>
#include <omega/pres_subs.h>

namespace omega {


 //***********************************************************************
// Methods added by D people, 
// Lei Zhou, Apr 19, 96.
//***********************************************************************

//***********************************************************************
// Static implementation added by D people.
// Lei Zhou, Apr 24, 96.
//***********************************************************************

#if defined QUESTIONABLE_CODEGEN_UPGRADE_MAYBE_NEEDED_FOR_TCODEGEN
static bool nonunit_coef(Variable_ID v, Conjunct *C){
  for(EQ_Iterator e(C); e; e++)
    if ((*e).get_coef(v) != 0 && abs((*e).get_coef(v)) != 1) return true;
  return false;
}

static Relation project_other_outputs(Variable_ID v, const Relation &Crel){
    // Ugh.  Must be some horrible thing Dave or a Davestudent might
    // come up with. 
  assert(v->kind() == Output_Var);
  Relation K = Crel;
  Section<Variable_ID> rest_of_output(&output_vars, v->get_position()+1,
				      K.n_out());
  if (rest_of_output.size()) K = Project(K, rest_of_output);
  Section<Variable_ID> beg_of_output(&output_vars,1, v->get_position()-1);
  if (beg_of_output.size())  K = Project(K, beg_of_output);
  return K;
}
#endif

//-------------------------------------------------------------------
// from pres_print.c, original name tryToPrintVarToStringWithDiv()
// this function never gets invoked during our code_gen tests.
//  assert(0 && "tryToPrintVarToStringWithDiv");
//-------------------------------------------------------------------

CG_outputRepr* tryToPrintVarToReprWithDiv(CG_outputBuilder* ocg,
					  Conjunct *C, Variable_ID v)
{
  CG_outputRepr *varRepr = CG_REPR_NIL;

  bool seen = false;
  // This assumes that there is one EQ involving v, and that v has a
  // non-unit coefficient.

  // Despite the above comment, if we add an assertion to this effect,
  // then it breaks on some cases where we got an answer before.
  // assert(nonunit_coef(v,C));

  for(EQ_Iterator e(C); e; e++) {
    if ((*e).get_coef(v) != 0) {
      assert(!seen);  // This asserts just one EQ with v 
      coef_t v_coef = (*e).get_coef(v);
      int v_sign = v_coef > 0 ? 1 : -1;
      v_coef *= v_sign;
      int sign_adj = -v_sign;
      
      CG_outputRepr* lopRepr = CG_REPR_NIL;

      for(Constr_Vars_Iter i(*e,false); i; i++) {
	if ((*i).var != v && (*i).coef != 0) {
	  coef_t this_coef = sign_adj*(*i).coef;

	  CG_outputRepr* varNameRepr = ocg->CreateIdent((*i).var->name());

	  if (this_coef == 1) {
	    lopRepr = ocg->CreatePlus(lopRepr, varNameRepr);
	  }
	  else if (this_coef == -1) {
	    lopRepr = ocg->CreateMinus(lopRepr, varNameRepr);
	  }
	  else {
	    CG_outputRepr* coefRepr = ocg->CreateInt(this_coef);
	    CG_outputRepr* termRepr = ocg->CreateTimes(coefRepr, varNameRepr);
	    lopRepr = ocg->CreatePlus(lopRepr, termRepr);
	  }
	}
      }

      coef_t the_const = (*e).get_const()* sign_adj;
      if (the_const != 0) {
	lopRepr = ocg->CreatePlus(lopRepr, ocg->CreateInt(the_const));
      }

      CG_outputRepr* divRepr 
	= ocg->CreateIntegerDivide(lopRepr, ocg->CreateInt(v_coef));
      varRepr = ocg->CreatePlus(varRepr, divRepr);

      seen = true;
    }
  }
  return varRepr;
}


// This function prints the output tuple of a relation with each one as a
// function of the input variables.

  /* There are two reasons Substitutions might fail.
     One is that v may have a non-unit coefficient, and requires an
     intDiv in the function computing it.  The other is just that
     substitutions just weren't smart enough, because of some kind of
     EQ chain.
     */

CG_outputRepr* print_outputs_with_subs_to_repr(Relation &R, 
					       CG_outputBuilder* ocg)	
{
  CG_outputRepr* oRepr = ocg->CreateArguList();

  Relation S(R);
  S.setup_names();
  assert(S.has_single_conjunct());
  Conjunct *C = S.single_conjunct();

  Substitutions subs(S,C);
  // Print output names with substitutions in them
  CG_outputRepr *tRepr = CG_REPR_NIL;
  for(int i=1; i<=S.n_out(); i++) {
    Variable_ID v = output_vars[i];
#if defined QUESTIONABLE_CODEGEN_UPGRADE_MAYBE_NEEDED_FOR_TCODEGEN
    if(subs.substituted(v) && !subs.sub_involves(v,Output_Var)) {
	tRepr = print_sub_to_repr(ocg, subs.get_sub(v));
//
//    Need to put this back in to make regression tests work--
//    but is this ok for tcodegen
//
//    } else if (!subs.substituted(v)){
//	tRepr = tryToPrintVarToReprWithDiv(ocg, C, output_vars[i]);
//
    } else {
	// Too difficult to find a sub for, or wants to sub with other
	// output vars; do projection of other outputs to simplify
	Relation Kp = project_other_outputs(v,S);
        assert(Kp.has_single_conjunct());
	Conjunct *Kc = Kp.single_conjunct();
	Substitutions Ksubs(Kp,Kc);
	if(Ksubs.substituted(v)) {
	  tRepr = print_sub_to_repr(ocg, Ksubs.get_sub(v));
	} else {
	  if (!nonunit_coef(v,Kc))
	    {
	      fprintf(stderr, "substitution too difficult.\n");
	      Exit(1);
	    }
	  tRepr = tryToPrintVarToReprWithDiv(ocg, Kc, v);
	}
    }
#else
    // code from 1.1.0 verbatim (except for this comment)
    if(subs.substituted(v)) {
	tRepr = print_sub_to_repr(ocg, subs.get_sub(v));
    } else {
	tRepr = tryToPrintVarToReprWithDiv(ocg, C, output_vars[i]);
    }
#endif
    oRepr = ocg->ArguListInsertLast(oRepr, tRepr);
  }
  return oRepr;
}

//-------------------------------------------------------------------------
// function changed in oc_print.c, 
// original name is 
// String Problem::print_term_to_string(const eqn * e,int c) const 
//-------------------------------------------------------------------------
CG_outputRepr* print_term_to_repr(CG_outputBuilder* ocg, const Sub_Handle &s, int c)
{
  CG_outputRepr *cgRepr = CG_REPR_NIL;

  Variable_ID wentFirst = 0;

  for(Constr_Vars_Iter si(s); si; si++) {
    coef_t sc = (*si).coef;
    Variable_ID sv = (*si).var;
    if (c * sc > 0) {
      wentFirst = sv;

      if (c * sc == 1) {
	cgRepr = ocg->CreateIdent(sv->name());
      }
      else {
	cgRepr = ocg->CreateTimes(ocg->CreateInt(c * sc),
	                          ocg->CreateIdent(sv->name()));
      }
      break;
    }
  }

  for(Constr_Vars_Iter si2(s); si2; si2++) {
    coef_t sc = (*si2).coef;
    Variable_ID sv = (*si2).var;
    if (sv != wentFirst && c * sc != 0) {
      if (c * sc == 1) {
	cgRepr = ocg->CreatePlus(cgRepr, ocg->CreateIdent(sv->name()));
      }
      else if (c * sc == -1) {
	cgRepr = ocg->CreateMinus(cgRepr, ocg->CreateIdent(sv->name()));
      }
      else {
	// the original code here distinguishes the print_in_code_gen_style
	// if yes, the output looks like: 12*term, else 12term. Lei
	CG_outputRepr* cgRepr1 = ocg->CreateTimes(ocg->CreateInt(c*sc), 
	                                             ocg->CreateIdent(sv->name()));
	cgRepr = ocg->CreatePlus(cgRepr, cgRepr1);
      }
    }
  }

  if (c * s.get_const() != 0 || !wentFirst) {
    if ( c * s.get_const() >= 0) {
      cgRepr = ocg->CreatePlus(cgRepr, ocg->CreateInt(c * s.get_const()));
    }
    else {
      cgRepr = ocg->CreateMinus(cgRepr, ocg->CreateInt(- c * s.get_const()));
    }
  }
  
  return cgRepr;
}


//-------------------------------------------------------------------------
// function changed in oc_print.c, 
// original name is String Problem::print_sub_to_string(int v) const 
//-------------------------------------------------------------------------
CG_outputRepr* print_sub_to_repr(CG_outputBuilder* ocg, const Sub_Handle &s)
{
  CG_outputRepr *cgRepr;
  cgRepr = print_term_to_repr(ocg, s, 1);
  return cgRepr;
}

} // end namespace omega
