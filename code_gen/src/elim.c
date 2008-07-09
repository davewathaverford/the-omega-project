
#include <omega.h>
#include <code_gen/elim.h>

namespace omega {

int max_fs_arity(const Constraint_Handle &c) {
  int max_arity=0;
  for(Constr_Vars_Iter cv(c); cv; cv++)
    if((*cv).var->kind() == Global_Var)
      max_arity = omega::max(max_arity,(*cv).var->get_global_var()->arity());
  return max_arity;
}

elimination_info::elimination_info() : eliminated(0),never_eliminate_lt(100000)
{}


elimination_info::elimination_info(const Tuple<Relation> &sets) {
  int max_arity=0;
  for(int i = 1; i <= sets.size(); i++) {
    Relation R = sets[i];
    assert(R.is_set());
    for(DNF_Iterator D(R.query_DNF()); D; D++)
      for(Constraint_Iterator c(*D); c; c++)
	max_arity = omega::max(max_arity,max_fs_arity(*c));
  }
  never_eliminate_lt = max_arity;
}

bool elimination_info::may_eliminate(Variable_ID v) { 
  if(v->kind() == Input_Var || v->kind() == Output_Var)
    return v->get_position() > never_eliminate_lt;
  else
    return true;
}

Relation elimination_info::project_eliminated(NOT_CONST Relation &input_R) {
  
  Relation R = consume_and_regurgitate(input_R);

  if(!any_eliminated()) {
    if (code_gen_debug > 2)
      fprintf(DebugFile,":::projectEliminatedVariables: no vars eliminated\n");
    return R;
  } 
  
  
  R = Project(R,eliminated);
  R.finalize();
  R.simplify();

  if(code_gen_debug > 2) {
    fprintf(DebugFile, "::: projectEliminatedVariables: "
	    "variables eliminated: ");
    print_eliminated(DebugFile);
    R.prefix_print(DebugFile);
  }
  return R;
}

void elimination_info::print_eliminated(FILE *out) {
  for(int i = 1; i <= eliminated.size(); i++)
    fprintf(out, "%s (%s %d)  ", eliminated[i]->char_name(),
	    (eliminated[i]->kind() == Input_Var? "input var" : "output var"),
	    eliminated[i]->get_position());
  fprintf(out, "\n");
}

String elimination_info::print_eliminated_to_string() {
  String res;
  for(int i = 1; i <= eliminated.size(); i++)
    res += eliminated[i]->name() 
        + (eliminated[i]->kind() == Input_Var? "(input " : "(output ")
        + itoS(eliminated[i]->get_position())
        + ") ";
  return res;
}

} // end namespace omega
