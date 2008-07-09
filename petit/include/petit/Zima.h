/* $Id: Zima.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/*
 * Interface that lets the user generate Presburger Formulae that
 *  represent dependences from program source.  Uses lang-interf.
 * Patterned after the notation used in Hans Zima\'s book.
 *
 * Questions: -Where are "variables" declared - are they global, or
 *             at the same level as the associated indices?
 *            -How to implement "variables" including non-affine
 *             expression "variables"
 */


#if ! defined Already_Included_Zima
#define Already_Included_Zima

#include <basic/List.h>  /* should be sets */
#include <basic/Bag.h>
#include <basic/Iterator.h>
#include <petit/pres-interf.h>
#include <petit/lang-interf.h>

namespace omega {



typedef Set<Omega_Var *> Varset;
typedef Any_Iterator<Omega_Var *> VarsetIterator;

typedef struct _context_descr_for_assert {
	context_iterator  ci; //if or loop context surrounding assert
	F_Or  *      N; //corresponding node in the formula
	Variable_ID v; //local var in relation corresponding to the loop index var
	struct _context_descr_for_assert * prev_cd; // descriptor of the prev context
} context_descr_for_assert;


/* class that represents an instance of an array access, e.g. A\[I\] */
class AccessIteration {
public:

    AccessIteration();
    AccessIteration(a_access a, Relation * r, const Variable_ID_Tuple *);
    AccessIteration(a_access a, Relation * r, Argument_Tuple _of);
    int depth();  /* number of surrounding loops */

    a_access access() const { return acc; }

/* Add, below the formula node N, constraints for A[I] << B[J]
    for a dependence carried at the given level.
 */

    friend void precedes(F_And *N, const AccessIteration &AI,
			 const AccessIteration &BJ);


    friend void precedes(F_And *N, const AccessIteration &AI,
                         const AccessIteration &BJ, int carried_at_level);

    friend void    connected_by_diff(Formula *N, const AccessIteration &AI,
				     const AccessIteration &BJ, dd_current dd);
    friend void do_connected_by_diff(Formula *N, const AccessIteration &AI,
				     const AccessIteration &BJ, dd_current dd);


// Add constraints for I in [A] below N.
// If already_done != NULL, stop adding constraints when we get there

    friend void access_in_bounds(F_And *N, const AccessIteration &AI,
				 context_iterator already_done = 0,
				 bool assertions = true);

    friend void context_in_bounds(F_And *N, const AccessIteration &AI,
				 context_iterator c,
				 context_iterator already_done = 0,
				 bool assertions = true);

//
// For A[I] and B[J] to access the same memory location,
// their subscripts must be equal and, if the array is
// private within a loop, we must be in the same iteration
// of that loop.
//
// Add, below the node N,
// constraints for A[I] sub= B[J] and private loop indices are equal
//
    friend void access_same_memory(F_And *N, const AccessIteration &AI,
				   const AccessIteration &BJ);
    
    friend void subscripts_equal_to_tuple (F_And *N, const AccessIteration &AI,
                                const Variable_ID_Tuple *t);

    friend void add_one_local_assertion(pASSERT ASR, const AccessIteration &AI,
                            Formula *N, List<context_descr_for_assert *>L);

// private:

    a_access acc;
    List<Var_Id> index_var_ids;
    Varset variables;

    /* The following must be given a value before constraints are built.
       We could also cache the local id\'s for "variables" here. */

    Relation *rel;
    const Variable_ID_Tuple *indices;  // could cache other locals too
    Argument_Tuple of;  // or 0 if not input or output tuple

    void name();
    void load_all_vars();

    friend void precedes_inf(F_And *N, const AccessIteration &AI,
                             const AccessIteration &BJ);
};


extern int ZimaNonlinearity;

/*
 * Functions for splitting dependence relations into direction vectors
 */

void calculateDDVectors(Relation *dep_rel, a_access access1, a_access access2,
			ddnature oitype, ddnature iotype,
			unsigned int bnest, signed char *loop_dirs, dd_flags flags);


void generateAllDDVectors(a_access access1,a_access access2,
			  ddnature oitype, ddnature iotype,
			  unsigned int bnest, signed char *loop_dirs,dd_flags flags,
                          Relation & dd_rel);

/* assertion functions */
void add_assertions(F_And *bound);

}

#endif

