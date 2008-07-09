#if ! defined _pres_form_h
#define _pres_form_h 1

#include <basic/bool.h>


/* $Id: pres_form.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#if ! defined _pres_gen_h
#include <omega/pres_gen.h>
#endif


namespace omega {

typedef enum {Op_Relation, Op_Not, Op_And, Op_Or,
	      Op_Conjunct, Op_Forall, Op_Exists}  Node_Type;


//
// Presburger Formula base class
//

class Formula {
public:
    virtual Node_Type node_type()=0;

        F_Forall *add_forall();
        F_Exists *add_exists();
virtual F_And    *and_with();
        F_And    *add_and();
        F_Or     *add_or();
        F_Not    *add_not();
        void      add_unknown();

    virtual void finalize();
    virtual void print(FILE *output_file);

    inline Rel_Body *relation() { return myRelation; }

protected:
    virtual ~Formula();
private:
    Formula(Formula *, Rel_Body *);

    // The relational operations need to work with formula trees
    friend class Relation;
    friend_rel_ops;
    // as do the functions that build DNF's
    friend  class DNF;
    // or other parts of the tree
    friend class Conjunct;
    friend class F_Declaration;
    friend class F_Exists;
    friend class F_Forall;
    friend class F_Or;
    friend class F_And;
    friend class F_Not;
    friend class Rel_Body;


    // Operations needed for manipulation of formula trees:

                void remove_child(Formula *);
                void replace_child(Formula *child, Formula *new_child);
        virtual bool can_add_child();
                void add_child(Formula *);

           Conjunct *add_conjunct();
   virtual Conjunct *find_available_conjunct() = 0;

    virtual Formula *copy(Formula *parent, Rel_Body *reln);
           F_Exists *add_exists(Variable_ID_Tuple &S); 
    virtual     void push_exists(Variable_ID_Tuple &S);

    // Accessor functions for tree building

    inline List<Formula*>    &children()
        {return myChildren;}
    inline int                n_children() const
        {return myChildren.length();}
    inline const List<Formula*> &get_children() const
        {return myChildren;}

    inline Formula           &parent()                 {return *myParent;}
    inline void               set_parent(Formula *p)   {myParent = p;}


    virtual    int priority();

    void verify_tree();  // should be const, but iterators are used

    virtual void reverse_leading_dir_info();
    virtual void invalidate_leading_info(int changed = -1);
    virtual void enforce_leading_info(int guaranteed, int possible, int dir);



    virtual void remap();   
    virtual DNF* DNFize() = 0;
    virtual void beautify();
    virtual void rearrange();
    virtual void setup_names();

    virtual void print_separator(FILE *output_file);
    virtual void combine_columns();
    virtual void prefix_print(FILE *output_file, int debug = 1);
            void print_head(FILE *output_file);

            void set_relation(Rel_Body *r);
            void set_parent(Formula *parent, Rel_Body *reln);

            void assert_not_finalized();

    virtual Conjunct *really_conjunct();  // until we get RTTI

private:
    List<Formula*>     myChildren;
    Formula           *myParent;
    Rel_Body          *myRelation;

};

} // end of namespace omega


#endif
