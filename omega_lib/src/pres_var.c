/* $Id: pres_var.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <omega/pres_var.h>
#include <omega/pres_tree.h>
#include <omega/pres_conj.h>
#include <omega/omega_i.h>

namespace omega {

int wildCardInstanceNumber;

const int Global_Input_Output_Tuple::initial_allocation = 10;

// Declare named Variable
Var_Decl::Var_Decl(Const_String name, Var_Kind vkind, int pos) :
base_name(name),
instance(999),
remap(this),
var_kind(vkind), 
position(pos), 
global_var(NULL),
of((Argument_Tuple) 0) {
  assert((vkind==Input_Var || vkind==Output_Var) && pos>0 || pos==0);
}

// Declare unnamed variable
Var_Decl::Var_Decl(Var_Kind vkind, int pos) :
instance(999),
remap(this),
var_kind(vkind), 
position(pos), 
global_var(NULL),
of((Argument_Tuple) 0) {
  assert((vkind==Input_Var || vkind==Output_Var) && pos>0 || pos==0);
}

// Copy variable declaration
Var_Decl::Var_Decl(Variable_ID v) : 
base_name(v->base_name), 
instance(v->instance),
remap(this),
var_kind(v->var_kind), 
position(v->position), 
global_var(v->global_var),
of(v->of) {
}

Var_Decl::Var_Decl(Const_String name, Global_Var_ID v) :
base_name(name), 
instance(999),
remap(this),
var_kind(Global_Var), 
position(0), 
global_var(v),
of((Argument_Tuple) 0) {
  assert(v->arity() == 0);
}

Var_Decl::Var_Decl(Const_String name, Global_Var_ID v, Argument_Tuple function_of) :
base_name(name),
instance(999),
remap(this),
var_kind(Global_Var), 
position(0), 
global_var(v),
of(function_of) {}

int Var_Decl::get_position() {
  Assert(var_kind==Input_Var || var_kind==Output_Var,
	 "Var_Decl::get_position: bad var_kind");
  return(position);
}

Global_Var_ID Var_Decl::get_global_var() {
  Assert(var_kind==Global_Var,
	 "Var_Decl::get_global_var: bad var_kind");
  return(global_var);
}

Argument_Tuple Var_Decl::function_of() {
  assert(var_kind==Global_Var);
  return(of);
}


Omega_Var *Global_Var_Decl::really_omega_var() {
  assert(0);
  return(NULL);
}

Coef_Var_Decl *Global_Var_Decl::really_coef_var() {
  assert(0);
  return(NULL);
}

//
// Variable name.
//

const int N_greek_letters = 19;

char greek_letters[19][10] = {
   "alpha" , "beta" , "gamma" , "delta" , "tau" , "sigma" , "chi" ,
   "omega" , "pi" , "ni" , "Alpha" , "Beta" , "Gamma" , "Delta" ,
   "Tau" , "Sigma" , "Chi" , "Omega" , "Pi" 
};

const int numBuffers = 50;
const int bufferSize = 90;
char nameBuffers[numBuffers][bufferSize];
int nextBuffer = 0;

int use_ugly_names = 0;

const char *Var_Decl::char_name() {
  char *s = nameBuffers[nextBuffer++];
  char *start = s;
  if (nextBuffer >= numBuffers) nextBuffer = 0;
  int primes;

  if (use_ugly_names) 
	primes = 0;
  else 
    primes = instance;

    switch(var_kind) {
     case Input_Var:
	if (!use_ugly_names && !base_name.null())
	    sprintf(s,"%s",(const char *)base_name);
	else
	    {
	    primes = 0;
	    sprintf(s,"In_%d",position);
	    }
	break;
     case Output_Var:
	if (!use_ugly_names && !base_name.null())
	    sprintf(s,"%s",(const char *)base_name);
	else
	    {
	    primes = 0;
	    sprintf(s,"Out_%d",position);
	    }
	break;
      case Global_Var:
	assert(!base_name.null());
	if (use_ugly_names) 
	  sprintf(s,"%s@%p",(const char *)base_name,  this);
	else  {
	  sprintf(s,"%s",(const char *)base_name);
	  primes = get_global_var()->instance;
		}
	break;
    default: 
	if (use_ugly_names) {
               if (!base_name.null())
                       sprintf(s,"%s@%p",(const char *)base_name, this);
               else
                       sprintf(s,"?@%p", this);
               }
	else if (!base_name.null()) sprintf(s,"%s",(const char *)base_name);
		else {
			assert(instance < 999);
			sprintf(s,"%s",
			    greek_letters[instance % N_greek_letters]);
			primes = instance/N_greek_letters;
			};
	break;
	};

  while (*s) s++;
  int i;
  assert(primes < 999);
  for(i=1; i<= primes; i++) *s++ = '\'';
  *s = '\0';
  if (var_kind == Global_Var) {
    int a = get_global_var()->arity();
    if (a)
	{
	if (use_ugly_names) {
		static char *arg_names[4] = { "???", "In", "Out", "In == Out" };
		sprintf(s, "(%s[1#%d])", arg_names[function_of()], a);
		}
	else {
		int f_of = function_of();
		assert(f_of == Input_Tuple || f_of == Output_Tuple);
		*s++ = '(';
		for(i = 1;i<=a;i++) {
			if (f_of == Input_Tuple) 
		           sprintf(s,"%s",(const char *)input_vars[i]->char_name());
			else
		           sprintf(s,"%s",(const char *)output_vars[i]->char_name());
			while (*s) s++;
			if (i<a) *s++ = ',';
			};
		*s++ = ')';
		*s++ = 0;
		}
	};
    };

  assert(s < start+bufferSize);	
  return start;
}

String Var_Decl::name() {
  return (String) char_name();
}

//
// Copy variable declarations.
//
void copy_var_decls(Variable_ID_Tuple &new_vl, Variable_ID_Tuple &vl) {
  if (new_vl.size() < vl.size()) {
    new_vl.reallocate(vl.size());
  }
  for(int p=1; p<=vl.size(); p++) {
    Variable_ID v = vl[p];
    if(v->kind()==Global_Var) {
      new_vl[p] = v;
      v->remap = v;
    } else {
      new_vl[p] = new Var_Decl(vl[p]);
      v->remap = new_vl[p];
    }
  }
}

//
// Name a variable.
//
void Var_Decl::name_variable(char *newname) {
  base_name = newname;
}


static Const_String coef_var_name(int i, int v)
    {
    char s[100];
    sprintf(s, "c_%d_%d", i, v);
    return Const_String(s);
    }


//
// Global variables stuff.
//
Global_Var_Decl::Global_Var_Decl(Const_String baseName) :
    loc_rep1(baseName, this, Input_Tuple),
    loc_rep2(baseName, this, Output_Tuple)
    {
    }

Global_Kind Global_Var_Decl::kind() const {
    assert(0);
    return Coef_Var;
    }

Coef_Var_Decl::Coef_Var_Decl(int id, int var) : 
    Global_Var_Decl(coef_var_name(id, var))
    {
    i = id; 
    v = var;
    }

Coef_Var_Decl *Coef_Var_Decl::really_coef_var() {
    return this;
    }

int Coef_Var_Decl::stmt() const {
    return i;
    }

int Coef_Var_Decl::var() const {
    return v;
    }

Global_Kind Coef_Var_Decl::kind() const {
    return Coef_Var;
    }

Free_Var_Decl::Free_Var_Decl(Const_String name) : 
    Global_Var_Decl(name), _arity(0)
    {
    }

Free_Var_Decl::Free_Var_Decl(Const_String name, int arity) : 
    Global_Var_Decl(name), _arity(arity)
    {
    }

int Free_Var_Decl::arity() const {
  return _arity;
}

Global_Kind Free_Var_Decl::kind() const {
    return Free_Var;
    }


//
// Delete variable from variable list. Does not preserve order
//
bool rm_variable(Variable_ID_Tuple &vl, Variable_ID v) {
  int n = vl.size();
  int i;
  for(i = 1; i<n; i++) if (vl[i] == v) break;
  if (i>n) return 0;
  vl[i] = vl[n];
  vl.delete_last();
  return 1;
  };


//
// Destroy variable declarations.
//
void free_var_decls(Variable_ID_Tuple &c) {
  for(Variable_ID_Iterator p = c; p; p++) {
    Variable_ID v = *p;
    if(v->kind()!=Global_Var) {
      delete v;
    }
  }
}




Variable_ID input_var(int nth) {
  assert(1 <= nth && nth <= input_vars.size());
  return input_vars[nth];
}

Variable_ID output_var(int nth) {
  assert(1<= nth && nth <= output_vars.size());
  return output_vars[nth];
}

Variable_ID set_var(int nth) {
  assert(1 <= nth && set_vars.size());
  return input_vars[nth];
}


//
// Remap mappedVars in all conjuncts of formula.
// Uses the remap field of the Var_Decl.
//
void Formula::remap()
    {
    for(List_Iterator<Formula*> c(children()); c; c++)
	(*c)->remap();
    }

void Conjunct::remap() {
  for(Variable_Iterator VI(mappedVars); VI; VI++) {
    Variable_ID v = *VI;
    *VI = v->remap;
  }
  cols_ordered = false;
}

// Function to reset the remap field of a Variable_ID
void reset_remap_field(Variable_ID v) {
   v->remap = v;
   }

// Function to reset the remap fields of a Variable_ID_Seq
void reset_remap_field(Sequence<Variable_ID> &T) {
  for(Any_Iterator<Variable_ID> VI(T.any_iterator()); VI; VI++) {
    Variable_ID v = *VI;
    v->remap = v;
  }
}  

// Function to reset the remap fields of a Variable_ID_Tuple,
// more efficiently
void reset_remap_field(Variable_ID_Tuple &T) {
  for(Variable_Iterator VI(T); VI; VI++) {
    Variable_ID v = *VI;
    v->remap = v;
  }
}  
  
void reset_remap_field(Sequence<Variable_ID> &T, int var_no) {
  int i=1;
  for(Any_Iterator<Variable_ID> VI(T.any_iterator()); i <= var_no && VI; VI++){
    Variable_ID v = *VI;
    v->remap = v;
    i++;
  }
}    

void reset_remap_field(Variable_ID_Tuple &T, int var_no) {
  int i=1;
  for(Variable_Iterator VI(T); i <= var_no && VI; VI++){
    Variable_ID v = *VI;
    v->remap = v;
    i++;
  }
}    


// Global input and output variable tuples.
// These Tuples must be initialized as more in/out vars are needed.
//Variable_ID_Tuple  input_vars(0);
//Variable_ID_Tuple  output_vars(0);
//Variable_ID_Tuple& set_vars = input_vars;
Global_Input_Output_Tuple input_vars(Input_Var);
Global_Input_Output_Tuple output_vars(Output_Var);
Global_Input_Output_Tuple &set_vars = input_vars;

////////////////////////////////////////
//                                    //
// Variable and Declaration functions //
//                                    //
////////////////////////////////////////

//
// Constructors and properties.
//


// Allocate ten variables initially.  Most applications won't require more.
Global_Input_Output_Tuple::Global_Input_Output_Tuple(Var_Kind in_my_kind, int init): my_kind(in_my_kind)
{
    for (int i=1; i<=(init == -1? initial_allocation: omega::max(0,init)); i++) 
	this->append(new Var_Decl(Const_String(), my_kind, i));
}

Global_Input_Output_Tuple::~Global_Input_Output_Tuple() {
    for (int i=1; i<=size(); i++) 
	delete data[i-1];
}

Variable_ID & Global_Input_Output_Tuple::operator[](int index){
    assert(index > 0);
    while(size() < index)
	this->append(new Var_Decl(Const_String(), my_kind, size()+1));
    return data[index-1];
}

const Variable_ID & Global_Input_Output_Tuple::operator[](int index) const{
    assert(index > 0);
    Global_Input_Output_Tuple *unconst_this = 
      (Global_Input_Output_Tuple *) this;
    while(size() < index)
	unconst_this->append(new Var_Decl(Const_String(), my_kind, size()+1));
    return data[index-1];
}




Variable_ID  Var_Decl::UF_owner() {
	Variable_ID v = this;
	while (v->remap != v) v = v->remap;
        return v;
	}
void  Var_Decl::UF_union(Variable_ID b) {
	Variable_ID a  = this;
	while (a->remap != a) {
		Variable_ID tmp = a->remap;
		a->remap = tmp->remap;
		a = tmp;
		}
	while (b->remap != a) {
		Variable_ID tmp = b->remap;
		b->remap = a;
		b = tmp;
		}
	}


} // end of namespace omeganamespace omega {

