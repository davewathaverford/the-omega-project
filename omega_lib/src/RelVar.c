#include <basic/bool.h>
#include <omega/RelBody.h>
#include <omega/omega_i.h>
#include <stdlib.h>

namespace omega {

Variable_ID Rel_Body::get_local(const Variable_ID v)
    {
    Global_Var_ID g;
    if (v->kind() == Global_Var) {
	     	g = v->get_global_var();
	     	if (g->arity()) return get_local(g,v->function_of());
		return get_local(g);
		}
    if (is_set()) return set_var(v->get_position());
    if (v->kind() == Input_Var) return input_var(v->get_position());
    if (v->kind() == Output_Var) return output_var(v->get_position());
    assert(0 && "Can only get local for variable with global scope");
    exit(1);
    return 0;
    }

//
// Find or declare global variable.
// If the VarID does not exist, it is created. Otherwise it's returned.
// Note that this version now works only for 0-ary functions.
//
Variable_ID Rel_Body::get_local(const Global_Var_ID G)
    {
    assert(G->arity() == 0);
    for(Variable_Iterator i(Symbolic); i; i++)
	if ((*i)->get_global_var() == G)
	    return (*i);

    Variable_ID v = G->get_local();
    Symbolic.append(v);
    return v;
    }

Variable_ID Rel_Body::get_local(const Global_Var_ID G, Argument_Tuple of)
    {
    assert(G->arity() == 0 || of == Input_Tuple || of == Output_Tuple);

    for(Variable_Iterator i = Symbolic; i; i++)
	if ((*i)->get_global_var() == G && (G->arity() == 0 ||
					    of == (*i)->function_of()))
	    return (*i);

    Variable_ID V = G->get_local(of);
    Symbolic.append(V);
    return V;
    }

bool Rel_Body::has_local(const Global_Var_ID G)
    {
    assert(G->arity() == 0);
    for(Variable_Iterator i = Symbolic; i; i++)
	if ((*i)->get_global_var() == G)
	    return true;
    return false;
    }

bool Rel_Body::has_local(const Global_Var_ID G, Argument_Tuple of)
    {
    assert(G->arity() == 0 || of == Input_Tuple || of == Output_Tuple);

    for(Variable_Iterator i = Symbolic; i; i++)
	if ((*i)->get_global_var() == G && (G->arity() == 0 ||
					    of == (*i)->function_of()))
	    return true;
    return false;
    }


} // end of namespace omeganamespace omega {
