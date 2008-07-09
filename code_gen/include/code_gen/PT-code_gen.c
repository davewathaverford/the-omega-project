/* $Id: PT-code_gen.c,v 1.1.1.1 2004/09/13 21:07:47 mstrout Exp $ */

// For basic code gen:

#include <code_gen/spmd.h>
#include <code_gen/code_gen.h>

instantiate_Tuple(naming_info *);
instantiate_Tuple(spmd_stmt_info *);


// For "tcodegen" extensions:

#include <code_gen/MMap.h>

instantiate_Tuple(PartialMMap);
instantiate_Tuple(MMap);
instantiate_Tuple(PartialRead);
instantiate_Tuple(Read);
instantiate_Tuple(stm_info);
