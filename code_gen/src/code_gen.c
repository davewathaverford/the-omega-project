#include <basic/bool.h>
#include <omega.h>
#include <omega/Rel_map.h>
#include <code_gen/elim.h>
#include <code_gen/code_gen.h>
#include <basic/Collection.h>
#include <basic/Bag.h>
#include <basic/Map.h>
#include <basic/util.h>
#include <math.h>
#include <string>

//*****************************************************************************
// Omega code gen builder and internal representation
//*****************************************************************************
#include <code_gen/CG_outputBuilder.h>
#include <code_gen/CG_outputRepr.h>

#include <code_gen/CG_stringBuilder.h>
#include <code_gen/CG_stringRepr.h>

#include <code_gen/stmt_builder.h>

namespace omega {

#if ! defined DONT_EVEN_TRY_TO_COMPILE_CODE_GEN_WITH_CFRONT


static int last_level;// Should not be global, but it is.
static int OMEGA_WHINGE = -1;

SetTuple function_guards;
SetTuple new_IS;
SetTupleTuple projected_nIS;
Tuple<naming_info *> statementNameInfo;
bool gen_python=false;


// ******************************************
//  Added by davew 1/28/99 to keep g++ happy
// ******************************************


CG_result::~CG_result()
{
}

naming_info::~naming_info()
{
}


//*****************************************************************************
// Declarations added by D people. Lei Zhou Apr. 20, 96
//*****************************************************************************
static
CG_outputRepr* outputEasyBoundasRepr(CG_outputBuilder* ocg,
				     const Constraint_Handle &g, 
                                     Variable_ID v, bool ignoreWC,
		                     int ceiling, bool wrapInParens = false);

//*****************************************************************************
// static functions declared by Omega people. 
//*****************************************************************************
// used only by CG_leaf methods 
//-----------------------------------------------------------------------------
static
String outputEasyBoundasString(const Constraint_Handle &g, Variable_ID v, bool ignoreWC,
		       int ceiling, bool wrapInParens = false);

//*****************************************************************************
// static functions declared by D people. 
//*****************************************************************************
static
CG_outputRepr* outputStatements(CG_outputBuilder* ocg, int stmt, int indent, 
                                Relation &mapping, Relation &known);

//-----------------------------------------------------------------------------

//*****************************************************************************
// static functions declaration added by D people. Lei Zhou, Apr 23, 96
//*****************************************************************************
static 
bool outputAssignment(CG_outputBuilder* ocg,
                      Conjunct *c, Variable_ID v, int indent,
		      CG_outputRepr* &aRepr, 
		      Relation &new_known, elimination_info &eliminated);


static
bool printBounds(CG_outputBuilder* ocg,
                 Relation &b, Variable_ID v, int indent,
		 NOT_CONST Relation &input_known, Relation &new_known, 
		 CG_outputRepr* &ctrlRepr,
                 elimination_info &eliminated);
//-----------------------------------------------------------------------------

static
CG_outputRepr* output_guard(CG_outputBuilder* ocg, Relation &guards_in);

CG_outputRepr* outputAsGuard(CG_outputBuilder* ocg, 
			     NOT_CONST Constraint_Handle &e, 
                             bool is_geq);

CG_outputRepr* outputStrideAsRepr(CG_outputBuilder* ocg, 
				  NOT_CONST EQ_Handle &e);

CG_outputRepr* output_GEQ_strides(CG_outputBuilder* ocg, Relation &guards, 
				  Conjunct *c);
//-----------------------------------------------------------------------------

static
CG_outputRepr *outputLBasRepr(CG_outputBuilder* ocg, const GEQ_Handle &g, 
                              Relation &bounds, Variable_ID v,
			      coef_t stride, const EQ_Handle &strideEQ,
			      Relation known);

static
CG_outputRepr *outputUBasRepr(CG_outputBuilder* ocg, const GEQ_Handle &g, 
                              Relation &bounds, Variable_ID v,
			      coef_t stride, const EQ_Handle &strideEQ);

//*****************************************************************************
// static function added by D people. May 20.
//*****************************************************************************
static String GetString(CG_outputRepr* repr);


static bool boundHitsStride(const GEQ_Handle &g, Variable_ID v,
			    const EQ_Handle &strideEQ,
			    coef_t stride, Relation known);
static int countStrides(Conjunct *c, Variable_ID v, EQ_Handle &strideEQ, 
			bool &simple);




//-----------------------------------------------------------------------------

static CG_result * gen_recursive(int level, IntTuple &isActive);

// A utility functions for generating statements....
bool operator==(const naming_info &, const naming_info &) {
    assert(0);
    return false;
}


static
String outputSpaces(int indent_level) {
    String spaces;
    for(int i=1; i<indent_level; i++) spaces += "  ";
    return spaces;
}

//*****************************************************************************
CG_outputRepr* outputIdent(CG_outputBuilder* ocg, Variable_ID v){

  if(v->kind() != Global_Var || v->get_global_var()->arity() == 0)
    return ocg->CreateIdent(v->name());
  else {
    /* This should be improved to take into account the possible elimination
       of the set variables. */
    int arity = v->get_global_var()->arity();
    assert(arity <= last_level);
    CG_outputRepr *argList = ocg->CreateArguList();
    Relation R = Relation::True(arity);
    name_codegen_vars(R); // easy way to make sure the names are correct.
    for(int i = 1; i <= arity; i++)
      argList =ocg->ArguListInsertLast(argList,
				       ocg->CreateIdent(R.set_var(i)->name()));
    CG_outputRepr *call = 
      ocg->CreateInvoke((String) (const char *) v->get_global_var()->base_name(),
			argList);
    return call;
  }
}


//*****************************************************************************
CG_outputRepr* outputAsGuard(CG_outputBuilder* ocg, 
			     NOT_CONST Constraint_Handle &input_e, 
                             bool is_geq)
{
  Constraint_Handle &e = (Constraint_Handle&) input_e;
  CG_outputRepr *guardRepr;
  String s;
  // pick any v in EQ, print in terms of it.
  Variable_ID v=0;
  for(Constr_Vars_Iter cvi(e,false); cvi; cvi++) {
    if((*cvi).var->kind() != Wildcard_Var) {
      v = (*cvi).var;
      break;
    }
  }

  assert(v && "Must be some non-wildcards in the constraint");
  coef_t saved_coef = ((e).get_coef(v));
  assert(saved_coef);

  // The coef of v should have the same sign as saved_coef, but its abs
  // should be 1 so that outputEasy doesn't divide through by its coef
  int sign = saved_coef < 0 ? -1 : 1;
  (e).update_coef_during_simplify(v,-saved_coef+sign);
  // outputEasyBound fixes coefficients for the sign of v's coefficient,
  // so we should use abs here.

  CG_outputRepr* lop = outputIdent(ocg,v);
  if (abs(saved_coef) != 1) {
    lop = ocg->CreateTimes(ocg->CreateInt(abs(saved_coef)), lop);
  }

  CG_outputRepr* rop = outputEasyBoundasRepr(ocg, e, v, false, 0);
  if (is_geq) {
    if (saved_coef < 0) {
      guardRepr = ocg->CreateLE(lop, rop);
    }
    else {
      guardRepr = ocg->CreateGE(lop, rop);
    }
  } 
  else {
    guardRepr = ocg->CreateEQ(lop, rop);
  }

  (e).update_coef_during_simplify(v,saved_coef-sign);
  
  return guardRepr;
}

static
void get_stride(const Constraint_Handle &h, Variable_ID &wc, coef_t &step){
    wc = 0;
    for(Constr_Vars_Iter i(h,true); i; i++) {
	assert(wc == 0);
	wc = (*i).var;
	step = ((*i).coef);
    }
}


//-----------------------------------------------------------------------------
// generate the output loop stride as CG_outputRepr
// the original name is :
// String outputStrideAsString(EQ_Handle &e)
//-----------------------------------------------------------------------------
CG_outputRepr* outputStrideAsRepr(CG_outputBuilder* ocg, 
				  NOT_CONST EQ_Handle &input_e)
{
  Constraint_Handle &e = (Constraint_Handle&) input_e;
  Variable_ID wc;
  coef_t step;
  get_stride(e, wc, step);
  assert(step != 0);
  coef_t posstep = (step < 0) ? -step : step;
  e.update_coef_during_simplify(wc,1-step);

  CG_outputRepr* lop = outputEasyBoundasRepr(ocg, e, wc, false, 0);
  CG_outputRepr* rop = ocg->CreateInt(posstep);
  CG_outputRepr* intMod = ocg->CreateIntegerMod(lop, rop);
  CG_outputRepr* eqNode = ocg->CreateEQ(intMod, ocg->CreateInt(0));

  e.update_coef_during_simplify(wc,step-1);

  return eqNode;
}


//-----------------------------------------------------------------------------
// output_GEQ_strides
// original name is:
// String output_GEQ_strides(Relation &guards, Conjunct *c, bool &first)
//-----------------------------------------------------------------------------
CG_outputRepr* 
output_GEQ_strides(CG_outputBuilder* ocg,
		   Relation &/*guards*/, // currently unused
		   Conjunct *c)
{
  String s;
  CG_outputRepr* geqRepr = CG_REPR_NIL;

  // Collect wildcards
  // For each whildcard
  //   collect lower and upper bounds in which wildcard appears
  //   For each lower bound
  //     create constraint with each upper bound
	    
	    
  bool only_one;
  Tuple<Variable_ID> wcs;
  for(GEQ_Iterator g0(c); g0; g0++) {
    only_one = true;
    for(Constr_Vars_Iter cvi0(*g0,true); cvi0; cvi0++) {
      assert(only_one && "Can't generate multiple wildcard GEQ guards");
      if(!wcs.index((*cvi0).var)) 
	wcs.append((*cvi0).var);
      only_one = false;
    }
  }
  
  for (int i = 1; i <= wcs.length(); i++) {
    Variable_ID wc = wcs[i];
    Tuple<GEQ_Handle> lower, upper;
    for (GEQ_Iterator g(c); g; g++) {
      if((*g).get_coef(wc) > 0) 
	lower.append(*g); 
      else 
	if((*g).get_coef(wc) < 0) 
	  upper.append(*g);
    }

    //      low: c*alpha - x >= 0
    //      up:  -d*alpha + y >= 0
      
    for(Tuple_Iterator<GEQ_Handle> low(lower); low; low++) {
      for(Tuple_Iterator<GEQ_Handle> up(upper); up; up++) {
	coef_t low_coef = (*low).get_coef(wc);
	coef_t up_coef = (*up).get_coef(wc);
	assert(low_coef > 0 && up_coef < 0);
	// low_expr <= low_coef * intDiv(up_expr,up_coef)
	// Relation new_low = Relation::True(guards);
	// GEQ_Handle new_low_geq = new_low.and_with_GEQ(*low);
	// new_low_geq.update_coef(wc, 1-low_coef);
	// new_low.simplify();
	// new_low_geq = GEQ_Iterator(DNF_Iterator(new_low.query_DNF()).curr()).curr();
	    
	(*low).update_coef_during_simplify(wc, 1-low_coef);
	CG_outputRepr* lowExpr = outputEasyBoundasRepr(ocg, *low, wc, false, 0);
	(*low).update_coef_during_simplify(wc, low_coef-1);
	    
	// Relation new_up = Relation::True(guards);
	// GEQ_Handle new_up_geq = new_up.and_with_GEQ(*low);
	// new_up_geq.update_coef(wc, 1-low_coef);
	// new_up.simplify();
	// new_up_geq = GEQ_Iterator(DNF_Iterator(new_up.query_DNF()).curr()).curr();
	    
	// up_coef is negative, keep it that way
	// low: low_coef*alpha >= low_expr
	// up: -up_coef*alpha <= up_expr

	(*up).update_coef_during_simplify(wc, -1-up_coef);
	CG_outputRepr* upExpr = outputEasyBoundasRepr(ocg, *up, wc, false, 0);
	(*up).update_coef_during_simplify(wc, up_coef+1);
	
	CG_outputRepr* intDiv 
	  = ocg->CreateIntegerDivide(upExpr, ocg->CreateInt(-up_coef));
	CG_outputRepr* rop = ocg->CreateTimes(ocg->CreateInt(low_coef),
						       intDiv);
	CG_outputRepr* geqNode = ocg->CreateLE(lowExpr, rop);
	
	if ( geqRepr != CG_REPR_NIL) {
	  geqRepr = ocg->CreateAnd(geqRepr, geqNode);
	}
	else {
	  geqRepr = geqNode;
	}
      }
    }
  }

  return geqRepr;
}


//-----------------------------------------------------------------------------
// This returns a string containing the constraints in relation guards
// in a form suitable for C.  If there are no constraints in guards,
// nothing is returned.
//-----------------------------------------------------------------------------
static
CG_outputRepr* output_guard(CG_outputBuilder* ocg, Relation &guards_in) 
{
  CG_outputRepr* nodeRepr = CG_REPR_NIL;

  Relation guards = guards_in;
  assert(guards.has_single_conjunct());
  guards.setup_names();
  Conjunct *c = guards.single_conjunct();

  //---------------------------------------------------------------------------
  // equalities guards go here, like P1 == P2
  //---------------------------------------------------------------------------
  for(EQ_Iterator e(c); e; e++) {
    bool is_stride = (*e).has_wildcards();
    if (is_stride) {

      CG_outputRepr* eqNode = outputStrideAsRepr(ocg, *e);
      if (nodeRepr != CG_REPR_NIL) {
	nodeRepr = ocg->CreateAnd(nodeRepr, eqNode);
      }
      else {
	nodeRepr = eqNode;
      }
    } 
    else {
      // This is a normal EQ, no stride

      CG_outputRepr* eqNode = outputAsGuard(ocg, *e, false);
      if (nodeRepr != CG_REPR_NIL) {
	nodeRepr = ocg->CreateAnd(nodeRepr, eqNode);
      }
      else {
	nodeRepr = eqNode;
      }
      //old way:	    s += (*e).print_to_string();
    }
  }

  //---------------------------------------------------------------------------
  // The 0.96 had not the line s += output_GEQ_strides(guards,c,first);
  // fortunately s1 = output_GEQ_strides(guards,c,first) is always empty.
  // output_GEQ_strides has CONNECTOR there.
  //---------------------------------------------------------------------------
  // s += output_GEQ_strides(guards, c, first);
  CG_outputRepr *geqStrideRepr = output_GEQ_strides(ocg, guards, c);
  if ( geqStrideRepr != CG_REPR_NIL ) {
     if ( nodeRepr != CG_REPR_NIL ) {
       nodeRepr = ocg->CreateAnd(nodeRepr, geqStrideRepr);
     }
     else {
       nodeRepr = geqStrideRepr;
     }
  }
  
  //---------------------------------------------------------------------------
  // inequalities guards go here, like (n <= t2+63 && t1 <= t2-63)
  //---------------------------------------------------------------------------
  for(GEQ_Iterator g(c); g; g++) {
    if(!(*g).has_wildcards()) {

      CG_outputRepr* geqNode = outputAsGuard(ocg, *g, true);
      if(nodeRepr != CG_REPR_NIL) {
	nodeRepr = ocg->CreateAnd(nodeRepr, geqNode);
      }
      else {
	nodeRepr = geqNode;
      }
    }
  }

  if (nodeRepr != CG_REPR_NIL) {
    nodeRepr = ocg->CreateParens(nodeRepr);
  }

  return nodeRepr;
}


// Print the expression for the variable given as v.  Works for both 
// GEQ's and EQ's, but produces intDiv (not intMod) when v has a nonunit 
// coefficient.  So it is OK for loop bounds, but for checking stride
// constraints, you want to make sure the coef of v is 1, and insert the
// intMod yourself.
//
// make it a wrapper of CG_outputRepr* outputEasyBoundasRepr()
static
String outputEasyBoundasString(const Constraint_Handle &g, Variable_ID v, bool ignoreWC,
			       int ceiling, bool wrapInParens)
{
  CG_stringBuilder oscg;
  CG_outputRepr* boundRepr = outputEasyBoundasRepr(&oscg, g, v, ignoreWC, ceiling, 
						   wrapInParens);
  return GetString(boundRepr);
}

// one is 1 for LB
// make it a wrapper of CG_outputRepr* outputLBasRepr()
String outputLBasString(const GEQ_Handle &g, Relation &bounds, Variable_ID v,
			coef_t stride, const EQ_Handle &strideEQ, 
			Relation known)
{
  CG_stringBuilder oscg;
  CG_outputRepr* lbRepr = outputLBasRepr(&oscg, g, bounds, v, stride, 
					 strideEQ, known);
  return GetString(lbRepr);
}


// one is -1 for UB
// make it a wrapper of CG_outputRepr* outputUBasRepr()
String outputUBasString(const GEQ_Handle &g, Relation &bounds, Variable_ID v,
			coef_t stride, const EQ_Handle &strideEQ)
{
  CG_stringBuilder oscg;
  CG_outputRepr* ubRepr = outputUBasRepr(&oscg, g, bounds, v, stride, strideEQ);
  return GetString(ubRepr);
}



//-----------------------------------------------------------------------------
// one is 1 for LB
// this function is overloaded should replace the original one
//-----------------------------------------------------------------------------
CG_outputRepr *outputLBasRepr(CG_outputBuilder* ocg, const GEQ_Handle &g, 
			      Relation &bounds, Variable_ID v,
			      coef_t stride, const EQ_Handle &strideEQ,
			      Relation known)
{
#if ! defined NDEBUG
  coef_t v_coef;
  assert((v_coef = g.get_coef(v)) > 0);
#endif

  String s;
  CG_outputRepr *lbRepr;
  if (stride == 1) {
    lbRepr = outputEasyBoundasRepr(ocg, g,v,false,1);
  }
  else {
    if (!boundHitsStride(g,v,strideEQ,stride,known)) {
      bounds.setup_names(); // boundsHitsStride resets variable names

      String c = outputEasyBoundasString(strideEQ, v, true, 0, true);
      String LoverM = outputEasyBoundasString(g, v, false, 1, false);      //   L/m

      CG_outputRepr *cRepr = outputEasyBoundasRepr(ocg, strideEQ, v, true, 0, true);
      CG_outputRepr *LoverMRepr = outputEasyBoundasRepr(ocg, g, v, false, 1, false); 

      if (code_gen_debug > 2) {
	fprintf(DebugFile,"::: LoverM is %s\n",(const char *)LoverM);
	fprintf(DebugFile,"::: c is %s\n",(const char *)c);
      };

      if (c.length() < LoverM.length()) {
	// g * ceil((stride*intDiv(L/M
        CG_outputRepr *idUp = LoverMRepr;
	if (c != String("0")) {
	  CG_outputRepr *c1Repr = ocg->CreateCopy(cRepr); // make a copy of it
	  idUp = ocg->CreateMinus(idUp, c1Repr);
	}
	idUp = ocg->CreatePlus(idUp, ocg->CreateInt(stride-1));
	CG_outputRepr *idLow = ocg->CreateInt(stride);
	lbRepr = ocg->CreateTimes(ocg->CreateInt(stride),
				  ocg->CreateIntegerDivide(idUp, idLow));
	
	if (c != String("0")) {
	  lbRepr = ocg->CreatePlus(lbRepr, cRepr);
	}
      }
      else {
	if (LoverM == String("0")) {
	  lbRepr = ocg->CreateIntegerMod(cRepr, ocg->CreateInt(stride));
	}
	else {
	  CG_outputRepr *LoverM1Repr = ocg->CreateCopy(LoverMRepr); // make a copy
	  CG_outputRepr *imUp = ocg->CreateMinus(cRepr, LoverM1Repr);
	  imUp = ocg->CreateParens(imUp);
	  CG_outputRepr *imLow = ocg->CreateInt(stride);
	  CG_outputRepr *intMod = ocg->CreateIntegerMod(imUp, imLow);
	  lbRepr = ocg->CreatePlus(LoverMRepr, intMod);
	}
      }
    } 
    else {
      // boundsHitsStride resets variable names
      bounds.setup_names(); 
      lbRepr = outputEasyBoundasRepr(ocg, g, v, false, 0);
    }
  }

  return lbRepr;
}


//-----------------------------------------------------------------------------
// one is -1 for UB
// this function is overloaded should replace the original one
//-----------------------------------------------------------------------------
CG_outputRepr *outputUBasRepr(CG_outputBuilder* ocg, const GEQ_Handle &g, 
			      Relation &/*bounds*/, // currently unused
			      Variable_ID v,
			      coef_t /*stride*/, // currently unused
			      const EQ_Handle &/*strideEQ*/) //currently unused
{
  assert(g.get_coef(v) < 0);
  CG_outputRepr* upRepr = outputEasyBoundasRepr(ocg, g, v, false, 0);
  return upRepr;
}


//-----------------------------------------------------------------------------
// Print the expression for the variable given as v.  Works for both 
// GEQ's and EQ's, but produces intDiv (not intMod) when v has a nonunit 
// coefficient.  So it is OK for loop bounds, but for checking stride
// constraints, you want to make sure the coef of v is 1, and insert the
// intMod yourself.
//
// original name is outputEasyBound
//-----------------------------------------------------------------------------
static
CG_outputRepr* outputEasyBoundasRepr(CG_outputBuilder* ocg, 
					const Constraint_Handle &g, Variable_ID v, 
					bool ignoreWC,
					int ceiling, bool wrapInParens)
{
  // assert ignoreWC => g is EQ
  // rewrite constraint as foo (== or <= or >=) v, return foo as string 

  String s = "";
  CG_outputRepr* easyBoundRepr = CG_REPR_NIL;

  coef_t v_coef = g.get_coef(v);
  int v_sign = v_coef > 0 ? 1 : -1;
  v_coef *= v_sign;
  assert(v_coef > 0);
  // foo is (-constraint)/v_sign/v_coef 

  int sign_adj = -v_sign;
  int terms = 0;

  //----------------------------------------------------------------------
  // the following generates +- cf*varName
  //----------------------------------------------------------------------
  for(Constr_Vars_Iter c2(g,false); c2; c2++) {
    if ((*c2).var != v && (!ignoreWC || (*c2).var->kind()!=Wildcard_Var)) {

      coef_t cf = (*c2).coef*sign_adj;
      assert(cf != 0);
      terms++;

      CG_outputRepr* varName = outputIdent(ocg,(*c2).var);
      CG_outputRepr* cfRepr = CG_REPR_NIL;

      if (cf > 1) {
	cfRepr = ocg->CreateInt(cf);
	CG_outputRepr* rbRepr = ocg->CreateTimes(cfRepr, varName);
	easyBoundRepr = ocg->CreatePlus(easyBoundRepr, rbRepr);
      }
      else if (cf < -1) {
	cfRepr = ocg->CreateInt(-cf);
	CG_outputRepr* rbRepr = ocg->CreateTimes(cfRepr, varName);
	easyBoundRepr = ocg->CreateMinus(easyBoundRepr, rbRepr);
      }
      else if (cf == 1) {
	easyBoundRepr = ocg->CreatePlus(easyBoundRepr, varName);
      }
      else if (cf == -1) {
	easyBoundRepr = ocg->CreateMinus(easyBoundRepr, varName);
      }
    }
  }

  if (g.get_const()) {
    coef_t cf = g.get_const()*sign_adj;
    terms++;
    assert(cf != 0);
    if (cf > 0) {
      easyBoundRepr = ocg->CreatePlus(easyBoundRepr, ocg->CreateInt(cf));
    }
    else {
      easyBoundRepr = ocg->CreateMinus(easyBoundRepr, ocg->CreateInt(-cf));
    }
  }
  else {
    if(easyBoundRepr == CG_REPR_NIL) {
      easyBoundRepr = ocg->CreateInt(0);
    }
  }

  if (v_coef > 1) {
    assert(ceiling >= 0);
    if (ceiling) {
      easyBoundRepr= ocg->CreatePlus(easyBoundRepr, ocg->CreateInt(v_coef-1));
    }
    //-------------------------------------------------------------------------
    // extra parens if needed.
    // easyBoundRepr = ocg->CreateParens(easyBoundRepr);
    //-------------------------------------------------------------------------
    easyBoundRepr = ocg->CreateIntegerDivide(easyBoundRepr, ocg->CreateInt(v_coef));
  }
  else if (wrapInParens && terms > 1) {
    easyBoundRepr = ocg->CreateParens(easyBoundRepr);
  }
  
  if(code_gen_debug > 2) {
    fprintf(DebugFile, "::: generated bound of %s on %s\n",
	    (const char *)s, v->char_name());
    
    if (!ignoreWC) {
      for(Constr_Vars_Iter c3(g,true); c3; c3++) {
	fprintf(DebugFile,
		"::: found wildcard " coef_fmt " %s when generating bound %s on %s\n",
		(*c3).coef,(*c3).var->char_name(),(const char *) s, v->char_name());
      }
    }
  }

  return easyBoundRepr;
}


//*****************************************************************************
static 
bool outputAssignment(CG_outputBuilder* ocg,
                      Conjunct *c, Variable_ID v, int indent,
		      CG_outputRepr* &aRepr, 
		      Relation &new_known, elimination_info &eliminated) 
{
  CG_outputRepr *lopRepr = CG_REPR_NIL;
  CG_outputRepr *ropRepr = CG_REPR_NIL;
  CG_outputRepr *idLopRepr = CG_REPR_NIL;    // for lop in intDiv(lop, rop)
  
  EQ_Iterator I(c); 
  
  // Find first EQ that involves the interesting variable
  while ( I.live() && (*I).get_coef(v) == 0 ) 
    I++;
  
  if (!I.live()) {
    if (code_gen_debug > 2) 
      fprintf(DebugFile, "::: outputAssignment: no EQs?\n");
    return false;  // No EQ's but also no UB or LB's on this var
  }

  //---------------------------------------------------------------------------
  // If there is a simple substitution for this variable, (the
  // variable's coefficient is +-1) we can remove it from the generated
  // code.  In order to avoid seeing this variable in later loop bounds,
  // we will project it away (at that time) and replace it with a
  // variable that has no constraints on it.  Here we just mark it as
  // eliminated.
  //---------------------------------------------------------------------------

  coef_t divider = (*I).get_coef(v);
  int sign = 1;
  if (divider < 0) {
    divider = -divider;
    sign = -1;
  }
  assert(divider >= 1);
  if (divider == 1 && eliminated.may_eliminate(v)) {
    eliminated.eliminate(v);
  } 
  else {
    //-------------------------------------------------------------------------
    // left operand of the assignment
    //-------------------------------------------------------------------------
    lopRepr = outputIdent(ocg,v);

    for (Constr_Vars_Iter CVI(*I,false); CVI; CVI++) {

      //-----------------------------------------------------------------------
      // get the name of this constr_vars, may also include coef but no sign.(+-)
      //-----------------------------------------------------------------------
      CG_outputRepr *varRepr = outputIdent(ocg,(*CVI).var);

      //-----------------------------------------------------------------------
      // determine the sign and coefficient of this constr_vars
      //-----------------------------------------------------------------------
      if ((*CVI).var != v) {
	assert((*CVI).var->kind() != Wildcard_Var);

	if (-sign*(*CVI).coef == -1) {
          idLopRepr = ocg->CreateMinus(idLopRepr, varRepr);
        }
	else if (-sign*(*CVI).coef < -1) {
          varRepr = ocg->CreateTimes(ocg->CreateInt(sign*(*CVI).coef), varRepr);
          idLopRepr = ocg->CreateMinus(idLopRepr, varRepr);
        }
	else {
	  if (-sign*(*CVI).coef != 1) {
            varRepr = ocg->CreateTimes(ocg->CreateInt(-sign*(*CVI).coef), varRepr);
          }
	  idLopRepr = ocg->CreatePlus(idLopRepr, varRepr);
	}
      }
    }

    coef_t c_term = -((*I).get_const() * sign);

    if ( c_term ) {    // Not a zero
      if ( c_term > 0) {
        idLopRepr = ocg->CreatePlus(idLopRepr, ocg->CreateInt(c_term));
      }
    }
    else {
      if (idLopRepr == CG_REPR_NIL) {
        // Must print the zero if no other terms, now c_term == 0
        idLopRepr = ocg->CreateInt(c_term);
      }
    }

    if (divider != 1) { // divider might == 1 if !may_eliminate(v)
      idLopRepr = ocg->CreatePlus(idLopRepr, ocg->CreateInt(divider-1));
      idLopRepr = ocg->CreateParens(idLopRepr);
      ropRepr = ocg->CreateIntegerDivide(idLopRepr, ocg->CreateInt(divider));
    } else {
      ropRepr = idLopRepr;
    }


    aRepr = ocg->CreateAssignment(indent, lopRepr, ropRepr);
  }

  new_known.and_with_EQ(*I);

  // See if any more EQ's with this variable
  I++;
  while(I.live() && (*I).get_coef(v) == 0) 
    I++;

  if ((code_gen_debug > 2) && I.live())
    fprintf(DebugFile, "::: outputAssignment: more than 1 EQ found\n");

  return (!I.live());
}


//-----------------------------------------------------------------------------
// bool printBounds(CG_outputBuilder* ocg,
//                  Relation &b, Variable_ID v, int indent,
//		    Relation &known, Relation &new_known, 
//                  CG_outputRepr* &ctrlRepr,
//		    elimination_info eliminated)
// 
// add CG_outputBuilder as first argument and replace String with CG_outputRepr
//-----------------------------------------------------------------------------
static
bool printBounds(CG_outputBuilder* ocg,
                 Relation &b, Variable_ID v, int indent,
		 NOT_CONST Relation &input_known, Relation &new_known, 
		 CG_outputRepr* &ctrlRepr,
		 elimination_info &eliminated) 
{
  Relation known = consume_and_regurgitate(input_known);
  CG_outputRepr* indexRepr = CG_REPR_NIL;
  CG_outputRepr* lbRepr = CG_REPR_NIL;
  CG_outputRepr* lbListRepr = ocg->CreateArguList();
  CG_outputRepr* ubRepr = CG_REPR_NIL;
  CG_outputRepr* ubListRepr = ocg->CreateArguList();
  CG_outputRepr* stRepr = CG_REPR_NIL;

  if (code_gen_debug> 2) {
    fprintf(DebugFile,"::: printBounds generating bounds for\n");
    b.prefix_print(DebugFile);
  };
  name_codegen_vars(b);
  name_codegen_vars(known);
  b.setup_names();
  Conjunct *c = b.query_DNF()->single_conjunct();
  int lower_bounds=0, upper_bounds=0;
  coef_t coef;

  //---------------------------------------------------------------------------
  // Count bounds.
  //---------------------------------------------------------------------------
  for(GEQ_Iterator CI(c); CI; CI++) {
    if ((coef = (*CI).get_coef(v)) != 0) {  // In this constraint?
      if(coef > 0) // lb or ub
	lower_bounds++;
      else 
	upper_bounds++;
      new_known.and_with_GEQ(*CI);    // add to result
    }
  }

  new_known.finalize();
  if (upper_bounds == 0 || lower_bounds == 0) {
    if (code_gen_debug > 2) {
      fprintf(DebugFile,"::: printBounds looking for EQ:\n");
      b.prefix_print(DebugFile);
    }
    if (outputAssignment(ocg, c, v, indent, ctrlRepr, new_known, eliminated)) {
      if (code_gen_debug > 2) {
	if(ctrlRepr == CG_REPR_NIL)
	  fprintf(DebugFile,"::: variable substituted, no EQ generated\n");
	else
	  ctrlRepr->DumpToFile(DebugFile);
      }
      return true;
    }
  }
  
  b.setup_names();
  String variable_name = v->name();

  indexRepr = outputIdent(ocg,v);

  //---------------------------------------------------------------------------
  // EQ not found, really a loop.
  //---------------------------------------------------------------------------
  EQ_Handle strideEQ;
  coef_t step=1, stride_const=0;
  bool simpleStride=true;
  int strides = countStrides(c,v,strideEQ,simpleStride);
  
  if (strides > 1) {
    if (code_gen_debug > 2) 
      fprintf(DebugFile, "::: printBounds: Too many strides\n");
    return false;
  }
  if (strides == 1) {
    Constr_Vars_Iter it(strideEQ,true);
    assert(it.live());  // At least 1 wildcard
      int sign = strideEQ.get_coef(v) > 0 ? 1 : -1;
    step = (*it).coef * sign;
    stride_const = strideEQ.get_const() * -sign;
    assert(step > 1 || step < -1);
    if (step < 0) 
      step = -step;
    it++;
    assert(!it.live()); //  > 1 wildcard in stride constraint
    new_known.and_with_EQ(strideEQ);
    new_known.finalize();
  }

  bool seenUB = false, seenLB = false;
  coef_t const_ub = posInfinity, const_lb = negInfinity;

  b.setup_names();

  //---------------------------------------------------------------------------
  // Do lower bounds
  //---------------------------------------------------------------------------
  if (lower_bounds == 0) {
    lbRepr = ocg->CreateIdent("-infinity");
  }
  else {
    for (GEQ_Iterator g(c); g; g++) {
      if ((*g).get_coef(v) == 0) 
	continue;
      if ((*g).get_coef(v) > 0) { 
	// lower
	if ((*g).is_const(v) && !strides) { 
	  //no variables but v in constr
	  coef_t L,m;
	  L = -((*g).get_const());
	
	  m = (*g).get_coef(v);
	  coef_t sb  =  (int) (ceil(((float) L) /m));
	  set_max(const_lb, sb);
	}
	else if ((*g).is_const(v) && simpleStride) { 
	  // no variables but v in constr
	       //make LB fit the stride constraint
	       coef_t L,m,s,c;
	  L = -((*g).get_const());
	  m = (*g).get_coef(v);
	  s = step;
	  c = stride_const;
	  coef_t sb  =  (s * (int) (ceil( (float) (L - (c * m)) /(s*m))))+ c;
	  set_max(const_lb, sb);
	} 
	else {
	  if (!seenLB) {
	    seenLB = true;
	  } 
	  lbRepr = outputLBasRepr(ocg, *g, b, v, step, strideEQ, known);
	  if (lower_bounds > 1) {
	    lbListRepr = ocg->ArguListInsertLast(lbListRepr, lbRepr);
	  }
	}
      }
    }
  }

  if (const_lb != negInfinity) {
    assert(step == 1 || simpleStride);
    if (!seenLB) {
      lbRepr = ocg->CreateInt(const_lb);
    }
    lbListRepr = ocg->ArguListInsertLast(lbListRepr, ocg->CreateInt(const_lb));
  }

  if (lower_bounds > 1) {
    lbRepr = ocg->CreateInvoke("max", lbListRepr);
  }

  //---------------------------------------------------------------------------
  // Do upper bounds 
  //---------------------------------------------------------------------------
  if (upper_bounds == 0) {
    ubRepr = ocg->CreateIdent("+infinity");
  }
  else {
    for (GEQ_Iterator g(c); g; g++) {

      if ((*g).get_coef(v) == 0) 
	continue;
      if ((*g).get_coef(v) < 0) { 
	// upper
	if ((*g).is_const(v))  { 
	// no variables but v in constraint
	  set_min(const_ub,-(*g).get_const()/(*g).get_coef(v));
        } 
        else {
	  if (!seenUB) {
	    seenUB = true;
	    ubRepr = outputUBasRepr(ocg, *g, b, v, step, strideEQ);
	    if (upper_bounds >1) {
	      ubListRepr = ocg->ArguListInsertLast(ubListRepr, ubRepr);
	    }
	  } 
	  else {
	    ubRepr = outputUBasRepr(ocg, *g, b, v, step, strideEQ);
	    if (upper_bounds >1) {
	      ubListRepr = ocg->ArguListInsertLast(ubListRepr, ubRepr);
	    }
	  }
	}
      }
    }
  }

  if (const_ub != posInfinity) {  
    // some UB has only a constant
    if (!seenUB) {
      ubRepr = ocg->CreateInt(const_ub);
    }
    ubListRepr = ocg->ArguListInsertLast(ubListRepr, ocg->CreateInt(const_ub));
  }

  if (upper_bounds > 1) {
    ubRepr = ocg->CreateInvoke("min", ubListRepr);
  }


  //---------------------------------------------------------------------------
  // Steps are never negative since we scan in lex. order.  The actual 
  // representation might have a negative coefficient, though.
  //---------------------------------------------------------------------------
      
  step = abs(step);
  if (step != 1) {
    stRepr = ocg->CreateInt(abs(step));
  } 

//    if (step != 1) s += " step " + itoS(abs(step));
//     s+= " do\n";

  Relation k0,k1,k2;
  k1 = known;
  k2 = new_known;
  k0 = Intersection(k1,k2);
  if (code_gen_debug > 2) {
    fprintf(DebugFile,"::: Known from loop bounds:\n");
    k0.prefix_print(DebugFile);
  }
  b = Gist(b,k0,1);
  if (code_gen_debug > 2) {
    fprintf(DebugFile,"::: Constraints not yet expressed:\n");
    b.prefix_print(DebugFile);
  }

  ctrlRepr = ocg->CreateInductive(indexRepr, lbRepr, ubRepr, stRepr,gen_python);

  return true;
}


#ifndef NDEBUG
#define PAINFULLY_EXPENSIVE_DEBUGGING 1
#endif

int code_gen_debug=0;
int code_gen_check_zero_trip=1;

void name_codegen_vars(Relation &R) {
    for(int i = 1; i <= R.n_set(); i ++)
	R.name_set_var(i,String("t")+itoS(i));
}

void name_codegen_input_vars(Relation &R) {
    for(int i = 1; i <= R.n_inp(); i ++)
	R.name_input_var(i,String("t")+itoS(i));
}


// Return true if there are no variables in g except wildcards & v
static 
bool isSimpleStride(const EQ_Handle &g, Variable_ID v){
    EQ_Handle gg = g;  // should not be necessary, but iterators are
    // a bit brain-dammaged
    bool is_simple=true;
    for(Constr_Vars_Iter cvi(gg, false); cvi && is_simple; cvi++)
	is_simple = ((*cvi).coef == 0 || (*cvi).var == v 
		     || (*cvi).var->kind() == Wildcard_Var);
    return is_simple;
}


static 
int countStrides(Conjunct *c, Variable_ID v, EQ_Handle &strideEQ, 
		 bool &simple) {
    int strides=0;
    for(EQ_Iterator G(c); G; G++)
	for(Constr_Vars_Iter I(*G, true); I; I++)
	    if (((*I).coef != 0) && (*G).get_coef(v) != 0) {
		strides++;
		simple = isSimpleStride(*G,v);
		strideEQ = *G;
		break;
	    }
    return strides;
}


Relation project_onto_levels(Relation R, int last_level, bool wildcards) {

    assert(last_level >= 0 && R.is_set() && last_level <= R.n_set());
    if (last_level == R.n_set()) return R;

    int orig_vars = R.n_set();
    int num_projected = orig_vars - last_level;
    R = Extend_Set(R,num_projected);  // Project out vars numbered > last_level
    Mapping m1 = Mapping::Identity(R.n_set());  // now orig_vars+num_proj

    for(int i=last_level+1; i <= orig_vars; i++) {
	m1.set_map(Set_Var, i, Exists_Var, i);
	m1.set_map(Set_Var, i+num_projected, Set_Var, i);
    }

    MapRel1(R, m1, Comb_Id);
    R.finalize();
    R.simplify();
    if (!wildcards) 
        R = Approximate(R,1);
    assert(R.is_set());
    return R;
}


// Check if the lower bound already enforces the stride by
// (Where m is coef of v in g and L is the bound on m*v):
// Check if m divides L evenly and Check if this l.bound on v implies strideEQ 



static 
bool boundHitsStride(const GEQ_Handle &g, Variable_ID v,
		     const EQ_Handle &strideEQ,
		     coef_t /*stride*/, // currently unused
		     Relation known){
/* m = coef of v in g;
L = bound on v part of g;
*/ 
    // Check if m divides L evenly
    coef_t m = g.get_coef(v);
    Relation test(known.n_set());
    F_Exists *e = test.add_exists();       // g is "L >= mv"
    Variable_ID alpha = e->declare();      // want: "l = m alpha"
    F_And *a = e->add_and();
    EQ_Handle h = a->add_EQ(); 
    for(Constr_Vars_Iter I(g,false); I; I++)
	if((*I).var != v) {
	    if((*I).var->kind() != Global_Var)
		h.update_coef((*I).var, (*I).coef);
	    else
		h.update_coef(test.get_local((*I).var->get_global_var()), (*I).coef);
	}

    h.update_const(g.get_const());
    h.update_coef(alpha,m);                // set alpha's coef to m
    if (!(Gist(test,copy(known)).is_obvious_tautology()))      
	return false;
    // Check if this lower bound on v implies the strideEQ 
    Relation boundRel = known;    // want: "known and l = m v"
    boundRel.and_with_EQ(g);      // add in l = mv
    Relation strideRel(known.n_set());
    strideRel.and_with_EQ(strideEQ);
    return Gist(strideRel, boundRel).is_obvious_tautology();
}



inline bool isLB(const GEQ_Handle &g, Variable_ID v) {
    return g.get_coef(v) > 0;
}

inline bool isUB(const GEQ_Handle &g, Variable_ID v) {
    return g.get_coef(v) < 0;
}


/* This is dd_gcd from ddutil.c */
static
coef_t wgcd( coef_t a, coef_t b )
{
    coef_t g, r;
    coef_t g0, g1;

    if( a == 0 ){
	g = abs(b);
    }else if( b == 0 ){
	g = abs(a);
    }else{
	g0 = abs(a);
	g1 = abs(b);
	r = g0 % g1;
	while( r != 0 ){
	    g0 = g1; g1 = r;
	    r = g0 % g1;
	}
	g = g1;
    }
    return g;
}/* wgcd */


/* These next two are stolen from Wayne's gencode.c, slightly modified */
static
coef_t update_gcs(coef_t gcs, EQ_Handle new_stride, EQ_Handle first) {
    int number_wildcards = 0;
    gcs = wgcd(gcs, abs(new_stride.get_const() - first.get_const()));
    for(Constr_Vars_Iter j(new_stride,false); j; j++)
	if ((*j).var->kind() == Wildcard_Var)
	    {
		gcs=wgcd(gcs, abs((*j).coef));
		number_wildcards++;
	    }
	else
	    gcs=wgcd(gcs, abs((*j).coef - first.get_coef((*j).var)));
    assert(number_wildcards == 1);
    return gcs;
}

static
Relation greatest_common_step(SetTuple &I, IntTuple &active, int level,int vars, int nr_statements)
{

    coef_t gcs = 0;

    EQ_Handle first;
    bool firstStride = true;

    for (int stmt=1; stmt<=nr_statements; stmt++)
        if (active[stmt])
	    {
		bool sawStride = false;
		Relation &orig = I[stmt];
		Variable_ID t_col = set_var(level);
            
		Conjunct *c = orig.single_conjunct();
		for (EQ_Iterator k = c->EQs(); k.live(); k.next())
		    {
			if (abs((*k).get_coef(t_col)) == 1)
			    {
				for (Variable_ID_Iterator j(*(c->variables())); 
				     j.live(); 
				     j.next())
				    {
					if ((*j)->kind()==Wildcard_Var 
					    && (*k).get_coef(*j)!=0)
					    {
						if (code_gen_debug > 2)
						    {
							fprintf(DebugFile,
								"::: gcs found stride constraint, level %d, stmt %d, stride " coef_fmt "\n",
								level,stmt,(*k).get_coef(*j));
						    }
						sawStride = true;
// Here, get the gcs of this stride (*k)
						if(firstStride) {
						    first = *k;
						    firstStride=false;
						}
						gcs = update_gcs(gcs,*k,first);
						break;
					    }
				    }
			    }
		    }
		if(!sawStride) // There's no stride, so we can't enforce any at all
		    {
			if (code_gen_debug > 2)
			    fprintf(DebugFile,"::: gcs: stmt %d has no stride, using 1\n",stmt);
			return Relation::True(vars);
		    }
	    }

    if (firstStride)
	{
	    if (code_gen_debug > 2)
		fprintf(DebugFile,"::: gcs: no strides found for level %d\n",level);
	    return Relation::True(vars);
	}

    if (code_gen_debug > 2)
	fprintf(DebugFile, ":::   gcs == " coef_fmt "\n", gcs);

    if (gcs == 0) gcs = 1;

    Relation R0(vars); 
    F_And *a0 = R0.add_and();
    EQ_Handle new_eq = a0->add_stride(gcs);


    new_eq.update_const(first.get_const());
    for (Constr_Vars_Iter c(first, 0); c.live() ; c.next())
        {
	    switch (c.curr_var()->kind()) 
		{
		case Input_Var:
		case Output_Var:
		    new_eq.update_coef((*c).var, c.curr_coef());
		    break;
		case Global_Var:
		    {
		    Global_Var_ID g = c.curr_var()->get_global_var();
		    new_eq.update_coef(R0.get_local(g), c.curr_coef());
		    break;
		    }
		case Wildcard_Var:
		    /* don't set */
		    break;
		default:
		    assert(0);
		}
	}
    R0.finalize();
    return R0;
} /* greatest_common_step */


CG_outputRepr *
default_stmt_info::place_holder(CG_outputBuilder *ocg, int indent, 
				Relation *current_map)
{
  String stmtName;
  if(gen_python)
    stmtName=String("yield ");
  else
    stmtName = String("s") + itoS(stmt_num);
  CG_outputRepr* sList = print_outputs_with_subs_to_repr(*current_map,ocg);
  CG_outputRepr* stmtRepr = ocg->CreatePlaceHolder(indent, stmtName, sList,gen_python);
  return stmtRepr;  
}
  

//-----------------------------------------------------------------------------
// outputStatements is going to produce an empty statement like s1(t1, t2)
//-----------------------------------------------------------------------------
static
CG_outputRepr* outputStatements(CG_outputBuilder *ocg, int stmt, int indent, 
				Relation &mapping, Relation &known)
{
  Relation Inv_mapping = Restrict_Domain(Inverse(copy(mapping)), copy(known));
  name_codegen_input_vars(Inv_mapping);
  CG_outputRepr *stmtRepr = statementNameInfo[stmt]->place_holder(ocg, indent, 
								  &Inv_mapping);
  return stmtRepr;  
}



RelTuple transformations;

bool hasBound(Relation r, int level, int UB) {
    r.simplify();
    Variable_ID v = set_var(level);
    Conjunct *s_conj = r.single_conjunct();
    for(GEQ_Iterator G(s_conj); G; G++) {
	if (UB && (*G).get_coef(v) < 0) return true;
	if (!UB && (*G).get_coef(v) > 0) return true;
    }
    for(EQ_Iterator E(s_conj); E; E++) {
	if ((*E).get_coef(v)) return true;
    }
    return false;
}


bool hasEQ(Relation r, int level) {
    r.simplify();
    Variable_ID v = set_var(level);
    Conjunct *s_conj = r.single_conjunct();
    for(EQ_Iterator G(s_conj); G; G++)
      if ((*G).get_coef(v))
	return true;
    return false;
}

static
bool has_nonstride_EQ(Relation r, int level) {
    r.simplify();
    Variable_ID v = set_var(level);
    Conjunct *s_conj = r.single_conjunct();
    for(EQ_Iterator G(s_conj); G; G++)
      if ((*G).get_coef(v) && !(*G).has_wildcards())
	return true;
    return false;
}


static
Relation pickEQ(Relation r, int level) {
    r.simplify();
    Variable_ID v = set_var(level);
    Conjunct *s_conj = r.single_conjunct();
    for(EQ_Iterator E(s_conj); E; E++)
	if ((*E).get_coef(v)) {
	    Relation test_rel(r.n_set());
	    test_rel.and_with_EQ(*E);
	    return test_rel;
	}
    assert(0);
    return r;
}

/* pickBound will return an EQ as a GEQ if it finds one */
Relation pickBound(Relation r, int level, int UB) {
    r.simplify();
    Variable_ID v = set_var(level);
    Conjunct *s_conj = r.single_conjunct();
    for(GEQ_Iterator G(s_conj); G; G++) {
	if ((UB && (*G).get_coef(v) < 0)
	    ||  (!UB && (*G).get_coef(v) > 0) ) {
	    Relation test_rel(r.n_set());
	    test_rel.and_with_GEQ(*G);
	    return test_rel;
	}
    };
    for(EQ_Iterator E(s_conj); E; E++) {
	if ((*E).get_coef(v)) {
	    Relation test_rel(r.n_set());
	    test_rel.and_with_GEQ(*E);
	    if ((UB && (*E).get_coef(v) > 0)
		||  (!UB && (*E).get_coef(v) < 0) ) 
		test_rel = Complement(test_rel);
	    return test_rel;
	}
    }
    assert(0);
    return r;
}

Relation pickOverhead(Relation r, int liftTo) {
    r.simplify();
    Conjunct *s_conj = r.single_conjunct();
    for(GEQ_Iterator G(s_conj); G; G++) {
	Relation test_rel(r.n_set());
	test_rel.and_with_GEQ(*G);
	Variable_ID v;
	coef_t pos = -1;
	coef_t c= 0;
	for(Constr_Vars_Iter cvi(*G, false); cvi; cvi++) 
	    if ((*cvi).coef && (*cvi).var->kind() == Input_Var 
		&& (*cvi).var->get_position() > pos) {
		v = (*cvi).var;
		pos = (*cvi).var->get_position();
		c = (*cvi).coef;
	    }
#if 0
	fprintf(DebugFile,"Coef = %d, constraint = %s\n",
		c,(const char *)test_rel.print_formula_to_string());
#endif
	return test_rel;
    }
    for(EQ_Iterator E(s_conj); E; E++) {
	assert(liftTo >= 1);
	int pos = max((*E).max_tuple_pos(),max_fs_arity(*E)+1);
	
/* Pick stride constraints only when the variables with stride are outer
   loop variables */
	if ((*E).has_wildcards()  && pos < liftTo) {
	    Relation test_rel(r.n_set());
	    test_rel.and_with_EQ(*E);
	    return test_rel;
	}
	else 
	if (!(*E).has_wildcards()  && pos <= liftTo) {
	    Relation test_rel(r.n_set());
	    test_rel.and_with_EQ(*E);
	    test_rel.simplify();
	    test_rel = EQs_to_GEQs(test_rel,true);
	    return pickOverhead(test_rel,liftTo);
	}
    }
    if (code_gen_debug>1) {
	fprintf(DebugFile,"Could not find overhead:\n");
	r.prefix_print(DebugFile);
    }
    return Relation::True(r.n_set());
}

Relation minMaxOverhead(Relation r, int level) {
    r.finalize();
    r.simplify();
    Conjunct *s_conj = r.single_conjunct();
    GEQ_Handle LBs[50],UBs[50];
    int numLBs = 0;
    int numUBs = 0;
    Variable_ID v = set_var(level);
    for(GEQ_Iterator G(s_conj); G; G++) if ((*G).get_coef(v)) {
	GEQ_Handle g = *G;
	if (g.get_coef(v) > 0) LBs[numLBs++] = g;
	else UBs[numUBs++] = g;
    }
    if (numLBs <= 1 && numUBs <= 1) {
	return Relation::True(r.n_set());
    }
    Relation r1(r.n_set());
    Relation r2(r.n_set());
    if (numLBs > 1) {
	// remove a max in lower bound
	r1.and_with_GEQ(LBs[0]);
	r2.and_with_GEQ(LBs[1]);
	r1 = project_onto_levels(Difference(r1,r2),level-1,0);
    }
    else {
	// remove a min in upper bound
	r1.and_with_GEQ(UBs[0]);
	r2.and_with_GEQ(UBs[1]);
	r1 = project_onto_levels(Difference(r1,r2),level-1,0);
    }
#if 0
    fprintf(DebugFile,"Testing %s\n",(const char *)r1.print_formula_to_string());
    fprintf(DebugFile,"will removed overhead on bounds of t%d: %s\n",level,
	    (const char *)r.print_formula_to_string());
#endif
			

    return pickOverhead(r1, -1);
}



CG_split::CG_split(IntTuple &active, int lvl, Relation cond,
	     CG_result *T, CG_result *F) {
    assert(cond.has_single_conjunct());
    // If overhead is a GEQ, make it a LB on innermost loop var
    coef_t c=0;
    int pos=-1;  // initialize c to shut up the compiler
    GEQ_Iterator G(cond.single_conjunct());
    if (G) {
	for(Constr_Vars_Iter cvi(*G, false); cvi; cvi++) 
	    if ((*cvi).coef && (*cvi).var->kind() == Input_Var 
		&& (*cvi).var->get_position() > pos) {
		pos = (*cvi).var->get_position();
		c = (*cvi).coef;
	    }
	// Evan had an assert pos > 0 here, but we can allow pos == 0
#ifndef NDEBUG
	if(pos > lvl+1) {
		fprintf(DebugFile,"Illegal split at lvl %d\n", lvl);
		cond.prefix_print(DebugFile);
		code_gen_debug = 1;
		// fprintf(DebugFile,"Then clause:\n%s\n", (const char *) T->print(5));
		// fprintf(DebugFile,"Else clause:\n%s\n", (const char *) F->print(5));
		}
	assert(pos == 0 || pos <= lvl+1);
#endif
	if (pos > 0 && c > 0) {
	    CG_result *foo = T;
	    T = F;
	    F = foo;
	    if (code_gen_debug) {
		    fprintf(DebugFile,"Reversing clause in new split node at lvl %d\n",lvl);
			cond.prefix_print(DebugFile);
		    }
	    cond = Complement(cond);
	     // Simplify is Temporary workaround to gist problem (2/15/95)
	    cond.simplify(); // if we don't do this, later we must do 
             	             // high redundant conjunct effort 
	    assert(cond.has_single_conjunct());
	}
    } else { 
	assert(EQ_Iterator(cond.single_conjunct())); // Has >= 1 EQ
    }
    isActive = active;
    condition = cond;
    level = lvl;
    trueClause = T;
    falseClause = F;
}

CG_result *CG_split::new_copy() {
	Relation c = condition;
	return new CG_split(isActive,level,c,
			    trueClause->new_copy(),falseClause->new_copy());
}



String CG_split::printStructure(int indent) {
	    char buf[80];
	    sprintf(buf,"%p",(void *)this);
	    String result = outputSpaces(indent) 
		+ "// split at "
		+ buf
		+ "\n"
		+ outputSpaces(indent) 
		+"// if " + condition.print_formula_to_string()
		+ " then\n"
		+ trueClause->printStructure(indent+1)
		+ outputSpaces(indent) 
		+ "// else\n"
		+ falseClause->printStructure(indent+1);
	    return result;
}
//-----------------------------------------------------------------------------
// original CG_split::print(int) is now a wrapper of 
// printRepr(CG_outputBuilder*, int)
//-----------------------------------------------------------------------------
String CG_split::print(int indent)
{
  CG_stringBuilder oscg;
  CG_outputRepr* splitRepr = printRepr(&oscg, indent);
  return GetString(splitRepr);
}

//*****************************************************************************
// Added by D people. Lei Zhou Apr. 20, 96
//*****************************************************************************
//-----------------------------------------------------------------------------
// CG_split member function: added by D people
// CG_outputRepr* CG_split::printRepr(CG_outputBuilder* ocg, int indent)
// Note: if code_gen_debug is set true, then the statements are indented
// more than code_gen_debug is set false.
//-----------------------------------------------------------------------------
CG_outputRepr* CG_split::printRepr(CG_outputBuilder* ocg, int indent) 
{
  CG_outputRepr* stmtList = ocg->CreateStmtList();

  if (code_gen_debug) {
    char buf[80];
    sprintf(buf,"%p",(void *)this);

    String ifComment;
    CG_outputRepr *textRepr;
    ifComment = String("split at ") + buf;
    textRepr = ocg->CreateComment(indent, ifComment);
    stmtList = ocg->StmtListInsertLast(stmtList, textRepr);
    ifComment = "if " + condition.print_formula_to_string() + " then";
    textRepr = ocg->CreateComment(indent, ifComment);
    stmtList = ocg->StmtListInsertLast(stmtList, textRepr);

    stmtList = ocg->StmtListAppend(stmtList, trueClause->printRepr(ocg, indent+1));

    ifComment = "else";
    textRepr = ocg->CreateComment(indent, ifComment);
    stmtList = ocg->StmtListInsertLast(stmtList, textRepr);
    stmtList = ocg->StmtListAppend(stmtList, falseClause->printRepr(ocg, indent+1));
  }
  else {
    stmtList = ocg->StmtListAppend(stmtList, trueClause->printRepr(ocg, indent));
    stmtList = ocg->StmtListAppend(stmtList, falseClause->printRepr(ocg, indent));
  }

  return stmtList;
}


CG_result * CG_split::liftOverhead(int depth) {
	assert(depth >= 1);
	trueClause = trueClause->liftOverhead(depth);
	falseClause = falseClause->liftOverhead(depth);
	return this;
}

CG_result * CG_split::force_finite_bounds() {
	trueClause = trueClause->force_finite_bounds();
	falseClause = falseClause->force_finite_bounds();
	return this;
}
Relation CG_split::findOverhead(int liftTo) {
	Relation r = trueClause->findOverhead(liftTo);
	if (r.is_obvious_tautology()) r = falseClause->findOverhead(liftTo);
	return r;
}

CG_result * CG_split::recompute(const Relation k, const Relation r,
				  elimination_info e)  { // CG_split recompute
	known = k;
	restrictions = r;
	eliminated = e;
	known.simplify();
	restrictions.simplify();
	Relation kr = Intersection(copy(known),copy(restrictions));
	int numActive = 0;
	for(int s = 1; s<= isActive.size(); s++) if (isActive[s]) {
	    Relation I = projected_nIS[level][s];
	    if (!Intersection(copy(kr),I).is_upper_bound_satisfiable()) 
		isActive[s] = 0;
	    else numActive++;
	}
	if (numActive == 0) {
	    delete this;
	    return new CG_null;
	}
	else if (numActive == 1 && 0) {
	    // Need to rethink this -- can undo loop bounds
	    // overhead removal
	    CG_result * n = gen_recursive(level, isActive);
	    n = n->recompute(k,r,e);
	    delete this;
	    return n;
	}

	Relation c = condition;
	if (code_gen_debug) {
		fprintf(DebugFile,"\nRecomputing split node %p\n",
			(void *)this);
		fprintf(DebugFile,"Known: %s\n",
			(const char *) copy(k).print_formula_to_string());
		fprintf(DebugFile,"restriction: %s\n",
			(const char *) copy(r).print_formula_to_string());
		fprintf(DebugFile,"split: %s\n",
			(const char *) c.print_formula_to_string());
		}
	trueClause = trueClause->recompute(k, Intersection(copy(r), copy(c)),e);
	falseClause = falseClause->recompute(k, Difference(copy(r), copy(c)),e);
	if (trueClause->isNull()) {
	    if (code_gen_debug) {
		fprintf(DebugFile,"Pruning true branch of %p: %s\n",
			(void *) this,
			(const char *)condition.print_formula_to_string());
		fprintf(DebugFile,"Known: %s\n",
			(const char *) known.print_formula_to_string());
		fprintf(DebugFile,"restrictions: %s\n",
			(const char *) restrictions.print_formula_to_string());
		fprintf(DebugFile,"True branch: \n%s\n\n",
			(const char *)trueClause->print(2));
		fprintf(DebugFile,"False branch: \n%s\n\n",
			(const char *)falseClause->print(2));
	    };
	    CG_result * r = falseClause;
	    falseClause = 0;
	    delete this;
	    return r;
	};
	if (falseClause->isNull()) {
	    if (code_gen_debug) {
		fprintf(DebugFile,"Pruning false branch of %p: %s\n",
			(void *) this,
			(const char *)condition.print_formula_to_string());
		fprintf(DebugFile,"Known: %s\n",
			(const char *) known.print_formula_to_string());
		fprintf(DebugFile,"restrictions: %s\n",
			(const char *) restrictions.print_formula_to_string());
		fprintf(DebugFile,"True branch: \n%s\n\n",
			(const char *)trueClause->print(2));
		fprintf(DebugFile,"False branch: \n%s\n\n",
			(const char *)falseClause->print(2));
	    }
	    CG_result * r = trueClause;
	    trueClause = 0;
	    delete this;
	    return r;
	};
	return this;
}

// CG_leaf implementation

Relation CG_leaf::findOverhead(int liftTo) {
	for(int s=1; s<=isActive.size(); s++) if (isActive[s]) {
	    int bb = !guard[s].is_obvious_tautology();
	    if (bb) {
		if (code_gen_debug)
		    fprintf(DebugFile,"Trying to remove overhead on guard of statement s%d: %s\n",
			    s,(const char *) guard[s].print_formula_to_string());
		Relation r =  pickOverhead(guard[s], liftTo);
		if (!r.is_obvious_tautology()) return r;
	    }
        }
	return Relation::True(known.n_set());
}


String CG_leaf::printStructure(int) {
	String result;
	for(int s=1; s<=isActive.size(); s++) if (isActive[s]) {
		char  buf[80];
		sprintf(buf,"s%d ",s);
		result += buf;
		}
	return result+"\n";
}



//-----------------------------------------------------------------------------
// original CG_leaf::print(int) is now a wrapper of 
// printRepr(CG_outputBuilder*, int)
//-----------------------------------------------------------------------------
String CG_leaf::print(int indent)
{
  CG_stringBuilder oscg;
  CG_outputRepr* leafRepr = printRepr(&oscg, indent);
  return GetString(leafRepr);
}

//*****************************************************************************
// Added by D people. Lei Zhou Apr. 20, 96
//*****************************************************************************
//-----------------------------------------------------------------------------
// CG_leaf member function: 
// CG_outputRepr* CG_leaf::printRepr(CG_outputBuilder* ocg, int indent)
//-----------------------------------------------------------------------------
CG_outputRepr* CG_leaf::printRepr(CG_outputBuilder* ocg, int indent)
{
  String result;
  CG_outputRepr* stmtList = ocg->CreateStmtList();
  CG_outputRepr* stmtNode = CG_REPR_NIL;


  if (code_gen_debug>1) {
    result = "known: " + known.print_formula_to_string();
    stmtList = ocg->StmtListInsertLast(stmtList, ocg->CreateComment(indent, result));
  }

  for(int s=1; s<=isActive.size(); s++) if (isActive[s]) {
    if (guard[s].is_obvious_tautology()) {
      stmtNode = outputStatements(ocg, s, indent, transformations[s], known);
    }
    else {
      if (code_gen_debug>1) {
	result = "restrictions: " + restrictions.print_formula_to_string();
        stmtList = ocg->StmtListInsertLast(stmtList, 
                                           ocg->CreateComment(indent, result));
      }


      CG_outputRepr *ifGuard = output_guard(ocg, guard[s]);

      Relation g = Intersection(copy(guard[s]),	copy(known));

      CG_outputRepr *stmtList = outputStatements(ocg, s, indent+1, 
                                                    transformations[s], g);

      stmtNode = ocg->CreateIf(indent, ifGuard, stmtList);

    }

    stmtList = ocg->StmtListInsertLast(stmtList, stmtNode);
  }

  return stmtList;
}


CG_result * CG_leaf::recompute(const Relation k, const Relation r,
				  elimination_info e)  {
	known = k;
	restrictions = r;
	eliminated = e;
	known.simplify();
	restrictions.simplify();
	Relation kr = Intersection(copy(k),copy(r));
	guard.reallocate(isActive.size());
        if (code_gen_debug > 1) {
		fprintf(DebugFile,"Recomputing leaf node %p\n", (void *) this);
		fprintf(DebugFile,"Known: %s\n",
			(const char *) copy(k).print_formula_to_string());
		fprintf(DebugFile,"restrictions: %s\n",
			(const char *) copy(r).print_formula_to_string());
		}
	int count = 0;
	for(int s=1; s<=isActive.size(); s++) if (isActive[s]) {
	    if (code_gen_debug > 1) {
			fprintf(DebugFile,"s%d I: %s\n", s,
				(const char *) copy(new_IS[s]).print_formula_to_string());
			};
	    Relation I = 
		Intersection(copy(new_IS[s]),
			     copy(kr));
	    if (!I.is_upper_bound_satisfiable()) isActive[s] = 0;
	    else {
		count++;
		assert(!I.is_null());
		if (code_gen_debug > 1) {
			fprintf(DebugFile,"s%d Ikr: %s\n", s,
				(const char *) copy(I).print_formula_to_string());
			};
		guard[s] = Gist(eliminated.project_eliminated(copy(I)),
				eliminated.project_eliminated(copy(k)),1);
		guard[s] = Intersection(guard[s],copy(function_guards[s]));
		name_codegen_vars(guard[s]);
		if (code_gen_debug > 1) {
			fprintf(DebugFile,"s%d g: %s\n", s,
				(const char *) copy(guard[s]).print_formula_to_string());
			};
	    }
	}
	if (count == 0) {
	    if (code_gen_debug) {
		fprintf(DebugFile,"Pruning leaf node %p\n", (void *) this);
		fprintf(DebugFile,"Known: %s\n",
			(const char *) known.print_formula_to_string());
		fprintf(DebugFile,"restrictions: %s\n",
			(const char *) restrictions.print_formula_to_string());
	    };
	    delete this;
	    return new CG_null();
	}
	return this;
}

// CG_loop implementation




Relation CG_loop::findOverhead(int liftTo) {
// If a loop is nec., search for both guard and min/max overheads, otherwise
// skip those step
    if (needLoop) {
	int bb = !guard.is_obvious_tautology();
	if (bb) {
#if 0
	    fprintf(DebugFile,"Trying to remove overhead on guard of loop %x at level %d: %s\n",
		    (void *) this, level, (const char *) guard.print_formula_to_string());
#endif
	    Relation r = pickOverhead(guard, liftTo);
	    if (!r.is_obvious_tautology()) return r;
	}
	Relation b = bounds;
	Relation r = minMaxOverhead(b,level);
	if (!r.is_obvious_tautology()) {
#if 0
	    fprintf(DebugFile,"Trying to remove minMax overhead on guard of loop %x at level %d: %s\n",
		    (void *) this, level, (const char *) guard.print_formula_to_string());
#endif
	    return r;
	}
    } // if (needLoop)...
	Relation r = body->findOverhead(liftTo);
	if (r.is_obvious_tautology()) return r;

// If there's a loop at this level, the overhead returned may use the
// current loop variable we need to take it out.  If there is NO loop
// here, then we eliminated this var before recomputing subtree, so we
// don't need to.  More importantly, it can gist away the overhead
// since we didn't output the guard we really should
// have. (i.e. bounds are stricter since they weren't gisted according
// to the guard we normally output, and so would gist away the
// overhead)

   
     // fprintf(DebugFile,"Lifting out guard past level %x at level %d: %s\n",
		    // (void *) this, level, (const char *) r.print_formula_to_string());
      //fprintf(DebugFile,"Bounds are: %s\n",(const char *)bounds.print_formula_to_string());
    r = Intersection(r, copy(bounds));
      //fprintf(DebugFile,"Intersection: %s\n",(const char *)r.print_formula_to_string());
    r = project_onto_levels(r,level,0);
      //fprintf(DebugFile,"projected: %s\n",(const char *)r.print_formula_to_string());
    r = Gist(r,copy(bounds),1);
    //fprintf(DebugFile,"Got %s\n",
		 //(const char *) r.print_formula_to_string());
    return r;
}

bool find_any_constraint(int s, int level, Relation &kr, int direction,
			 Relation &S, bool approx) {
	  
  /* If we don't intersect I with restrictions, the combination 
     of S and restrictions can be unsatisfiable, which means that
     the new split node gets pruned away and we still don't have
     finite bounds -> infinite recursion. */

  Relation I = projected_nIS[level][s];
  I = Gist(I,copy(kr));
  if(approx) I = Approximate(I);
  if (hasBound(I,level,direction)) {
    Relation pickfrom;
    if(has_nonstride_EQ(I,level))
      pickfrom = pickEQ(I,level);
    else 
      pickfrom = pickBound(I,level,direction);
    S = pickOverhead(pickfrom,level);
    if(S.is_obvious_tautology()) S = Relation::Null();
    return !S.is_null();
  }
  return false;
}

CG_result * CG_loop::force_finite_bounds() {
	if (!needLoop || hasBound(bounds,level,0) && hasBound(bounds,level,1)) {
	    body = body->force_finite_bounds();
	    return this;
	}
    if (code_gen_debug) 
	fprintf(DebugFile,"Don't have finite bounds in:\n%s\n\n",
		(const char *) this->print(2));
	bool bb = !hasBound(bounds,level,0);
	int direction = bb ? 0 : 1;
	Relation kr = Intersection(copy(known),
				   copy(restrictions));
	int s;
	Relation S;
	for(s = 1; s <= isActive.size(); s++) if (isActive[s])
	  if(find_any_constraint(s,level,kr,direction,S,false))
	    break;
	// If we can't find one, approximate to remove GEQ strides and
	// try again.  Statement guards will ensure correctness.
	if(S.is_null())
	  for(s = 1; s <= isActive.size(); s++) if (isActive[s])
	    if(find_any_constraint(s,level,kr,direction,S,true))
	      break;
	
	if(S.is_null()) {
#if 0
	  fprintf(DebugFile,"hasBound(%s,%d,%s) said false, but overhead was tautology\n",(const char *)I.print_formula_to_string(),level,direction  ? "upper bound":"lower bound");
#endif
	  assert(0 && "Unable to force finite bounds");
	}
    
	if (code_gen_debug) 
	  fprintf(DebugFile,"Spliting on %s\n",
		  (const char *) S.print_formula_to_string());
	CG_result *spl = 
	  new CG_split(isActive,level,S,this,this->new_copy());
	Relation restrictions_copy = restrictions,
	  known_copy = known;
	spl = spl->recompute(known_copy,restrictions_copy,eliminated);
#if 0
	if (code_gen_debug) 
	  fprintf(DebugFile,"Split on to create finite bounds:\n%s\n\n",
		  (const char *) spl->print(2));
#endif
	    return spl->force_finite_bounds();
}


CG_result * CG_loop::liftOverhead(int depth) {
	assert(depth >= 1);
	if (needLoop) depth --;
	if (depth) {
	    body = body->liftOverhead(depth);
	    return this;
	}
	assert(needLoop);
	Relation c = body->findOverhead(level);
	if (c.is_obvious_tautology()) return this;
	assert(c.is_upper_bound_satisfiable());
	String before;
	if (code_gen_debug) {
	    fprintf(DebugFile,"Decided to lift out overhead of %s\n",
		    (const char *) c.print_formula_to_string());
	    before = print(1);
	    fprintf(DebugFile,"from: \n%s\n", (const char *)before);
	}
	CG_result *s = new CG_split(isActive,level,c,this,this->new_copy());
	Relation restrictions_copy = restrictions,
	    known_copy = known;
	s = s->recompute(known_copy,restrictions_copy,eliminated);
	if (code_gen_debug) {
	    String after = s->print(1);
	    fprintf(DebugFile,"Lifted out overhead of %s\n",
		    (const char *) c.print_formula_to_string());
	    fprintf(DebugFile,"from:\n%s\n",(const char *)before);
	    fprintf(DebugFile,"to get:\n%s\n",(const char *)after);
	}
	s = s->liftOverhead(depth+1);
	return s;
}


String CG_loop::printStructure(int indent) { // CG_loop print method
	String S = "";
	int indnt = indent;
	    char buf[80];
	    sprintf(buf,"%p",(void *)this);
	    S += outputSpaces(indent) 
		+ "// loop for t" + itoS(level) + " at "
		+ buf
		+ "\n";
	    if (eliminated.any_eliminated()) {
	      S += outputSpaces(indent) + "// eliminated: ";
	      S += eliminated.print_eliminated_to_string() + "\n";
	      }
	    // Print each statement's bounds in debugging info
		for(int s = 1; s<= isActive.size(); s++) if (isActive[s]) {
		    S += outputSpaces(indent)
			+ "// "
			+ statementNameInfo[s]->debug_name()
			+ ":" + projected_nIS[level][s].print_formula_to_string()
			+ "\n";
		}

	return S + body->printStructure(indnt+1) + outputSpaces(indnt+1)+ "}\n";
	} 



//-----------------------------------------------------------------------------
// original CG_loop::print(int) is now a wrapper of printRepr(CG_outputBuilder*, int)
//-----------------------------------------------------------------------------
String CG_loop::print(int indent)
{
  CG_stringBuilder oscg;
  CG_outputRepr* loopRepr = printRepr(&oscg, indent);
  return GetString(loopRepr);
}

//*****************************************************************************
// Added by D people. Lei Zhou Apr. 20, 96
//*****************************************************************************
//-----------------------------------------------------------------------------
// CG_loop member function: 
// CG_outputRepr* CG_loop::printRepr(CG_outputBuilder* ocg, int indent)
//-----------------------------------------------------------------------------
CG_outputRepr* CG_loop::printRepr(CG_outputBuilder* ocg, int indent)
{
  CG_outputRepr* loopRepr = CG_REPR_NIL;  // the entire loop
  CG_outputRepr* bodyRepr = CG_REPR_NIL;  // loop body
  CG_outputRepr* ctrlRepr = CG_REPR_NIL;  // loop control: index, low, up, step
  CG_outputRepr* guardRepr = CG_REPR_NIL; // guard 
#if 0
  CG_outputRepr* spaceRepr = CG_REPR_NIL; // indent space 
#endif
  CG_outputRepr* loopReprList = ocg->CreateStmtList();

  String S = "";
  int indnt = indent;
  int guarded = 0;

  if (code_gen_debug) {
    String loopComment;
    CG_outputRepr *textRepr;
    char buf[80];
    sprintf(buf,"%p",(void *)this);
    loopComment = "loop for t" + itoS(level) + " at " + buf;
    textRepr = ocg->CreateComment(indent, loopComment);
    loopReprList = ocg->StmtListInsertLast(loopReprList, textRepr);

    loopComment = "known: " + known.print_formula_to_string();
    textRepr = ocg->CreateComment(indent, loopComment);
    loopReprList = ocg->StmtListInsertLast(loopReprList, textRepr);

    loopComment = "restrictions: " + restrictions.print_formula_to_string();
    textRepr = ocg->CreateComment(indent, loopComment);
    loopReprList = ocg->StmtListInsertLast(loopReprList, textRepr);

    if (eliminated.any_eliminated()) {
      loopComment = "eliminated: ";
      loopComment += eliminated.print_eliminated_to_string();
      textRepr = ocg->CreateComment(indent, loopComment);
      loopReprList = ocg->StmtListInsertLast(loopReprList, textRepr);
    }

    loopComment = "active: ";
    for(int s = 1; s <= isActive.size(); s++) {
      if (isActive[s]) {
	loopComment += itoS(s) + " ";
      }
    }
    textRepr = ocg->CreateComment(indent, loopComment);
    loopReprList = ocg->StmtListInsertLast(loopReprList, textRepr);

    loopComment = "bounds: " + bounds.print_formula_to_string();
    textRepr = ocg->CreateComment(indent, loopComment);
    loopReprList = ocg->StmtListInsertLast(loopReprList, textRepr);

    loopComment = "guard: " + guard.print_formula_to_string();
    textRepr = ocg->CreateComment(indent, loopComment);
    loopReprList = ocg->StmtListInsertLast(loopReprList, textRepr);

    // Print each statement's bounds in debugging info
    int cc = !hasBound(bounds,level,0) || !hasBound(bounds,level,1);
    if (cc) {
      int s;
       for(s = 1; s<= isActive.size(); s++) if (isActive[s]) {
	 assert(Intersection(Intersection(copy(restrictions), copy(known)),
                                          copy(projected_nIS[level][s])).is_upper_bound_satisfiable());

	 loopComment = statementNameInfo[s]->debug_name() + ":"
                       + projected_nIS[level][s].print_formula_to_string();
	 textRepr = ocg->CreateComment(indent, loopComment);
	 loopReprList = ocg->StmtListInsertLast(loopReprList, textRepr);
       }
		
       // Print hull in debugging info

       Relation hull(Hull(projected_nIS[level],isActive,1));
       loopComment = "hull: " + hull.print_formula_to_string();
       textRepr = ocg->CreateComment(indent, loopComment);
       loopReprList = ocg->StmtListInsertLast(loopReprList, textRepr);
     }

  } // if (code_gen_debug)

  // Generate guard if necessary
  // If code_gen_check_zero_trip is set, use the guard as is; otherwise,
  // omit the parts of the guard that check if this is zero-trip loop

  Relation current_guard;
  if (! code_gen_check_zero_trip) {
    current_guard = guard;
  }
  else {
    // gist guard given the zero trip check
    Relation zero_trip_check = Project(Gist(copy(bounds),copy(guard)),
				       level, set_var(level)->kind());
    current_guard = Gist(copy(guard),zero_trip_check);
  }

  if (!current_guard.is_obvious_tautology()) {
    guarded = 1;
    guardRepr = output_guard(ocg, current_guard);
    indnt++;
  }

  Variable_ID v = set_var(level);       // get ID of current loop var

  Relation enforced(known);
  // b: actual bounds are gisted given info in guards
  Relation b = Gist(copy(bounds),copy(current_guard),1); 
  name_codegen_vars(b);

  bool r = printBounds(ocg, b, v, indnt, copy(known), enforced, ctrlRepr,
		       eliminated);

  // Below should never happen: says the current variable v generates an
  // assignment rather than a loop, but we thought we needed a loop

  if (needLoop && eliminated.is_eliminated(v))
     r = 0; 
  if (!r) {
    fprintf(DebugFile,"Code generation failure\n");
    fprintf(DebugFile,"Loop %p at lvl %d\n", (void *) this, level);
    if (!r) fprintf(DebugFile,"printBounds failed\n");
    fprintf(DebugFile,"bounds:\n");
    bounds.prefix_print(DebugFile);
    fprintf(DebugFile,"known:\n");
    known.prefix_print(DebugFile);
    fprintf(DebugFile,"restrictions:\n");
    restrictions.prefix_print(DebugFile);
    fprintf(DebugFile,"active:");

    int s;
    for(s = 1; s <= isActive.size(); s++) 
      if (isActive[s]) fprintf(DebugFile,"%d ",s);
    fprintf(DebugFile,"\n");
    SetTuple I(isActive.size());
    Relation hull = Intersection(Hull(projected_nIS[level],isActive,1),
				 greatest_common_step(projected_nIS[level], 
						      isActive, 
						      level,last_level, isActive.size()));
    fprintf(DebugFile,"new hull:\n");
    hull.prefix_print(DebugFile);
    assert(0 && "printBounds failure, see debugging file for details");
  } // if (!r)... (failure)

  if (!needLoop) {
    if (ctrlRepr) /* check if assignment was made */
      loopReprList = ocg->StmtListInsertLast(loopReprList, ctrlRepr); 
    bodyRepr = body->printRepr(ocg, indnt);
    loopRepr = bodyRepr;
  }
  else {
    bodyRepr = body->printRepr(ocg, indnt+1);
    loopRepr = ocg->CreateLoop(indnt, ctrlRepr, bodyRepr,gen_python);
  }


  loopReprList = ocg->StmtListInsertLast(loopReprList, loopRepr);

  if (!current_guard.is_obvious_tautology()) {
    CG_outputRepr* ifstmt = ocg->CreateIf(indent, guardRepr, loopReprList);
    CG_outputRepr* ifList= ocg->CreateStmtList();
    ifList = ocg->StmtListInsertLast(ifList, ifstmt);

    return ifList;
  }

  return loopReprList;
}


CG_result * CG_loop::recompute(const Relation k, const Relation r,
				  elimination_info e)  { // CG_loop recompute
// We generate a guard (and change known) only when a real loop is generated.
	known = k;
	restrictions = r;
	eliminated = e;
	known.simplify();
	restrictions.simplify();
	Relation kr = Intersection(copy(known),
				   copy(restrictions));
	kr.simplify();
	if (code_gen_debug) {
		fprintf(DebugFile,"\nRecomputing loop for t%d (node %p)\n",
			level, (void *)this);
		fprintf(DebugFile,"known: %s\n",
			(const char *) known.print_formula_to_string());
		if (!restrictions.is_obvious_tautology()) {
			fprintf(DebugFile,"restrictions: %s\n",
				(const char *) restrictions.print_formula_to_string());
			fprintf(DebugFile,"Known and restrictions: %s\n",
				(const char *) kr.print_formula_to_string());
			}
		}
	SetTuple I(isActive.size());
	int anyActive = 0;
	int s;
	for(s = 1; s<= isActive.size(); s++) if (isActive[s]) {
	    if (!Intersection(copy(kr),copy(projected_nIS[level][s])).is_upper_bound_satisfiable())  {
		
		if (code_gen_debug) {
			fprintf(DebugFile,"Thought s%d was active but it isn't\n",
					s);
			fprintf(DebugFile,"Iteration space: %s\n",
				(const char *) projected_nIS[level][s].print_formula_to_string());
			}
		isActive[s] = 0;
		}
	    else anyActive = 1;
	};
	if (!anyActive) {
	    CG_result *r = new CG_null();
	    if (code_gen_debug) {
		fprintf(DebugFile,"Pruning loop node %p\n",
			(void *)this);
		fprintf(DebugFile,"Known: %s\n",
			(const char *) known.print_formula_to_string());
		fprintf(DebugFile,"restrictions: %s\n",
			(const char *) restrictions.print_formula_to_string());
	    };
	    delete this;
	    return r;
	};
	Relation projected_hull = Hull(projected_nIS[level],isActive,1);

	Relation the_gcs =
	    greatest_common_step(projected_nIS[level], isActive, 
					      level,last_level,isActive.size());
	if (code_gen_debug) {
		fprintf(DebugFile,"greatest common step is: %s\n",
				(const char *) the_gcs.print_formula_to_string());
		fprintf(DebugFile,"Projected Hull is: %s\n",
				(const char *) projected_hull.print_formula_to_string());
		}
	Relation hull = Intersection(projected_hull, the_gcs);

	hull.simplify();
	assert(hull.is_upper_bound_satisfiable());
	if (code_gen_debug) {
		fprintf(DebugFile,"first hull is: %s\n\n",
				(const char *) hull.print_formula_to_string());
		}
	hull = Intersection(hull,copy(kr));
	hull = eliminated.project_eliminated(hull);
	hull.simplify();
	Relation k0 = copy(known);
	k0 = eliminated.project_eliminated(k0);
	bounds = Gist(copy(hull),copy(k0),1);
	bounds.simplify();
	name_codegen_vars(bounds);
	needLoop = 1;
	Relation eq_constraint = Relation::True(bounds);
	Conjunct *c = bounds.query_DNF()->single_conjunct();
	EQ_Iterator eq(c);
	Variable_ID v = set_var(level);
	// Search for EQs that involves the interesting variable
	for(;eq.live() && needLoop;eq++) {
	    if ((*eq).get_coef(v)) {
		for(Constr_Vars_Iter w(*eq, 1); w; w++)
		    if ((*w).coef != 0) goto nextEQ;
		// Found EQ involving v and no wildcards
		assert(needLoop); // there are not two of these
		eq_constraint.and_with_EQ(*eq);
		eq_constraint.finalize();
		needLoop = 0;
	    }
	  nextEQ: ;
	}
	if (!needLoop) {
#if 0
	    fprintf(DebugFile,"No loop needed\n");
#endif
	    if(eliminated.may_eliminate(set_var(level)))
	      eliminated.eliminate(set_var(level));
	    // don't generate guard, exclude that info from known
	    guard = Relation::True(bounds.n_set());
	    // add only info about current loop var to known, not guard info
	    bounds = eq_constraint;
	    Relation new_known = Intersection(copy(known),eq_constraint);
	    Relation new_restrictions = Gist(copy(restrictions),copy(new_known));
	    body = body->recompute(new_known,new_restrictions,eliminated);
	    return this;
	}
	
	Relation loop_guard = Gist(project_onto_levels(copy(bounds),level-1,1),
			  copy(k0),1);
	loop_guard.simplify();
	// A real loop will be generated
	guard = loop_guard;
	name_codegen_vars(guard);
#if 0
	fprintf(DebugFile,"Loop guard at lvl %d for %x is:\n",
		level,
		(void *) this);
	guard.prefix_print(DebugFile);
#endif
	guard = Approximate(guard,1); // Should check for exactness
#if 0
	fprintf(DebugFile,"Approximate guard is:\n");
	guard.prefix_print(DebugFile);
	fprintf(DebugFile,"Bounds are:\n");
	bounds.prefix_print(DebugFile);
	fprintf(DebugFile,"hull is:\n");
	hull.prefix_print(DebugFile);
	fprintf(DebugFile,"known is:\n");
	known.prefix_print(DebugFile);
	fprintf(DebugFile,"kr is:\n");
	kr.prefix_print(DebugFile);
	fprintf(DebugFile,"k0 is:\n");
	k0.prefix_print(DebugFile);
	Relation b99 = Gist(copy(hull),copy(k0),1);
	fprintf(DebugFile,"b99 is:\n");
	b99.prefix_print(DebugFile);
	fprintf(DebugFile,"active: ");
	for(s = 1; s <= isActive.size(); s++) 
	    if (isActive[s])
		fprintf(DebugFile,"%d ",s);
	fprintf(DebugFile,"\n");
	if (eliminated.any_eliminated()) {
	  fprintf(DebugFile,"eliminated: ");
	  eliminated.print_eliminated(DebugFile);
	}
	fprintf(DebugFile,"\n\n\n");
#endif
	Relation newRestrictions = Relation::True(hull.n_set());
	body = body->recompute(Intersection(hull,kr),
			       newRestrictions,eliminated);
	return this;
}



SetTuple filter_function_symbols(SetTuple &sets, bool keep_fs){
  SetTuple new_sets(sets.size());
  for(int i = 1; i <= sets.size(); i++) {
    Relation R = sets[i];
    Relation &S = new_sets[i];
    assert(R.is_set());
    S = Relation(R.n_set());
    S.copy_names(R);
    F_Exists *fe = S.add_exists();
    F_Or *fo = fe->add_or();
    for(DNF_Iterator D(R.query_DNF()); D; D++) {
      F_And *fa = fo->add_and();
      Variable_ID_Tuple &oldlocals = (*D)->locals();
      Section<Variable_ID> newlocals = fe->declare_tuple(oldlocals.size());

      /* copy constraints.  This is much more difficult than it needs
	 to be, but add_EQ(Constraint_Handle) doesn't work because it can't
	 keep track of existentially quantified varaibles across calls.
	 Sigh.  */

      for(EQ_Iterator e(*D); e; e++)
        if(max_fs_arity(*e) > 0 == keep_fs){
	  EQ_Handle n = fa->add_EQ();
	  for(Constr_Vars_Iter cvi(*e,false);cvi;cvi++)
	    if((*cvi).var->kind() == Wildcard_Var)
	      n.update_coef(newlocals[oldlocals.index((*cvi).var)],
			    (*cvi).coef);
	    else
	      if((*cvi).var->kind() == Global_Var)
		n.update_coef(S.get_local((*cvi).var->get_global_var(),
					  (*cvi).var->function_of()),
			      (*cvi).coef);
	      else
		n.update_coef((*cvi).var,(*cvi).coef);
	  n.update_const((*e).get_const());
	  n.finalize();
	}

      for(GEQ_Iterator g(*D); g; g++)
        if(max_fs_arity(*g) > 0 == keep_fs){
	  GEQ_Handle n = fa->add_GEQ();
	  for(Constr_Vars_Iter cvi(*g,false);cvi;cvi++)
	    if((*cvi).var->kind() == Wildcard_Var)
	      n.update_coef(newlocals[oldlocals.index((*cvi).var)],
			    (*cvi).coef);
	    else
	      if((*cvi).var->kind() == Global_Var)
		n.update_coef(S.get_local((*cvi).var->get_global_var(),
					  (*cvi).var->function_of()),
			      (*cvi).coef);
	      else
		n.update_coef((*cvi).var,(*cvi).coef);
	  n.update_const((*g).get_const());
	  n.finalize();
	}
    }
    S.finalize();
  }
  return new_sets;
}


RelTuple strip_function_symbols(SetTuple &sets){
  return filter_function_symbols(sets,false);
}

RelTuple extract_function_symbols(SetTuple &sets){
  return filter_function_symbols(sets,true);
}


String MMGenerateCode(RelTuple &T, SetTuple &old_IS, 
		      Tuple<naming_info *> &name_func_tuple,
		      Relation &known, int effort){
  CG_stringBuilder oscg;
  CG_outputRepr *sRepr = MMGenerateCode(&oscg, T, old_IS, name_func_tuple, 
                                        known, effort);
#if DUMP_TEST
  cout << "#" << endl;
  cout << "# ----- Beginning of CG_stringBuilder ------" << endl;
  sRepr->Dump();
  cout << "# -------- End of CG_stringBuilder ---------" << endl << endl;
#endif

  return GetString(sRepr);
}

String MMGenerateCode(RelTuple &T, SetTuple &old_IS, 
		      Relation &known, int effort){
  Tuple<naming_info *> NameInfo;
  for (int stmt = 1; stmt <= T.size(); stmt++)
  {
    NameInfo.append(new default_stmt_info(stmt));
  }
  return MMGenerateCode(T, old_IS, NameInfo, known, effort);
}

String MMGeneratePythonCode(RelTuple &T, SetTuple &old_IS, 
		      Relation &known, int effort){
  Tuple<naming_info *> NameInfo;
  for (int stmt = 1; stmt <= T.size(); stmt++)
  {
    NameInfo.append(new default_stmt_info(stmt));
  }
  gen_python=true;
  String s=MMGenerateCode(T, old_IS, NameInfo, known, effort);
  std::string str=(const char*)s;
  str=str.substr(0,str.length()-1);
  gen_python=false;
  return String(str.c_str());
}


//*****************************************************************************
// MMGenerateCode implementation, added by D people. Lei Zhou, Apr. 24, 96
//*****************************************************************************
CG_outputRepr*
MMGenerateCode(CG_outputBuilder* ocg,
               RelTuple &T, SetTuple &old_IS, Relation &known, int effort)
{
  Tuple<naming_info *> NameInfo;
  for (int stmt = 1; stmt <= T.size(); stmt++)
  {
    NameInfo.append(new default_stmt_info(stmt));
  }
  return MMGenerateCode(ocg, T, old_IS, NameInfo, known, effort);
}


CG_outputRepr*
MMGenerateCode(CG_outputBuilder* ocg, RelTuple &T, SetTuple &old_IS, 
               Tuple<naming_info *> &name_func_tuple,
	       Relation &known, int effort)
{

  int i;
  int stmts = T.size();
  if (OMEGA_WHINGE < 0)
    OMEGA_WHINGE = getenv("OMEGA_WHINGE") ? atoi(getenv("OMEGA_WHINGE")) : 0;
  
  new_IS.reallocate(stmts);
  transformations.reallocate(stmts);
  statementNameInfo.reallocate(stmts);    
  last_level = T[1].n_out();
  projected_nIS.clear();
  projected_nIS.reallocate(last_level);
  int maxStmt = 1;
  int stmt;
  
  for (stmt = 1; stmt <= stmts; stmt++) {
    for (i = 1; i <= last_level; i ++) {
      T[stmt].name_output_var(i,String("t")+itoS(i));
    }
    
    Relation R = Range(Restrict_Domain(copy(T[stmt]), copy(old_IS[stmt])));
    
    R.simplify();
    name_codegen_vars(R);
    while(R.is_upper_bound_satisfiable()) {
      new_IS.reallocate(maxStmt);
      transformations.reallocate(maxStmt);
      statementNameInfo.reallocate(maxStmt);
      DNF *dnf = R.query_DNF();
      DNF_Iterator c(dnf);
      new_IS[maxStmt] = Relation(R,*c);
      new_IS[maxStmt].simplify();
      transformations[maxStmt] = T[stmt];
      statementNameInfo[maxStmt] = name_func_tuple[stmt];
      maxStmt++;
      c.next();
      if (!c.live()) 
	break;
      if(code_gen_debug) {
	fprintf(DebugFile, "splitting iteration space for disjoint form\n");
	fprintf(DebugFile, "Original iteration space: \n");
	R.print_with_subs(DebugFile);
	fprintf(DebugFile, "First conjunct: \n");
	new_IS[maxStmt-1].print_with_subs(DebugFile);
      }
      Relation remainder(R,*c);
      c.next();
      while (c.live()) {
	remainder = Union(remainder,Relation(R,*c));
	c.next();
      }
      R = Difference(remainder, copy( new_IS[maxStmt-1]));
      R.simplify();
      if(code_gen_debug) {
	fprintf(DebugFile, "Remaining iteration space: \n");
	R.print_with_subs(DebugFile);
      }
    }
  }


  /* Function symbols are removed from the iteration space at the top,
     then inserted into statement guards at the leaves, since set
     variables can't be projected away when they are arguments to
     function symbols.  */
  function_guards = extract_function_symbols(new_IS);
  new_IS = strip_function_symbols(new_IS);
  elimination_info eliminated(function_guards);
  
  stmts = maxStmt-1;
  if(stmts == 0) { // Empty region
    return ocg->CreateComment(1, "No points in any of the iteration spaces!");
  }
  
  
  if (code_gen_debug) {
    fprintf(DebugFile, "::: MMGenerateCode\n");
  }
  
  for(i = 1; i <= last_level; i++ ) {
    projected_nIS[i].reallocate(stmts);
  }
  
  for (stmt = 1; stmt <= stmts; stmt++) {
    Relation I = new_IS[stmt];
    for(i = last_level; i >= 1; i-- ) {
      I = project_onto_levels(I, i,0);
      projected_nIS[i][stmt] = I;
    }
    for(i = 2; i <= last_level; i++ ) {
      projected_nIS[i][stmt] = Intersection(projected_nIS[i][stmt],I);
      projected_nIS[i][stmt].simplify();
      I = projected_nIS[i][stmt];
    }
    new_IS[stmt] = Intersection(new_IS[stmt],I);
    
    
    if (code_gen_debug > 1) {
      for(i = 1; i <= last_level; i++ ) {
	fprintf(DebugFile,"Stmt s%d iteration space at level %d: \n  %s\n",stmt,i,
		(const char *) projected_nIS[i][stmt].print_formula_to_string());
	
      }
    }
  }
  
  if(known.is_null()) {
    known = Relation::True(last_level);
  }
  int maxRank = 0;
  for (stmt = stmts; stmt > 0; stmt--) {
    int rank,tmp;
    new_IS[stmt].dimensions(rank,tmp);
    assert(rank == tmp);
    if (maxRank < rank) maxRank = rank;
  }
  IntTuple allStmts(stmts);
  for(i=1; i<=stmts; i++) {
    allStmts[i] = 1;
  }
  CG_result *r = gen_recursive(1, allStmts); 
  print_in_code_gen_style++;
  known.simplify();
  known=Approximate(known,1);
  known=Hull(known,1);

  if (code_gen_debug) {
    fprintf(DebugFile,"Before recompute:\n%s\n\n",
	    (const char *)r->printStructure(1));
  }
  r = r->recompute(known,Relation::True(last_level),eliminated);
  int rdepth = r->depth();
#if !defined(NDEBUG)
  if(code_gen_debug && maxRank != rdepth) {
    fprintf(DebugFile, "ERROR: dimension computation failed!\n"
	    "should be %d, computed as %d\n",rdepth,maxRank);
    fprintf(stderr, "// ERROR: dimension computation failed!\n"
	    "// should be %d, computed as %d\n",rdepth,maxRank);
  }
#endif

  if (code_gen_debug) {
    fprintf(DebugFile,"Before overhead removal:\n%s\n\n",
	    (const char *)r->print(1));
  }

  if (effort >= -1) {
    r = r->force_finite_bounds();
  }
  if (effort >= 0)  {
    for(int ov = rdepth; ov >= rdepth-effort && ov >= 1; ov--) {
      r = r->liftOverhead(ov);
#if !defined(NDEBUG)
      if(code_gen_debug && ! r->verify_overhead_removal(ov))
	fprintf(DebugFile,"ERROR: overhead removal failure,"
		" overhead NOT removed to level %d\n",ov);
#endif

      if (code_gen_debug) {
	fprintf(DebugFile,"Overhead removed to depth %d:\n%s\n\n",ov,
		(const char *)r->print(1));
      }
    }
  }
  
#if !defined(NDEBUG)
  if(! r->verify_overhead_removal(rdepth-effort))
    fprintf(stderr,"ERROR: overhead removal failure,"
	    " overhead NOT removed to level %d\n",rdepth-effort);
  
  if(code_gen_debug > 1) {
    int pick;
    bool lastone=true;
    for(pick = 0; pick <= rdepth && lastone; pick++)
      lastone = !r->verify_overhead_removal(pick);
    fprintf(stderr,"Innermost overhead nested inside level %d\n", pick-1);
  }
#endif


  //--------------------------------------------------------------
  // really print out the loop
  //--------------------------------------------------------------
  CG_outputRepr* sRepr;
  if(gen_python)
    sRepr = r->printRepr(ocg, 2);
  else
    sRepr = r->printRepr(ocg, 1);

  print_in_code_gen_style--;
  delete r;
  r = 0;
  projected_nIS.clear();
  transformations.clear();
  new_IS.clear();

  return sRepr;
}

CG_result * gen_recursive( int level, IntTuple &isActive)
{

    int stmts = isActive.size();

    Set<int> active;
    int s;
    for(s = 1; s <= stmts; s++)
	if(isActive[s]) active.insert(s);

    assert (active.size() >= 1);
    if(level > last_level) return new CG_leaf(isActive);

    if (active.size() == 1)
	    return new CG_loop(isActive,level, gen_recursive(level+1,isActive));

    if(code_gen_debug) {
	fprintf(DebugFile, "::: gen_recursive: level %d\n",level);
	fprintf(DebugFile, ":::   incoming isActive statements: ");
	foreach(i, int, active, fprintf(DebugFile, "%d ", i));
	if(code_gen_debug > 1) {
	    fprintf(DebugFile, "\n::: incoming transformed iteration spaces:\n");
	    for(int i=1; i<=stmts; i++) {
		if (isActive[i]) {
		    fprintf(DebugFile, "statement %d:\n", i);
		    projected_nIS[level][i].prefix_print(DebugFile);
		}
	    }
	}
	fprintf(DebugFile, "\n");
    }

#if PAINFULLY_EXPENSIVE_DEBUGGING
    // Assert that the isActive list is in correspondence
    // with the iteration spaces
    if (code_gen_debug) {
	foreach(check,int,active,
		{ 
		  assert(!isActive[check] 
		       || (!projected_nIS[level][check].is_null() && 
		       projected_nIS[level][check].is_upper_bound_satisfiable()));});
	}
#endif


	bool constantLevel = true;
   
	int test_rel_size;
	coef_t start,finish; 
	finish = -(posInfinity-1);	// -(MAXINT-1);
	start = posInfinity;    	// MAXINT;
        Tuple<coef_t> when(stmts);
	for(s=1; s<=stmts; s++) if (isActive[s]) {
		coef_t lb,ub;
		test_rel_size = projected_nIS[level][s].n_set();
		projected_nIS[level][s].single_conjunct()
			->query_variable_bounds(
				projected_nIS[level][s].set_var(level),
				lb,ub);
	        if(code_gen_debug) {
			fprintf(DebugFile, "IS%d:  " coef_fmt " <= t%d <= " coef_fmt "\n",s,
					lb,level,ub);
			projected_nIS[level][s].prefix_print(DebugFile);
			}
		if (lb != ub) {
			constantLevel = false;
			break;
			}
		else {
			set_max(finish,lb);
			set_min(start,lb);
			when[s] = lb;
		};
		}

	
	if (constantLevel && finish-start <= stmts) {
    		IntTuple newActive(isActive.size());
	        for(int i=1; i<=stmts; i++)  
			newActive[i] = isActive[i] && when[i] == start;
		CG_result *r  = new CG_loop(isActive,level, 
				gen_recursive(level+1,newActive));
		for(coef_t time = start+1; time <= finish; time++) {
		    int count = 0;
		    for(int i=1; i<=stmts; i++)   {
			newActive[i] = isActive[i] && when[i] == time;
			if (newActive[i]) count++;
			}
		    if (count) {
			Relation test_rel(test_rel_size);
			GEQ_Handle g = test_rel.and_with_GEQ();	
			g.update_coef(test_rel.set_var(level),-1);
			g.update_const(time-1);
			
		        r = new CG_split(isActive,level,test_rel,r,
				new CG_loop(isActive,level, 
					gen_recursive(level+1,newActive)));
			}
		
		}
		return r;
		}


// Since the Hull computation is approximate, we will get regions that
// have no stmts.  (since we will have split on constraints on the
// hull, and thus we are looking at a region outside the convex hull
// of all the iteration spaces.)


#if 1
    Relation hull = Hull(projected_nIS[level],isActive,1);
#else
    Relation hull = Hull(projected_nIS[level],isActive,0);
#endif

#if PAINFULLY_EXPENSIVE_DEBUGGING
    if(code_gen_debug)
	foreach(s,int,active,assert(Must_Be_Subset(copy(projected_nIS[level][s]),copy(hull))));
#endif


    if(code_gen_debug) {
	fprintf(DebugFile, "Hull (level %d) is:\n",level);
	hull.prefix_print(DebugFile);
    }
    IntTuple firstChunk(isActive);
    IntTuple secondChunk(isActive);

    foreach(s,int,active,
	    {
	    Relation gist = Gist(copy(projected_nIS[level][s]),copy(hull),1);
	    if (gist.is_obvious_tautology()) break;
	    gist.simplify();
	    Conjunct *s_conj = gist.single_conjunct();
	    for(GEQ_Iterator G(s_conj); G; G++) {
		Relation test_rel(gist.n_set());
		test_rel.and_with_GEQ(*G);
		Variable_ID v = set_var(level);
		coef_t sign = (*G).get_coef(v);
		if(sign > 0) test_rel = Complement(test_rel);
		if(code_gen_debug) {
		    fprintf(DebugFile, "Considering split from stmt %d:\n",s);
		    test_rel.prefix_print(DebugFile);
		}
		
		firstChunk[s] = sign <= 0;
		secondChunk[s] = sign > 0;
		int numberFirst = sign <= 0;
		int numberSecond = sign > 0;
		foreach(s2,int,active, if (s!=s2) {
		    if(code_gen_debug) 
			fprintf(DebugFile,"Consider stmt %d\n",s2);
		    bool t = Intersection(copy(projected_nIS[level][s2]),
					  copy(test_rel)).is_upper_bound_satisfiable();
		    bool f = Difference(copy(projected_nIS[level][s2]),
					copy(test_rel)).is_upper_bound_satisfiable();
		    assert(t || f);
		    if(code_gen_debug  && t&&f) 
			fprintf(DebugFile, "Slashes stmt %d\n",s2);
		    if (t&&f) goto nextGEQ;
		    if(code_gen_debug) {
			if (t)
			    fprintf(DebugFile, "true for stmt %d\n",s2);
			else 
			    fprintf(DebugFile, "false for stmt %d\n",s2);
		    }
		    if (t) numberFirst++;
		    else numberSecond++;
		    firstChunk[s2] = t;
		    secondChunk[s2] = !t;
		});
		assert(numberFirst+numberSecond>1 && "Can't handle wildcard in iteration space");
	       if(code_gen_debug) 
			fprintf(DebugFile, "%d true, %d false\n",
						numberFirst,
						numberSecond);
		if (numberFirst && numberSecond) {
		    // Found a dividing constraint
		    if(code_gen_debug) {
			fprintf(DebugFile, "Splitting on:");
			test_rel.prefix_print(DebugFile);
			fprintf(DebugFile, "First chunk:");
			    foreach(s3,int,active,
				    {assert(firstChunk[s3] || secondChunk[s3]);
				    assert(!(firstChunk[s3] && secondChunk[s3]));
				    if (firstChunk[s3]) fprintf(DebugFile,"s%d ",s);
				});
			fprintf(DebugFile, "\n");
		    }
			
		    return new CG_split(isActive,level,test_rel,
					gen_recursive(level,firstChunk),
					gen_recursive(level,secondChunk));
		}
	      nextGEQ: ;
	    }
	    }
	);

    // No way found to divide stmts without splitting, generate loop

    return new CG_loop(isActive,level, gen_recursive(level+1,isActive));
}


// VERIFY FUNCTIONS


bool CG_result::verify_overhead_removal(int) {
    return true;
}

bool CG_null::verify_overhead_removal(int) {
    return true;
}

bool CG_split::verify_overhead_removal(int depth) {
    bool b1 = trueClause->verify_overhead_removal(depth);
    bool b2 = falseClause->verify_overhead_removal(depth);
    return b1 && b2;
}

bool CG_leaf::verify_overhead_removal(int depth) {
    bool no_guards = true;
    if(depth < 0)
	for(int i = 1; i <= isActive.size(); i++)
	    if(isActive[i]) {
		if (!guard[i].is_obvious_tautology()) {
/* Exclude strides that can't be move further out */
		    for(DNF_Iterator d(guard[i].query_DNF()); d; d++)
			for(EQ_Iterator e(*d); e; e++)
  /* FERD -- Should be able to check on fs_arity more precisely. */
			    if(!(((*e).has_wildcards() 
				  && (*e).max_tuple_pos() == last_level)
				 || max_fs_arity(*e) > 0 )) {
				fprintf(stderr,"statement guard overhead found on "
					"statement %d\n",i);
				no_guards = false;
			    }
		}
            }
    return no_guards;
}




/*
    Overheads here are in the form of min/maxes and guards
    Zero-trip loops are checked by the guards, and z-t guards are
    not removed from them till printing, so we don't have to examine 
    bounds for those.
*/
bool CG_loop::verify_overhead_removal(int depth) {
    bool no_over_guard=true,no_over_minmax=true; 
    if(depth < 0) {
	if(!guard.is_obvious_tautology()) {
	    /* Exclude strides that can't be moved further out */
	    for(DNF_Iterator d(guard.query_DNF()); d; d++)
		for(EQ_Iterator e(*d); e; e++)
		    if(!(((*e).has_wildcards() 
			  && (*e).max_tuple_pos() == level-1)
			 || max_fs_arity(*e) == level-1)) {
			no_over_guard = false;
			fprintf(stderr,"loop guard overhead found at level %d\n",level);
		    }
	}
	coef_t coef;
	int upper_bounds=0,lower_bounds=0;
	Variable_ID v = set_var(level);
	
	if(needLoop) {
	    for(GEQ_Iterator CI(bounds.single_conjunct()); CI; CI++) 
		if ((coef = (*CI).get_coef(v)) != 0) {  // In this constraint?
		    if(coef > 0) // lb or ub
			lower_bounds++;
		    else 
			upper_bounds++;
		}
//	    assert(lower_bounds > 0 && upper_bounds > 0);
	    if (lower_bounds > 1 || upper_bounds > 1 ) {
		fprintf(stderr,"min/max overhead found at level %d\n",level);
		no_over_minmax = false;
	    }
	} else {
	    // no overheads result from a single-valued level
	    no_over_minmax = true;
	}
    }
    bool no_over_body = body->verify_overhead_removal(needLoop?depth-1:depth);
    return (no_over_guard && no_over_minmax && no_over_body);
}

//*****************************************************************************
// static function added by D people. May 20.
//*****************************************************************************
static String GetString(CG_outputRepr* repr)
{
  String result = ((CG_stringRepr *) repr)->GetString();
  delete repr;
  return result;
}

#else

int code_gen_debug=0;

String MMGenerateCode(Tuple<Relation> &, Tuple<Relation> &, Relation &,
			int)
    {
    assert(0);
    abort();
    return "";
    }

String MMGenerateCode(Tuple<Relation> &, Tuple<Relation> &, Tuple<char*> &,
			Relation, int)
    {
    assert(0);
    abort();
    return "";
    }


#endif
} // using namespace omega
