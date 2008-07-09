#if ! defined _closure_h
#define _closure_h

#include <omega/Relation.h>

namespace omega {


void InvestigateClosure(Relation r, Relation r_closure, Relation bounds);
void print_given_bounds(const Relation & R1, NOT_CONST Relation& input_Bounds);

#define printConjunctClosure   (closure_presburger_debug & 0x1) 
#define detailedClosureDebug   (closure_presburger_debug & 0x2)



} // end of namespace omega
 
#endif
