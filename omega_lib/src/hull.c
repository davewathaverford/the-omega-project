/* $Id: hull.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <omega.h>
#include <omega/farkas.h>
#include <omega/hull.h>
#include <basic/Bag.h>
#include <basic/Map.h>

namespace omega {

int hull_debug = 0; 

Relation DecoupledConvexHull(NOT_CONST Relation &R) {
  Relation S = Approximate(consume_and_regurgitate(R));
  if (S.has_single_conjunct()) return S;
  return Farkas(Farkas(S,Decoupled_Farkas),Convex_Combination_Farkas);
}

Relation ConvexHull(NOT_CONST Relation &R) {
  Relation S = Approximate(consume_and_regurgitate(R));
  if (S.has_single_conjunct()) return S;
  return Farkas(Farkas(S,Basic_Farkas),Convex_Combination_Farkas);
}

Relation FastTightHull(NOT_CONST Relation &input_R, 
		       NOT_CONST Relation &input_H) {
  Relation R = Approximate(consume_and_regurgitate(input_R));
  Relation H = Approximate(consume_and_regurgitate(input_H));
#ifdef DEBUG
  Relation orig_R = R;
#endif
  if (hull_debug) {
    fprintf(DebugFile,"[ Computing FastTightHull of:\n");
    R.prefix_print(DebugFile);
    fprintf(DebugFile,"given known hull of:\n");
    H.prefix_print(DebugFile);
  }

  if (!H.has_single_conjunct())  {
    if (hull_debug) 
      fprintf(DebugFile,
	      "] bailing out of FastTightHull, known hull not convex\n");
    return H;
  }
  if (!H.is_obvious_tautology()) {
    R = Gist(R,copy(H));
    R.simplify(1,0);
  }
  if (R.has_single_conjunct()) {
    R = Intersection(R,H);
    if (hull_debug)  {
      fprintf(DebugFile,
	      "] quick easy answer to FastTightHull\n");
      R.prefix_print(DebugFile);
    }
    return R;
  }
  if (R.has_local(coefficient_of_constant_term)) {
    if (hull_debug)  {
      fprintf(DebugFile,
	      "] Can't handle recursive application of Farkas lemma\n");
    }
    return H;
  }
		
  if (hull_debug) {
    fprintf(DebugFile,"Gist of R given H is:\n");
    R.prefix_print(DebugFile);
  }
  if (1) {
    Set<Variable_ID> vars;
    int conjuncts = 0;
    for (DNF_Iterator s(R.query_DNF()); s.live(); s.next())  {
      conjuncts++;
      for (Variable_ID_Iterator v(*((*s)->variables())); v.live(); v++)  {
	  bool found = false;
	  for (EQ_Iterator eq = (*s)->EQs(); eq.live(); eq.next())
	    if ((*eq).get_coef(*v) != 0) {
		if (!found) vars.insert(*v);
		found = true;
		break;
		}
	  if (!found) for (GEQ_Iterator geq = (*s)->GEQs(); 
	       geq.live(); geq.next())
	    if ((*geq).get_coef(*v) != 0) {
		if (!found) vars.insert(*v);
		found = true;
		break;
	    }
	}
      }

     
    // We now know which variables appear in R
    if (hull_debug) {
      fprintf(DebugFile,"Variables we need a better hull on are: ");
      foreach(v,Variable_ID,vars,
	      fprintf(DebugFile," %s",v->char_name()));
      fprintf(DebugFile,"\n");
    }
    Conjunct *c = H.single_conjunct();
    int total=0;
    int copied = 0;
    for (EQ_Iterator eq = c->EQs(); eq.live(); eq.next()) {
      total++;
      foreach(v,Variable_ID,vars,
	      if ((*eq).get_coef(v) != 0) {
		R.and_with_EQ(*eq);
		copied++;
		break; // out of variable loop
	      }
	      );
    }
    for (GEQ_Iterator geq = c->GEQs(); geq.live(); geq.next()) {
      total++;
      foreach(v,Variable_ID,vars,
	      if ((*geq).get_coef(v) != 0) {
		R.and_with_GEQ(*geq);
		copied++;
		break; // out of variable loop
	      }
	      );
    }
    if (copied < total) {
      R = Approximate(R);

      if (hull_debug) {	
	fprintf(DebugFile,"Decomposed relation, copied only %d of %d constraints\n",copied,total);
	fprintf(DebugFile,"Original R:\n");
	R.prefix_print(DebugFile);
	fprintf(DebugFile,"Known hull:\n");
	H.prefix_print(DebugFile);
	fprintf(DebugFile,"New R:\n");
	R.prefix_print(DebugFile);
      }
    }

  }

  Relation F = Farkas(copy(R),Basic_Farkas);
  if (hull_debug)  
    fprintf(DebugFile,"Farkas Difficulty = " coef_fmt "\n",
	    farkasDifficulty);
  if (farkasDifficulty > 260) {
    if (hull_debug)  {
      fprintf(DebugFile,
	      "] bailing out, farkas is way too complex\n");
      fprintf(DebugFile,"Farkas:\n");
      F.prefix_print(DebugFile);
    }
    return H;
  }
  else if (farkasDifficulty > 130) {
    // Bail out
    if (hull_debug)  {
      fprintf(DebugFile, coef_fmt " non-zeros in original farkas\n",
	      farkasDifficulty);
    }
    Relation tmp = Farkas(R,Decoupled_Farkas);
	   
    if (hull_debug)  {
      fprintf(DebugFile, coef_fmt " non-zeros in decoupled farkas\n",
	      farkasDifficulty);
    }
    if (farkasDifficulty > 260)  {
      if (hull_debug)  {
        fprintf(DebugFile,
	        "] bailing out, farkas is way too complex\n");
        fprintf(DebugFile,"Farkas:\n");
        F.prefix_print(DebugFile);
      }
      return H;
    }
    else {
      if (farkasDifficulty > 130) 
      R = Intersection(H, Farkas(tmp,Affine_Combination_Farkas));
    else R = Intersection(H,
			  Intersection(Farkas(tmp,Convex_Combination_Farkas),
				       Farkas(F,Affine_Combination_Farkas)));
    if (hull_debug)  {
      fprintf(DebugFile,
	      "] bailing out, farkas is too complex, using affine hull\n");
      fprintf(DebugFile,"Farkas:\n");
      F.prefix_print(DebugFile);
      fprintf(DebugFile,"Affine hull:\n");
      R.prefix_print(DebugFile);
    }
    return R;
  }}
	
  R = Intersection(H,
		   Farkas(F,Convex_Combination_Farkas));
  if (hull_debug)  {
    fprintf(DebugFile,
	    "] Result of FastTightHull:\n");
    R.prefix_print(DebugFile);
  }
  return R;
}

Relation AffineHull(NOT_CONST Relation &R) {
  return Farkas(Farkas(R,Basic_Farkas),Affine_Combination_Farkas);
}

Relation LinearHull(NOT_CONST Relation &R) {
  return Farkas(Farkas(R,Basic_Farkas),Linear_Combination_Farkas);
}

Relation ConicHull(NOT_CONST Relation &R) {
  return Farkas(Farkas(R,Basic_Farkas),Positive_Combination_Farkas);
}


Relation ConicClosure (NOT_CONST Relation &R)
{
  assert(R.n_inp() == R.n_out());
  int n = R.n_inp();

  return DeltasToRelation(ConicHull(Deltas(R)),n,n);

} /* ConicClosure */


bool parallel(const GEQ_Handle &g1, const GEQ_Handle &g2) {
  for(Constr_Vars_Iter cvi(g1, false); cvi; cvi++) {
    coef_t c1 = (*cvi).coef;
    coef_t c2 = g2.get_coef((*cvi).var);
    if (c1 != c2) return false;
  }
  {
    for(Constr_Vars_Iter cvi(g2, false); cvi; cvi++) {
      coef_t c1 = g1.get_coef((*cvi).var);
      coef_t c2 = (*cvi).coef;
      if (c1 != c2) return false;
    }
  }
  return true;
}

bool hull(const EQ_Handle &e, const GEQ_Handle &g, coef_t &hull) {
  int sign = 0;
  for(Constr_Vars_Iter cvi(e, false); cvi; cvi++) {
    coef_t c1 = (*cvi).coef;
    coef_t c2 = g.get_coef((*cvi).var);
    if (sign == 0) sign = (c1*c2>=0?1:-1);
    if (sign*c1 != c2) return false;
  }
  assert(sign != 0);
  {for(Constr_Vars_Iter cvi(g, false); cvi; cvi++) {
    coef_t c1 = e.get_coef((*cvi).var);
    coef_t c2 = (*cvi).coef;
    if (sign*c1 != c2) return false;
  }
  }
  hull = max(sign * e.get_const(), g.get_const());
  if (hull_debug) {
    fprintf(DebugFile,"Hull of:\n %s\n",(const char *)e.print_to_string());
    fprintf(DebugFile," %s\n",(const char *)g.print_to_string());
    fprintf(DebugFile,"is " coef_fmt "\n\n",hull);
  }
  return true;
}

bool eq(const EQ_Handle &e1, const EQ_Handle &e2) {
  int sign = 0;
  for(Constr_Vars_Iter cvi(e1, false); cvi; cvi++) {
    coef_t c1 = (*cvi).coef;
    coef_t c2 = e2.get_coef((*cvi).var);
    if (sign == 0) sign = (c1*c2>=0?1:-1);
    if (sign*c1 != c2) return false;
  }
  assert(sign != 0);
  {for(Constr_Vars_Iter cvi(e2, false); cvi; cvi++) {
    coef_t c1 = e1.get_coef((*cvi).var);
    coef_t c2 = (*cvi).coef;
    if (sign*c1 != c2) return false;
  }
  }
  return sign * e1.get_const() == e2.get_const();
}

 


static Relation QuickHull(Tuple<Relation> &Rs) {
  assert(!Rs.empty());
  if (Rs.size() == 1) return Rs[1];

  Relation result = Relation::True(Rs[1]);
  result.copy_names(Rs[1]);

  use_ugly_names++; 

  Relation R = copy(Rs[1]);
  for (int i = 2; i <= Rs.size(); i++) 
    R = Union(R,copy(Rs[i]));

#if 0
  if (!R.is_set()) {
    if (R.n_inp() == R.n_out()) {
      Relation AC = DeltasToRelation(Hull(Deltas(copy(R),
						 min(R.n_inp(),R.n_out()))),
				     R.n_inp(),R.n_out());
      Relation dH = Hull(Domain(copy(R)),false);
      Relation rH = Hull(Range(copy(R)),false);
      result = Intersection(AC,Cross_Product(dH,rH)); 
    }
    else {
      Relation dH = Hull(Domain(copy(R)),false);
      Relation rH = Hull(Range(copy(R)),false);
      result = Cross_Product(dH,rH); 
      assert(Must_Be_Subset(copy(R),copy(result)));
    }
  }

#endif
	
  Conjunct *first;

  if (hull_debug > 1) 
    Rs[1].prefix_print(DebugFile);
  Rs[1] = EQs_to_GEQs(Rs[1]);
  if (hull_debug > 1) 
    for (int i = 1; i <= Rs.size(); i++) {
      fprintf(DebugFile,"#%d \n",i);
      Rs[i].prefix_print(DebugFile);
    }
  first = Rs[1].single_conjunct();

  for (GEQ_Iterator candidate(first->GEQs()); candidate.live(); candidate.next()) {
    coef_t maxConstantTerm = (*candidate).get_const();
    bool found = 1; 
    if (hull_debug > 1) {
      fprintf(DebugFile,"searching for bound on:\n %s\n",
	      (const char *)(*candidate).print_to_string());
    }
    for (int i = 2; i <= Rs.size(); i++) {
      Conjunct *C = Rs[i].single_conjunct();
      bool found_for_i = false;
      for (GEQ_Iterator target(C->GEQs()); target.live(); target.next()) {
	if (hull_debug > 2) {
	  fprintf(DebugFile,"candidate:\n %s\n",
		  (const char *)(*candidate).print_to_string());
	  fprintf(DebugFile,"target:\n %s\n",
		  (const char *)(*target).print_to_string());
	}
	if (parallel(*candidate,*target)) {
	  if (hull_debug > 1)
	    fprintf(DebugFile,"Found bound:\n %s\n",
		    (const char *)(*target).print_to_string());
	  maxConstantTerm = max(maxConstantTerm,(*target).get_const());
	  found_for_i = true;
	  break;
	}
      };
      if (!found_for_i) {
	for (EQ_Iterator target_e(C->EQs()); target_e.live(); target_e.next()) {
	  coef_t h;
	  if (hull(*target_e,*candidate,h)) {
	    if (hull_debug > 1)
	      fprintf(DebugFile,"Found bound of " coef_fmt ":\n %s\n",
		      h,(const char *)(*target_e).print_to_string());
	    maxConstantTerm = max(maxConstantTerm,h);
	    found_for_i = true;
	    break;
	  }
	};
	if (!found_for_i) {
	  if (hull_debug > 1) {
	    fprintf(DebugFile,"No bound found in:\n");
	    fprintf(DebugFile,Rs[i].print_with_subs_to_string());
	  }
	  //if nothing found 
	  found = 0;
	  break;
	}
      }
    }
	  
    if (found) {
      GEQ_Handle  h = result.and_with_GEQ();
      copy_constraint(h,*candidate);
      if (hull_debug > 1)
	fprintf(DebugFile,"Setting constant term to " coef_fmt " in\n %s\n",
		maxConstantTerm,
		(const char *) (h).print_to_string());
      h.update_const(maxConstantTerm - (*candidate).get_const());
      if (hull_debug > 1)
	fprintf(DebugFile,"Updated constraint is\n %s\n",
		(const char *)
		(h).print_to_string());
    }
  }


  for (EQ_Iterator candidate_eq(first->EQs()); candidate_eq.live(); candidate_eq.next()) {
    bool found = true;
    for (int i = 2; i <= Rs.size(); i++) {
      Conjunct *C = Rs[i].single_conjunct();
      bool found_for_i = false;

      for (EQ_Iterator target(C->EQs()); target.live(); target.next()) {
	if (eq(*candidate_eq,*target)) {
	  found_for_i = true;
	  break;
	}
      };
      if (!found_for_i) {
	//if nothing found 
	found = false;
	break;
      }
    }
	  
    if (found) {
      EQ_Handle  h = result.and_with_EQ();
      copy_constraint(h,*candidate_eq);
      if (hull_debug > 1)
	fprintf(DebugFile,"Adding eq constraint: %s\n",
		(const char *)
		(h).print_to_string());
    }
  }




  use_ugly_names--;
  if (hull_debug > 1) {
    fprintf(DebugFile,"quick hull is of:");
    result.print_with_subs(DebugFile);
  }
  return result;
}

void printRs(Tuple<Relation> &Rs) {
  fprintf(DebugFile,"Rs:\n");
  for (int i = 1; i <= Rs.size(); i++)
    fprintf(DebugFile,"#%d : %s\n",i,
	    (const char *) Rs[i].print_with_subs_to_string());
}

Relation BetterHull(Tuple<Relation> &Rs, bool stridesAllowed, bool checkSubsets,
		    NOT_CONST Relation &input_knownHull = Relation::Null()) {
  Relation knownHull = consume_and_regurgitate(input_knownHull);
  static int OMEGA_WHINGE = -1;
  if (OMEGA_WHINGE < 0)
    {
      OMEGA_WHINGE = getenv("OMEGA_WHINGE") ? atoi(getenv("OMEGA_WHINGE")) : 0;
    }
  assert(!Rs.empty());
  if (Rs.size() == 1) {
    if (stridesAllowed) return Rs[1];
    else return Approximate(Rs[1]);
  }

  if (checkSubsets) {
    Tuple<bool> live(Rs.size());
    if (hull_debug) {
      fprintf(DebugFile,"Checking subsets in hull computation:\n");
      printRs(Rs);
    }
    int i;
    for(i=1;i <=Rs.size(); i++) live[i] = true;
    for(i=1;i <=Rs.size(); i++) 
      for(int j=1;j <=Rs.size(); j++) if (i != j && live[j]) {
	if (hull_debug) fprintf(DebugFile,"checking %d Is_Obvious_Subset %d\n",i,j);
	if (Is_Obvious_Subset(copy(Rs[i]),copy(Rs[j]))) {
	  if (hull_debug) fprintf(DebugFile,"yes...\n");
	  live[i] = false;
	  break;
	}
      }
    for(i=1;i <=Rs.size(); i++) if (!live[i]) {
      if (i < Rs.size()) {
	Rs[i] = Rs[Rs.size()];
	live[i] = live[Rs.size()];
      };
      Rs[Rs.size()] = Relation();
      Rs.delete_last();
      i--;
    }
  }
  Relation hull;
  if (hull_debug) {
    fprintf(DebugFile,"Better Hull:\n");
    printRs(Rs);
    fprintf(DebugFile,"known hull: %s\n",
	    (const char *)knownHull.print_with_subs_to_string());
  };
  if (knownHull.is_null()) hull = QuickHull(Rs);
  else hull = Intersection(QuickHull(Rs),knownHull);
  hull.simplify();
  if (hull_debug) {
    fprintf(DebugFile,"quick hull: %s\n",
	    (const char *)hull.print_with_subs_to_string());
  };

  


  Relation orig = Relation::False(Rs[1]);
  int i;
  for (i = 1; i <= Rs.size(); i++) 
    orig = Union(orig,copy(Rs[i]));

  orig.simplify();

  for (i = 1; i <= Rs.size(); i++) {
    if (!hull.is_obvious_tautology()) Rs[i] = Gist(Rs[i],copy(hull));
    Rs[i].simplify();
    if (Rs[i].is_obvious_tautology()) return hull;
    if (Rs[i].has_single_conjunct()) {
    Rs[i] = EQs_to_GEQs(Rs[i],true);
    if (hull_debug) {
      fprintf(DebugFile,"Checking for hull constraints in:\n  %s\n",
	      (const char *)Rs[i].print_with_subs_to_string());
    }
    Conjunct *c = Rs[i].single_conjunct();
    for (GEQ_Iterator g(c->GEQs()); g.live(); g.next()) {
      Relation tmp = Relation::True(Rs[i]);
      tmp.and_with_GEQ(*g);
      if (!Difference(copy(orig),tmp).is_upper_bound_satisfiable()) 
	hull.and_with_GEQ(*g);
    }
    for (EQ_Iterator e(c->EQs()); e.live(); e.next()) {
      Relation tmp = Relation::True(Rs[i]);
      tmp.and_with_EQ(*e);
      if (!Difference(copy(orig),tmp).is_upper_bound_satisfiable()) 
	hull.and_with_EQ(*e);
    }
    }
  }


  hull = FastTightHull(orig,hull);

  assert(hull.has_single_conjunct());

  if (stridesAllowed) return hull;
  else return Approximate(hull);

}



Relation  Hull(NOT_CONST Relation &S, 
	       bool stridesAllowed,
	       int effort,
	       NOT_CONST Relation &knownHull
	       ) {
  Relation R = consume_and_regurgitate(S);
  R.simplify(1,0);
  if (!R.is_upper_bound_satisfiable()) return R;
  Tuple<Relation> Rs;
  for (DNF_Iterator c(R.query_DNF()); c.live(); ) {
    Rs.append(Relation(R,c.curr()));
    c.next();
  };
  if (effort == 1)
    return BetterHull(Rs,stridesAllowed,false,knownHull);
  else
    return QuickHull(Rs);
}

Relation Hull(Tuple<Relation> &Rs, 
	      Tuple<int> &validMask, 
	      int effort, 
	      bool stridesAllowed,
	      NOT_CONST Relation &knownHull
	      ) {
  // Use relation of index i only when validMask[i] != 0
  Tuple<Relation> Rs2;
  for(int i = 1; i <= Rs.size(); i++) {
    if (validMask[i]) {
      Rs[i].simplify();
      for (DNF_Iterator c(Rs[i].query_DNF()); c.live(); ) {
	Rs2.append(Relation(Rs[i],c.curr()));
	c.next();
      };
    }
  }
  assert(effort == 0 || effort == 1);
  if (effort == 1)
    return BetterHull(Rs2,stridesAllowed,true,knownHull);
  else
    return QuickHull(Rs2);
}


Relation CheckForConvexPairs(NOT_CONST Relation &S) {
  Relation R = consume_and_regurgitate(S);
  Relation hull = FastTightHull(copy(R),Relation::True(R));
  R.simplify(1,0);
  if (!R.is_upper_bound_satisfiable() || R.number_of_conjuncts() < 2) return R;
  Tuple<Relation> Rs;
  for (DNF_Iterator c(R.query_DNF()); c.live(); ) {
    Rs.append(Relation(R,c.curr()));
    c.next();
  };

  bool * dead = new bool[Rs.size()+1];
  int i;
  for(i = 1; i<=Rs.size();i++) dead[i] = false;

  for(i = 1; i<=Rs.size();i++) if (!dead[i]) 
    for(int j = i+1; j<=Rs.size();j++) if (!dead[j]) {
      if (hull_debug)  {
	fprintf(DebugFile,"Comparing #%d and %d\n",i,j);
      }
      Relation U = Union(copy(Rs[i]),copy(Rs[j]));
      Relation H_ij = FastTightHull(copy(U),copy(hull));
      if (!Difference(copy(H_ij),U).is_upper_bound_satisfiable()) {
	Rs[i] = H_ij;
	dead[j] = true;
	if (hull_debug)  {
	  fprintf(DebugFile,"Combined them\n");
	}
      }
    }
  i = 1;
  while(i<=Rs.size() && dead[i]) i++;
  assert(i<=Rs.size());
  R = Rs[i];
  i++;
  for(; i<=Rs.size();i++) if (!dead[i]) 
    R = Union(R,Rs[i]);
  delete dead;
  return R;
}

	

		
Relation VennDiagramForm(NOT_CONST Relation &Context_In,
	      Tuple<Relation> &Rs, 
	      int next,
	      bool anyPositives, 
	      int weight) {

     Relation Context = consume_and_regurgitate(Context_In);
    if (hull_debug) {
      fprintf(DebugFile,"[VennDiagramForm, next = %d, anyPositives = %d, weight = %d \n",
			next,anyPositives,weight);
      fprintf(DebugFile,"context:\n");
	Context.prefix_print(DebugFile);
    }
	     if (anyPositives && weight > 3) {
			Context.simplify();
			if (!Context.is_upper_bound_satisfiable())  {
			    if (hull_debug) 
			      fprintf(DebugFile,"] not satisfiable\n");
			    return Context;
			    }
			weight = 0;
			}
	     if (next > Rs.size()) {
		if (!anyPositives) {
			if (hull_debug) 
			      fprintf(DebugFile,"] no positives\n");
			return Relation::False(Context);
			}
		Context.simplify();
		if (hull_debug)  {
			      fprintf(DebugFile,"] answer is:\n");
			      Context.prefix_print(DebugFile);
			      }
	        return Context;
		}
	     Relation Pos = VennDiagramForm(Intersection(copy(Context),copy(Rs[next])),
					Rs,
					next+1,
					true,
					weight+2);
	     Relation Neg = VennDiagramForm(Difference(Context,copy(Rs[next])),
					Rs,
					next+1,
					anyPositives,
					weight+1);
		if (hull_debug)  {
		      fprintf(DebugFile,"] VennDiagramForm\n");
		      fprintf(DebugFile,"pos part:\n");
		      Pos.prefix_print(DebugFile);
		      fprintf(DebugFile,"neg part:\n");
		      Neg.prefix_print(DebugFile);
		}
	     return Union(Pos,Neg);
	     }
		

			
			
	
Relation VennDiagramForm(
		Tuple<Relation> &Rs, 
		NOT_CONST Relation &Context_In) {
        Relation Context = consume_and_regurgitate(Context_In);
	if (Context.is_null()) Context = Relation::True(Rs[1]);
        if (hull_debug) {
           fprintf(DebugFile,"Starting computation of VennDiagramForm\n");
           	fprintf(DebugFile,"Context:\n");
	   Context.prefix_print(DebugFile);
	   for(int i = 1; i <= Rs.size(); i++) {
           	fprintf(DebugFile,"#%d:\n",i);
		Rs[i].prefix_print(DebugFile);
		}
	  }
	return VennDiagramForm(Context,Rs,1,false,0);
	}
	
Relation VennDiagramForm(
		NOT_CONST Relation &R_In, 
		NOT_CONST Relation &Context_In) {
		
        Relation R = consume_and_regurgitate(R_In);
        Relation Context = consume_and_regurgitate(Context_In);
	Tuple<Relation> Rs;
	  for (DNF_Iterator c(R.query_DNF()); c.live(); ) {
	    Rs.append(Relation(R,c.curr()));
	    c.next();
	  };
	return VennDiagramForm(Rs,Context);
	}


Relation CheckForConvexRepresentation(NOT_CONST Relation &R_In) {
        Relation R = consume_and_regurgitate(R_In);
	Relation h = Hull(copy(R));
	if (!Difference(copy(h),copy(R)).is_upper_bound_satisfiable())
		return h;
	else
		return R;
	}




} // end of namespace omega
