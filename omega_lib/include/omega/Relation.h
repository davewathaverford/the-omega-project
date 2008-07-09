#if ! defined _Relation_h
#define _Relation_h 1

/* $Id: Relation.h,v 1.2 2005/02/09 22:23:20 mstrout Exp $ */

#include <basic/bool.h>


#if ! defined _RelBody_h
#include <omega/RelBody.h>
#endif
#if ! defined _pres_cnstr_h
#include <omega/pres_cnstr.h>
#endif

//#include <iostream.h>
#include <iostream>
#include <basic/NonCoercible.h>

namespace omega {

//
// Relation representative.
// Body and representative are separated to do reference counting.
//
class Relation {
public:
    Relation();

    Relation(int n_input, int n_output);
    // Creates a set, as in "Relation(3)", but you can't convert int->Relation
#if ! defined NONCOERCIBLE_WONT_WORK_AT_ALL
    Relation(NonCoercible<int> nci);
#else
    Relation(NonCoercibleInt nci);
#endif
    Relation(const Relation &r);
    Relation(const Relation &r, Conjunct *c);
    Relation &operator=(const Relation &r);
    Relation(Rel_Body &r, int foo);

    static Relation Null();
    static Relation Empty(const Relation &R);
    static Relation True(const Relation &R);
    static Relation True(int setvars);
    static Relation True(int in, int out);
    static Relation False(const Relation &R);
    static Relation False(int setvars);
    static Relation False(int in, int out);
    static Relation Unknown(const Relation &R);
    static Relation Unknown(int setvars);
    static Relation Unknown(int in, int out);


    bool is_null() const;

    ~Relation();

    inline F_Forall *add_forall()
      { return rel_body->add_forall(); }
    inline F_Exists *add_exists()
      { return rel_body->add_exists(); }
    inline F_And    *add_and()
      { return rel_body->add_and(); }
    inline F_And    *and_with()
      { return rel_body->and_with(); }
    inline F_Or     *add_or()
      { return rel_body->add_or(); }
    inline F_Not    *add_not()
      { return rel_body->add_not(); }
    inline void finalize()
      { rel_body->finalize(); }
    inline bool is_finalized() const
      { return rel_body->finalized; }
    inline bool is_set() const
      { return rel_body->is_set();   }  
    inline int n_inp() const
      { return rel_body->n_inp(); }
    inline int n_out() const
      { return rel_body->n_out(); }
    inline int n_set() const
      { return rel_body->n_set(); }

    inline const Variable_ID_Tuple *global_decls() const
      { return rel_body->global_decls(); }
    inline int max_ufs_arity() const
      { return rel_body->max_ufs_arity(); }
    inline int max_ufs_arity_of_in() const
      { return rel_body->max_ufs_arity_of_in(); }
    inline int max_ufs_arity_of_set() const
      { return rel_body->max_ufs_arity_of_set(); }
    inline int max_ufs_arity_of_out() const
      { return rel_body->max_ufs_arity_of_out(); }
    inline int max_shared_ufs_arity() const
      { return rel_body->max_shared_ufs_arity(); }

    inline Variable_ID input_var(int nth)
      { return rel_body->input_var(nth); }
    inline Variable_ID output_var(int nth)
      { return rel_body->output_var(nth); }
    inline Variable_ID set_var(int nth)
      { return rel_body->set_var(nth); }
    inline bool has_local(const Global_Var_ID G)
      { return  rel_body->has_local(G); } 
    inline bool has_local(const Global_Var_ID G, Argument_Tuple of)
      { return  rel_body->has_local(G, of); } 
    inline Variable_ID get_local(const Variable_ID v)
      { return split()->get_local(v); }
    inline Variable_ID get_local(const Global_Var_ID G)
      { return split()->get_local(G); }
    inline Variable_ID get_local(const Global_Var_ID G, Argument_Tuple of)
      { return split()->get_local(G, of); }

    inline void        name_input_var(int nth, Const_String S)
      { split()->name_input_var(nth, S); }
    inline void        name_output_var(int nth, Const_String S)
      { split()->name_output_var(nth, S); }
    inline void        name_set_var(int nth, Const_String S)
      { split()->name_set_var(nth, S); }


    inline F_And      *and_with_and()
      { return split()->and_with_and(); }
    inline EQ_Handle   and_with_EQ()
      { return split()->and_with_EQ(); }
     inline EQ_Handle   and_with_EQ(const Constraint_Handle &c)
      { return split()->and_with_EQ(c); }
    inline GEQ_Handle  and_with_GEQ()
      { return split()->and_with_GEQ(); }
    inline GEQ_Handle  and_with_GEQ(const Constraint_Handle &c)
      { return split()->and_with_GEQ(c); }

    inline void print()
      { rel_body->print(); }
    inline void print(FILE *output_file)
      { rel_body->print(output_file); }
    inline void print_with_subs()
      { rel_body->print_with_subs(); }
    inline void print_with_subs(FILE *output_file, bool printSym=false, 
				bool newline=true)
      { rel_body->print_with_subs(output_file, printSym, newline); }
    inline String print_with_subs_to_string(bool printSym=false, 
					    bool newline=true)
      { return rel_body->print_with_subs_to_string(printSym, newline); }
    inline String print_outputs_with_subs_to_string()
      { return rel_body->print_outputs_with_subs_to_string(); }
    inline String print_outputs_with_subs_to_string(int i)
      { return rel_body->print_outputs_with_subs_to_string(i); }
    inline void prefix_print()
      { rel_body->prefix_print(); }
    inline void prefix_print(FILE *output_file, int debug = 1)
      { rel_body->prefix_print(output_file, debug); }
    inline String print_formula_to_string() {
	return rel_body->print_formula_to_string();
	}
    void dimensions(int & ndim_all, int &ndim_domain);

    inline bool is_lower_bound_satisfiable()
      { return rel_body->is_lower_bound_satisfiable(); }
    inline bool is_upper_bound_satisfiable()
      { return rel_body->is_upper_bound_satisfiable(); }
    inline bool is_satisfiable()
      { return rel_body->is_satisfiable(); }

    inline bool is_tautology()
      { return rel_body->is_definite_tautology(); }  // for compatibility
    inline bool is_obvious_tautology()
      { return rel_body->is_obvious_tautology(); }

    // return x s.t. forall conjuncts c, c has >= x leading 0s
    // for set, return -1 (pass this in, in case there are no conjuncts
    inline int    number_of_conjuncts()
      { return rel_body->query_DNF()->length(); }

    // return x s.t. forall conjuncts c, c has >= x leading 0s
    // for set, return -1 (pass this in, in case there are no conjuncts
    inline int    query_guaranteed_leading_0s()
      { return rel_body->query_DNF()->query_guaranteed_leading_0s(this->is_set() ? -1 : 0); }

    // return x s.t. forall conjuncts c, c has <= x leading 0s
    // if no conjuncts return min of input and output tuple sizes, or -1 if relation is a set
    inline int    query_possible_leading_0s()
      { return rel_body->query_DNF()->query_possible_leading_0s(
           this->is_set()? -1 : min(n_inp(),n_out())); }

    // return +-1 according to sign of leading dir, or 0 if we don't know
    inline int    query_leading_dir()
      { return rel_body->query_DNF()->query_leading_dir(); }

    inline DNF*    query_DNF()
      { return rel_body->query_DNF(); }
    inline DNF*    query_DNF(int rdt_conjs, int rdt_constrs)
      { return rel_body->query_DNF(rdt_conjs, rdt_constrs); }
    inline void    simplify()
      { rel_body->simplify(); }
    inline void    simplify(int rdt_conjs, int rdt_constrs)
      { rel_body->simplify(rdt_conjs, rdt_constrs); }
    inline bool is_simplified()
      { return rel_body->is_simplified(); }
    inline bool is_compressed() const
      { return rel_body->is_compressed(); }
    inline Conjunct *rm_first_conjunct()
      { return rel_body->rm_first_conjunct(); }
    inline Conjunct *single_conjunct()
      { return rel_body->single_conjunct(); }
    inline bool has_single_conjunct()
      { return rel_body->has_single_conjunct(); }

    inline void query_difference(Variable_ID v1, Variable_ID v2,
				 coef_t &lowerBound, coef_t &upperBound,
				 bool &guaranteed)
    {
    rel_body->query_difference(v1, v2, lowerBound, upperBound, guaranteed);
    }

    inline void query_variable_bounds (Variable_ID v,
				       coef_t &lowerBound, coef_t &upperBound)
    {
    rel_body->query_variable_bounds(v,lowerBound,upperBound);
    }



    inline void make_level_carried_to(int level)
    {
    split()->make_level_carried_to(level);
    }

    inline Relation extract_dnf_by_carried_level(int level, int direction)
    {
    return split()->extract_dnf_by_carried_level(level, direction);
    }

    inline void compress()
      {
#if defined(INCLUDE_COMPRESSION)
          split()->compress(); 
#endif
      }
    void uncompress()
      { rel_body->uncompress(); }

    inline bool is_exact() const
      { return !(rel_body->unknown_uses() & (and_u | or_u)) ; }
    inline bool is_inexact() const
      { return !is_exact(); }
    inline bool is_unknown() const
      { return rel_body->is_unknown(); }
    inline Rel_Unknown_Uses unknown_uses() const
      { return rel_body->unknown_uses(); }

    void setup_names() {rel_body->setup_names();}
    void copy_names(Relation &r) {
	copy_names(*r.rel_body);
	};
    void copy_names(Rel_Body &r);

private:
    // Functions that have to create sets from relations:
    friend class Rel_Body;
    friend_rel_ops;

    Rel_Body *split();

    DNF* simplified_DNF() {
		simplify();
		return rel_body->simplified_DNF;
		};

    inline void invalidate_leading_info(int changed = -1)
    { split()->invalidate_leading_info(changed); }
    inline void enforce_leading_info(int guaranteed, int possible, int dir)
        {
        split()->enforce_leading_info(guaranteed, possible, dir);
        }


    void    makeSet();
    void    markAsSet();
    void    markAsRelation();

    friend int operator==(const Relation &, const Relation &);

    void reverse_leading_dir_info()
    { split()->reverse_leading_dir_info(); }
    void interpret_unknown_as_true()
      { split()->interpret_unknown_as_true(); }
    void interpret_unknown_as_false()
      { split()->interpret_unknown_as_false(); }


    Rel_Body *rel_body;
};

inline Relation Null_Relation() { return Relation::Null(); }

inline ostream & operator<<(ostream &o, Relation &R)
    {
    return o << R.print_with_subs_to_string();
    }

} // end of namespace omega



#if ! defined _Relations_h
#include <omega/Relations.h>
#endif


#endif
