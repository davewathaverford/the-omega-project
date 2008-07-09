#if ! defined _pres_cmpr_h
#define _pres_cmpr_h 1

/* $Id: pres_cmpr.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <omega/omega_core/oc.h>

namespace omega {

//
// Compressed problem: rectangular non-0 cut from the big problem.
//
class Comp_Constraints {
public:
  Comp_Constraints(Eqn constrs, int no_constrs, int no_vars);
  void UncompressConstr(Eqn constrs, short &pn_constrs);
  ~Comp_Constraints();
  bool no_constraints() const
    { return n_constrs == 0; }
  int n_constraints() const
    { return n_constrs; }

protected:
  inline int coef_index(int e, int v)
    {return e*(n_vars+1) + v;}

private:
  int n_constrs;
  int n_vars;
  coef_t *coefs;
};

class Comp_Problem {
public:
  Comp_Problem(Problem *problem);
  Problem *UncompressProblem();
  bool no_constraints() const
    { return eqs.no_constraints() && geqs.no_constraints(); }

private:
/* === data === */
  int             _nVars, _safeVars;
  const char   *(*_get_var_name)(unsigned int var, void *args);
  void           *_getVarNameArgs;
  Comp_Constraints eqs;
  Comp_Constraints geqs;
};

} // end of namespace omega

#endif
