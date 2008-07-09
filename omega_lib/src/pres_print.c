/* $Id: pres_print.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <omega/pres_gen.h>
#include <omega/pres_var.h>
#include <omega/pres_tree.h>
#include <omega/pres_conj.h>
#include <omega/Relation.h>
#include <basic/Bag.h>
#include <omega/omega_i.h>
#include <basic/Exit.h>
#include <omega/omega_core/oc.h>

namespace omega {


////////////////////////////////////////
//                                    //
//        Print functions.            //
//                                    //
////////////////////////////////////////

void Conjunct::reorder_for_print(bool reverseOrder,
		int first_pass_input,
		int first_pass_output,
		bool sort
		) {
    
    Conjunct *C2 = copy_conj_same_relation();
    Variable_ID_Tuple newpos(0),wcvars(0),gvars(0);

// We reorder the original Variable_ID's into the newpos list; later, we 
// copy from their original column (using find_column) to the new one.
    int n = mappedVars.size();
    int i;
    // there may be more inp/outp vars than maxVars; must do dynamically
    skip_set_checks++;
    Tuple<bool> input_used(myRelation->n_inp());
    Tuple<bool> output_used(myRelation->n_out());
    for(i=1; i<=myRelation->n_inp();i++) input_used[i] = false;
    for(i=1; i<=myRelation->n_out();i++) output_used[i] = false;
    for(i=1; i<=n;i++) {
	if (mappedVars[i]->kind() == Input_Var)
	  input_used[mappedVars[i]->get_position()] = true;	
	else if (mappedVars[i]->kind() == Output_Var)
	  output_used[mappedVars[i]->get_position()] = true;	
	else if(mappedVars[i]->kind() == Global_Var)
	  gvars.append(mappedVars[i]);
	}


    if(sort)
      for(i=1; i<=gvars.size();i++)
	for(int j=1; j <= gvars.size(); j++)
	  if(gvars[j]->get_global_var()->base_name()
	     < gvars[j+1]->get_global_var()->base_name()) {
	    Variable_ID t = gvars[j]; gvars[j] = gvars[j+1]; gvars[j+1] = t;
	  }

    newpos.join(gvars);

    if(!reverseOrder) {
	    for(i=1; i<=min(myRelation->n_inp(),first_pass_input);i++)
		if (input_used[i]) newpos.append(input_vars[i]);
	    for(i=1; i<=min(myRelation->n_out(),first_pass_output);i++)
		if (output_used[i]) newpos.append(output_vars[i]);
	    for(i=omega::max(1,first_pass_input+1); i<=myRelation->n_inp();i++)
		if (input_used[i]) newpos.append(input_vars[i]);
	    for(i=omega::max(1,first_pass_output+1); i<=myRelation->n_out();i++)
		if (output_used[i]) newpos.append(output_vars[i]);
	    }
    else {
	    for(i=1; i<=min(myRelation->n_out(),first_pass_output);i++)
		if (output_used[i]) newpos.append(output_vars[i]);
	    for(i=1; i<=min(myRelation->n_inp(),first_pass_input);i++)
		if (input_used[i]) newpos.append(input_vars[i]);
	    for(i=omega::max(1,first_pass_output+1); i<=myRelation->n_out();i++)
		if (output_used[i]) newpos.append(output_vars[i]);
	    for(i=omega::max(1,first_pass_input+1); i<=myRelation->n_inp();i++)
		if (input_used[i]) newpos.append(input_vars[i]);
         }


    for(i=1; i<=n;i++)
	if (mappedVars[i]->kind() == Wildcard_Var)
	    wcvars.append(mappedVars[i]);

    if(sort)
      for(i=1; i<=gvars.size();i++)
	for(int j=1; j <= gvars.size(); j++)
	  if(gvars[j]->name() < gvars[j+1]->name()) {
	    Variable_ID t = gvars[j]; gvars[j] = gvars[j+1]; gvars[j+1] = t;
	  }

    newpos.join(wcvars);

    assert(problem->nVars == newpos.size()); // i.e. no other variable types

    // Copy coef columns into new order.  Constant column is unchanged.
    for(int e=0; e<problem->nGEQs; e++) problem->GEQs[e].touched = 1;
	
    for(i=1; i<=problem->nVars; i++) {
	int col = find_column(newpos[i]); // Find column in original conj.
	assert(col != 0);
	copy_column(problem, i,      // Copy it from orig. column in the copy.
		    C2->problem, col, 0, 0);
	problem->var[i] = i;
    }
    for(i=1; i<=problem->nVars; i++)   
    	problem->forwardingAddress[i] = i;

    mappedVars = newpos;
    delete C2;
    skip_set_checks--;
}

void Rel_Body::print_with_subs(FILE *output_file, bool printSym, bool newline) 
    {
	String s = this->print_with_subs_to_string(printSym, newline);
	fprintf(output_file, (const char *) s);
    }

void Rel_Body::print_with_subs() 
    {
    this->print_with_subs(stdout, 0, 1);
    }

static
String tryToPrintVarToStringWithDiv(Conjunct *C, Variable_ID v) {
    String s;
    bool seen = false;
// This assumes that there is one EQ involving v, that v cannot be 
// substituted and hence has a non-unit coefficient.
    for(EQ_Iterator e(C); e; e++) {
	if ((*e).get_coef(v) != 0) {
	    assert(!seen);  // This asserts just one EQ with v 
	    coef_t v_coef = (*e).get_coef(v);
	    int v_sign = v_coef > 0 ? 1 : -1;
	    v_coef *= v_sign;
	    int sign_adj = -v_sign;

	    s += "intDiv(";
	    bool first=true;
	    for(Constr_Vars_Iter i(*e,false); i; i++) {
		if ((*i).var != v && (*i).coef != 0) {
		    coef_t this_coef = sign_adj*(*i).coef;
		    if(!first && this_coef > 0)
			s+= "+";
		    if (this_coef == 1)
			s += (*i).var->name();
		    else if (this_coef == -1) 
			{
			    s +=  "-"; s += (*i).var->name();
			}
		    else
			{
			    s += itoS(this_coef) + "*" + (*i).var->name();
			}
		first = false;
		}
	    }
	    coef_t the_const = (*e).get_const()* sign_adj;
	    if (the_const > 0 && !first)
		s+= "+";
	    if (the_const != 0)
		s += itoS(the_const);
	    s += "," + itoS(v_coef) + ")";
	    seen = true;
	}
    }
    return s;
}


// This function prints the output tuple of a relation with each one as a 
// function of the input variables.  
// This will fail or look goofy  if the outputs are not affine functions of
// the inputs.
// BIG WARNING: Call this only from the codegen stuff, since it assumes things
// about coefficients
String Rel_Body::print_outputs_with_subs_to_string() 
{
    Rel_Body S(this),Q(this);
    S.setup_names();
    Conjunct *C = S.single_conjunct();
    Conjunct *D = Q.single_conjunct(); // ordered_elimination futzes with conj
    String s; //  = "[";
    C->reorder_for_print();
    C->ordered_elimination(S.global_decls()->length());
    // Print output names with substitutions in them
    for(int i=1; i<=S.n_out(); i++) {
	String t;
	int col = C->find_column(output_vars[i]);
	if (col != 0)
	    t = C->print_sub_to_string(col);
	if (col == 0 || t == output_vars[i]->name()) // no sub found
	    // Assume you can't get a unit coefficient on v, must use div
	    t = tryToPrintVarToStringWithDiv(D,output_vars[i]);
	s += t;
	if (i < S.n_out())  s += ",";
    }
    // s += "] ";
    return s;
}

String Rel_Body::print_outputs_with_subs_to_string(int i) 
{
    Rel_Body S(this),Q(this);
    S.setup_names();
    Conjunct *C = S.single_conjunct();
    Conjunct *D = Q.single_conjunct(); // ordered_elimination futzes with conj
    String s; 
    C->reorder_for_print();
    C->ordered_elimination(S.global_decls()->length());
    // Print output names with substitutions in them
    {
	String t;
	int col = C->find_column(output_vars[i]);
	if (col != 0)
	    t = C->print_sub_to_string(col);
	if (col == 0 || t == output_vars[i]->name()) // no sub found?
	    t = tryToPrintVarToStringWithDiv(D,output_vars[i]);
	    // should check for failure
	s += t;
    }
    return s;
}

String Rel_Body::print_with_subs_to_string(bool printSym, bool newline) 
{
    String s="";

    if (pres_debug) {
	fprintf(DebugFile,"print_with_subs_to_string:\n");
	prefix_print(DebugFile);
	};

    int anythingPrinted = 0;

    if (this->is_null())
        {
	s = "NULL Rel_Body\n";
        return s;
        }

    Rel_Body R(this);
    bool firstRelation = true;

    for(DNF_Iterator DI(R.query_DNF()); DI.live(); DI.next()) {

	Rel_Body S(&R, DI.curr());
	Conjunct *C = S.single_conjunct();
        if(!simplify_conj(C, true, 4, black))  continue;

	S.setup_names();

	if(! firstRelation) 
	    {
	    s += " union";
	    if (newline) s += "\n ";
	    }
	else
	    firstRelation = false;

	anythingPrinted = 1;

	C->reorder_for_print(false,C->max_ufs_arity_of_in(),
				C->max_ufs_arity_of_out());
	C->ordered_elimination(S.Symbolic.length()
				+C->max_ufs_arity_of_in()
				+C->max_ufs_arity_of_out());
//	assert(C->problem->variablesInitialized);

	if (pres_debug) S.prefix_print(DebugFile);

	/* Do actual printing of Conjunct C as a relation */
	s += "{";
	
	if (!Symbolic.empty())
	    {
	    if (printSym) s += "Sym=[";
	    for (Variable_ID_Iterator Sym_I = S.Symbolic; Sym_I;) 
                {
		if (printSym)
		    s += (*Sym_I)->name();
		Sym_I++;
		if (printSym && Sym_I) s+=  ",";
		}
	    if (printSym) s += "] ";
	    }

	int i, col;
	
	if (S.number_input != 0) {
	    s +=  "[";
	    // Print input names with substitutions in them
	    for(i=1; i<=S.number_input; i++) {
	        col = C->find_column(input_vars[i]);
	        if (col != 0)
		    s += C->problem->print_sub_to_string(col);
	        else
		    s += input_vars[i]->name();
	        if (i<S.number_input) s += ",";
	    }
	    s +=  "]";
        }
	
	if (! S.is_set())
	    s +=  " -> ";

	if (S.number_output != 0)
	    {
	    s += "[";
	
	    // Print output names with substitutions in them
	    for(i=1; i<=S.number_output; i++) {
	        col = C->find_column(output_vars[i]);
	        if (col != 0)
		    s += C->problem->print_sub_to_string(col);
	        else
		    s +=  output_vars[i]->name();
	        if (i < S.number_output)  s += ",";
	        }
	    s += "] ";
	    }
	
        if (C->is_unknown())
	   {
	   if (S.number_input != 0 || S.number_output != 0)
               s += ":";
           s += " UNKNOWN";
	   }
        else
	  {

	  // Empty conj means TRUE, so don't print colon
	  if (C->problem->nEQs != 0 || C->problem->nGEQs != 0)  {
		C->problem->clearSubs();
	        if (S.number_input != 0 || S.number_output != 0)
	            s += ":";
	        s += " ";
		s += C->print_to_string(false);
		}
          else
		{
	        if (S.number_input == 0 && S.number_output == 0)
		    s += " TRUE ";
		}
	  }

	s +=  "}";
    }

    if (!anythingPrinted)
	{
	R.setup_names();
	s = "{";
        s += R.print_variables_to_string(printSym);
	if (R.number_input != 0 || R.number_output != 0)
            s += " :";
        s += " FALSE }";
	if (newline) s += "\n";
	return s;
	}

    if (newline) s += "\n";
    return s;
}


void print_var_addrs(String &s, Variable_ID v) {
  if(pres_debug>=2) {
    char ss[20];
    sprintf(ss, "(%p,%p)", v, v->remap);
    s += ss;
  }
}

String Rel_Body::print_variables_to_string(bool printSym)
{
  String s="";

  if (! Symbolic.empty())
      {
      if (printSym) s += " Sym=[";
      for (Variable_ID_Iterator Sym_I(Symbolic); Sym_I; ) 
	  {
	  if (printSym) s +=  (*Sym_I)->name();
	  print_var_addrs(s, *Sym_I);
	  Sym_I++;
	  if (printSym && Sym_I)  s += ",";
	  }
      if (printSym) s += "] ";
      }
  int i;

  if (number_input) {
      s += "[";
      for (i=1;i<=number_input;i++) {
          s += input_vars[i]->name();
          print_var_addrs(s, input_vars[i]);
          if(i < number_input) s += ",";
      }
      s += "] ";
  }

  if (number_output)
      {
      s += "-> [";
      for (i=1;i<=number_output;i++) {
          s += output_vars[i]->name();
          print_var_addrs(s, output_vars[i]);
          if(i < number_output) s += ",";
          }
      s += "] ";
      }

  return s;  
}


int F_Declaration::priority() {
	return 3;
	};
int Formula::priority () {
	return 0;
	};

int F_Or::priority() {
	return 0;
	};
int F_And::priority() {
	return 1;
	};
int Conjunct::priority() {
	return 1;
	};

int F_Not::priority() {
	return 2;
	};


//
// print() functions
//
void Formula::print(FILE *output_file)
    {
    if(myChildren.empty())
	{
	if(node_type()==Op_Relation || node_type()==Op_Or)
	    fprintf(output_file, "FALSE");
	else if(node_type()==Op_And)
	    fprintf(output_file, "TRUE");
	else
	    {
	    assert(0);
	    }
	}
    
    for(List_Iterator<Formula*> c(myChildren); c;)
	{
	if (node_type() == Op_Exists || node_type() == Op_Forall 
	    || (*c)->priority() < priority()) 
	    fprintf(output_file, "( ");
	(*c)->print(output_file);
	if (node_type() == Op_Exists || node_type() == Op_Forall 
	    || (*c)->priority() < priority()) 
	    fprintf(output_file, " )");
	c++;
	if(c.live())
	    print_separator(output_file);
	}
    }

String Rel_Body::print_formula_to_string() {
    String s;
    setup_names();
    for(DNF_Iterator DI(query_DNF()); DI.live();)  {

	s += (*DI)->print_to_string(1);
	DI.next();
	if (DI.live()) s += " && ";
	if (pres_debug) fprintf(DebugFile,"Partial print to string: %s\n",
		(const char *) s);
	};
    return s;
    };

void Rel_Body::print(FILE *output_file, bool printSym) {
  if (this->is_null()) {
    fprintf(output_file, "NULL Rel_Body\n");
    return;
  }

  setup_names();

  fprintf(output_file, "{");

  String s = print_variables_to_string(printSym);
  fprintf(output_file, (const char *) s);

  if(simplified_DNF==NULL) {
    Formula::print(output_file);
  } else {
    assert(children().empty());
    simplified_DNF->print(output_file);
  }

  fprintf(output_file, " }\n");
}

void Rel_Body::print() {
  this->print(stdout);
}

void F_Not::print(FILE *output_file) {
  fprintf(output_file, " not ");
  Formula::print(output_file);
}

void F_And::print_separator(FILE *output_file) {
  fprintf(output_file, " and ");
}

void Conjunct::print(FILE *output_file) {
  String s = print_to_string(true);
  fprintf(output_file, (const char *) s);
}

String Conjunct::print_to_string(int true_printed) {
  String s=""; 

  int num = myLocals.size();
  if(num) {
    s += "Exists ( ";
    int i;
    for (i = 1; i <= num; i++) {
      Variable_ID v = myLocals[i];
      s += v->char_name();
      if(i < num) s += ",";
    }
    if (true_printed || !(is_true())) s += " : ";
  }

  if(is_true()) {
    s += true_printed ? "TRUE" : "";
  } else { 
    if (is_unknown()) 
      s += "UNKNOWN";
    else {
      s += problem->prettyPrintProblemToString();
      if (!exact)
        s += " && UNKNOWN";
    } 
  }


  if (num) s += ")";
  return s;
}

void F_Or::print_separator(FILE *output_file) {
  fprintf(output_file, " or ");
}

void F_Declaration::print(FILE *output_file) {
  String s="";
  for(Variable_ID_Iterator VI(myLocals); VI; ) {
    s += (*VI)->name();
    VI++;
    if(VI) s += ",";
  }
  fprintf(output_file, "( %s : ", (const char *) s);
  Formula::print(output_file);
  fprintf(output_file, ")");
}

void F_Forall::print(FILE *output_file) {
  fprintf(output_file, "forall ");
  F_Declaration::print(output_file);
}

void F_Exists::print(FILE *output_file) {
  fprintf(output_file, "exists ");
  F_Declaration::print(output_file);
}

void Formula::print_separator(FILE *) {
  assert(0);    // should never be called, it's only for derived classes
}

// 
// Setup names in formula.  
//

typedef  Set<Global_Var_ID> g_set;
void Rel_Body::setup_names() {
  int i;


  if (use_ugly_names) return;

  if (pres_debug>=2)
    fprintf(DebugFile,"Setting up names for 0x%p\n", this);

  for(i=1; i<=number_input; i++) {
    input_vars[i]->base_name = In_Names[i];
  }
  for(i=1; i<=number_output; i++) {
    output_vars[i]->base_name = Out_Names[i];
  }

  g_set gbls;

  wildCardInstanceNumber = 0;

  for(i=1; i<= Symbolic.size(); i++) {
    gbls.insert(Symbolic[i]->get_global_var());
  }

  foreach(g,Global_Var_ID,gbls,
	g->instance = g->base_name()++);

  for(i=1; i<=number_input; i++) {
    if (!input_vars[i]->base_name.null())
	input_vars[i]->instance = input_vars[i]->base_name++;
  }
  for(i=1; i<=number_output; i++) {
    if (!output_vars[i]->base_name.null())
	output_vars[i]->instance = output_vars[i]->base_name++;
  }

  if (simplified_DNF != NULL)  // It is simplified
      simplified_DNF->setup_names();
  else                         // not simplified
      Formula::setup_names();

  for(i=1; i<=number_output; i++) {
    if (!output_vars[i]->base_name.null())
	output_vars[i]->base_name--;
  }
  for(i=1; i<=number_input; i++) {
    if (!input_vars[i]->base_name.null())
	input_vars[i]->base_name--;
  }
  foreach(g,Global_Var_ID,gbls, g->base_name()--);
}

void Formula::setup_names()
    {
    if (pres_debug>=2)
	fprintf(DebugFile,"Setting up names for formula 0x%p\n", this);

    for(List_Iterator<Formula*> c(myChildren); c; c++)
	(*c)->setup_names();
    }

void DNF::setup_names()
    {
    if (pres_debug>=2)
	fprintf(DebugFile,"Setting up names for DNF 0x%p\n", this);

    for(DNF_Iterator DI(this); DI.live(); DI.next()) 
	  DI.curr()->setup_names();
    }


void F_Declaration::setup_names() {
  if (pres_debug>=2)
    fprintf(DebugFile,"Setting up names for Declaration 0x%p\n", this);

  // Allow re-use of wc names in other scopes
  int savedWildCardInstanceNumber = wildCardInstanceNumber;

  for(Tuple_Iterator<Variable_ID> VI(myLocals); VI; VI++) {
    Variable_ID v = *VI;
    if (!v->base_name.null()) v->instance = v->base_name++;
    else  v->instance = wildCardInstanceNumber++;
  }

  for(List_Iterator<Formula*> c(children()); c; c++)
      (*c)->setup_names();

  for(Tuple_Iterator<Variable_ID> VI2(myLocals); VI2; VI2++) {
    Variable_ID v = *VI2;
    if (!v->base_name.null()) v->base_name--;
  }
  wildCardInstanceNumber = savedWildCardInstanceNumber;
}


//
// Prefix_print functions.
// Print Formula Tree, used in debugging.
//
static int level = 0;

void Formula::prefix_print(FILE *output_file, int debug)
    {
    for(List_Iterator<Formula*> c(children()); c; c++)
	(*c)->prefix_print(output_file, debug);
    level--;
    }


void Rel_Body::prefix_print() {
  this->prefix_print(stdout, 1);
}

void Rel_Body::prefix_print(FILE *output_file, int debug) {
   int old_use_ugly_names = use_ugly_names;
   use_ugly_names = 0;

  setup_names();

  level = 0;
  if(pres_debug>=2) fprintf(output_file, "(@%p)", this);
  fprintf(output_file, is_set() ? "SET: " : "RELATION: ");
  String s = print_variables_to_string(true);
  fprintf(output_file, "%s\n", (const char *) s);

  if(simplified_DNF==NULL) {
    Formula::prefix_print(output_file, debug);
  } else {
    assert(children().empty());
    simplified_DNF->prefix_print(output_file, debug,true);
  }
  fprintf(output_file, "\n");
  use_ugly_names = old_use_ugly_names;
}


void F_Forall::prefix_print(FILE *output_file, int debug) {
  Formula::print_head(output_file);
  fprintf(output_file, "FORALL [");
  F_Declaration::prefix_print(output_file, debug);
  for (Variable_ID_Iterator VI(myLocals); VI; VI++)
     assert((*VI)->kind() == Forall_Var);

}

void F_Exists::prefix_print(FILE *output_file, int debug) {
  Formula::print_head(output_file);
  if(pres_debug>=2) fprintf(output_file, "(@%p)", this);
  fprintf(output_file, "EXISTS [");
  F_Declaration::prefix_print(output_file, debug);
  for (Variable_ID_Iterator VI(myLocals); VI; VI++)
     assert((*VI)->kind() == Exists_Var);
}

void F_Declaration::prefix_print(FILE *output_file, int debug) {
  String s = "";
  for (Variable_ID_Iterator VI(myLocals); VI; ) {
    s += (*VI)->name();
    print_var_addrs(s, *VI);
    VI++;
    if(VI) s += ",";
  }
  s += "]\n";
  fprintf(output_file, (const char *) s);
  Formula::prefix_print(output_file, debug);
}

void F_Or::prefix_print(FILE *output_file, int debug) {
  Formula::print_head(output_file);
  fprintf(output_file, "OR\n");
  Formula::prefix_print(output_file, debug);
}

void F_And::prefix_print(FILE *output_file, int debug) {
  Formula::print_head(output_file);
  fprintf(output_file, "AND\n");
  Formula::prefix_print(output_file, debug);
}

void F_Not::prefix_print(FILE *output_file, int debug) {
  Formula::print_head(output_file);
  fprintf(output_file, "NOT\n");
  Formula::prefix_print(output_file, debug);
}

void Conjunct::prefix_print(FILE *output_file, int debug) {
  static char dir_glyphs[] = { '-', '?', '+' };

  if (debug)
      {
      Formula::print_head(output_file);
      if(pres_debug>=2) fprintf(output_file, "(@%p)", this);
      fprintf(output_file, "%s CONJUNCT, ",
	      exact ? "EXACT" : "INEXACT");
      if (simplified) fprintf(output_file, "simplified, ");
      if (verified) fprintf(output_file, "verified, ");
      if (possible_leading_0s != -1 && guaranteed_leading_0s != -1)
	    assert (guaranteed_leading_0s <= possible_leading_0s);
      if (guaranteed_leading_0s != -1 
	    && guaranteed_leading_0s == possible_leading_0s)
	    fprintf(output_file,"# leading 0's = %d,",  possible_leading_0s);
      else if (possible_leading_0s != -1 || guaranteed_leading_0s != -1) {
	      if (guaranteed_leading_0s != -1)
		    fprintf(output_file,"%d <= ",guaranteed_leading_0s);
	      fprintf(output_file,"#O's");
	      if (possible_leading_0s != -1)
		    fprintf(output_file," <= %d",possible_leading_0s);
	      fprintf(output_file,", ");
	    }
      if (dir_glyphs[leading_dir+1] != '?')
	    fprintf(output_file," first = %c, ", dir_glyphs[leading_dir+1]);
      fprintf(output_file,"myLocals=[");
      String s="";
      for (Variable_ID_Iterator VI(myLocals); VI; ) {
        assert( (*VI)->kind() == Wildcard_Var);
        s += (*VI)->name();
        print_var_addrs(s, *VI);
        VI++;
        if(VI) s +=  ",";
      }
      s += "] mappedVars=[";
      for(Variable_ID_Iterator MVI(mappedVars); MVI; ) {
        s += (*MVI)->name();
        print_var_addrs(s, *MVI);
        MVI++;
        if(MVI) s += ",";
      }
      fprintf(output_file, "%s]\n", (const char *) s);
      }
  else
      level++;

  setOutputFile(output_file);
  setPrintLevel(level+1);
  problem->printProblem(debug);
  setPrintLevel(0);
  Formula::prefix_print(output_file, debug);
}

void Formula::print_head(FILE *output_file) {
  level++;
  int i;
  for(i=0; i<level; i++) {
    fprintf(output_file, ". ");
  }
}

//
// Print DNF.
//
void DNF::print(FILE *out_file) {
  DNF_Iterator p(this);
  if (!p.live())
    fprintf(out_file, "FALSE");
  else
    for(; p.live(); ) {
      p.curr()->print(out_file);
      p.next();
      if(p.live())
        fprintf(out_file, " OR ");
    }
}

void DNF::prefix_print(FILE *out_file, int debug, bool parent_names_setup) {

    wildCardInstanceNumber = 0;
    Variable_ID_Tuple all_vars;
    if(!use_ugly_names && !parent_names_setup) {
	// We need to manually set up all of the input,output, and symbolic 
        // variables, since during simplification, a dnf's conjuncts may not 
        // be listed as part of a relation, or perhaps as part of different
        // relations (?) (grr).
	for(DNF_Iterator p0(this); p0.live(); p0.next()) {
	    for(Variable_Iterator vi((*p0)->mappedVars); vi; vi++)
		if((*vi)->kind() != Wildcard_Var && all_vars.index(*vi) == 0)
		    all_vars.append(*vi);
	    (*p0)->setup_names();
	}
	foreach(v,Variable_ID,all_vars,
           if (!v->base_name.null()) v->instance = v->base_name++;
    		else  v->instance = wildCardInstanceNumber++;
		);
	}

    int i = 1;
    level = 0;
    for(DNF_Iterator p(this); p.live(); p.next(), i++) {
	fprintf(out_file, "#%d ", i);
	if(*p == NULL)
	    fprintf(out_file, "(NULL)\n");
	else 
	    (*p)->prefix_print(out_file, debug);
    }
    
    foreach(v,Variable_ID,all_vars,if (!v->base_name.null()) v->base_name--);

    fprintf(out_file, "\n");
}

String Constraint_Handle::print_to_string() const {
    assert(0);
    Exit(1);
    return "FOO";
}

String EQ_Handle::print_to_string() const {
    relation()->setup_names();
    String s =  c->print_EQ_to_string(e);
    return s;
}

String GEQ_Handle::print_to_string() const {
    relation()->setup_names();
    String s =  c->print_GEQ_to_string(e);
    return s;
}

String Constraint_Handle::print_term_to_string() const {
    assert(0);
    Exit(1);
    return "FOO";
}

String EQ_Handle::print_term_to_string() const {
    relation()->setup_names();
    String s =  c->print_EQ_term_to_string(e);
    return s;
}

String GEQ_Handle::print_term_to_string() const {
    relation()->setup_names();
    String s =  c->print_GEQ_term_to_string(e);
    return s;
}

} // end of namespace omeganamespace omega {

