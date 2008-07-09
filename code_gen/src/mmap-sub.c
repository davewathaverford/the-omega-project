#include <code_gen/mmap-sub.h>
#include <code_gen/mmap-util.h>
#include <basic/Exit.h>

namespace omega {

#if ! defined DONT_QUIT_FOR_FAILED_SUBSTITUTION
#define DONT_QUIT_FOR_FAILED_SUBSTITUTION (getenv("DONT_QUIT_FOR_FAILED_SUBSTITUTION") ? atoi(getenv("DONT_QUIT_FOR_FAILED_SUBSTITUTION")) : 0)
#endif


// information to build the expression
//  "(input_var_coef * input_var + offset) mod modbase = 0"

struct modinfo {
  int input_var;
  int input_var_coef;
  int offset;
  int modbase;   // 0 = failure
};


// check_mod
// Check to see if output variable #o is defined as (i_coef * i + offset) mod m,
//  where i is an input var and offset and m are constants.
// In fact, we check for constraints of the form
//  exists k s.t. 0 <= o < modbase &&  o = i_coef * i + offset + k * modbase
// if "tightmod" is set, allow o < x for any x < modbase instead of o < modbase

static modinfo check_mod(Relation &R, int o, bool tightmod)
{
  // R must be a single conjunct

  modinfo m, badm;
  m.modbase = 0;
  m.input_var = 0;
  m.input_var_coef = 0;
  badm = m;

  if (tcodegen_debug)
    {
      fprintf(DebugFile, "%s%s%d%s\n", debug_mark_cp,
	      "looking for mod substitution for output variable #",
	      o,
	      " in relation");
      fprintf(DebugFile, "%s", debug_mark_cp);
      R.print_with_subs(DebugFile);
    }


  // look for:  exists k s.t. 0 <= o < modbase && o = i + offset + k * modbase

  Variable_ID ov = R.output_var(o);
  Variable_ID k = 0;

  // Start by looking for
  //	exists k s.t.  o = i_coef * i + offset + k * modbase
  // We may also need to ignore "modbase * anything_else" terms,
  //  which we call "junk terms".
  // This is most easily done if we just hunt down modbase first

  DNF_Iterator di(R.query_DNF());
  for (EQ_Iterator ei = (*di)->EQs(); ei; ei++)
    {
      int o_coef = (*ei).get_coef(ov);
      if (o_coef != 0)
	{
	  if (m.modbase   != 0 ||  // there can be ... only one
	      abs(o_coef) != 1)    // above form requires this
	    return badm;

	  assert(m.input_var == 0);

	  // First, just figure out what modbase is
	  // Currently just find the (hopefully unique) wildcard/exists var.
	  // This does not allow junk wildcard terms.

	  for (Constr_Vars_Iter cvi(*ei); cvi; cvi++)
	    {
	      Variable_ID v = (*cvi).var;
	      int v_coef    = (*cvi).coef;

	      if ((v->kind() == Wildcard_Var ||
		   v->kind() == Exists_Var))
		{
		  if (m.modbase == 0)
		    {
		      m.modbase = abs(v_coef);
		      k = v;
		      assert(m.modbase != 0);
		    }
		  else
		    {
		      if (tcodegen_debug)
			{
			  fprintf(DebugFile, "%s%s\n", debug_mark_cp,
				  "giving up on possible junk wildcard term --"
				  " test, though simple, is not implemented");
			}
			return badm;
		    }
		}
	    }

 	  if (m.modbase == 0)
	    return badm;
	  assert(k);

	  {   // extra braces apparently avoid Visual C++ bug
	  for (Constr_Vars_Iter cvi(*ei); cvi; cvi++)
	    {
	      Variable_ID v = (*cvi).var;
	      int v_coef    = (*cvi).coef;
	      // we could find i, k (again), ov (again), junk, or a problem
	      // throw out junk terms first, in case of junk i before real i
	      if ((v_coef % m.modbase) != 0)
		{
		  if ((v->kind() == Input_Var) && // found i
		      m.input_var == 0)		  // there can be ... only one
		    {
		      assert(v_coef);
		      assert(abs(o_coef) == 1);
		      m.input_var_coef = v_coef * -o_coef;
		      m.input_var = v->get_position();
		      assert(m.input_var != 0);
		    }
		  else if (v != ov)
		    {
		      // Anything else is a legal junk term or "k" again
		      // as long as its divisible by m.modbase.
		      return badm;
		    }
		}
	    }
	  }

	  if (m.input_var == 0) // found eq, but not base
	    return badm;

	  m.offset = -o_coef * (*ei).get_const();
	  // now, as long as we don't find another eq, we should be set
	}
    }

  if (m.modbase == 0)   // no eq involving ov
    return badm;


  // only that one eq should use k

  assert(k);
  bool k_used_already = false;
  {
  for (EQ_Iterator ei = (*di)->EQs(); ei; ei++)
    {
      if ((*ei).get_coef(k))
	if (k_used_already)
	  return badm;
	else
	  k_used_already = true;
    }
  }

  // we've found the right eq, now check for the inequalities 0 <= o < modbase
  // also test for other (inappropriate) uses of k

  bool found_lb = false, found_ub = false;
  
  for (GEQ_Iterator gi = (*di)->GEQs(); gi; gi++)
    {
      int ov_coef = (*gi).get_coef(ov);
      if (ov_coef)
	{
	  if (ov_coef > 0)  //    hope for ov >= 0
	    {
	      if (ov_coef != 1 || (*gi).get_const() != 0)
		return badm;
	      found_lb = true;
	    }
	  else  // ov_coef < 0    hope for modbase-1 - ov >= 0
	    {
	      if (ov_coef != -1)
		return badm;
	      if (!(
		    ((*gi).get_const() == m.modbase-1)  // got it exactly
		    // we'll settle for x-ov >= 0 for x<=modbase-1 if looking for "tight" mod
		    || (tightmod && (*gi).get_const() <= m.modbase-1)))
		return badm;
	      if (tcodegen_debug && (*gi).get_const() < m.modbase-1)
		{
		  fprintf(DebugFile, "%s%s%d%s%d\n", debug_mark_cp,
			  "found \"tight\" mod constraint output < ",
			  (*gi).get_const(),
			  " rather than output < ",
			  m.modbase-1);
		}
	      found_ub = true;
	    }
	  // all other coefficients must be 0 for this geq
	  for (Constr_Vars_Iter cvi(*gi); cvi; cvi++)
	    {
	      if ((*cvi).var != ov && (*cvi).coef)
		return badm;
	    }
	}

      // also, k should not appear in any inequalities
      if ((*gi).get_coef(k))
	return badm;
    }
  
  if (!found_lb || !found_ub)
    return badm;

  if (tcodegen_debug)
    {
      fprintf(DebugFile, "%s%s%d%s%d%s%d%s%d\n", debug_mark_cp,
	      "found substitution: (",
	      m.input_var_coef,
	      "* input variable #",
	      m.input_var,
	      "+",
	      m.offset,
	      ") mod ",
	      m.modbase);
    }

#if ! defined NDEBUG
  if (m.modbase)
    {
      // build { [ ... i ... ] -> [ ... (ic * i + offset) mod modbase ... ] }
      // build { [ ... i ... ] -> [ ... o ... ] : exists k s.t. 
      //	    0 <= o < modbase &&  o = ic * i + offset + k * modbase }
      Relation checkmod(R.n_inp(), R.n_out());
      Variable_ID ic = checkmod.input_var(m.input_var);
      Variable_ID oc = checkmod.output_var(o);
      F_Exists *exists_k = checkmod.add_exists();
      Variable_ID  k = exists_k->declare("o_div_modbase");
      F_And *suchthat = exists_k->add_and();
      // 0 <= o
      GEQ_Handle o_geq_0 = suchthat->add_GEQ();
      o_geq_0.update_coef(oc,1);
      // o < modbase --> modbase-1-o >= 0
      GEQ_Handle o_lt_checkmod = suchthat->add_GEQ();
      o_lt_checkmod.update_coef(oc,-1);
      o_lt_checkmod.update_const(m.modbase-1);
      // ic * i + offset + k * modbase - o = 0
      EQ_Handle o_eq = suchthat->add_EQ();
      o_eq.update_coef(ic,m.input_var_coef);
      o_eq.update_const(m.offset);
      o_eq.update_coef(k,m.modbase);
      o_eq.update_coef(oc,-1);
      checkmod.finalize();

      checkmod = Intersection(checkmod, copy(R));

      // by definition, checkmod is a subset of R;
      // if the above mod constraint is redundant, R is a subset of checkmod

      assert(Must_Be_Subset(copy(checkmod),copy(R)));  // the obvious one
      assert(Must_Be_Subset(copy(R),checkmod));  // mod constraint is redundant
    }
#endif

  return m;
}


/*
    The functionality above is sufficient for the 1d and 2d time skewing
    examples I've tried, except for six cases in which we need to get
    substitutions for Out_1 and Out_3 from the equality:
	exists alpha : In_8 = 7 + 8 Out_1 + 8 In_4 + Out_3 + 16 alpha
    and we know that Out_1 and Out_3 are both 0 or 1.
    Furthermore, alpha is not in any other constraints, though this
    does not matter.

    If this case applies, we return the substitution as shown below;
    if not, we return "".

    The above equality and bounds on Out_1 and Out_3 imply:

	 Out_3 = (In_8-8In_4-7) mod 8
	8Out_1 = (In_8-8In_4-7-Out_3) mod 16

    The first can be generated as a substitution - first we verify
    the redundancy of adding it as a constraint, thus:

	exists k : 0 <= Out_3 < 8 &&  Out_3 = In_8 - 8*In_4 - 7 + 8*k

    We rewrite the second as follows:

	 Out_1 = ( (In_8-8In_4-7-Out_3) mod 16 ) div 8
	       = ( (In_8-8In_4-7-((In_8-8In_4-7) mod 8)) mod 16 ) div 8
	       = ( (8*((In_8-8In_4-7) div 8)) mod 16 ) div 8

    We generate that substitution after verifying the redundancy of
    adding it as a constraint, thus:

         exists stuff_floor_to_8, that_mod_16 :
	 	stuff_floor_to_8 <= In_8-8In_4-7 and
	 	stuff_floor_to_8 >  In_8-8In_4-7 - 8 and
		exists div8 : stuff_floor_to_8 = 8 div8 and
		0 <= that_mod_16 < 16 and 
		exists div16 : that_mod_16 + 16 div16 = stuff_floor_to_8 and
		8 Out_1 <= that_mod_16 < 8 (Out_1+1)
 */

static String desperate_check_mod_kludge_for_time_skewing(Relation &R, int o)
{
  Variable_ID Out_1, Out_3, In_4, In_8, alpha, v, k1, k2, k3, k4;
  int Out_1_coef, Out_3_coef, v_coef, const_term;
  int sign;
  F_Exists *exists_k1, *exists_k1k2, *exists_k3, *exists_k4;
  F_And *st_k1, *st_k1k2, *st_k3, *st_k4;
  GEQ_Handle geq_L_k1, geq_U_k1, geq_L_k3, geq_U_k3, geq_L_k4, geq_U_k4;
  EQ_Handle eq_k1, eq_k3, eq_k4;

  // R must be a single conjunct


  // PROBLEM:  FOR SOME REASON, THE RELEVANT EQUALITY SHOWS UP AS
  // 	t8 == 7+8*xbmod2+8*t4+Out_3+16*alpha  // note Out_1 is xbmod2
  // WHEN WE DO PRINT_WITH SUBS, BUT THE "TRUE" FORM IS
  //	8*xbmod2+8*t4+t8 == 7+Out_3+16*alpha
  // AS EVIDENCED BY THIS prefix_print.
  //
  // IS THIS CORRECT?  PRESUMABLY SO...
  // BASED ON AN UNDERSTANDING OF HOW ITS CORRECT, WE NEED TO
  // GENERALIZE OR CHANGE THE TEST BELOW.

  // R.prefix_print(stdout);

  if (tcodegen_debug)
    {
      fprintf(DebugFile, "%s%s\n", debug_mark_cp,
	      "desperately looking for special case mod and div substitution.");
    }

  // check that the only EQ constraint using Out_1 and Out_3 is of the form
  //	exists alpha : 8*Out_1 + 8*In_4 + In_8 == 7 + Out_3 + 16*alpha

  Out_1 = R.output_var(1);
  Out_3 = R.output_var(3);
  In_4=0, In_8=0, alpha=0;	// look for these vars in the loop --
				// In_4 must play the appropriate role
				// in the equality above, but may not
				// actually be the 4th input variable.
  DNF_Iterator di(R.query_DNF());

  // This loop looks for any one equality of the right form.
  // We we find it, we break out of the loop with alpha, In_4, and In_8 set;
  // When we discover we've got the wrong form, we continue the loop.

  for (EQ_Iterator ei = (*di)->EQs(); ei; ei++, alpha = In_4 = In_8 = 0)
    {
      Out_1_coef = (*ei).get_coef(Out_1);
      Out_3_coef = (*ei).get_coef(Out_3);
      if (abs(Out_3_coef) == 1 && Out_1_coef * Out_3_coef == -8)
	{
	  sign = Out_3_coef;
	  const_term = -sign * (*ei).get_const();
	  for (Constr_Vars_Iter cvi(*ei); cvi; cvi++)
	    {
	      v = (*cvi).var;
	      v_coef = (*cvi).coef;

	      if (v->kind() == Wildcard_Var ||
		  v->kind() == Exists_Var)	// must be alpha
		{
		  if (alpha || v_coef != 16)
		    goto continue_ei;
		  alpha = v;
		}
	      
	      else if (v_coef % 16 == 0)  // anything else doesn't matter
		continue;

	      else if (v->kind() == Output_Var)	// must be Out_3 or Out_1
		{
		  if (v != Out_1 && v != Out_3)
		    goto continue_ei;
		}

	      else if (v->kind() == Input_Var)	// must be In_4 or In_8
		{
		  if (v_coef * sign == -1)
		    {
		      if (In_8)
			goto continue_ei;
		      In_8 = v;
		    }
		  else if (v_coef * sign == -8)
		    {
		      if (In_4)
			goto continue_ei;
		      In_4 = v;
		    }
		  else
		    goto continue_ei;
		}

	      else // some other kind of variable I haven't considered
		goto continue_ei;

	    }  // for all vars in constraint

	  if (alpha && In_4 && In_8)
	    break;

	}  // end of "if Out and In have reasonable coefficients"

    continue_ei:
      ;
    }   // end of equality iteration

  if (! (alpha && In_4 && In_8))
    return "";

  // if o=3, assert (and return) substitution (In_8-8In_4-7) mod 8
  if (o == 3)
    {
#if ! defined NDEBUG
      // DO AN ASSERTION THAT THIS IS CORRECT

      Relation checkmod(R.n_inp(),R.n_out());

      assert(Out_1->kind() == Output_Var && Out_1->get_position() == 1);
      assert(Out_3->kind() == Output_Var && Out_3->get_position() == 3);
      assert(In_4->kind() == Input_Var);      // Note then In_4 may not actually be input var #4.
      assert(In_8->kind() == Input_Var);
      //Variable_ID cm_out_1 = checkmod.output_var(Out_1->get_position());
      Variable_ID cm_out_3 = checkmod.output_var(Out_3->get_position());
      Variable_ID cm_in_4  = checkmod.input_var (In_4->get_position());
      Variable_ID cm_in_8  = checkmod.input_var (In_8->get_position());

      exists_k1=checkmod.add_exists();
      k1=(*exists_k1).declare("k1_modbase");
      st_k1=(*exists_k1).add_and();
      geq_L_k1=(*st_k1).add_GEQ();
      geq_L_k1.update_coef(cm_out_3,1);
      geq_U_k1=(*st_k1).add_GEQ();
      geq_U_k1.update_coef(cm_out_3,-1);
      geq_U_k1.update_const(7);
      eq_k1=(*st_k1).add_EQ();
      eq_k1.update_coef(cm_out_3,-1);
      eq_k1.update_coef(cm_in_8,1);
      eq_k1.update_coef(cm_in_4,-8);
      eq_k1.update_coef(k1,8);
      eq_k1.update_const(-7);
      checkmod.finalize();

      checkmod = Intersection(checkmod, copy(R));
      // by definition, checkmod is a subset of R;
      // if the above mod constraint is redundant, R is a subset of checkmod
      assert(Must_Be_Subset(copy(checkmod),copy(R)));  // the obvious one
      assert(Must_Be_Subset(copy(R),checkmod));  // mod constraint is redundant

#endif
      return (String) "intMod(" + In_8->char_name() + "-8*" +
	In_4->char_name() + "+" + itoS(const_term) + ",8)";
      // return "intMod(t8-8*t4-7,8)";
    }


  // if o=1, assert&return the sub ( (8*((In_8-8In_4-7) div 8)) mod 16 ) div 8
  else if (o == 1)
    {
#if ! defined NDEBUG
      Relation checkmod(R.n_inp(),R.n_out());

      assert(Out_1->kind() == Output_Var && Out_1->get_position() == 1);
      assert(Out_3->kind() == Output_Var && Out_3->get_position() == 3);
      assert(In_4->kind() == Input_Var);      // Note then In_4 may not actually be input var #4.
      assert(In_8->kind() == Input_Var);
      Variable_ID cm_out_1 = checkmod.output_var(Out_1->get_position());
      //Variable_ID cm_out_3 = checkmod.output_var(Out_3->get_position());
      Variable_ID cm_in_4  = checkmod.input_var (In_4->get_position());
      Variable_ID cm_in_8  = checkmod.input_var (In_8->get_position());
  
      exists_k1k2=checkmod.add_exists();
      k1=(*exists_k1k2).declare("k1_modbase");
      k2=(*exists_k1k2).declare("k2_modbase");
      st_k1k2=(*exists_k1k2).add_and();
      geq_L_k1=(*st_k1k2).add_GEQ();
      geq_L_k1.update_coef(k1,1);  
      geq_L_k1.update_coef(cm_in_8,-1); 
      geq_L_k1.update_coef(cm_in_4,8); 
      geq_L_k1.update_const(14); 
      geq_U_k1=(*st_k1k2).add_GEQ();
      geq_U_k1.update_coef(k1,-1);
      geq_U_k1.update_coef(cm_in_8,1);
      geq_U_k1.update_coef(cm_in_4,-8);
      geq_U_k1.update_const(-7);

      exists_k3=(*st_k1k2).add_exists();
      k3=(*exists_k3).declare("k3_modbase");
      st_k3=(*exists_k3).add_and();
      geq_L_k3=(*st_k3).add_GEQ();
      geq_L_k3.update_coef(k2,1);
      geq_U_k3=(*st_k3).add_GEQ();
      geq_U_k3.update_coef(k2,-1);
      geq_U_k3.update_const(15);
      eq_k3=(*st_k3).add_EQ();
      eq_k3.update_coef(k1,1);
      eq_k3.update_coef(k3,-8);
     
      exists_k4=(*st_k3).add_exists();
      k4=(*exists_k4).declare("k4_modbase");
      st_k4=(*exists_k4).add_and();
      geq_L_k4=(*st_k4).add_GEQ();
      geq_L_k4.update_coef(k2,1);
      geq_L_k4.update_coef(cm_out_1,-8);
      geq_U_k4=(*st_k4).add_GEQ();
      geq_U_k4.update_coef(k2,-1);
      geq_U_k4.update_coef(cm_out_1,8);
      geq_U_k4.update_const(7);
      eq_k4=(*st_k4).add_EQ();
      eq_k4.update_coef(k1,-1);
      eq_k4.update_coef(k2,1);
      eq_k4.update_coef(k4,16);

      checkmod.finalize();
      checkmod = Intersection(checkmod, copy(R));

      // by definition, checkmod is a subset of R;
      // if the above mod constraint is redundant, R is a subset of checkmod
      assert(Must_Be_Subset(copy(checkmod),copy(R)));  // the obvious one
      assert(Must_Be_Subset(copy(R),checkmod));  // mod constraint is redundant


#endif
      return (String) "intDiv(intMod(8*intDiv(" + (*In_8).char_name() + 
        "-8*" + (*In_4).char_name() + "+" + itoS(const_term) + ",8),16),8)";
      // return "intDiv(intMod(8*intDiv(t8-8*t4-7,8),16),8)";
    }

  else return "";
}



/*  get_sub
    Try to get a substitution for output variable o of Relation R,
    first using the Substitution s, and otherwise by checking for
    a "mod" constraint.
 */

String get_sub(Relation &R, int o, Substitutions &subs)
{
  // R must be a single conjunct,
  // and s must be substitutions for that conjunct

  
  assert(R.query_DNF()->length() == 1);
  Variable_ID ov = R.output_var(o);
  modinfo m;

  if (subs.substituted(ov))
    {
      return subs.get_sub(ov).print_term_to_string();
    }
  else if (m = check_mod(R, o, true), m.modbase)
    {
      return (String) "intMod(" + itoS(m.input_var_coef) + "*" + R.input_var(m.input_var)->char_name() + "+" + itoS(m.offset) + "," + itoS(m.modbase) + ")";
    }
  else 
    {
      String result = "";
      if (R.n_out() >= 3)
	  result = desperate_check_mod_kludge_for_time_skewing(R, o);
      if (result != (String) "")
	{
	  return result;
	}
      else
	{
	  if (DONT_QUIT_FOR_FAILED_SUBSTITUTION)
	    {
	      if (tcodegen_debug)
		{
		  fprintf(DebugFile, "%s%s\n", debug_mark_cp,
			  "failed to find substitution.\n");
		  if (tcodegen_debug > 1)
		    {
		      fprintf(DebugFile, "%s%s\n", debug_mark_cp,
			      "In prefix_print form, R is:");
		      R.prefix_print(DebugFile);
		    }
		}
	      return "In tcodegen, no substitution was possible for the expression that was needed here so this won't compile - for details, run with -Dg1 and look for \"failed to find substitution\" in oc.out.";
	    }
	  else
	    {	  
	      cerr << "The current tcodegen implementation needs to find a subsitution for each\n";
	      cerr << "output variable, but can't do so for output variable #" << o << " in\n";
	      cerr << R.print_with_subs_to_string();
	      Exit(1);
	      return "Shut up little man"; // make the compiler quit complaining
	    }
	}
    }
}

} // end namespace omega
