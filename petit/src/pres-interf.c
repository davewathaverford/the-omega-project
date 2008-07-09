/* $Id: pres-interf.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/String.h>
#include <basic/assert.h>
#include <petit/find_affine.h>
#include <petit/pres-interf.h>

namespace omega {


Global_Kind Omega_Var::kind() const
    {
    return Bomega_Var;
    }

Omega_Var *Omega_Var::really_omega_var()
    {
    return this;
    }

static Const_String intBaseName(int i)
    {
    char s[100];
    sprintf(s, "_c%d", i);
    return Const_String(s);
    }

static Const_String var_idBaseName(Var_Id v)
    {
    if (var_id_is_const(v))
	{
	assert(v);
        if (v->original)
	  {
           assert(v->original->symname);
	   assert(*v->original->symname != 0);
	   return Const_String(v->original->symname);
	  }
        else
	  {
           return Const_String(v->symname);
	  }
	}
    else
	{
	assert(*v->symname != 0);
	return Const_String(v->symname);
	}
    }

Omega_Var::Omega_Var(int id) : Global_Var_Decl(intBaseName(id))
    {
    k = Offset_Var;
    i = id;
    art = 0;
    }

Omega_Var::Omega_Var(Var_Id v) : Global_Var_Decl(var_idBaseName(v))
    {
    assert(v);
    vid = v; 
    if (var_id_is_const(v)) {
	k = Program_Const_Var;
        if (get_nodevalue_sym(var_id_def_node(v)))
          art = var_id_dims(get_nodevalue_sym(var_id_def_node(v)));
        else
          art = 0;
      }
    else {
	k = Var_Id_Var;
        if (!var_id_def_node(vid) )
            art = IRRELEVANT_ARITY;
        else {
            if (var_id_def_node(vid)->function)
	    {
	      assert(var_id_def_node(vid)->function == vid);
	      art =  expr_invariant_at_depth(var_id_def_node(vid));
	      }
	else
	      {
	      art = access_depth(var_id_def_node(vid));
	      }
 
           } 
 	}
    } 
     
Omega_Var::Omega_Var(Var_Id v, int a) : Global_Var_Decl(var_idBaseName(v))
    {
    assert(v);
    assert(!var_id_is_const(v) || 
           var_id_dims(get_nodevalue_sym(var_id_def_node(v))));
    assert (a >=0);
    vid = v;
    k = Var_Id_Var;
    art = a;
  }


Omega_Var_Kind Omega_Var::global_kind()
    {
    return k;
    }


int Omega_Var::arity() const
    {
    if (k == Unknown_Var) 
        { 
	assert(0 && "Omega_Var::arity() called on uninitialized variable");
	return 0;
	}
    else if (art==IRRELEVANT_ARITY)
        {
         assert(0 && "Taking arity for var without def node");
         return 0;
        }
     else
      return art;
    }

} // end omega namespace

#include <petit/tree.h>

namespace omega {
// hack to avoid memory leaks - wak
#define maxOmegaVars	10000
Omega_Var* all_omega_vars[maxOmegaVars];
int nr;

//add omega var to the entry in sym table s with given arity a

void add_omega_var(symtabentry *s, int a)
    {
     if (s->omega_vars==NULL)
       {
        s->omega_vars= new List<Omega_Var *>;
	Omega_Var *v = new Omega_Var(s, a);
	all_omega_vars[nr++] = v;
        s->omega_vars->append(v);
       }
     else 
       {
        int found=0;
        for (List_Iterator<Omega_Var *> li(*(s->omega_vars)); li; li++)
	     {
              if (a == li.curr()->arity())
                {found=1;break;}
	     }
      
        if (found)
          {
           assert (s->def_node);
           assert (!var_id_def_node(s)->function || 
                   var_id_def_node(s)->function == s);
          }
        else
	   {
	   Omega_Var *v = new Omega_Var(s,a);
	   all_omega_vars[nr++] = v;
	   s->omega_vars->append(v);
	   }
      }
   } 
	   
	   
void add_omega_var(symtabentry *s)
    {
      if (s->omega_vars == NULL)
        {
         s->omega_vars = new List<Omega_Var *>;
	 Omega_Var *v = new Omega_Var(s);
	 all_omega_vars[nr++] = v;
         s->omega_vars->append(v);
        }
    }


void add_omega_vars()
    {
    nametabentry *n;
    for( n = NameHead; n != NULL; n = n->namenext )
	if (n->namest.symtype != symcomment)
	{
	add_omega_var(&n->namest);
	}
    }

void zap_omega_vars()
    {
    nametabentry *n;
    for( n = NameHead; n != NULL; n = n->namenext )
	if (n->namest.symtype != symcomment)
	{
	if (n->namest.omega_vars == NULL) continue;
        for (List_Iterator<Omega_Var *> li(*(n->namest.omega_vars)); li; li++)
          delete li.curr();
	delete n->namest.omega_vars;
	n->namest.omega_vars = 0;
	}
    }

Omega_Var * get_omega_var(symtabentry *s, int a)
    {
     assert(s->omega_vars);
     if (a == IRRELEVANT_ARITY)
      {
        assert(s->omega_vars->length() == 1);
        return (s->omega_vars->front());
      }
     else
      {
        for (List_Iterator<Omega_Var *> li(*(s->omega_vars)); li; li++)
          if (li.curr()->arity() == a)
            return (li.curr());
      }

      assert(0 && "Omega var with required arity is not in the list");
      return 0;
   }      

} // end namespace omega
