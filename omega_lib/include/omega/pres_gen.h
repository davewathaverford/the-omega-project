#if ! defined _pres_gen_h
#define _pres_gen_h 1

#include <basic/bool.h>


/* $Id: pres_gen.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */


//
// general presburger stuff thats needed everywhere
//

/* The following allows us to avoid warnings about passing 
   temporaries as non-const references.  This is useful but 
   has suddenly become illegal.  */

#if !defined(LIE_ABOUT_CONST_TO_MAKE_ANSI_COMMITTEE_HAPPY)
#if defined(__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7))
#define LIE_ABOUT_CONST_TO_MAKE_ANSI_COMMITTEE_HAPPY 1
#else
#define LIE_ABOUT_CONST_TO_MAKE_ANSI_COMMITTEE_HAPPY 0
#endif
#endif

#if LIE_ABOUT_CONST_TO_MAKE_ANSI_COMMITTEE_HAPPY
#define NOT_CONST const
#else
#define NOT_CONST 
#endif

//#include <omega/enter_omega.h>
#include <basic/assert.h>
#include <stdlib.h>
#include <omega/omega_core/oc.h>
#include <basic/String.h>
#include <basic/ConstString.h>
#include <basic/Iterator.h>
#include <basic/List.h>
#include <basic/Tuple.h>

namespace omega {

//
// I/O and error processing and control flags (also in omega_core/debugging.h)
//
extern int   pres_debug;
extern FILE *DebugFile;

extern int mega_total;
extern int use_ugly_names;

extern negation_control pres_legal_negations;


//
// Lots of things refer to each other,
//  so we forward declare these classes:
//

class Var_Decl;
typedef enum {Input_Var, Output_Var, Set_Var,
              Global_Var, Forall_Var, Exists_Var, Wildcard_Var} Var_Kind;
class Global_Var_Decl;
typedef enum {Unknown_Tuple = 0, Input_Tuple = 1, Output_Tuple = 2,
				 Set_Tuple = Input_Tuple } Argument_Tuple;

class Constraint_Handle;
class EQ_Handle;
class GEQ_Handle;
typedef EQ_Handle Stride_Handle;

class Formula;
class F_Declaration;
class F_Forall;
class F_Exists;
class F_And;
class F_Or;
class F_Not;
class Conjunct;
class Relation;
class Rel_Body;
class DNF;
class Mapping;
class Omega_Var;
class Coef_Var_Decl;

typedef Var_Decl *Variable_ID;
typedef Global_Var_Decl *Global_Var_ID;

typedef Tuple<Variable_ID>       Variable_ID_Tuple;
typedef Sequence<Variable_ID>    Variable_ID_Sequence;  // use only for rvalues
typedef Tuple_Iterator<Variable_ID>   Variable_ID_Tuple_Iterator;
typedef Tuple_Iterator<Variable_ID>   Variable_ID_Iterator;

typedef Variable_ID_Iterator Variable_Iterator;

typedef enum {Comb_Id, Comb_And, Comb_Or, Comb_AndNot} Combine_Type;


// things that are (hopefully) used only privately
class Comp_Problem;
class Comp_Constraints;

// this has to be here rather than in pres_conj.h because
// MergeConj has to be a friend of Constraint_Handle
typedef enum {MERGE_REGULAR, MERGE_COMPOSE, MERGE_GIST} Merge_Action;


// Conjunct can be exact or lower or upper bound.
// For lower bound conjunct, the upper bound is assumed to be true;
// For upper bound conjunct, the lower bound is assumed to be false

typedef enum {EXACT_BOUND, UPPER_BOUND, LOWER_BOUND, UNSET_BOUND} Bound_Type;


#if defined STUDY_EVACUATIONS
typedef enum { in_to_out = 0, out_to_in = 1} which_way;

enum evac { evac_trivial = 0,
	    evac_offset = 1,
	    evac_subseq = 2,
	    evac_offset_subseq = 3,
//	    evac_permutation = ,
	    evac_affine = 4,
	    evac_nasty = 5 };

extern char *evac_names[];

#endif

// the following list should be updated in sync with Relations.h

#define  friend_rel_ops \
friend Relation  Union(NOT_CONST Relation &r1, NOT_CONST Relation &r2);		\
friend Relation  Intersection(NOT_CONST Relation &r1, NOT_CONST Relation &r2);	\
friend Relation  After(NOT_CONST Relation &R, int carried_by, int new_output, int dir);\
friend Relation  Extend_Domain(NOT_CONST Relation &R);			\
friend Relation  Extend_Domain(NOT_CONST Relation &R, int more);		\
friend Relation  Extend_Range(NOT_CONST Relation &R);			\
friend Relation  Extend_Range(NOT_CONST Relation &R, int more);		\
friend Relation  Extend_Set(NOT_CONST Relation &R);			\
friend Relation  Extend_Set(NOT_CONST Relation &R, int more);		\
friend Relation  Restrict_Domain(NOT_CONST Relation &r1, NOT_CONST Relation &r2);	\
friend Relation  Restrict_Range(NOT_CONST Relation &r1, NOT_CONST Relation &r2);	\
friend Relation  Domain(NOT_CONST Relation &r);				\
friend Relation  Range(NOT_CONST Relation &r);				\
friend Relation  Cross_Product(NOT_CONST Relation &A, NOT_CONST Relation &B);	\
friend Relation  Inverse(NOT_CONST Relation &r);				\
friend Relation  Deltas(NOT_CONST Relation &R);				\
friend Relation  Deltas(NOT_CONST Relation &R, int eq_no);		\
friend Relation  DeltasToRelation(NOT_CONST Relation &R, int n_input, int n_output); \
friend Relation  Complement(NOT_CONST Relation &r);			\
friend Relation  Project(NOT_CONST Relation &R, Global_Var_ID v);		\
friend Relation  Project(NOT_CONST Relation &r, int pos, Var_Kind vkind);	\
friend Relation  Project(NOT_CONST Relation &S, Sequence<Variable_ID> &s); \
friend Relation  Project_Sym(NOT_CONST Relation &R);			\
friend Relation  Project_On_Sym(NOT_CONST Relation &R, NOT_CONST Relation &context); \
friend Relation  GistSingleConjunct(NOT_CONST Relation &R1, NOT_CONST Relation &R2, int effort); \
friend Relation  Gist(NOT_CONST Relation &R1, NOT_CONST Relation &R2, int effort);	\
friend Relation  Difference(NOT_CONST Relation &r1, NOT_CONST Relation &r2);	\
friend Relation  Approximate(NOT_CONST Relation &R);			\
friend Relation  Approximate(NOT_CONST Relation &R, int strides_allowed);	\
friend Relation  Identity(int n_inp);				\
friend Relation  Identity(NOT_CONST Relation &r);				\
friend bool do_subset_check(NOT_CONST Relation &r1, NOT_CONST Relation &r2); \
friend bool Must_Be_Subset(NOT_CONST Relation &r1, NOT_CONST Relation &r2); \
friend bool Might_Be_Subset(NOT_CONST Relation &r1, NOT_CONST Relation &r2); \
friend bool May_Be_Subset(NOT_CONST Relation &r1, NOT_CONST Relation &r2); \
friend bool      Is_Obvious_Subset(NOT_CONST Relation &r1, NOT_CONST Relation &r2);	\
friend Relation  Join(NOT_CONST Relation &G, NOT_CONST Relation &F);		\
friend Relation  Composition(NOT_CONST Relation &F, NOT_CONST Relation &G);		\
friend bool      can_do_exact_composition(NOT_CONST Relation &F, NOT_CONST Relation &G); \
friend Relation  EQs_to_GEQs(NOT_CONST Relation &, bool excludeStrides);	\
friend Relation  Symbolic_Solution(NOT_CONST Relation &S);                \
friend Relation  Symbolic_Solution(NOT_CONST Relation &S, Sequence<Variable_ID> &T); \
friend Relation  Sample_Solution(NOT_CONST Relation &S);                  \
friend Relation  Solution(NOT_CONST Relation &S, Sequence<Variable_ID> &T); \
friend void MapRel1(Relation &inputRel, const Mapping &map,	\
		    Combine_Type ctype, int number_input, 	\
		    int number_output, bool, bool);		\
friend Relation MapAndCombineRel2(Relation &R1, Relation &R2, 	\
				  const Mapping &mapping1,	\
				  const Mapping &mapping2,	\
				  Combine_Type ctype,		\
				  int number_input,		\
				  int number_output);		\
friend void align(Rel_Body *, Rel_Body *, F_Exists *,		\
		  Formula *, const Mapping &, bool &,		\
		  List<int> &, Variable_ID_Tuple &);            \
friend Relation  Lower_Bound(NOT_CONST Relation &r);	\
friend Relation  Upper_Bound(NOT_CONST Relation &r)

// REMEMBER - THE LAST LINE OF THE MACRO SHOULD NOT HAVE A ;
/* TransitiveClosure doesn't need to be in friend_rel_ops */


} // end of namespace omega

#endif
