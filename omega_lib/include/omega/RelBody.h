#if ! defined _RelBody_h
#define _RelBody_h 1

/* $Id: RelBody.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>


#if ! defined _pres_form_h
#include <omega/pres_form.h>
#endif
#if ! defined _pres_dnf_h
#include <omega/pres_dnf.h>
#endif


namespace omega {

typedef enum {under_construction, compressed, uncompressed} Rel_Body_Status;
typedef unsigned char Rel_Unknown_Uses;
const Rel_Unknown_Uses no_u = 1;
const Rel_Unknown_Uses and_u = 2;
const Rel_Unknown_Uses or_u = 4;

//
// Relation body.
// Body and representative are separated to do reference counting.
//

class Rel_Body : public Formula {
public:
    bool is_null() const;

    inline Node_Type node_type()
      { return Op_Relation; }

    inline bool is_set() const      {  return _is_set; }
    int n_inp() const;
    int n_out() const;
    int n_set() const;

    inline Variable_ID_Tuple *global_decls()
      { return  &Symbolic; }
    int max_ufs_arity();
    int max_shared_ufs_arity();
    int max_ufs_arity_of_set();
    int max_ufs_arity_of_in();
    int max_ufs_arity_of_out();

    Variable_ID input_var(int nth);
    Variable_ID output_var(int nth);
    Variable_ID set_var(int nth);
    Variable_ID get_local(const Variable_ID v);
    Variable_ID get_local(const Global_Var_ID G);
    Variable_ID get_local(const Global_Var_ID G, Argument_Tuple of);
    bool        has_local(const Global_Var_ID G);
    bool        has_local(const Global_Var_ID G, Argument_Tuple of);
    void        name_input_var(int, Const_String);
    void        name_output_var(int, Const_String);
    void        name_set_var(int, Const_String);

    F_And      *and_with_and();
    EQ_Handle   and_with_EQ();
    EQ_Handle   and_with_EQ(const Constraint_Handle &c);
    GEQ_Handle  and_with_GEQ();
    GEQ_Handle  and_with_GEQ(const Constraint_Handle &c);

    void   print();
    void   print(FILE *output_file) { print(output_file, true); }
    void   print(FILE *output_file, bool printSym);
    String print_variables_to_string(bool printSym);
    void   print_with_subs(FILE *output_file, bool printSym, bool newline);
    void   print_with_subs();
    String print_with_subs_to_string(bool printSym, bool newline);
    String print_outputs_with_subs_to_string();
    String print_outputs_with_subs_to_string(int i);
    String print_formula_to_string();
    void   prefix_print();
    void   prefix_print(FILE *output_file, int debug = 1);

           bool is_satisfiable();
           bool is_lower_bound_satisfiable();
           bool is_upper_bound_satisfiable();
           bool is_obvious_tautology();
           bool is_definite_tautology();
           bool is_unknown();
           DNF*    query_DNF();
           DNF*    query_DNF(int rdt_conjs, int rdt_constrs);
           void    simplify();		
           void    simplify(int rdt_conjs, int rdt_constrs);		
           void    finalize();
    inline bool is_finalized() { return finalized; }
    inline bool is_shared()    { return ref_count > 1; }

    void query_difference(Variable_ID v1, Variable_ID v2,
			  coef_t &lowerBound, coef_t &upperBound,
			  bool &quaranteed);
    void query_variable_bounds(Variable_ID,
			       coef_t &lowerBound, coef_t &upperBound);


    Relation extract_dnf_by_carried_level(int level, int direction);
    void make_level_carried_to(int level);

    // these are only public to allow the creation of "null_rel"
    Rel_Body();
    ~Rel_Body();
    void setup_names();

private:

    // These are manipulated primarily as parts of Relations
    friend class Relation;
    friend_rel_ops;

    friend void remap_DNF_vars(Rel_Body *new_rel, Rel_Body *old_rel);

    Rel_Unknown_Uses unknown_uses();

    inline bool is_simplified() const
      { return (simplified_DNF!=NULL && get_children().empty()); }
    bool is_compressed();
    Conjunct *rm_first_conjunct();
    Conjunct *single_conjunct();
    bool has_single_conjunct();

    void beautify();
    void rearrange();

    friend class EQ_Handle;  // these set up names for printing
    friend class GEQ_Handle; // and check if simplified
    friend class Constraint_Handle;  // and update coefficients

    void compress();
    void uncompress();

    void interpret_unknown_as_true();
    void interpret_unknown_as_false();
 
    Rel_Body(int n_input, int n_output);
    Rel_Body(Rel_Body *r);
    Rel_Body(Rel_Body *r, Conjunct *c);
    Rel_Body &operator=(Rel_Body &r);

    inline Formula *formula()    { return children().front(); }
    inline Formula *rm_formula() { return children().remove_front(); }
    bool can_add_child();
    
    void reverse_leading_dir_info();
    void invalidate_leading_info(int changed = -1) 
	{ Formula::invalidate_leading_info(changed); }
    void enforce_leading_info(int guaranteed, int possible, int dir)
	{ 
	Formula::enforce_leading_info(guaranteed, possible, dir); 
	}
    // re-declare this so that Relation (a friend) can call it

    DNF* DNFize();
    void DNF_to_formula();
    
    Conjunct *find_available_conjunct();
    F_And *find_available_And();


/* === data === */
private:

    int ref_count;
    Rel_Body_Status status;

    int number_input, number_output;
    Tuple<Const_String> In_Names, Out_Names;
    Variable_ID_Tuple Symbolic;

    DNF* simplified_DNF;
    short r_conjs;		// are redundant conjuncts eliminated?
    bool finalized;
    bool _is_set;
};


} // end of namespace omega

#endif
