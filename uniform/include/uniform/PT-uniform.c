/* $Id: PT-uniform.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/Dynamic_Array.h>
#include <basic/Tuple.h>
#include <basic/Dynamic_Array.h>
#include <uniform/parallelism.h>
#include <uniform/depend_dir.h>
#include <uniform/uniform.h>
#include <uniform/search.h>
#include <uniform/search.h>

template float  min(float,float);
template double max(double,double);
template search_cost min(search_cost,search_cost);

instantiate_Tuple(sinfo);
instantiate_Tuple(depend_info);
instantiate_Tuple(Global_Var_Decl *);
instantiate_Tuple(Variable *);

instantiate_Dynamic_Array2(extended_dir_list *);
instantiate_Dynamic_Array2(dir_list *);
instantiate_Dynamic_Array4(search_cost);
instantiate_Dynamic_Array4(int);
instantiate_Dynamic_Array2(IntTuple);
instantiate_Dynamic_Array2(Relation);
instantiate_Dynamic_Array2(Coef_Var_Decl *);
instantiate_Dynamic_Array2(String);
