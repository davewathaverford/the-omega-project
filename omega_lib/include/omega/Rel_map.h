#if ! defined _Rel_map_h
#define _Rel_map_h 1

/* $Id: Rel_map.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#if ! defined _pres_gen_h
#include <omega/pres_gen.h>
#endif
#if ! defined _pres_var_h
#include <omega/pres_var.h>
#endif


namespace omega {

//
// Mapping for relations
// When a relation operation needs to re-arrange the variables,
//  it describes the re-arragement with a mapping, and then calls
//  align to re-arrange them.
//
// In a mapping, map_in (map_out/map_set) gives the new type and
//  position of each of the old input (output/set) variables.
// For variables being mapped to Input, Output, or Set variables,
//  the position is the new position in the tuple.
// For variables being mapped to Exists_Var, Forall_Var, or
//  Wildcard_Var, the positions can be used to map multiple
//  variables to the same quantified variable, by providing
//  the same position.  Each variable with a negative position
//  is given a unique quantified variable that is NOT listed
//  in the seen_exists_ids list.
// I'm not sure what the positions mean for Global_Vars - perhaps
//  they are ignored?
//
// Currently, align seems to support only mapping to Set, Input,
//  Output, and Exists vars.
//

class Mapping {
public:
  inline  Mapping(int no_in, int no_out): n_input(no_in), n_output(no_out) {}
  inline  Mapping(int no_set): n_input(no_set), n_output(0){}
  inline  Mapping(const Mapping &m): n_input(m.n_input), n_output(m.n_output) {
    int i;
    for(i=1; i<=n_input; i++) map_in_kind[i] = m.map_in_kind[i];
    for(i=1; i<=n_input; i++) map_in_pos[i]  = m.map_in_pos[i];
    for(i=1; i<=n_output;i++) map_out_kind[i] = m.map_out_kind[i];
    for(i=1; i<=n_output;i++) map_out_pos[i] = m.map_out_pos[i];
  }

  inline void set_map (Var_Kind in_kind, int pos, Var_Kind type, int map) {
    if(in_kind==Input_Var)
      set_map_in(pos,type,map);
    else if(in_kind==Set_Var)
      set_map_in(pos,type,map);
    else if(in_kind==Output_Var)
      set_map_out(pos,type,map);
    else
      assert(0);
  }

  inline void set_map_in (int pos, Var_Kind type, int map) {
    assert(pos>=1 && pos<=n_input);
    map_in_kind[pos] = type;
    map_in_pos[pos] = map;
  }
  inline void set_map_set (int pos, Var_Kind type, int map) {
    assert(pos>=1 && pos<=n_input);
    map_in_kind[pos] = type;
    map_in_pos[pos] = map;
  }

  inline void set_map_out(int pos, Var_Kind type, int map) {
    assert(pos>=1 && pos<=n_output);
    map_out_kind[pos] = type;
    map_out_pos[pos] = map;
  }

  inline Var_Kind get_map_in_kind(int pos)  const {
    assert(pos>=1 && pos<=n_input);
    return map_in_kind[pos];
  }

  inline int get_map_in_pos(int pos)  const {
    assert(pos>=1 && pos<=n_input);
    return map_in_pos[pos];
  }

  inline Var_Kind get_map_out_kind(int pos)  const {
    assert(pos>=1 && pos<=n_output);
    return map_out_kind[pos];
  }

  inline int get_map_out_pos(int pos)  const {
    assert(pos>=1 && pos<=n_output);
    return map_out_pos[pos];
  }

  inline int n_in()  const { return n_input;  }
  inline int n_out() const { return n_output; }

  // If a tuple as a whole becomes the new Input or Output tuple,
  //  return the Tuple of they will become (Input, Output)
  // Return Unknown_Tuple otherwise

  inline Argument_Tuple get_tuple_fate(Argument_Tuple t, int prefix = -1) const
	{ return   t== Input_Tuple ?  get_input_fate(prefix) :
                  (t==Output_Tuple ? get_output_fate(prefix) : Unknown_Tuple); }

  inline Argument_Tuple get_set_fate(int prefix = -1)  const
        { return get_input_fate(prefix); }

  inline Argument_Tuple get_input_fate(int prefix = -1)  const
	{
	    if (prefix < 0) prefix = n_input;
	    assert(n_input >= prefix);
	    if (n_input < prefix)
		return Unknown_Tuple;
	    Var_Kind vf = map_in_kind[1];
	    for (int i = 1; i<=prefix; i++)
		if (map_in_pos[i]!=i || map_in_kind[i]!=vf)
		    return Unknown_Tuple;

	    return vf == Input_Var  ? Input_Tuple
		 : vf == Set_Var    ? Set_Tuple
	         : vf == Output_Var ? Output_Tuple
		 : Unknown_Tuple;
	}
  inline Argument_Tuple get_output_fate(int prefix = -1) const
	{
	    if (prefix < 0) prefix = n_output;
	    assert(n_output >= prefix);
	    if (n_output < 1)
		return Unknown_Tuple;
	    Var_Kind vf = map_out_kind[1];
	    for (int i = 1; i<=prefix; i++)
		if (map_out_pos[i]!=i || map_out_kind[i]!=vf)
		    return Unknown_Tuple;
	    return vf == Input_Var  ? Input_Tuple
		 : vf == Set_Var    ? Set_Tuple
	         : vf == Output_Var ? Output_Tuple
		 : Unknown_Tuple;
	}

  inline static Mapping Identity(int inp, int outp) 
    {
	Mapping m(inp, outp); int i;
	for(i=1; i<=m.n_input; i++) m.set_map(Input_Var, i, Input_Var, i);
	for(i=1; i<=m.n_output;i++) m.set_map(Output_Var, i, Output_Var, i);
	return m;
    }

  inline static Mapping Identity(int setvars) 
    {
	Mapping m(setvars); int i;
	for(i=1; i<=setvars; i++) m.set_map(Set_Var, i, Set_Var, i);
	return m;
    }

private:
  int  n_input;
  int  n_output;
  Var_Kind map_in_kind[maxVars];
  int      map_in_pos[maxVars];
  Var_Kind map_out_kind[maxVars];
  int      map_out_pos[maxVars];
};


} // end of namespace omega

#endif
