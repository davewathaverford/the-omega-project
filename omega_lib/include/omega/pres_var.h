#if ! defined _pres_var_h
#define _pres_var_h 1

#include <basic/bool.h>


/* $Id: pres_var.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#if ! defined _pres_gen_h
#include <omega/pres_gen.h>
#endif


namespace omega {

//
// Variable declaration.
// Variables are free or quantified.
// Free variables are classified as input, output and global.
// Quantified variables are classified as forall, exists and wildcard.
// All global variables are functions symbols of (possibly 0) arguments
// Local variables that correspond to >0-ary functions are identified
//   as functions of a prefix of the input, output, or both tuples
//
// 
// typedef enum {Input_Var, Output_Var, Set_Var,
//               Global_Var, Forall_Var, Exists_Var, Wildcard_Var} Var_Kind;

typedef enum {Free_Var, Coef_Var, Bomega_Var} Global_Kind;

// NOW IN PRES_GEN.H, as its used as an argument and can't
// be forward declared:
// typedef enum {Unknown_Tuple = 0, Input_Tuple = 1, Output_Tuple = 2,
//				 Set_Tuple = Input_Tuple } Argument_Tuple;
// Only Input, Output, and Set can be passed to get_local,
// but the values 0 and 3 are also used internally.


class Var_Decl {
public:
  inline Var_Kind    kind() { return var_kind; }
         int         get_position();
  Global_Var_ID      get_global_var();
  Argument_Tuple     function_of();  // valid iff kind() == Global_var

        Const_String base_name;
                void name_variable(char *newname);

  // The following should be used with care, as they are only valid
  // after setup_names has been used on the relation containing this
  // variable.
         String      name();
  const  char       *char_name();
  void set_kind(Var_Kind v) { var_kind = v; }

  // Operation to allow the remap field to be used for
  // union-find operations on variables.
  // Be sure to reset the remap fields afterward
  void 	       UF_union(Variable_ID v);
  Variable_ID  UF_owner();

private:
  Var_Decl(Const_String name, Var_Kind vkind, int pos);
  Var_Decl(Var_Kind vkind, int pos);
  Var_Decl(Variable_ID v);
  Var_Decl(Const_String name, Global_Var_ID v);
  Var_Decl(Const_String name, Global_Var_ID v, Argument_Tuple function_of);

  friend class F_Declaration;    // creates local variables
  friend class Global_Var_Decl;  // its constructors create Var_Decls.

  friend class Global_Input_Output_Tuple;
  friend void copy_var_decls(Variable_ID_Tuple &new_vl, Variable_ID_Tuple &vl);

private:
  int  instance;
  void setup_name();

    // these set up the names
  friend class Rel_Body;
//  friend class F_Declaration; already a friend

private:
  Variable_ID   remap;          // pointer to new copy of this node

    // lots of things need to get at "remap" - lots of relation ops,
    // and functions that move UFS's around:
    //     dnf::make_level_carried_to and Conjunct::move_UFS_to_input()
    // Also of course Conjunct::remap and push_exists
  friend_rel_ops;
  friend class DNF;
  friend class Conjunct;

    // this prints remap to the debugging output
  friend void print_var_addrs(String &s, Variable_ID v);

  friend void reset_remap_field(Variable_ID v);
  friend void reset_remap_field(Sequence<Variable_ID> &S);
  friend void reset_remap_field(Sequence<Variable_ID> &S, int var_no);
  friend void reset_remap_field(Variable_ID_Tuple &S);
  friend void reset_remap_field(Variable_ID_Tuple &S, int var_no);

private:

  Var_Kind      var_kind;
  int           position;	// only for Input_Var, Output_Var
  Global_Var_ID global_var;     // only for Global_Var
  Argument_Tuple of;		// only for Global_Var
};

bool rm_variable(Variable_ID_Tuple &vl, Variable_ID v);
void reset_remap_field(Sequence<Variable_ID> &S);
void reset_remap_field(Sequence<Variable_ID> &S, int var_no);
void reset_remap_field(Variable_ID v);
void reset_remap_field(Variable_ID_Tuple &S);
void reset_remap_field(Variable_ID_Tuple &S, int var_no);

class Global_Input_Output_Tuple: public Tuple<Variable_ID> {
public:
    Global_Input_Output_Tuple(Var_Kind in_my_kind, int init=-1);
    virtual ~Global_Input_Output_Tuple();
    virtual Variable_ID &operator[](int index);
    virtual const Variable_ID &operator[](int index) const;
private:
    Var_Kind my_kind;
    static const int initial_allocation;
};

extern Global_Input_Output_Tuple input_vars;
extern Global_Input_Output_Tuple output_vars;
// This allows the user to refer to set_vars to query sets, w/o knowing
// they are really inputs.
extern Global_Input_Output_Tuple &set_vars;

Variable_ID input_var(int nth);
Variable_ID output_var(int nth);
Variable_ID set_var(int nth);



//
// Global_Var_ID uniquely identifies global var-s through the whole program.
// Global_Var_Decl is an ADT with the following operations:
// - create global variable,
// - find the arity of the variable, (default = 0, for symbolic consts)
// - get the name of global variable, 
// - tell if two variables are the same (if they are the same object)
//

class Global_Var_Decl {
public:
    Global_Var_Decl(Const_String baseName);
    virtual ~Global_Var_Decl() {}

    virtual Const_String base_name() const
	{
	return loc_rep1.base_name;
	}

    virtual void set_base_name(Const_String newName)
	{
	loc_rep1.base_name = newName;
	loc_rep2.base_name = newName;
	}

    virtual int arity() const
	{
	return 0;   // default compatible with old symbolic constant stuff
	}

    virtual Omega_Var *really_omega_var();  // until we get RTTI in C++
    virtual Coef_Var_Decl *really_coef_var();  // until we get RTTI in C++
    virtual Global_Kind kind() const;

private:

    friend class Rel_Body;  // Rel_Body::get_local calls this get_local

    Variable_ID get_local()
	{
	assert(arity() == 0);
	return &loc_rep1;
	}
    Variable_ID get_local(Argument_Tuple of)
	{
	assert(arity() == 0 || of == Input_Tuple || of == Output_Tuple);
	return ((arity() == 0 || of == Input_Tuple) ? &loc_rep1 : &loc_rep2);
	}

    // local representative, there is just 1 for every 0-ary global variable
    Var_Decl loc_rep1; // arity == 0, or arity > 0 and of == In
    Var_Decl loc_rep2; // arity > 0 and of == Out

private:

//    friend class Rel_Body;  // Rel_Body::setup_names sets instance
    friend class Var_Decl;
    int instance;
};


class Coef_Var_Decl : public Global_Var_Decl {
public:
  Coef_Var_Decl(int id, int var);
  virtual ~Coef_Var_Decl() {}
  int stmt() const;
  int var() const;
  virtual Global_Kind kind() const;
  virtual Coef_Var_Decl *really_coef_var();  // until we get RTTI in C++

private:
    int i, v;
};



//
// Test subclass for Global_Var: named global variable
//
class Free_Var_Decl : public Global_Var_Decl {
public:
  Free_Var_Decl(Const_String name);
  Free_Var_Decl(Const_String name, int arity);
  int arity() const;
  virtual Global_Kind kind() const;

private:
  int _arity;
};


/* === implementation functions === */
void copy_var_decls(Variable_ID_Tuple &new_vl, Variable_ID_Tuple &vl);
void free_var_decls(Variable_ID_Tuple &vl);

extern int wildCardInstanceNumber;

} // end of namespace omega

#endif
