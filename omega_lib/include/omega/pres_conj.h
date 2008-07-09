#if ! defined _pres_conj_h
#define _pres_conj_h 1

#include <basic/bool.h>


/* $Id: pres_conj.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#if ! defined _pres_decl_h
#include <omega/pres_decl.h>
#endif
#if ! defined _pres_logic_h
#include <omega/pres_logic.h>
#endif
#if ! defined _pres_cnstr_h
#include <omega/pres_cnstr.h>
#endif


namespace omega {

// friend function for Conjunct
const char* get_var_name(unsigned int col, void * void_conj);

//
// Conjunct
//
// About variables in Conjunct:
// All varaibles appear in exactly one declaration.
// All variables used in Conjunct are referenced in mappedVars.
// Wildcard variables are referenced both in mappedVars and in myLocals, 
//   since they are declared in the conjunct.
// All other variables are declared at the levels above.
// Column number is: 
//   in forwardingAddress in Problem if variablesInitialized is set, 
//   equal to position of Variable_ID in mappedVars list otherwise.
//

class Conjunct : public F_Declaration {
public:
  Constraint_Iterator constraints();
  Variable_ID_Tuple  *variables();
  EQ_Iterator         EQs();
  GEQ_Iterator        GEQs();
  inline int          n_EQs() { return problem->nEQs; }
  inline int          n_GEQs() { return problem->nGEQs; }

  void promise_that_ub_solutions_exist(Relation &R);

  inline Node_Type node_type() {return Op_Conjunct;}

  inline int is_true() {return problem->nEQs==0 && problem->nGEQs==0 
                               && exact;}

  void query_difference(Variable_ID v1, Variable_ID v2,
			coef_t &lowerBound, coef_t &upperBound, bool &guaranteed);
  void query_variable_bounds(Variable_ID v, coef_t &lowerBound, coef_t &upperBound);
  bool query_variable_used(Variable_ID v);

  int countNonzeros() const {
        int numberNZs;
	coef_t maxCoef, SumAbsCoef;
	problem->difficulty(numberNZs,maxCoef,SumAbsCoef);
	return numberNZs;
	}

  void difficulty(int &numberNZs, coef_t &maxCoef, coef_t &SumAbsCoef) const {
	problem->difficulty(numberNZs,maxCoef,SumAbsCoef);
	}

  int query_guaranteed_leading_0s() {
	count_leading_0s();
	return guaranteed_leading_0s;
	}

  int query_possible_leading_0s() {
	count_leading_0s();
	return possible_leading_0s;
	}

  int query_leading_dir() {
	count_leading_0s();
	return leading_dir;
	}

  void calculate_dimensions(Relation &R, int &ndim_all, int &ndim_domain);
   int max_ufs_arity_of_set();
   int max_ufs_arity_of_in();
   int max_ufs_arity_of_out();

  int rank();

  ~Conjunct();

         bool is_unknown() const;
  inline bool is_exact() const { return exact;}
  inline bool is_inexact() const { return !exact;}
  inline void make_inexact()   { exact=false;}
   

#if ! defined NDEBUG
  void		   assert_leading_info();
#else
  void		   assert_leading_info() {}
#endif


    // PRINTING FUNCTIONS
  void print(FILE *output_file);
  void prefix_print(FILE *output_file, int debug = 1);
  String print_to_string(int true_printed);
  String print_EQ_to_string(Eqn e) { return problem->print_EQ_to_string(e); }
  String print_GEQ_to_string(Eqn e) { return problem->print_GEQ_to_string(e); }
  String print_EQ_to_string(int e) 
    { return problem->print_EQ_to_string(&(problem->EQs[e])); }
  String print_GEQ_to_string(int e) 
    { return problem->print_GEQ_to_string(&(problem->GEQs[e])); }
  String print_term_to_string(Eqn e) { return problem->print_term_to_string(e,1); }
  String print_EQ_term_to_string(int e) 
     { return problem->print_term_to_string(&(problem->EQs[e]),1); }
  String print_GEQ_term_to_string(int e) 
     { return problem->print_term_to_string(&(problem->GEQs[e]),1); }
  String print_sub_to_string(int col) { return problem->print_sub_to_string(col); }

private:

  inline void interpret_unknown_as_true()   { exact=true;}

  friend Relation approx_closure(NOT_CONST Relation &r, int n);

  virtual Conjunct *really_conjunct();


      // create new constraints with all co-efficients 0
      // These are public in F_And, use them from there.
  EQ_Handle   add_stride(int step, int preserves_level = 0);
  EQ_Handle   add_EQ(int preserves_level = 0);
  GEQ_Handle  add_GEQ(int preserves_level = 0);
  EQ_Handle   add_EQ(const Constraint_Handle &c, int preserves_level = 0);
  GEQ_Handle  add_GEQ(const Constraint_Handle &c, int preserves_level = 0);

  friend class GEQ_Handle;
  friend class  EQ_Handle;
  friend class Sub_Handle;
  friend class Constraint_Handle;
  friend class Constraint_Iterator;
  friend class GEQ_Iterator;
  friend class  EQ_Iterator;
  friend class Sub_Iterator;
  friend class Constr_Vars_Iter;


    // FUNCTIONS HAVING TO DO WITH BUILDING FORMULAS/DNFs 
  bool     can_add_child();
  void remap();
  void beautify();
  DNF* DNFize();
  int priority();
  virtual Conjunct *find_available_conjunct();
  void finalize();

  friend class DNF;



    // CREATING CONJUNCTS
  Conjunct();
  Conjunct(Conjunct &);
  Conjunct(Formula *, Rel_Body *);

  friend class Formula; // add_conjunct (a private function) creates Conjuncts
  friend class F_Not;
  friend class F_Or;
    //   class F_And; is a friend below
  

    // VARIOUS FUNCTIONS TO CREATE / WORK WITH VARIABLES
  Variable_ID declare(Const_String s);
  Variable_ID declare();
  Variable_ID declare(Variable_ID v);  
  
  friend const char *get_var_name(unsigned int, void *);
  void push_exists(Variable_ID_Tuple &S);
  int  get_column(Variable_ID);
  int  find_column(Variable_ID);
  int  map_to_column(Variable_ID);
  void combine_columns();
  void reorder();
  void reorder_for_print(bool reverseOrder=false,
			int first_pass_input=0,
			int first_pass_output=0,
			bool sort=false);

  friend void      remap_DNF_vars(Rel_Body *new_rel, Rel_Body *old_rel);

  void localize_var(Variable_ID D);


    // this creates variables in conjuncts for us:
  friend int       new_WC(Conjunct *nc, Problem *np);


    // UFS/LEADING ZEROS STUFF

  void move_UFS_to_input();

  void count_leading_0s();
  void invalidate_leading_info(int changed = -1);
  void enforce_leading_info(int guaranteed, int possible, int dir);

  void reverse_leading_dir_info();

 

    // CONJUNCT SPECIFIC STUFF

  void      rm_color_constrs();
  inline int N_protected() { return problem->safeVars; }


  void ordered_elimination(int symLen) { problem->ordered_elimination(symLen);}
  void convertEQstoGEQs();

  int cost();
  
  inline Formula*  copy(Formula *parent, Rel_Body *reln)
    { return copy_conj_diff_relation(parent,reln); }
  Conjunct* copy_conj_diff_relation(Formula *parent, Rel_Body *reln);
  inline Conjunct* copy_conj_same_relation()
    { return copy_conj_diff_relation(&(parent()), relation()); }
  friend void internal_copy_conjunct(Conjunct* to, Conjunct* fr);
  friend void copy_constraint(Constraint_Handle H,
			      const Constraint_Handle initial);

#if defined STUDY_EVACUATIONS
    // The core function of "evac.c" does lots of work with conjuncts:
  friend bool check_subseq_n(Conjunct *c, Sequence<Variable_ID> &evac_from, Sequence<Variable_ID> &evac_to, int n_from, int n_to, int max_arity, int n, bool allow_offset);
  friend void assert_subbed_syms(Conjunct *c);
  friend bool check_affine(Conjunct *d, Sequence<Variable_ID> &evac_from, Sequence<Variable_ID> &evac_to, int n_from, int n_to, int max_arity);
  friend evac study(Conjunct *C, Sequence<Variable_ID> &evac_from, Sequence<Variable_ID> &evac_to, int n_from, int n_to, int max_arity);
#endif

    // The relational ops tend to do lots of demented things to Conjuncts:
  friend class Rel_Body;
  friend_rel_ops;

    // F_And sometimes absorbs conjuncts
  friend class F_And;

    // Various DNFize functions also get a the problem:

  friend DNF* conj_and_not_dnf(Conjunct *pos_conj, DNF *neg_conjs, bool weak);
  friend class F_Exists;

    // Substitutions are a wrapper around a low-level Problem operation
  friend class Substitutions;

  // private functions to call problem functions 
  int simplifyProblem();
  int simplifyProblem(int verify, int subs, int redundantElimination);
  int redSimplifyProblem(int effort, int computeGist);

  friend int       simplify_conj(Conjunct* conj, int ver_sim, int elim_red, int color);
  friend DNF*      negate_conj(Conjunct* conj);
  friend Conjunct* merge_conjs(Conjunct* conj1, Conjunct* conj2,
			      Merge_Action action, Rel_Body *body = 0);
  friend void      copy_conj_header(Conjunct* to, Conjunct* fr);


  // === at last, the data ===

  Variable_ID_Tuple mappedVars;

  int              n_open_constraints;
  bool             cols_ordered;
  bool             simplified;
  bool             verified;

  int		   guaranteed_leading_0s;  // -1 if unknown
  int		   possible_leading_0s;  // -1 if unknown
  int		   leading_dir; // 0 if unknown, else +/- 1
  int		   leading_dir_valid_and_known();

  bool             exact;

  short		   r_constrs;	// are redundant constraints eliminated?
  Problem         *problem;

  bool is_compressed();
  void compress();
  void uncompress();

  friend class Comp_Problem;
  Comp_Problem    *comp_problem;
};


/* === Misc. problem manipulation utilities === */

const int CantBeNegated = MAXINT-10;
const int AvoidNegating = MAXINT-11;

void copy_column(Problem *tp,  int to_col,
		 Problem *fp,  int fr_col,
		 int start_EQ, int start_GEQ);
void zero_column(Problem *tp,  int to_col,
		 int start_EQ, int start_GEQ,
	         int no_EQs,   int no_GEQs);


} // end of namespace omega

#endif
