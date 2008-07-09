#if ! defined _pres_cnstr_h
#define _pres_cnstr_h 1

#include <basic/bool.h>


/* $Id: pres_cnstr.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#if ! defined _pres_var_h
#include <omega/pres_var.h>
#endif

namespace omega {

//
// Constraint handles
//

class CG_outputRepr; // Kludge due to misbehaved code_gen code
class CG_outputBuilder; // Kludge due to misbehaved code_gen code

void copy_constraint(Constraint_Handle H, const Constraint_Handle initial);

class Constraint_Handle {
public:
  inline Constraint_Handle() {}
  virtual ~Constraint_Handle() {}
  void   update_coef(Variable_ID, coef_t delta);
  void   update_const(coef_t delta);
  coef_t get_coef(Variable_ID v) const;
  coef_t get_const() const;
  bool   has_wildcards() const;
  int    max_tuple_pos() const;
  bool is_const(Variable_ID v);

  virtual String print_to_string() const;
  virtual String print_term_to_string() const;
  
  Variable_ID get_local(const Global_Var_ID G);
  Variable_ID get_local(const Global_Var_ID G, Argument_Tuple of);
    // not sure that the second one can be used in a meaningful
    // way if the conjunct is in multiple relations

  void   finalize();
  void   multiply(int multiplier);
  Rel_Body *relation() const;

protected:
  Conjunct *c;
  Eqn      *eqns;
  int      e;

  friend class Constr_Vars_Iter;
  friend class Constraint_Iterator; 

  Constraint_Handle(Conjunct *, Eqn *, int);

#if defined PROTECTED_DOESNT_WORK
    friend class EQ_Handle;
    friend class GEQ_Handle;
#endif

private:
  friend class Conjunct;  // assert_leading_info updates coef's
			  // as does move_UFS_to_input
  friend class DNF;       // and DNF::make_level_carried_to

  friend void copy_constraint(Constraint_Handle H,
			      const Constraint_Handle initial);
    // copy_constraint does updates and gets at c and e

    friend CG_outputRepr* outputAsGuard(CG_outputBuilder* ocg,
					NOT_CONST Constraint_Handle &e,
					bool is_geq);
    friend CG_outputRepr* output_GEQ_strides(CG_outputBuilder* ocg,
				      Relation &guards, Conjunct *c);
    friend CG_outputRepr* outputStrideAsRepr(CG_outputBuilder* ocg, 
					     NOT_CONST EQ_Handle &e);
    // for some reason these call the functions below


  void   update_coef_during_simplify(Variable_ID, coef_t delta);
  void   update_const_during_simplify(coef_t delta);
  coef_t    get_const_during_simplify() const;
  coef_t    get_coef_during_simplify(Variable_ID v) const;
};

class GEQ_Handle : public Constraint_Handle {
public:
    inline GEQ_Handle() {}
    virtual ~GEQ_Handle() {}

    virtual String print_to_string() const;
    virtual String print_term_to_string() const;
    bool operator==(const Constraint_Handle &that);

    void   negate();

private:
    friend class Conjunct;
    friend class GEQ_Iterator;
    GEQ_Handle(Conjunct *, int);
};


class EQ_Handle : public Constraint_Handle {
public:
    inline EQ_Handle() {}
    virtual ~EQ_Handle() {}

    virtual String print_to_string() const;
    virtual String print_term_to_string() const;
    bool operator==(const Constraint_Handle &that);

private:
    friend class Conjunct;
    friend class EQ_Iterator;
    EQ_Handle(Conjunct *, int);
};


//
// Conjuct iterators -- for querying resulting DNF.
//
class Constraint_Iterator : public Generator<Constraint_Handle> {
public:
    Constraint_Iterator(Conjunct *);
    int  live() const;
    void operator++(int);
    void operator++();
    Constraint_Handle operator* ();
    Constraint_Handle operator* () const;

private:
    Conjunct *c;
    int current,last;
    Eqn *eqns;
};


class EQ_Iterator : public Generator<EQ_Handle> {
public:
    EQ_Iterator(Conjunct *);
    int  live() const;
    void operator++(int);
    void operator++();
    EQ_Handle operator* ();
    EQ_Handle operator* () const;

private:
    Conjunct *c;
    int current, last;
};


class GEQ_Iterator : public Generator<GEQ_Handle> {
public:
    GEQ_Iterator(Conjunct *);
    int  live() const;
    void operator++(int);
    void operator++();
    GEQ_Handle operator* ();
    GEQ_Handle operator* () const;

private:
    Conjunct *c;
    int current, last;
};


//
// Variables of constraint iterator.
//
struct Variable_Info {
  Variable_ID var;
  coef_t      coef;
  Variable_Info(Variable_ID _var, coef_t _coef)
    { var = _var; coef = _coef; }
};

class Constr_Vars_Iter : public Generator<Variable_Info> {
public:
  Constr_Vars_Iter(const Constraint_Handle &ch, bool _wild_only = false);
  int         live() const;
  void        operator++(int);
  void        operator++();
  Variable_Info operator*() const;

  Variable_ID curr_var() const;
  coef_t      curr_coef() const;

private:
  Eqn               *eqns;
  int               e;
  Problem           *prob;
  Variable_ID_Tuple &vars;
  bool              wild_only;
  int               current;
};

} // end of namespace omega

#endif
