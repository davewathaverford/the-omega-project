#if !defined(Already_Included_stmt_builder)
#define Already_Included_stmt_builder

#include <omega/pres_gen.h>
#include <code_gen/CG_outputBuilder.h>
#include <code_gen/CG_outputRepr.h>

namespace omega {

CG_outputRepr* tryToPrintVarToReprWithDiv(CG_outputBuilder* ocg,
					       Conjunct *C, Variable_ID v);
CG_outputRepr* print_outputs_with_subs_to_repr(Relation &R, 
					       CG_outputBuilder* ocg);
CG_outputRepr* print_sub_to_repr(CG_outputBuilder* ocg, const Sub_Handle &s);
CG_outputRepr* print_term_to_repr(CG_outputBuilder* ocg, const Sub_Handle &s, int c);

}

#endif
