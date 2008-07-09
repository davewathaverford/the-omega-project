/* Interface to omega core's substitutions.

   Creating an object of class Substitutions causes ordered elimination, 
   i.e. variables in the input and output tuples are substituted for by
   functions of earlier variables.  Could conceivablely create a more 
   flexible interface to orderedElimination if we developed a way to 
   specify the desired variable order.  

   This is not an entirely consistent interface, since Sub_Handles
   shouldn't really permit update_coef on SUBs.  It is not a real
   problem since subs are now no longer part of a conjunct, but it is
   a slightly odd situation.

   Don't try to simplify r after performing orderedElimination.
*/

#if !defined(pres_subs_h)
#define pres_subs_h

   
#include <omega/pres_gen.h>
#include <omega/Relation.h>
#include <omega/pres_conj.h>
#include <omega/pres_cnstr.h>

namespace omega {


class Sub_Handle;
class Sub_Iterator;

class Substitutions  {
public: 
    Substitutions(Relation &input_R, Conjunct *input_c);
    ~Substitutions();
    Sub_Handle get_sub(Variable_ID v);
    bool substituted(Variable_ID v);
    bool sub_involves(Variable_ID v, Var_Kind kind);
private:
    friend class Sub_Iterator;
    friend class Sub_Handle;
    Relation *r;
    Conjunct *c;
    eqn *subs;
    Variable_ID_Tuple subbed_vars;
};


//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


class Sub_Handle: public Constraint_Handle {
public:
    inline Sub_Handle() {}

    virtual String print_to_string() const;
    virtual String print_term_to_string() const;
    Variable_ID variable() {return v;}

private:
    friend class Substitutions;
    friend class Sub_Iterator;
    Sub_Handle(Substitutions *, int, Variable_ID);
//    Sub_Handle(Substitutions *, int);

    Variable_ID v;
};

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


class Sub_Iterator : public Generator<Sub_Handle> {
public:
    Sub_Iterator(Substitutions *input_s): s(input_s), current(0),
                                          last(s->c->problem->nSUBs-1) {}
    int  live() const;
    void operator++(int);
    void operator++();
    Sub_Handle operator* ();
    Sub_Handle operator* () const;

private:
    Substitutions *s;
    int current, last;
};

} // end of namespace omega


#endif
