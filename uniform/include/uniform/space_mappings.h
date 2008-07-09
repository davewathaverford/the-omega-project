/* $Id: */
#ifndef Already_Included_Space_Mappings
#define Already_Included_Space_Mappings

#include <basic/Dynamic_Array.h>
#include <uniform/uniform_misc.h>

namespace omega {

extern int Use_Cyclic;
extern int Use_Cyclic;
extern int best_space[max_stmts];
extern Dynamic_Array2<extended_dir_list *> extended_original_dir;
extern Dynamic_Array1<Relation> space;
extern Dynamic_Array1<Relation> pos_space;
extern Dynamic_Array1<int> neg_space;

extern void minimize_communication();

}

#endif
