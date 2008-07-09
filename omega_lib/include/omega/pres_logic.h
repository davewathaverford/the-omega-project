#if ! defined _pres_logic_h
#define _pres_logic_h 1

#include <basic/bool.h>


/* $Id: pres_logic.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#if ! defined _pres_form_h
#include <omega/pres_form.h>
#endif

namespace omega {

//
// Presburger formula classes for logical operations: and, or not
//

class F_And    : public Formula {
public:
    inline Node_Type node_type() {return Op_And;}

    // "preserves level" should be 0 unless we know this will not
    // change the "level" of the constraints - ie the number of
    // leading corresponding in,out variables known to be equal
    GEQ_Handle     add_GEQ(int preserves_level = 0);
    EQ_Handle      add_EQ(int preserves_level = 0);
    Stride_Handle  add_stride(int step, int preserves_level = 0);
    EQ_Handle   add_EQ(const Constraint_Handle &c, int preserves_level = 0);
    GEQ_Handle  add_GEQ(const Constraint_Handle &c, int preserves_level = 0);

    F_And    *and_with();
    void add_unknown();

private:
    friend class Formula;  // add_and()
    F_And(Formula *p, Rel_Body *r);

private:
    Formula *copy(Formula *parent, Rel_Body *reln);
    virtual Conjunct *find_available_conjunct();
    int priority();
    void print_separator(FILE *output_file);
    void prefix_print(FILE *output_file, int debug = 1);
    void beautify();
    DNF* DNFize();
 
    Conjunct *pos_conj;
};


class F_Or     : public Formula {
public:
    inline Node_Type node_type() {return Op_Or;}

private:
    friend class Formula; // add_or
    F_Or(Formula *, Rel_Body *);

private:
    Formula *copy(Formula *parent, Rel_Body *reln);

    virtual Conjunct *find_available_conjunct();
    void print_separator(FILE *output_file);
    void prefix_print(FILE *output_file, int debug = 1);
    void beautify();
    int priority();
    DNF* DNFize();
    void push_exists(Variable_ID_Tuple &S);
};


class F_Not    : public Formula {
public:
    inline Node_Type node_type() {return Op_Not;}
    void finalize();

private:
    friend class Formula;
    F_Not(Formula *, Rel_Body *);

private:
    Formula *copy(Formula *parent, Rel_Body *reln);

    virtual Conjunct *find_available_conjunct();
    friend class F_Forall;
    bool can_add_child();
    void beautify();
    void rearrange();
    int priority();
    DNF* DNFize();
    void print(FILE *output_file);
    void prefix_print(FILE *output_file, int debug = 1);
};

} // end of namespace omega

#endif
