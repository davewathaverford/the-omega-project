#if ! defined _MMAP_CHECKS_H
#define _MMAP_CHECKS_H 1

#include <omega.h>
#include <code_gen/MMap.h>

namespace omega {

bool checkDataFlow(Tuple<stm_info> &info);
bool checkIS(Tuple<stm_info> info);
bool checkPartials(Tuple<stm_info> info);
bool checkMemoryMapping(Tuple<stm_info> new_info);
bool checkBounds(Tuple<stm_info> noconst);
bool check_arities(Tuple<Relation> &sets, int arity);
bool check_arities(Tuple<stm_info> &info);

}

#endif
