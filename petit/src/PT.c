/* $Id: PT.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#if defined DONT_INCLUDE_TEMPLATE_CODE

/* DONT BOTHER COMPILING THIS FILE UNLESS WE ARE USING G++ >= 260
   AND  -fno-implicit-templates AND -DDONT_INCLUDE_TEMPLATE_CODE  */

#undef DONT_INCLUDE_TEMPLATE_CODE

#include <omega/PT-omega.c>
#include <code_gen/PT-code_gen.c>
#if !defined(EXCLUDE_UNIFORM)
#include <uniform/PT-uniform.c>
#endif
#include <petit/tree.h>
#include <petit/lang-interf.h>
#include <petit/Zima.h>
#include <petit/hpp.h>

namespace omega {

template long min(long,long);
template unsigned short min(unsigned short, unsigned short);
template int max(int, int);

instantiate_List(Var_Id);
instantiate_List(Omega_Var *);
instantiate_List(node *);
instantiate_List(context_descr_for_assert *);
instantiate_List(dist_info);

instantiate_Set(Omega_Var *);
instantiate_Set(node *);

typedef Tuple<Free_Var_Decl *> FreeVarDeclPointerTuple;
instantiate_Tuple(FreeVarDeclPointerTuple);

instantiate_Map(String,Relation *);

typedef List<int> IntList;
instantiate_Map(String,IntList*);

class template_info;
instantiate_List(template_info *);
instantiate_Map(String,String);
instantiate_Map(node *,String);
}

#endif
