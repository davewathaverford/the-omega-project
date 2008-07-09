#include <code_gen/MMap.h>
#include <stdio.h>
#include <code_gen/code_gen.h>
#include <code_gen/CG_stringBuilder.h>
#include <code_gen/CG_stringRepr.h>

#ifndef MMAPCODEGEN_H 
#define MMAPCODEGEN_H 

namespace omega {

// the main "tcodegen" function
// (stands for either "Tina's codegen" or "time-skewed codegen")
String tcodegen(int effort, Tuple<stm_info> &info, const Relation &known, bool check_input = true);

}

#endif
