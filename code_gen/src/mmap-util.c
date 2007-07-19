#include <code_gen/mmap-util.h>

namespace omega {

/* simplify everything */

static void simplify_everything(Tuple<stm_info> &info)
{
   for (int s = 1; s <= info.size(); s++)
     {
      info[s].IS.simplify();
      for (int p = 1; p <= info[s].map.partials.size(); p++)
	info[s].map.partials[p].mapping.simplify();
      for (int r = 1; r <= info[s].read.size(); r++)
	for (int p = 1; p <= info[s].read[r].partials.size(); p++)
	  info[s].read[r].partials[p].dataFlow.simplify();
     }
}


Tuple<stm_info> &Trans_IS(Tuple<stm_info> &info, const Relation &Trans)
{
  // SHOULD ADD SOME CODE TO CHECK THE LEGALITY OF THE TRANSFORM:
  // THIS JUST NEEDS TO MAKE SURE ITS 1-1 FOR EVERY STATEMENT IN Trans_IS
  // AND DOES NOT MAKE ANY FLOW DEPS BACK IN TIME.

  Relation Tinv = Inverse(copy(Trans));

  for (int s=1; s<=info.length(); s++)
    {
      stm_info &stm = info[s];

      // update the iteration space
      stm.IS = Join(stm.IS, copy(Trans));

      // update the ranges and domains of all data flow to this statement
      for (int rn=1; rn<=stm.read.size(); rn++)
	{
	  Read &r = stm.read[rn];
	  for (int pn=1; pn<=r.partials.size(); pn++)
	    {
	      PartialRead &p = r.partials[pn];
	      p.dataFlow = Join(copy(Tinv),Join(p.dataFlow, copy(Trans)));
	    }
	}

      // update the domains of all the memory maps for this statement
      for (int m=1; m<=stm.map.partials.length(); m++)
	stm.map.partials[m].mapping =
	  Join(copy(Tinv), stm.map.partials[m].mapping);
    }

  simplify_everything(info);
  return info;
}



Tuple<stm_info> &Set_MMap(Tuple<stm_info> &info, int stm, const MMap &new_MMap)
{
  info[stm].map = new_MMap;

  Relation IS = info[stm].IS;
  MMap &m = info[stm].map;
  for (int p=1; p<=m.partials.length(); p++)
    m.partials[p].mapping = Restrict_Domain(m.partials[p].mapping, copy(IS));

  simplify_everything(info);
  return info;
}


//
// In Replace_Stm_With_Stms, we must set up the new "read" information.
// Whenever we find a DF from "which", put in a collection of reads
// instead.  Always transform "to" iteration space of every data flow
// with "my_trans", in case this is a data flow to an IS that is being split.
//
static void setup_reads(Tuple<Read> &new_reads, Tuple<Read> &old_reads, int which, int n_added, Tuple<Relation> &trans_relations, Relation &my_trans)
{
  new_reads = Tuple<Read>(old_reads.length());

  // replace each partial read from statement "which" with
  // a collection of partial reads from which..which+n_added

  for (int rn=1; rn<=old_reads.size(); rn++)
    {
      Read &r = old_reads[rn];
      int changes=0;
      {   // extra braces apparently avoid Visual C++ bug
      for (int pn=1; pn<=r.partials.length(); pn++)
	{
	  if (r.partials[pn].from == which)
	    changes++;
	}
      }

      new_reads[rn].partials = Tuple<PartialRead>(r.partials.length() +
						   changes * n_added);
      Tuple<PartialRead> &newp = new_reads[rn].partials;
      for (int pn=1, new_pn=1; pn<=r.partials.size(); pn++)
	{
	  int from = r.partials[pn].from;
	  if (from == which)
	    {
	      // need to do added + 1 (for the original)
	      for (int a = 0; a<=n_added; a++)
		{
		  newp[new_pn].from = r.partials[pn].from+a;
		  newp[new_pn++].dataFlow =
		    Join(Inverse(copy(trans_relations[a+1])),
			 Join(copy(r.partials[pn].dataFlow), copy(my_trans)));
		}
	    }
	  else
	    {
	      newp[new_pn].from = from + (from > which ? n_added : 0);
	      newp[new_pn++].dataFlow =
		Join(copy(r.partials[pn].dataFlow), copy(my_trans));
	    }
	}
    }
}


// Replace all references to info[stm] with a group of references
// to info[stm...stm+mask_relations.length()-1],
// each of which has its IS joined with the corresponding trans_relations.
// This is needed for either peeling or unrolling a loop.

static void Replace_Stm_With_Stms(Tuple<stm_info> &info, int which, int n_added, Tuple <Relation> &trans_relations)
{
  Tuple<stm_info> result(info.length()+n_added);
  assert(n_added == trans_relations.length()-1);

  // COULD ADD SOME CODE TO CHECK THE LEGALITY OF THE TRANSFORM...

  for (int s=1; s<=info.length(); s++)
    {
      // update the iteration space
      if (s == which)
	{
	  // set up replacement for s, and n_added more
	  for (int n=0; n <= n_added; n++)
	    {
	      int new_s = s+n;
	      result[new_s].IS = Join(copy(info[s].IS),
				      copy(trans_relations[n+1]));
	      result[new_s].map = info[s].map;
	      for (int pm=1 ; pm <= result[new_s].map.partials.length(); pm++)
		{
		  result[new_s].map.partials[pm].mapping = 
		    Join(Inverse(copy(trans_relations[n+1])),
			 result[new_s].map.partials[pm].mapping);
		}
	      result[new_s].stm = info[s].stm;
	      setup_reads(result[new_s].read, info[s].read, which, n_added, trans_relations, trans_relations[n+1]);
	    }
	}
      else
	{
	  int new_s = s < which ? s : (s+n_added);
	  result[new_s].IS = info[s].IS;
	  result[new_s].map = info[s].map;
	  result[new_s].stm = info[s].stm;
	  Relation id = Identity(trans_relations[1].n_out());
	  setup_reads(result[new_s].read, info[s].read, which, n_added, trans_relations, id);
	}
    }
  info = result;
}


// this relation adds the "u" level to all statements
// e.g. [tb,s,tt] -> [tb,s,0,tt]
static Relation extender(int level, int old_size)
{
  Relation extender(old_size, old_size+1);
  F_And *a = extender.add_and();
  for (int l = 1; l<=old_size; l++)
    {
      EQ_Handle h = a->add_EQ();
      h.update_coef(extender.input_var(l), -1);
      if (l<=level)
	h.update_coef(extender.output_var(l), 1);
      else
	h.update_coef(extender.output_var(l+1), 1);
    }
  EQ_Handle h = a->add_EQ();
  h.update_coef(extender.output_var(level+1),1);
  h.update_const(1);  // u=-1 by default
  extender.finalize();

  return extender;
}



static bool Extend_All_IS(Tuple<stm_info> &info, int stm, int level, int old_size, bool new_level_before)
{
  stm_info &s = info[stm];

  // Check to make sure other statements differ at an outer level:
  // IS_mask contains stm.IS, but should not contain any other IS,
  // if unrolling loop "level" is legal.
  // IS_mask is later used to construct the unrolling transformation
  Relation outer = Extend_Domain(Identity(level-1),old_size-(level-1));
  Relation IS_mask = Extend_Set(Join(copy(s.IS),outer),old_size-(level-1));

  IS_mask.simplify();

  for (int o=1; o<=info.length(); o++)
    {
      if (o != stm)
	{
	  // these should be fast if there are differing constant levels
	  if (Intersection(copy(info[o].IS),copy(IS_mask)).is_satisfiable())
	    {
	      cerr << "Can't extend level " << level <<
		" of statement " << stm <<
		" because of interference from I.S. of statement " << o <<endl;
	      cerr << "Statement " << o << "'s I.S. is " << info[o].IS;
	      return false;
	    }
	}
      else
	{
	  assert(Must_Be_Subset(copy(s.IS),copy(IS_mask)));
	}
    }

  // Now that we know its legal, add the new level
  // "extender" just adds a level thats all -1's
  Trans_IS(info, extender(level - (new_level_before?1:0), old_size));

  return true;
}


// Make a relation giving one "phase" of the unrolled loop:
// The IS is transformed by
//   * replacing "level" with "(level-u)/factor"
//   * replacing the -1 at the next level with u.
// e.g. [1,l,-1,1] --> [1,(l-u)/f,u,1]
// that is,   In[l] = (Out[l] + u) * factor && Out[l+1] - u == 0

static Relation unroll(int new_size, int level, int factor, int u)
{
  Relation unroll(new_size, new_size);
  F_And *a = unroll.add_and();
  for (int l=1; l<=new_size; l++)
    {
      EQ_Handle h = a->add_EQ();
      if (l==level)
	{
	  h.update_coef(unroll.input_var(l),  -1);
	  h.update_coef(unroll.output_var(l), factor);
	  h.update_const(u);
	}
      else if (l == level+1)
	{
	  h.update_coef(unroll.output_var(l), 1);
	  h.update_const(-u);
	}
      else
	{
	  h.update_coef(unroll.input_var(l), -1);
	  h.update_coef(unroll.output_var(l), 1);
	}
    }
  unroll.finalize();
  return unroll;
}

// Comments below refer to example [2,tb,s,tt] -> [2,tb,s',u,tt]
//  where s' = (s-u)/factor and u=s%factor
// Other statements must differ in an outer level (i.e. "2"),
//  so we don't have to worry about what to do with them -
//  we just add a constant level for u (=-1).
//
Tuple<stm_info> &Unroll_One_IS(Tuple<stm_info> &info, int stm, int level, int factor)
{
  stm_info &s = info[stm];
  int old_size = s.IS.n_set();
  int new_size = old_size+1;

  // First, extend all iteration spaces
  if (!Extend_All_IS(info, stm, level, old_size, false))
    return info;

  // Now we've extended the system by adding a new "level" of all -1's
  // We need to replace all occurances of the statement to be unrolled
  // with a collection of the unrolled pieces.
  // Unrolled piece "u" is transformed by
  //   * replacing "level" with "(level-u)/factor"
  //   * replacing the -1 at the next level with u.
  // that is, Out[l] * factor = In[l] - Out[l+1] && Out[l+1] = u

  Tuple<Relation> unroll_relations(factor);
  for (int u=0; u<factor; u++)
    {
      unroll_relations[u+1] = unroll(new_size, level, factor, u);
    }
  Replace_Stm_With_Stms(info, stm, factor-1, unroll_relations);

  simplify_everything(info);
  return info;
}



// create a relation of size n
// that leaves all levels alone except "level", which is mapped to "c"
static Relation Remap_To_Const(int size, int level, int c)
{
  Relation remap(size, size);
  F_And *a = remap.add_and();
  for (int l=1; l<=size; l++)
    {
      EQ_Handle h = a->add_EQ();
      if (l==level)
	{
	  h.update_coef(remap.output_var(l), 1);
	  h.update_const(-c);
	}
      else
	{
	  h.update_coef(remap.input_var(l), -1);
	  h.update_coef(remap.output_var(l), 1);
	}
    }
  remap.finalize();
  return remap;
}

//
// Here we basically just have to call the "replace..." function,
// but turn the sets into relations that can be joined to perform intersection
//

Tuple<stm_info> &Peel_One_IS(Tuple<stm_info> &info, int stm, int level, Relation &peel)
{
  stm_info &s = info[stm];
  int old_size = s.IS.n_set();
  int new_size = old_size+1;

  // First, extend all iteration spaces
  if (!Extend_All_IS(info, stm, level, old_size, true))
    return info;

  Relation newPeel = Join(copy(peel), extender(level-1, old_size));
  Relation notNewPeel = Join(Complement(copy(peel)), extender(level-1, old_size));

  Tuple<Relation> peel_relations(2);
  peel_relations[1] = Restrict_Domain(Remap_To_Const(new_size,level,0), newPeel);
  peel_relations[2] = Restrict_Domain(Remap_To_Const(new_size,level,1), notNewPeel);

  Replace_Stm_With_Stms(info, stm, 1, peel_relations);

  simplify_everything(info);
  return info;
}


Tuple<stm_info> &Peel_One_IS(Tuple<stm_info> &info, int stm, int level, Relation &peel_before, Relation &peel_after)
{
  stm_info &s = info[stm];
  int old_size = s.IS.n_set();
  int new_size = old_size+1;

  // First, extend all iteration spaces
  if (!Extend_All_IS(info, stm, level, old_size, true))
    return info;

  Relation newPeel1 = Join(copy(peel_before), extender(level-1, old_size));
  Relation newPeel3 = Join(copy(peel_after),  extender(level-1, old_size));
  Relation newPeel2 = Join(Complement(Union(copy(peel_before),
					    copy(peel_after))),
			   extender(level-1, old_size));

  Tuple<Relation> peel_relations(3);
  peel_relations[1] = Restrict_Domain(Remap_To_Const(new_size,level,0), newPeel1);
  peel_relations[2] = Restrict_Domain(Remap_To_Const(new_size,level,1), newPeel2);
  peel_relations[3] = Restrict_Domain(Remap_To_Const(new_size,level,2), newPeel3);

  Replace_Stm_With_Stms(info, stm, 2, peel_relations);

  simplify_everything(info);
  return info;
}



/* useful from debugger */
void DoDebug3(const char *message, Relation &r)
{
  fprintf(DebugFile, "%s: ", message);
  copy(r).print_with_subs(DebugFile);
  fprintf(DebugFile, "%s (prefix print)\n", message);
  r.prefix_print(DebugFile);
}

void DoDebug2(char *message, const Tuple<stm_info> &info)
{
  DoDebug(message, -1, info, Relation::True(0));
}

void DoDebug(const char *message, int effort, const Tuple<stm_info> &info, const Relation &known)
{
   String result="";
   Tuple<stm_info> &noconst = (Tuple<stm_info> &)info;
   Relation &known_noconst  = (Relation &)known;

   result += debug_mark + message + "\n";
   result += debug_mark + "  effort     = " + itoS(effort) + "\n";
   result += debug_mark + "  known      = " +
   known_noconst.print_with_subs_to_string();
   result += debug_mark + "  statements = \n";
   for (int s = 1; s <= noconst.size(); s++)
     {
      result += debug_mark + "    statement " + itoS(s) + "\n";
      result += debug_mark + "\tstm = \"" + noconst[s].stm + "\"\n";
      result += debug_mark + "\tIS  = " + noconst[s].IS.print_with_subs_to_string();
      result += debug_mark + "\tmap has " + itoS(noconst[s].map.partials.size()) + " partial write(s):\n";
      for (int p = 1; p <= noconst[s].map.partials.size(); p++)
	{
	 result += debug_mark + "\t# " + itoS(p) + ":  var = " + noconst[s].map.partials[p].var +
		    " mapping = " + noconst[s].map.partials[p].mapping.print_with_subs_to_string();
	}
      for (int r = 1; r <= noconst[s].read.size(); r++)
	{
	 result += debug_mark + "\tread " + itoS(r) + " has " +
		itoS(noconst[s].read[r].partials.size()) + " partials read(s):\n";
	 for (int p = 1; p <= noconst[s].read[r].partials.size(); p++)
	   {
	    result += debug_mark + "\t  # " + itoS(p) + ":  from = " + itoS(noconst[s].read[r].partials[p].from) +
		    " dataflow = " + noconst[s].read[r].partials[p].dataFlow.print_with_subs_to_string();
	   }
	}
     }
   result += debug_mark + "---------------------------\n";

   fprintf(DebugFile, "%s", (const char *) result);
}

} // end namespace omega
