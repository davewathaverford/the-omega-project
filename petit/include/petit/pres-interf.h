/* $Id: pres-interf.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Pres_Interf
#define Already_Included_Pres_Interf

#include <basic/assert.h>
#include <basic/String.h>
#include <omega/Relations.h>
#include <petit/lang-interf.h>

namespace omega {

/* Var_Id should really be "Variable", as its just like
   a Program_Const_Var except that its not constant */

typedef enum {Offset_Var, Program_Const_Var, Unknown_Var, Var_Id_Var}
Omega_Var_Kind;
#define IRRELEVANT_ARITY -1


class Omega_Var : public Global_Var_Decl {
public:
//  Omega_Var();
  Omega_Var(int id);

  Omega_Var(Global_Var_ID g);

  Omega_Var(Var_Id v);

  Omega_Var(Var_Id v, int a);

  Omega_Var *really_omega_var();

  Global_Kind kind() const;

  Omega_Var_Kind global_kind();
  int arity() const;

  Var_Id var_id() {
      assert(k == Program_Const_Var || k == Var_Id_Var);
      return vid;
  }

private:
  Omega_Var_Kind k;
  int i;
  Var_Id vid;
  int art;
};

struct symtabentry;

void add_omega_var(symtabentry * s);
void add_omega_var(symtabentry * s, int a);
void add_omega_vars();  /* add omega_var everywhere */
void zap_omega_vars();
Omega_Var * get_omega_var(symtabentry *s, int a);

typedef Omega_Var * Omega_Var_Id;

}

#endif

