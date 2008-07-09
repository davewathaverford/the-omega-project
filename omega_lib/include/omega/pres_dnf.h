#if ! defined _pres_dnf_h
#define _pres_dnf_h 1

#include <basic/bool.h>


/* $Id: pres_dnf.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#if ! defined _pres_gen_h
#include <omega/pres_gen.h>
#endif


namespace omega {

//
// Disjunctive Normal Form -- list of Conjuncts
//
class DNF {
public:
  void print(FILE *out_file);
  void prefix_print(FILE *out_file, int debug = 1, bool parent_names_setup=false);

  bool is_definitely_false() const;
  bool is_definitely_true() const;
  int  length() const;

  Conjunct *single_conjunct() const;
  bool      has_single_conjunct() const;
  Conjunct *rm_first_conjunct();
  void clear();
  int query_guaranteed_leading_0s(int what_to_return_for_empty_dnf);
  int query_possible_leading_0s(int what_to_return_for_empty_dnf);
  int query_leading_dir();

private:
    // all DNFize functions need to access the dnf builders:
  friend class F_And;
  friend class F_Or;
  friend class Conjunct;
  friend DNF * negate_conj(Conjunct *);

  friend class Rel_Body;
  friend_rel_ops;

  DNF();
  ~DNF();

  DNF* copy(Rel_Body *);

  void simplify();
  void make_level_carried_to(int level);
  void count_leading_0s();

  void add_conjunct(Conjunct*);
  void join_DNF(DNF*);
  void rm_conjunct(Conjunct *c);

  void rm_redundant_conjs(int effort);
  void rm_redundant_inexact_conjs();
  void DNF_to_formula(Formula* root);


  friend void  remap_DNF_vars(Rel_Body *new_rel, Rel_Body *old_rel);
  void remap();

  void setup_names();

  void remove_inexact_conj();

    // These may need to get at the conjList itself:
  friend DNF*  DNF_and_DNF(DNF*, DNF*);
  friend DNF*  DNF_and_conj(DNF*, Conjunct*);
  friend DNF*  conj_and_not_dnf(Conjunct *pos_conj, DNF *neg_conjs, bool weak);

  friend class DNF_Iterator;

  List<Conjunct*> conjList;
};

DNF* conj_and_not_dnf(Conjunct *pos_conj, DNF *neg_conjs, bool weak=false);

//
// DNF iterator
//
class DNF_Iterator : public List_Iterator<Conjunct*> {
public:
    DNF_Iterator(DNF*dnf) : List_Iterator<Conjunct*>(dnf->conjList) {}
    DNF_Iterator() {}
    void curr_set(Conjunct *c) { *(*this) = c; }
};

} // end of namespace omega

#endif
