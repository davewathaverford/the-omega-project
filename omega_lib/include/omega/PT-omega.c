#undef DONT_INCLUDE_TEMPLATE_CODE

#include <basic/bool.h>
#include <basic/util.h>
#include <basic/List.h>
#include <basic/SimpleList.h>
#include <basic/Bag.h>
#include <basic/Map.h>
#include <basic/Tuple.h>
#include <basic/Section.h>
#include <basic/Exit.h>
#include <basic/Dynamic_Array.h>
#include <omega.h>
#include <omega/AST.h>


namespace omega {

template int max(int, int);
template int min(int, int);
template unsigned int min(unsigned int, unsigned int);
template void set_max(int&,int);
template void set_min(int&,int);
template void swap(int&,int&);
template void swap(short&,short&);
template void swap(signed char&,signed char&);
template Relation copy(const Relation &);

instantiate_NonCoercible(int);

instantiate_Set(int);
instantiate_Set(Global_Var_ID);
instantiate_Set(Variable_ID);

instantiate_List(int);
instantiate_List(exit_func);
instantiate_List(Formula *);
instantiate_List(Conjunct *);
instantiate_List(DNF *);
instantiate_List(Relation *);
instantiate_Simple_List(Relation);

typedef Tuple<Relation> RelationTuple;
instantiate_Tuple(bool);
instantiate_Tuple(int);
instantiate_Tuple(coef_t);
instantiate_Tuple(char *);
instantiate_Tuple(Const_String);
instantiate_Tuple(Conjunct *);
instantiate_Tuple(Relation);
instantiate_Tuple(RelationTuple);
instantiate_Tuple(Variable_ID);
instantiate_Tuple(Free_Var_Decl *);
instantiate_Tuple(String);
instantiate_Tuple(GEQ_Handle);

instantiate_Section(Variable_ID);

instantiate_Generator(Variable_Info);
instantiate_Generator(GEQ_Handle);
instantiate_Generator(EQ_Handle);
instantiate_Generator(Constraint_Handle);
instantiate_Generator(Sub_Handle);

instantiate_Map(Variable_ID,int);
instantiate_Map(Global_Var_ID, Variable_ID);
instantiate_Map(GEQ_Handle,Variable_ID);
instantiate_Map(EQ_Handle,Variable_ID);
instantiate_Map(Variable_ID,Set<Variable_ID>);
instantiate_Map(Const_String, Relation *);

instantiate_Dynamic_Array1(Coef_Var_Decl *);
instantiate_Dynamic_Array1(Relation);
instantiate_Dynamic_Array2(Relation);


/* Stuff required by calculator: */
instantiate_Bag(Exp *);
instantiate_Bag(strideConstraint *);
instantiate_Bag(Variable_Ref *);
instantiate_Bag(char *);
instantiate_Map(Variable_Ref *, int);
instantiate_Map(Variable_Ref *, Variable_Ref *);
instantiate_Map(Const_String, Variable_Ref *);
instantiate_Set(Free_Var_Decl *);
instantiate_Tuple(Variable_Ref *);

} // end of namespace omega
