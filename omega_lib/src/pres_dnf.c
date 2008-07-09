/* $Id: pres_dnf.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <basic/bool.h>
#include <basic/Bag.h>
#include <omega/pres_dnf.h>
#include <omega/pres_conj.h>
#include <omega/pres_tree.h>  /* all DNFize functions are here */
#include <omega/Relation.h>
#include <omega/omega_i.h>

namespace omega {

void DNF::remap() {
  for(DNF_Iterator DI(this); DI.live(); DI.next()) {
    Conjunct *C = DI.curr();
    C->remap();
    }
}


//
// DNF1 & DNF2 -> DNF.
// Free arguments.
//
DNF* DNF_and_DNF(DNF* dnf1, DNF* dnf2) {
  DNF* new_dnf = new DNF;
  for(DNF_Iterator p(dnf2); p.live(); p.next()) {
    new_dnf->join_DNF(DNF_and_conj(dnf1, p.curr()));
  }
  delete dnf1;
  delete dnf2;
  if(new_dnf->length() > 1) {
    new_dnf->simplify();
  }

  if(pres_debug) {
    fprintf(DebugFile, "+++ DNF_and_DNF OUT +++\n");
    new_dnf->prefix_print(DebugFile);
  }
  return(new_dnf);
}


/*
 * Remove redundant conjuncts from given DNF.
 * If (C1 => C2), remove C1.
 * C1 => C2 is TRUE: when problem where C1 is Black and C2 is Red 
 * Blk   Red       : has no red constraints.
 * It means that C1 is a subset of C2 and therefore C1 is redundant.
 *
 * Exception: C1 => UNKNOWN - leave them as they are
 */
void DNF::rm_redundant_conjs(int effort) {
  if(is_definitely_false() || has_single_conjunct())
    return;

  use_ugly_names++;
  skip_set_checks++;

  int count = 0;
  for(DNF_Iterator p(this); p.live(); p.next()) count++;

  if(pres_debug) {
    int i = 0;
    fprintf(DebugFile, "@@@ rm_redundant_conjs IN @@@[\n");
    prefix_print(DebugFile);
    for(DNF_Iterator p(this); p.live(); p.next())
      fprintf(DebugFile, "#%d = %p\n", ++i, p.curr());
  }

  DNF_Iterator pdnext;
  DNF_Iterator pdel(this);
  for(; pdel.live(); pdel=pdnext) {
    pdnext = pdel;
    pdnext.next();
    Conjunct *cdel = pdel.curr();
    int del_min_leading_zeros = cdel->query_guaranteed_leading_0s();
    int del_max_leading_zeros = cdel->query_possible_leading_0s();

    for(DNF_Iterator p(this); p.live(); p.next()) {
      Conjunct *c = p.curr();
      if(c != cdel) {
        int c_min_leading_zeros = cdel->query_guaranteed_leading_0s();
        int c_max_leading_zeros = cdel->query_possible_leading_0s();
	if(pres_debug)
	  fprintf(DebugFile, "@@@ rm_redundant_conjs @%p => @%p[\n", cdel, c);

        if (c->is_inexact() && cdel->is_exact()) {
          if (pres_debug)
             fprintf(DebugFile, "]@@@ rm_redundant_conjs @@@ Exact Conj => Inexact Conj is not tested\n");
        } else if (del_min_leading_zeros  >=0 && c_min_leading_zeros >= 0
			&& c_max_leading_zeros >= 0 && del_max_leading_zeros >=0
		&& (del_min_leading_zeros  > c_max_leading_zeros
			|| c_min_leading_zeros > del_max_leading_zeros)) {
             if (1 || pres_debug)
                fprintf(DebugFile, "]@@@ not redundant due to leading zero info\n");
	     }
	  else {
		
	  Conjunct *cgist = merge_conjs(cdel, c, MERGE_GIST);

	  if (!cgist->redSimplifyProblem(effort,0)) {
	    if(pres_debug) {
	      fprintf(DebugFile, "]@@@ rm_redundant_conjs @@@ IMPLICATION TRUE @%p\n", cdel);
	      cdel->prefix_print (DebugFile);
	      fprintf(DebugFile, "=>\n");
	      c->prefix_print (DebugFile);
	    }
            rm_conjunct(cdel);
	    delete cdel;
            delete cgist;
	    break;
	  } else {
	    if(pres_debug) 
	    {
	      fprintf(DebugFile, "]@@@ rm_redundant_conjs @@@ IMPLICATION FALSE @%p\n", cdel);
	      if(pres_debug > 1)
	        cgist->prefix_print(DebugFile);
	    }
            delete cgist;
	  }
	}
      }
    }
  }

  if(pres_debug) {
    fprintf(DebugFile, "]@@@ rm_redundant_conjs OUT @@@\n");
    prefix_print(DebugFile);
  }
  skip_set_checks--;
  use_ugly_names--;
}


 /* Remove  inexact conjuncts from given DNF if it contains UNKNOWN
  * conjunct
  */

void DNF::rm_redundant_inexact_conjs() {
  if (is_definitely_false() || has_single_conjunct())
    return;

 bool has_unknown=false;
 bool has_inexact=false;

 Conjunct * c_unknown = 0;  // make compiler shut up
 for (DNF_Iterator p(this); p.live(); p.next()) {
   assert (p.curr()->problem!=NULL);
   if (p.curr()->is_inexact()) {
      if (p.curr()->is_unknown()) {
        has_unknown=true;
        c_unknown = p.curr();
      }
      else
        has_inexact=true;
    }
 }

 if (! has_unknown || ! has_inexact)
    return;

 use_ugly_names++;
 skip_set_checks++;  

 DNF_Iterator pdnext;
 DNF_Iterator pdel(this);

 for (; pdel.live(); pdel=pdnext) {
   pdnext = pdel;
   pdnext.next();
   Conjunct * cdel=pdel.curr();
   if (cdel->is_inexact() && cdel!=c_unknown) {
     rm_conjunct(cdel);
     delete cdel;
   }
 }

 use_ugly_names--;
 skip_set_checks--;
}
  


//
// DNF properties.
//
bool DNF::is_definitely_false() const {
  return(conjList.empty());
}

bool DNF::is_definitely_true() const {
  return(has_single_conjunct() && single_conjunct()->is_true());
}

int DNF::length() const {
  return conjList.length();
}

Conjunct *DNF::single_conjunct() const {
  assert(conjList.length()==1);
  return(conjList.front());
}

bool DNF::has_single_conjunct() const {
  return (conjList.length()==1);
}

Conjunct *DNF::rm_first_conjunct() {
  if(conjList.empty()) {
    return NULL;
  } else {
    return conjList.remove_front();
  }
}


//
// Convert DNF to Formula and add it root.
// Free this DNF. 
//
void DNF::DNF_to_formula(Formula* root) {
  Formula *new_or;
  if (conjList.length()!=1) {
    skip_finalization_check++;
    new_or = root->add_or();
    skip_finalization_check--;
  } else {
    new_or = root;
  }
  while(!conjList.empty()) {
    Conjunct *conj = conjList.remove_front();
    new_or->add_child(conj);
  }
  delete this;
}


//
// DNF functions.
//
DNF::DNF() : conjList() {
}

DNF::~DNF() {
  for(DNF_Iterator p(this); p.live(); p.next()) {
    if(p.curr() != NULL)
      delete p.curr();
  }
}

//
// Copy DNF
//
DNF* DNF::copy(Rel_Body *rel_body) {
  DNF *new_dnf = new DNF;
  for(DNF_Iterator pd(this); pd.live(); pd.next()) {
    Conjunct *conj = pd.curr();
    if(conj)
      new_dnf->add_conjunct(conj->copy_conj_diff_relation(rel_body,rel_body));
  }
  return(new_dnf);
}

//
// Add Conjunct to DNF
//
void DNF::add_conjunct(Conjunct* conj) {
  conjList.append(conj);
}

//
// Add DNF to DNF.
// The second DNF is reused.
//
void DNF::join_DNF(DNF* dnf) {
  conjList.join(dnf->conjList);
  delete dnf;
}

//
// Remove conjunct from DNF.
// Conjunct itself is not deleted.
//
void DNF::rm_conjunct(Conjunct *c) {
  if(conjList.front() == c) {
    conjList.remove_front();
  } else {
    List_Iterator<Conjunct*> p, pp;
    for(p=List_Iterator<Conjunct*> (conjList); p; p++) {
      if((*p)==c) {
        conjList.del_after(pp);
        return;
      }
      pp = p;
    }
    PresErrAssert("DNF::rm_conjunct: no such conjunct");
  }
}


// remove (but don't delete) all conjuncts

void DNF::clear()
    {
    conjList.clear();
    }


//
// DNF & CONJ -> new DNF.
// Don't touch arguments.
//
DNF* DNF_and_conj(DNF* dnf, Conjunct* conj) {
  DNF* new_dnf = new DNF;
  for(DNF_Iterator p(dnf); p.live(); p.next()) {
    Conjunct* new_conj = merge_conjs(p.curr(), conj, MERGE_REGULAR);
    new_dnf->add_conjunct(new_conj);
  }
  if(new_dnf->length() > 1) {
    new_dnf->simplify();
  }
  return(new_dnf);
}

//
// Compute C0 and not (C1 or C2 or ... CN).
// Reuse/delete its arguments.
//
DNF* conj_and_not_dnf(Conjunct *positive_conjunct, DNF *neg_conjs, bool weak) {
  DNF *ret_dnf = new DNF;
  int recursive = 0;
  use_ugly_names++;

  if(pres_debug) {
    fprintf(DebugFile, "conj_and_not_dnf [\n");
    fprintf(DebugFile, "positive_conjunct:\n");
    positive_conjunct->prefix_print(DebugFile);
    fprintf(DebugFile, "neg_conjs:\n");
    neg_conjs->prefix_print(DebugFile);
    fprintf(DebugFile, "\n\n");
  }

  if (simplify_conj(positive_conjunct, true, false, black) == false) {
    positive_conjunct = NULL;
    goto ReturnDNF;
  }

  /* Compute gists of negative conjuncts given positive conjunct */


  int c0_updated;
  c0_updated = true;
  while(c0_updated) {
    c0_updated = false;
    for(DNF_Iterator p(neg_conjs); p.live(); p.next()) {
      Conjunct *neg_conj = p.curr();
      if(neg_conj==NULL) continue;
      if (!positive_conjunct->is_exact()
		&& !neg_conj->is_exact()) {
	// C1 and unknown & ~(C2 and unknown) = C1 and unknown
	delete neg_conj;
	p.curr_set(NULL);
	continue;
	}
	  Conjunct *cgist = merge_conjs(positive_conjunct, neg_conj, MERGE_GIST);
          if(simplify_conj(cgist, false, true, _red) == false) 
	      {
	      // C1 & ~FALSE = C1
	      delete neg_conj;
	      p.curr_set(NULL);
              } 
	  else 
	      {
	      cgist->rm_color_constrs();
	      if(cgist->is_true()) 
		  {
	          // C1 & ~TRUE = FALSE
	          delete cgist;
	          goto ReturnDNF;
	          } 
	      else 
		  {
	          if(cgist->cost()==1) 
		      {        // single inequality
	              DNF *neg_dnf = negate_conj(cgist);
	              delete cgist;
	              Conjunct *conj =
	              merge_conjs(positive_conjunct, neg_dnf->single_conjunct(), 
				  MERGE_REGULAR);
	              delete positive_conjunct;
	              delete neg_dnf;
	              positive_conjunct = conj;
	              delete neg_conj;
	              p.curr_set(NULL);
	              if(!simplify_conj(positive_conjunct, false, false, black)) 
			  {
	                  positive_conjunct = NULL;
	                  goto ReturnDNF;
	                  }
	              c0_updated = true;
	              } 
		  else 
		      {
	              delete neg_conj;
	              p.curr_set(cgist);
	              }
	          }
              }
      }
  }

  if(pres_debug) {
    fprintf(DebugFile, "--- conj_and_not_dnf positive_conjunct NEW:\n");
    positive_conjunct->prefix_print(DebugFile);
    fprintf(DebugFile, "--- conj_and_not_dnf neg_conjs GISTS:\n");
    neg_conjs->prefix_print(DebugFile);
    fprintf(DebugFile, "--- conj_and_not_dnf ---\n\n");
  }

  /* Find minimal negative conjunct */
  { Conjunct *min_conj = NULL;
    int min_cost = MAXINT;
    DNF_Iterator min_p;
    int live_count = 0;
    for(DNF_Iterator q(neg_conjs); q.live(); q.next()) {
      Conjunct *neg_conj = q.curr();
      if(neg_conj!=NULL) {
	live_count++;
	if(neg_conj->cost() < min_cost) {
	  min_conj = neg_conj;
	  min_cost = neg_conj->cost();
	  min_p = q;
	}
      }
    }

    /* Negate minimal conjunct, AND result with positive conjunct */
    if(weak || min_conj==NULL) {
      ret_dnf->add_conjunct(positive_conjunct);
      positive_conjunct = NULL;
    } else if (min_cost == CantBeNegated) {
	static int OMEGA_WHINGE = -1;
	if (OMEGA_WHINGE < 0)
	    {
	    OMEGA_WHINGE = getenv("OMEGA_WHINGE") ? atoi(getenv("OMEGA_WHINGE")) : 0;
	    }
	if (OMEGA_WHINGE)
	    {
	    fprintf(stderr, "Ignoring negative clause that can't be negated and generating inexact result\n");
	    if (!pres_debug) fprintf(DebugFile, "Ignoring negative clause that can't be negated and generating inexact result\n");
	    }

	positive_conjunct->make_inexact();
	ret_dnf->add_conjunct(positive_conjunct);
	positive_conjunct = NULL;
        if(pres_debug) 
	    fprintf(DebugFile, "Ignoring negative clause that can't be negated and generating inexact upper bound\n");
    } else {
      DNF *neg_dnf = negate_conj(min_conj);
      delete min_conj;
      min_p.curr_set(NULL);
      DNF *new_pos = DNF_and_conj(neg_dnf, positive_conjunct);
      delete neg_dnf;
      delete positive_conjunct;
      positive_conjunct = NULL;
      // new_dnf->rm_redundant_conjs(2);
      if(live_count>1) {
	recursive = 1;
	for(DNF_Iterator pd(new_pos); pd.live(); pd.next()) {
	  Conjunct *conj = pd.curr();
	  ret_dnf->join_DNF(conj_and_not_dnf(conj, neg_conjs->copy(conj->relation())));
	  pd.curr_set(NULL);
	}
	delete new_pos;
      } else {
	ret_dnf->join_DNF(new_pos);
      }
    }
  }

 ReturnDNF:;
  delete positive_conjunct;
  delete neg_conjs;

  //if (recursive) ret_dnf->rm_redundant_conjs(1);

  if(pres_debug) {
    fprintf(DebugFile, "] conj_and_not_dnf RETURN:\n");
    ret_dnf->prefix_print(DebugFile);
    fprintf(DebugFile, "\n\n");
  }
  use_ugly_names--;
  return ret_dnf;
}

/* first some functions for manipulating oc "problems" */

static void EqnnZero(Eqn e, int s) {
  memset((char*)e, 0, (headerWords+1+s)*sizeof(int));
}

/*
 * Make a new black equation in a given problem 
 */
static int NewEquation(Problem *p) {
  int e = p->newEQ();
  EqnnZero(&p->EQs[e], p->nVars);
  return e;
}

/*
 * Make a new black inequality in a given problem 
 */
static int NewInequality(Problem *p) {
  int g = p->newGEQ();
  EqnnZero(&p->GEQs[g], p->nVars);
  return g;
}

//
// ~CONJ -> DNF
//
DNF* negate_conj(Conjunct* conj) {
  if(pres_debug) {
    fprintf(DebugFile, "%%%%%% negate_conj IN %%%%%%\n");
    conj->prefix_print(DebugFile);
    fprintf(DebugFile, "\n");
  }

  DNF* new_dnf = new DNF;
  Problem *p = conj->problem;
  int i, j,k;

  if (!conj->is_exact()) new_dnf->add_conjunct(conj->copy_conj_same_relation());

  Conjunct* true_part = new Conjunct(NULL, conj->relation());
    Problem *tp = true_part->problem;
    copy_conj_header(true_part, conj);
    true_part->invalidate_leading_info();
  int *wildCard = new int[p->nGEQs];
  int *handleIt = new int[p->nVars+1];
  for(j=1; j<=p->nVars; j++) handleIt[j] = false;

  for(i=0; i<p->nGEQs; i++) {
    wildCard[i] = 0;
    for(j=1; j<=p->nVars; j++) {
      Variable_ID v = conj->mappedVars[j];
      if(v->kind()==Wildcard_Var && p->GEQs[i].coef[j]!=0) {
        assert(wildCard[i] == 0);
  	handleIt[j] = true;
	if (p->GEQs[i].coef[j] > 0) wildCard[i] = j;
	else wildCard[i] = -j;
	}
    }}

  for(i=0; i<p->nGEQs; i++) if (wildCard[i] == 0){
    /* ~(ax + by + c >= 0) = (-ax -by -c-1 >= 0) */
    Conjunct* new_conj = true_part->copy_conj_same_relation();
    Problem *np = new_conj->problem;
    new_conj->exact=true;
    int n_e = NewInequality(np);
    int t_e = NewInequality(tp);
    np->GEQs[n_e].coef[0] = -p->GEQs[i].coef[0]-1;
    tp->GEQs[t_e].coef[0] = p->GEQs[i].coef[0];
    for(j=1; j<=p->nVars; j++) {
      Variable_ID v = conj->mappedVars[j];
      if(v->kind()==Wildcard_Var && p->GEQs[i].coef[j]!=0) {
	Assert(0, "negate_conj: wildcard in inequality");
      }
      np->GEQs[n_e].coef[j] = -p->GEQs[i].coef[j];
      tp->GEQs[t_e].coef[j] = p->GEQs[i].coef[j];

    }
    assert(j-1 == p->nVars);
    assert(j-1 == conj->mappedVars.size());
    new_dnf->add_conjunct(new_conj);
  }


  for(i=0; i<p->nEQs; i++) {
    int wc_no = 0;
    int wc_j = 0;  // make complier shut up
    for(j=1; j<=p->nVars; j++) {
      Variable_ID v = conj->mappedVars[j];
      if(v->kind()==Wildcard_Var && p->EQs[i].coef[j]!=0) {
	wc_no++;
	wc_j = j;
      }
    }

    if(wc_no!=0) {
#if ! defined NDEBUG
      int i2;
      assert(!handleIt[wc_j]);
      for(i2=0; i2<p->nEQs; i2++)
	  if(i != i2 && p->EQs[i2].coef[wc_j] != 0)  break;
      assert(i2 >= p->nEQs);
#endif
      Assert(wc_no==1, "negate_conj: more than 1 wildcard in equality");

      // === Negating equality with a wildcard for K>0 ===
      // ~(exists v st expr + K v  + C = 0) =
      //  (exists v st 1 <= - expr - K v - C <= K-1)

      Conjunct *nc = true_part->copy_conj_same_relation();
      Problem *np = nc->problem;
      nc->exact=true;

      // -K alpha = expr  <==>  K alpha = expr
      if(p->EQs[i].coef[wc_j]<0)
	p->EQs[i].coef[wc_j] = -p->EQs[i].coef[wc_j];

      if(p->EQs[i].coef[wc_j]==2) {
	// ~(exists v st  expr +2v +C = 0) =
	//  (exists v st -expr -2v -C = 1)
	// That is (expr +2v +C+1 = 0)
	int e = NewEquation(np);
	np->EQs[e].coef[0] = p->EQs[i].coef[0] +1;
	for(j=1; j<=p->nVars; j++) {
	  np->EQs[e].coef[j] = p->EQs[i].coef[j];
	}

      } else {
	// -expr -Kv -C-1 >= 0
	int e = NewInequality(np);
	np->GEQs[e].coef[0] = -p->EQs[i].coef[0] -1;
	for(j=1; j<=p->nVars; j++) {
	  np->GEQs[e].coef[j] = -p->EQs[i].coef[j];
	}

	// +expr +Kv +C+K-1 >= 0
	e = NewInequality(np);
	np->GEQs[e].coef[0] = p->EQs[i].coef[0] +p->EQs[i].coef[wc_j] -1;
	for(j=1; j<=p->nVars; j++) {
	  np->GEQs[e].coef[j] = p->EQs[i].coef[j];
	}
      }

      new_dnf->add_conjunct(nc);

    } else {
      /* ~(ax + by + c = 0) = (-ax -by -c-1 >= 0) Or (ax + by + c -1 >= 0) */
      Conjunct *nc1 = true_part->copy_conj_same_relation();
      Conjunct *nc2 = true_part->copy_conj_same_relation();
      Problem* np1 = nc1->problem;
      Problem* np2 = nc2->problem;
      nc1->invalidate_leading_info();
      nc2->invalidate_leading_info();
      nc1->exact=true;
      nc2->exact=true;
      int n_e1 = NewInequality(np1);
      int n_e2 = NewInequality(np2);
      np1->GEQs[n_e1].coef[0] = -p->EQs[i].coef[0]-1;
      np2->GEQs[n_e2].coef[0] =  p->EQs[i].coef[0]-1;
      for(j=1; j<=p->nVars; j++) {
	coef_t coef = p->EQs[i].coef[j];
	np1->GEQs[n_e1].coef[j] = -coef;
	np2->GEQs[n_e2].coef[j] =  coef;
      }
      new_dnf->add_conjunct(nc1);
      new_dnf->add_conjunct(nc2);
    }
   { int e = NewEquation(tp);
      tp->EQs[e].coef[0] =  p->EQs[i].coef[0];
      for(j=1; j<=p->nVars; j++) 
	 tp->EQs[e].coef[j] = p->EQs[i].coef[j];
    }
  }

  for(j=1; j<=p->nVars; j++) if (handleIt[j]) {
        for(i=0; i<p->nGEQs; i++) if (wildCard[i] == j)
          for(k=0; k<p->nGEQs; k++) if (wildCard[k] == -j){
		// E_i <= c_i alpha
		//        c_k alpha <= E_k
		// c_k E_i <= c_i c_k alpha <= c_i E_k
		// c_k E_i <= c_i c_k floor (c_i E_k / c_i c_k)
		// negating:
		// c_k E_i > c_i c_k floor (c_i E_k / c_i c_k)
		// c_k E_i > c_i c_k beta > c_i E_k - c_i c_k
		// c_k E_i - 1 >= c_i c_k beta >= c_i E_k - c_i c_k + 1
	    Conjunct* new_conj = true_part->copy_conj_same_relation();
	    Problem *np = new_conj->problem;
	    coef_t c_k = - p->GEQs[k].coef[j];
	    coef_t c_i = p->GEQs[i].coef[j];
	    assert(c_k > 0);
	    assert(c_i > 0);
	    new_conj->exact=true;
	    int n_e = NewInequality(np);
		// c_k E_i - 1 >= c_i c_k beta 
	    int v;
	    for(v=0; v<=p->nVars; v++) {
	      np->GEQs[n_e].coef[v] = - c_k * p->GEQs[i].coef[v];
	    }
	    np->GEQs[n_e].coef[j] = -c_i * c_k;
	    np->GEQs[n_e].coef[0]--;

	    n_e = NewInequality(np);
		// c_i c_k beta >= c_i E_k - c_i c_k + 1
		// c_i c_k beta + c_i c_k -1 >= c_i E_k 
	    for(v=0; v<=p->nVars; v++) {
	      np->GEQs[n_e].coef[v] = - c_i * p->GEQs[k].coef[v];
	    }
	    np->GEQs[n_e].coef[j] = c_i * c_k;
	    np->GEQs[n_e].coef[0] += c_i * c_k -1;

	    new_dnf->add_conjunct(new_conj);
	  }
	}

  if(pres_debug) {
    fprintf(DebugFile, "%%%%%% negate_conj OUT %%%%%%\n");
    new_dnf->prefix_print(DebugFile);
  }
  delete true_part;
  delete wildCard;
  delete handleIt;
  return(new_dnf);
}




///////////////////////////////////////////////////////
// DNFize formula -- this is the real simplification //
// It also destroys the formula it simplifies        //
///////////////////////////////////////////////////////



//
// Try to separate positive and negative clauses below the AND,
// letting us use the techniques described in Pugh & Wonnacott:
// "An Exact Method for Value-Based Dependence Analysis"
//


DNF* F_And::DNFize()
    {
    Conjunct *positive_conjunct = NULL;
    DNF *neg_conjs = new DNF;
    List<DNF*> pos_dnfs;
    List_Iterator<DNF*> pos_dnf_i;
    DNF *new_dnf = new DNF;
    int JustReturnDNF = 0;

    use_ugly_names++;

    if(pres_debug)
	{
	fprintf(DebugFile, "\nF_And:: DNFize [\n");
	prefix_print(DebugFile);
	}

    if(children().empty())
	{
        Conjunct * c=new Conjunct(NULL, relation());
        new_dnf->add_conjunct(c);
	}
    else 
	{
	while(!children().empty()) 
	    {
	    Formula* carg = children().remove_front();
	    if(carg->node_type()==Op_Not) 
		{
		// DNF1 & ~DNF2 -> DNF
		DNF *dnf = carg->children().remove_front()->DNFize();
		delete carg;
		neg_conjs->join_DNF(dnf);       // negative conjunct
		}
	    else 
		{
		// DNF1 & DNF2 -> DNF
		DNF *dnf = carg->DNFize();
		int dl = dnf->length();
		if(dl==0) 
		    {
		    // DNF & false -> false
		    delete this;
		    JustReturnDNF = 1;
		    break;
		    }
		else if(dl==1) 
		    {
		    // positive conjunct
		    Conjunct *conj = dnf->rm_first_conjunct();
		    delete dnf;
		    if(positive_conjunct==NULL) 
			{
			positive_conjunct = conj;
			}
		    else 
			{
			Conjunct *new_conj = merge_conjs(positive_conjunct, conj, MERGE_REGULAR);
			delete conj;
			delete positive_conjunct;
			positive_conjunct = new_conj;
			}
		    }
		else 
		    {
		    // positive DNF
		    pos_dnfs.append(dnf);
		    }
		}
	    }

	if (!JustReturnDNF)
	    {
	    Rel_Body * my_relation = relation();
	    delete this;

	    // If we have a positive_conjunct, it can serve as the 1st arg to
	    // conj_and_not_dnf.  Otherwise, if pos_dnfs has one DNF,
	    // use each conjunct there for this purpose.
	    // Only pass "true" here if there is nothing else to try,
	    // as long as TRY_TO_AVOID_TRUE_AND_NOT_DNF is set.
	    //
	    // Perhaps we should even try to and multiple DNF's?

	    if (!positive_conjunct && pos_dnfs.length() == 1)
		{
		if(pres_debug)
		    {
		    fprintf(DebugFile,
			    "--- F_AND::DNFize() Single pos_dnf:\n");
		    pos_dnfs[1]->prefix_print(DebugFile);
		    fprintf(DebugFile,
			    "--- F_AND::DNFize() vs neg_conjs:\n");
		    neg_conjs->prefix_print(DebugFile);
		    }

		DNF *real_neg_conjs = new DNF;
		for (DNF_Iterator nc(neg_conjs); nc; nc++)
		    {
		    if (simplify_conj((*nc), true, false, black) != false)
			real_neg_conjs->add_conjunct(*nc);
		    (*nc) = 0;
		    }
		delete neg_conjs;
		neg_conjs = real_neg_conjs;

		for(DNF_Iterator pc(pos_dnfs[1]); pc; pc++)
		    {
		    new_dnf->join_DNF(conj_and_not_dnf((*pc), neg_conjs->copy((*pc)->relation())));
		    (*pc) = 0;
		    }
		}
	    else if(positive_conjunct==NULL && neg_conjs->is_definitely_false()) {
		pos_dnf_i = List_Iterator<DNF*>(pos_dnfs);
		new_dnf = *pos_dnf_i;
	        *pos_dnf_i = NULL;
		pos_dnf_i++;
		for ( ;
		     pos_dnf_i;
		     pos_dnf_i++)
		    {
		    DNF *pos_dnf = *pos_dnf_i;
		    new_dnf = DNF_and_DNF(new_dnf, pos_dnf);
		    *pos_dnf_i = NULL;
		    }
		
		}	
	    else 
		{
		if(positive_conjunct==NULL) 
		    {		
		    static int OMEGA_WHINGE = -1;
		    if (OMEGA_WHINGE < 0)
			{
			OMEGA_WHINGE = getenv("OMEGA_WHINGE") ? atoi(getenv("OMEGA_WHINGE")) : 0;
			}

		    if (pres_debug || OMEGA_WHINGE) {
			fprintf(DebugFile, "Uh-oh: F_AND::DNFize() resorting to TRUE and not DNF\n");
		        fprintf(DebugFile, "--- F_AND::DNFize() neg_conjs\n");
			neg_conjs->prefix_print(DebugFile);
		        fprintf(DebugFile, "--- F_AND::DNFize() pos_dnfs:\n");
		        for (pos_dnf_i=List_Iterator<DNF*>(pos_dnfs);
			     pos_dnf_i;
			     pos_dnf_i++) {
				(*pos_dnf_i)->prefix_print(DebugFile);
				fprintf(DebugFile,"---- --\n");
			    }
		        }
		    if (OMEGA_WHINGE)
			{
			fprintf(stderr, "Uh-oh: F_AND::DNFize() resorting to TRUE and not DNF\n");
		        fprintf(stderr, "--- F_AND::DNFize() neg_conjs\n");
			neg_conjs->prefix_print(stderr);
		        fprintf(stderr, "--- F_AND::DNFize() pos_dnfs:\n");
		        for (pos_dnf_i=List_Iterator<DNF*>(pos_dnfs);
			     pos_dnf_i;
			     pos_dnf_i++) {
				    (*pos_dnf_i)->prefix_print(stderr);
				    fprintf(stderr,"---- --\n");
				    }
			}
		    positive_conjunct = new Conjunct(NULL, my_relation);
		    }
	
		if(!neg_conjs->is_definitely_false())
		    {
		    new_dnf->join_DNF(conj_and_not_dnf(positive_conjunct, neg_conjs));
		    neg_conjs = NULL;
		    }
		else
		    {
		    new_dnf->add_conjunct(positive_conjunct);
		    }
		positive_conjunct = NULL;

		//
		// AND it with positive DNFs
		//
		if(pres_debug)
		    {
		    fprintf(DebugFile, "--- F_AND::DNFize() pos_dnfs:\n");
		    for (pos_dnf_i=List_Iterator<DNF*>(pos_dnfs);
			 pos_dnf_i;
			 pos_dnf_i++)
			(*pos_dnf_i)->prefix_print(DebugFile);
		    }
		for (pos_dnf_i = List_Iterator<DNF*>(pos_dnfs);
		     pos_dnf_i;
		     pos_dnf_i++)
		    {
		    DNF *pos_dnf = *pos_dnf_i;
		    new_dnf = DNF_and_DNF(new_dnf, pos_dnf);
		    *pos_dnf_i = NULL;
		    }
		}
	    }
	}

    delete positive_conjunct;
    delete neg_conjs;
    for (pos_dnf_i = List_Iterator<DNF*>(pos_dnfs); pos_dnf_i; pos_dnf_i++)
	delete *pos_dnf_i;

    if(pres_debug)
	{
	fprintf(DebugFile, "] F_AND::DNFize() OUT \n");
	new_dnf->prefix_print(DebugFile);
	}

    use_ugly_names--;

    return new_dnf;
    }

//
// ~ dnf  = true ^ ~ dnf, so just call conj_and_not_dnf
//

DNF* F_Not::DNFize() {
  Conjunct *positive_conjunct = new Conjunct(NULL, relation());  
  DNF *neg_conjs = children().remove_front()->DNFize();
  delete this;
  DNF *new_dnf = conj_and_not_dnf(positive_conjunct, neg_conjs);

  if(pres_debug) {
    fprintf(DebugFile, "=== F_NOT::DNFize() OUT ===\n");
    new_dnf->prefix_print(DebugFile);
  }
  return new_dnf;
}


//
// or is almost in DNF already:
//

DNF* F_Or::DNFize() {
  DNF* new_dnf = new DNF;
  bool empty_or=true;

  while(!children().empty()) {
    DNF* c_dnf = children().remove_front()->DNFize();
    new_dnf->join_DNF(c_dnf);
    empty_or=false; 
  }

    
  delete this;

  if(pres_debug) {
    fprintf(DebugFile, "=== F_OR::DNFize() OUT ===\n");
    new_dnf->prefix_print(DebugFile);
  }
  return(new_dnf);
}


//
// exists x : (c1 v c2 v ...) --> (exists x : c1) v (exists x : c2) v ...
//

DNF* F_Exists::DNFize() {
  DNF *dnf = children().remove_front()->DNFize();

  for (DNF_Iterator pd(dnf); pd.live(); pd.next()) {
    Conjunct *conj = pd.curr();

    // can simply call localize_vars for DNF with a single conjunct
    Variable_ID_Tuple locals_copy(myLocals.size());
    copy_var_decls(locals_copy, myLocals);
    conj->push_exists(locals_copy);
    conj->remap();
    reset_remap_field(myLocals);

    conj->r_constrs = 0;
    conj->simplified = false; // who knows
    conj->cols_ordered = false;
  }
  delete this;

  if(pres_debug) {
    fprintf(DebugFile, "=== F_EXISTS::DNFize() OUT ===\n");
    dnf->prefix_print(DebugFile);
  }
  return(dnf);
}


//
// Single conjunct is already in DNF.
//

DNF* Conjunct::DNFize()
    {
    assert(!is_compressed());
    DNF *results = new DNF;

    if (is_true())
	{
	simplified = true;
	verified = true;
	results->add_conjunct(this);
	}
    else 
	{
	results->add_conjunct(this);
	}

    return results;
}


//
// Foralls should have been removed before we get to DNFize
//

DNF* F_Forall::DNFize() {
  assert(0);
  return(NULL);
}

void DNF::count_leading_0s()
    {
    if (conjList.empty())
	return;

    for (DNF_Iterator conj(this); conj; conj++)
	{
	(*conj)->count_leading_0s();
	}
    }


// return x s.t. forall conjuncts c, c has >= x leading 0s
// if set, always returns -1; arg tells you if it's a set or relation.

int DNF::query_guaranteed_leading_0s(int what_to_return_for_empty_dnf)
    {
    count_leading_0s();
    int result = what_to_return_for_empty_dnf; // if set, -1; if rel, 0
    bool first = true;

    for (DNF_Iterator conj(this); conj; conj++)
	{
	int tmp = (*conj)->query_guaranteed_leading_0s();
	assert(tmp >= 0 || ((*conj)->relation()->is_set() && tmp == -1));
	if (first || tmp < result) result = tmp;
	first = false;
	}
    
    return result;
    }

// return x s.t. forall conjuncts c, c has <= x leading 0s
// if no conjuncts, return the argument

int DNF::query_possible_leading_0s(int n_input_and_output)
    {
    count_leading_0s();
    int result = n_input_and_output;
    bool first = true;

    for (DNF_Iterator conj(this); conj; conj++)
	{
	int tmp = (*conj)->query_possible_leading_0s();
	assert(tmp >= 0 || (tmp == -1 && (*conj)->relation()->is_set()));
	if (first || tmp > result) result = tmp;
	first = false;
	}

    return result;
    }


// return 0 if we don't know, or +-1 if we do

int DNF::query_leading_dir()
    {
    count_leading_0s();
    int result = 0;
    bool first = true;

    for (DNF_Iterator conj(this); conj; conj++)
	{
	int glz = (*conj)->query_guaranteed_leading_0s();
	int plz = (*conj)->query_possible_leading_0s();
	int rlz = 0; // shut the compiler up
	if (glz != plz)
	    return 0;

	if (first)
	    {
	    rlz = glz;
	    result = (*conj)->query_leading_dir();
	    first = false;
	    }
	else
	    if (glz != rlz || result != (*conj)->query_leading_dir())
		return 0;
	}

    return result;
    }

void Conjunct::count_leading_0s()
    {
    Rel_Body *body = relation();
    int max_depth = min(body->n_inp(), body->n_out());
    if(body->is_set())
	{
	assert(guaranteed_leading_0s == -1 && possible_leading_0s == -1);
//	guaranteed_leading_0s = possible_leading_0s = -1;
	leading_dir = 0;
	return;
	}


#if ! defined NDEBUG
    assert_leading_info();
#endif
    if (guaranteed_leading_0s < 0) 
	{
	int L;
	for (L=1; L <= max_depth; L++) 
	    {
	    Variable_ID in = body->input_var(L), out = body->output_var(L);
	    coef_t min, max;
	    bool guaranteed;
	    
	    query_difference(out, in, min, max, guaranteed);
	    if (min < 0 || max > 0)
		{
		if (min > 0 || max < 0)  // we know guaranteed & possible
		    {
		    guaranteed_leading_0s = possible_leading_0s = L-1;
		    if (min > 0)	// We know its 0,..,0,+
			leading_dir = 1;
		    else		// We know its 0,..,0,-
			leading_dir = -1;
		    return;
		    }
		break;
		}
	    }
	guaranteed_leading_0s = L-1;
	for ( ; L <= max_depth; L++)
	    {
	    Variable_ID in = body->input_var(L),
		out = body->output_var(L);
	    coef_t min, max;
	    bool guaranteed;
	    
	    query_difference(out, in, min, max, guaranteed);
	    
	    if (min > 0 || max < 0) break;
	    }
	possible_leading_0s = L-1;
	}
#if ! defined NDEBUG
    assert_leading_info();
#endif
    }

//
// add level-carried DNF form out to level "level"
//


void DNF::make_level_carried_to(int level)
    {
    count_leading_0s();
    Rel_Body *body = 0;  // make compiler shut up
    if (length() > 0 && !(body = conjList.front()->relation())->is_set())
	// LCDNF makes no sense otherwise
	{
    
        Relation tmp;
#ifndef NDEBUG
        tmp = Relation(*body,42);
#endif

	DNF *newstuff = new DNF;
	int shared_depth = min(body->n_inp(), body->n_out());
	int split_to     = level >= 0 ? min(shared_depth,level) : shared_depth;

        skip_finalization_check++;
        EQ_Handle e;

	for (DNF_Iterator conj(this); conj; conj++)
	    {
	    assert(body = (*conj)->relation());
	    int leading_eqs;

	    bool is_guaranteed = (*conj)->verified;

  	    for (leading_eqs=1; leading_eqs <= split_to; leading_eqs++)
		{
		Variable_ID in = body->input_var(leading_eqs),
		    out = body->output_var(leading_eqs);
		coef_t min, max;
		bool guaranteed;

		if (leading_eqs > (*conj)->possible_leading_0s &&
		    (*conj)->leading_dir_valid_and_known())
		    {
		    leading_eqs--;
		    break;
		    }

		if (leading_eqs > (*conj)->guaranteed_leading_0s)
		    {
		    (*conj)->query_difference(out, in, min, max, guaranteed);
		    if (min > 0 || max < 0) guaranteed = true;
//		    fprintf(DebugFile,"Make level carried, %d <= diff%d <= %d (%d):\n",
//				min,leading_eqs,max,guaranteed); 
//		    use_ugly_names++;
//		    (*conj)->prefix_print(DebugFile);
// 		    use_ugly_names--;
		    if (!guaranteed) is_guaranteed = false;
		    bool generateLTClause = min < 0;
		    bool generateGTClause = max > 0;
		    bool retainEQClause = 
			    (leading_eqs <= (*conj)->possible_leading_0s
				&& min <= 0 && max >= 0);
		    if (!(generateLTClause || generateGTClause || 
				retainEQClause) )
			{
			// conjunct is infeasible
			if (pres_debug)
			    {
			    fprintf(DebugFile, "Conjunct discovered to be infeasible during make_level_carried_to(%d):\n", level);
			    (*conj)->prefix_print(DebugFile);
			    }
#if ! defined NDEBUG
			Conjunct *cpy = (*conj)->copy_conj_same_relation();
			assert(!simplify_conj(cpy, true, 32767, 0));
#endif
			}

		    if (generateLTClause) 
			{
			Conjunct *lt;
			if (!generateGTClause && !retainEQClause)
				lt = *conj;
			else lt = (*conj)->copy_conj_same_relation();
			if (max >= 0) {
			  GEQ_Handle l = lt->add_GEQ(); // out<in ==> in-out-1>=0
			  l.update_coef_during_simplify(in, 1);
			  l.update_coef_during_simplify(out, -1);
			  l.update_const_during_simplify(-1);
			  };
			lt->guaranteed_leading_0s 
			    = lt->possible_leading_0s = leading_eqs-1;
			lt->leading_dir = -1;
			if (is_guaranteed) {
			    /*
			    fprintf(DebugFile,"Promising solutions to: %d <= diff%d <= %d (%d):\n",
					min,leading_eqs,max,guaranteed); 
			    use_ugly_names++;
			    lt->prefix_print(DebugFile);
			    use_ugly_names--;
			    */
				lt->promise_that_ub_solutions_exist(tmp);
			   }
			 else if (0) {
				fprintf(DebugFile,"Can't guaranteed solutions to:\n");
				use_ugly_names++;
				lt->prefix_print(DebugFile);
				use_ugly_names--;
				}
			if (generateGTClause || retainEQClause)
				newstuff->add_conjunct(lt);
			}

		    if (generateGTClause)
			{
			Conjunct *gt;
			if (retainEQClause) gt = (*conj)->copy_conj_same_relation();
			else gt = *conj;
			if (min <= 0) {
			  GEQ_Handle g = gt->add_GEQ(); // out>in ==> out-in-1>=0
			  g.update_coef_during_simplify(in, -1);
			  g.update_coef_during_simplify(out, 1);
			  g.update_const_during_simplify(-1);
			  };
			gt->guaranteed_leading_0s =
			    gt->possible_leading_0s = leading_eqs-1;
			gt->leading_dir = 1;
			if (is_guaranteed) {
			    /*
			    fprintf(DebugFile,"Promising solutions to: %d <= diff%d <= %d (%d):\n",
					min,leading_eqs,max,guaranteed); 
			    use_ugly_names++;
			    gt->prefix_print(DebugFile);
			    use_ugly_names--;
			    */
				gt->promise_that_ub_solutions_exist(tmp);
			   }
			 else if (0) {
				fprintf(DebugFile,"Can't guaranteed solutions to:\n");
				use_ugly_names++;
				gt->prefix_print(DebugFile);
				use_ugly_names--;
				}
			if (retainEQClause) newstuff->add_conjunct(gt);
			}

		    if (retainEQClause) {
			assert(min <= 0 && 0 <= max);

			if (min < 0 || max > 0) {
				e = (*conj)->add_EQ(1);
				e.update_coef_during_simplify(in, -1);
				e.update_coef_during_simplify(out, 1);
				}

			assert((*conj)->guaranteed_leading_0s == -1
			  || leading_eqs > (*conj)->guaranteed_leading_0s);
			assert((*conj)->possible_leading_0s == -1
			  || leading_eqs <= (*conj)->possible_leading_0s);

			(*conj)->guaranteed_leading_0s = leading_eqs;
			}
		    else break;
		    }

		{
		Set<Global_Var_ID> already_done;
		int remapped = 0;

		assert((*conj)->guaranteed_leading_0s == -1
		       || leading_eqs <= (*conj)->guaranteed_leading_0s);

		for (Variable_ID_Iterator func(*body->global_decls());
		     func;
		     func++)
		    {
		    Global_Var_ID f = (*func)->get_global_var();
		    if (!already_done.contains(f) &&
			body->has_local(f, Input_Tuple) &&
			body->has_local(f, Output_Tuple) &&
			f->arity() == leading_eqs)
			{
			already_done.insert(f);

			// add f(in) = f(out), project one away
			e = (*conj)->add_EQ(1);
			Variable_ID f_in  =body->get_local(f,Input_Tuple);
			Variable_ID f_out =body->get_local(f,Output_Tuple);

			e.update_coef_during_simplify(f_in, -1);
			e.update_coef_during_simplify(f_out, 1);

			f_out->remap = f_in;
			remapped = 1;
			is_guaranteed = false;
			}	    
		    }

		if (remapped)
		    {
		    (*conj)->remap();
		    (*conj)->combine_columns();
		    reset_remap_field(*body->global_decls());
		    remapped = 0;
		    }
		}
		}
	    if (is_guaranteed) 
		(*conj)->promise_that_ub_solutions_exist(tmp); 
	    else if (0) {
		fprintf(DebugFile,"Can't guaranteed solutions to:\n");
		use_ugly_names++;
		(*conj)->prefix_print(DebugFile);
		use_ugly_names--;
		}
	    }

	skip_finalization_check--;
	join_DNF(newstuff);
	}

#if ! defined NDEBUG
    for (DNF_Iterator c(this); c; c++)
	(*c)->assert_leading_info();
#endif

    simplify();
    }

void DNF::remove_inexact_conj() {
  bool found_inexact=false;

  do 
  {
    bool first=true;
    found_inexact=false;
    DNF_Iterator c_prev;
    for (DNF_Iterator c(this); c; c++)
    {
      if (!(*c)->is_exact()) // remove it from the list
      {
        found_inexact=true;
	delete (*c);
        if (first)
          conjList.del_front();
        else
          conjList.del_after(c_prev);
        break;
      }  
      else
      {
        first=false;
        c_prev=c;
      }
    }
  } 
  while (found_inexact);
} //end DNF::remove_inexact_conj


int s_rdt_constrs;

//
// Simplify all conjuncts in a DNF
//
void DNF::simplify() {
  for (DNF_Iterator pd(this); pd.live(); ) {
    Conjunct *conj = pd.curr();
    pd.next();
    if(s_rdt_constrs >= 0 && !simplify_conj(conj, true, s_rdt_constrs, black)) {
      rm_conjunct(conj);
    }
  }
}
  

} // end of namespace omega
