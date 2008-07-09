// All calculations of the transitive closure are now here

#include <basic/bool.h>
#include <basic/assert.h>
#include <omega.h>
#include <omega/hull.h>
#include <basic/Exit.h>
#include <basic/Iterator.h>
#include <basic/List.h>
#include <basic/SimpleList.h>

#ifdef TC_STATS
extern int clock_diff();
extern void start_clock();
FILE *statsfile;
int singles, totals=0;
#endif


namespace omega {

int closure_presburger_debug=0;

bool is_lex_forward(Relation R){
    if(R.n_inp() != R.n_out()) {
	fprintf(stderr, "relation has wrong inputs/outpts\n");
	exit(1);
    }
    Relation forw(R.n_inp(), R.n_out());
    F_Or * o = forw.add_or();
    for(int a = 1; a <= forw.n_inp(); a++) {
	F_And * andd = o->add_and();
	GEQ_Handle g = andd->add_GEQ();
	g.update_coef(input_var(a), -1);
	g.update_coef(output_var(a), 1);
	g.update_const(1);
	for(int b = 1; b < a; b++) {
	    EQ_Handle e = andd->add_EQ();
	    e.update_coef(input_var(a),1);
	    e.update_coef(output_var(a),-1);
	}
    }
    Relation test = Difference(R, forw);
    return !test.is_upper_bound_satisfiable();
}



static Relation compose_n(NOT_CONST Relation &input_r, int n)
    {
    Relation r = consume_and_regurgitate(input_r);
    if (n == 1)
	return r;
    else
	return Composition(r, compose_n(copy(r), n-1));
    } /* compose_n */




Relation approx_closure(NOT_CONST Relation &input_r, int n)
    {
    Relation r = consume_and_regurgitate(input_r);
    Relation r_closure; 

    r_closure=r;
    int i;
    for(i=2; i<=n; i++)
	r_closure=Union(r_closure,compose_n(copy(r), n));
    r_closure = Union(r_closure, Relation::Unknown(r_closure));
    
    return r_closure;
    } /* approx_closure */


static bool is_closure_itself(NOT_CONST Relation &r)
    {
    return  Must_Be_Subset(Composition(copy(r),copy(r)),copy(r));
    }




/*****
 * get a D form of the Relation  (single conjunct).
 *  D = {[ i_1,i_2,...,i_m] -> [j_1, j_2, ..., j_m ] :
 *         (forall p, 1<= p <= m) L_p <= j_p - i_p <= U_p && 
 *      j_p - i_p == M_p alpha_p};
 *  Right now only wildcards that are in stride constraints are treated.
 *****/

Relation get_D_form (Relation & R)
    {
    Relation D(R.n_inp(), R.n_out());
    
    R.make_level_carried_to(R.n_inp());
    assert(R.has_single_conjunct());
    int n_zero=0;
    for (DNF_Iterator d(R.query_DNF()); d.live(); d.next())
       n_zero=d.curr()->query_guaranteed_leading_0s();
    
    Relation Diff=Deltas(copy(R)); 

    if (detailedClosureDebug) 
	{
	fprintf(DebugFile, "The relation projected onto differencies is:\n");
	Diff.print_with_subs(DebugFile);
	}


    /* now form D */

    int i;
    coef_t l,u;
    F_And * N = D.add_and();
    GEQ_Handle g;
    for (i=1; i<=Diff.n_set(); i++)
	{
	Diff.query_variable_bounds(Diff.set_var(i), l,u);
/*        if (i== n_zero+1 && l==negInfinity)
          l=1; */
	if (l!=negInfinity)
	    {
	    g=N->add_GEQ();
	    g.update_coef(D.input_var(i),-1);
	    g.update_coef(D.output_var(i),1);
	    g.update_const(-l);
	    g.finalize();
	    }
	if (u!=posInfinity)
	    {
	    g=N->add_GEQ();
	    g.update_coef(D.input_var(i),1);
	    g.update_coef(D.output_var(i),-1);
	    g.update_const(u);
	    g.finalize();
	    }
	} /* end for */

    /* add all stride constrains if they do exist */

    Conjunct *c = Diff.single_conjunct();

    if (c->locals().size()>0) // there are local variables
	{
	// now go through all the equalities
   
	coef_t coef=0;
	int pos=0;
	for (EQ_Iterator eq = c->EQs(); eq.live(); eq.next())
	    {
	    // constraint is in stride form if it has 2 vars, 
	    // one of which is wildcard. Count number if vars and wildcard vars
	    int nwild=0,nvar=0;
      
	    for (Constr_Vars_Iter cvi(*eq,false); cvi; cvi++)
		{
		if ((*cvi).var->kind() == Wildcard_Var)
		    {
		    coef=(*cvi).coef;
		    nwild++;
		    }
		else
		    pos=(*cvi).var->get_position();
		nvar++;
		}
	    if (nvar==2 && nwild==1) //stride constraint
		{
		EQ_Handle e=N->add_stride(coef);
		e.update_coef(D.input_var(pos),-1);
		e.update_coef(D.output_var(pos),1);
		e.finalize();
		}
	    }
	} // end search of stride constrains
 
    D.finalize();
    D.simplify();
    return D;
    }  /* end get_D_form */

/****
 * get relation A x A describing a region of domain and range:
 *   A=Hull(Domain(R), Range(R)) intersection IterationSpace
 *   returns cross product A x A
 ***/

Relation form_region(const Relation &R, const Relation& IterationSpace)
    {
    Relation H=Union(Domain(copy(R)), Range(copy(R)));
    H.simplify(1,1);
    H = EQs_to_GEQs(H);
    H=Hull(H);
    Relation A=Intersection(H, copy(IterationSpace));
    Relation A1=A;
    return Cross_Product(A,A1);
    } /* end form_region */

Relation form_region1(const Relation &R, const Relation& IterationSpace)
    {
    Relation Dom=Intersection(Domain(copy(R)), copy(IterationSpace));
    Relation Ran=Intersection(Range(copy(R)), copy(IterationSpace));
    return Cross_Product(Dom,Ran);
    } /* end form_region */


/****
 * Check if we can use D instead of R 
 *  i.e.  D intersection (A cross A) is Must_Be_Subset of R
 ***/

bool isD_OK(Relation &R, Relation &D, Relation &AxA)
    {
    Relation B=Intersection(copy(D), copy(AxA));
    B.simplify();

    if (detailedClosureDebug) 
	{
	fprintf(DebugFile, "Intersection of D and AxA is:\n");
	B.print_with_subs(DebugFile);
	}  
    assert (Must_Be_Subset(copy(R),copy(B)));

    return Must_Be_Subset(B, copy(R));
    } /* end isD_OK */



/****
 * check if the constraint is a stride one. Here we say that an equality
 * constraint is a stride constraint if it has exatly one wildcard.
 * The function returns number of the wildcards in the constraint.
 * So if we know that constraint is from the relation in D form, then
 * it cannot have more than 1 wildcard variables, and the result of
 * this functions can be treated as bool.
 ***/

static int is_stride(const EQ_Handle &eq)
    {
    int n=0;
 
    for (Constr_Vars_Iter cvi(eq,true); cvi; cvi++)
	n++;

    return n;
    } /* end is_stride */
  


/*****
 * check if the constraint is in the form i_k' - i_k comp_op  c
 * return v - the number of the var and the type of the comp_op:
 *  1 - >,  -1 - <, 0  - not in the right form
 * if this is equality constraint in the right form any 1 or -1 can be
 * returned
 ******/

static coef_t is_constraint_in_D_form (
    Relation & r, const Constraint_Handle &h,  int&  v)
    {
    int i;
    coef_t c_in,c_out=0;


    v=-1;

    for (i=1; i<=r.n_inp(); i++)
	{
	c_in=h.get_coef(r.input_var(i));
	if (c_in)
	    {
	    if (v!=-1)
		return 0; 
	    v=i;
	    c_out=h.get_coef(r.output_var(i));
	    if (c_in*c_out!=-1)
		return 0;
	    }
	}
    return c_out;
    } /* end is_constraint_in_D_form */  


/***
 * Check if relation is in the D form
 *  D = {[ i_1,i_2,...,i_m] -> [j_1, j_2, ..., j_m ] :
 *         (forall p, 1<= p <= m) L_p <= j_p - i_p <= U_p && 
 *      j_p - i_p == M_p alpha_p};
 *  Right now we do not check for multiple stride constraints for one var.
 *  Probably they cannot exist in simplified conjunct
 *  This function will be used in assertions
 *****/

bool is_in_D_form(Relation & D)
    {
    /* check that D has one conjunct */

    if (! D.has_single_conjunct())
	return false;

    Conjunct * c=D.single_conjunct();

    if (D.global_decls()->size() != 0) // there are symbolic vars
	return false;

    if (D.n_inp() != D.n_out())
	return false;

    int n=D.n_inp();

    Tuple<int> bl(n), bu(n);

    for (int i=1; i<= n; i++)
	bl[i]=bu[i]=0;

    int v;
    coef_t res;

    for (EQ_Iterator eq = c->EQs(); eq.live(); eq.next())
	{
	if ((res=is_constraint_in_D_form(D,*eq,v))==0)
	    return false;
	int n_wild=is_stride(*eq);
	if (n_wild>=2) 
	    return false;
	if (n_wild==0) // not stride constraint
	    {
	    if (bl[v]  || bu[v])
		return false;
	    bl[v]=bu[v]=1;
	    }
	}

    for (GEQ_Iterator geq = c->GEQs(); geq.live(); geq.next())
	{
	if ((res=is_constraint_in_D_form(D,*geq,v))==0)
	    return false;
	if ((res>0 && bl[v]) || (res<0 && bu[v]))
	    return false;
	if (res>0)
	    bl[v]=1;
	else
	    bu[v]=1;
	}
 
    return true;
    } /* is_in_D_form */
 
        
#define get_D_plus_form(R) (get_D_closure(R,1))
#define  get_D_star_form(R) (get_D_closure(R,0))

/****
 * Get D+ or D* from the relation that is in D form
 * To get D+ calculate:
 *    D+= {[i1, i2 .. i_m] -> {j1, j2, ..., j_m]:
 *     exists s s.t. s>=1 and 
 *         (forall p, 1<= p <= m) L_p * s<= j_p - i_p <= U_p*s && 
 *      j_p - i_p == M_p alpha_p};
 * To get D* calculate almost the same relation but s>=0.
 * Parameter n is 1 for getting D+ and 0 for  D*
 ****/


Relation get_D_closure(Relation & D, int n)
    {
    assert (is_in_D_form(D));
    assert(n==0 || n==1);
 
    Conjunct *c=D.single_conjunct();
 
    Relation R(D.n_inp(), D.n_out());

    F_Exists * ex = R.add_exists();
    Variable_ID s = ex->declare("s");
    F_And * N = ex->add_and();

    /* add s>=1 or s>=0 */

    GEQ_Handle geq= N->add_GEQ();
    geq.update_coef(s,1);
    geq.update_const(-n);
    geq.finalize();

 
    /* copy and modify all the EQs */

    for (EQ_Iterator j= c->EQs(); j.live(); j.next())
	{
	EQ_Handle eq=N->add_EQ();
	copy_constraint(eq, *j);

	// if it's stride constraint do not change it 
     
	if (!is_stride(*j))
	    {
	    /* eq is j_k -i_k = c, replace c buy s*c */

	    eq.update_coef(s, (*j).get_const());
	    eq.update_const(-(*j).get_const());
	    }
	eq.finalize();
	}

    /* copy and modify all the GEQs */

    for (GEQ_Iterator gi= c->GEQs(); gi.live(); gi.next())
	{
	geq=N->add_GEQ();
	copy_constraint(geq, *gi);
    
	/* geq is j_k -i_k >=c or i_k-j_k >=c, replace c buy s*c */

	geq.update_coef(s,(*gi).get_const());
	geq.update_const(-(*gi).get_const());
	geq.finalize();
	}

    R.finalize();

    if (detailedClosureDebug) 
	{
	fprintf(DebugFile, "Simplified D%c is:\n", n==1?'+':'*');
	R.print_with_subs(DebugFile);
	}

    return R;
    } /* end get_D_closure */


/***
 * Check if we can easily calculate the D* (D* will be convex).
 * We can calculate D* if all differences have both lower and upper
 * bounds to be non -/+ infinity
 ***/


bool can_get_D_star_form(Relation &D)
    {
    assert(is_in_D_form(D));
    Conjunct *c=D.single_conjunct();
 
    int n=D.n_inp();
    Tuple<int> bl(n), bu(n);
    int i;

    for (i=1; i<=n; i++)
	bl[i]=bu[i]=0;

    for (EQ_Iterator  eq = c->EQs(); eq.live(); eq.next())
	{
	// do not check stride constraints
	if (!is_stride(*eq))
	    {
	    for (i=1; i<=n; i++) 
		{
		if ((*eq).get_coef(D.input_var(i)) !=0 )
		    bl[i]=bu[i]=1;
		}
	    }
	}
    
  
    for (GEQ_Iterator geq = c->GEQs(); geq.live(); geq.next())
	{
	for (i=1; i<=n; i++)
	    {
	    coef_t k;
	    if ((k=(*geq).get_coef(D.input_var(i))) != 0)
		{
		if (k>0)
		    bu[i]=1;
		else
		    bl[i]=1;
		} 
	    }
	}

    for (i=1; i<=n; i++)
	if (!bl[i] || !bu[i])
	    return false;

    return true;
    } /* end can_get_D_star_form */



/*****
 * Check whether the relation intersect with identity or not
 ****/

bool does_intersect_with_identity(Relation &R)
    {
    assert (R.n_inp() == R.n_out());

    Relation I=Identity(R.n_inp());
    Relation C=Intersection(I, copy(R));
    return C.is_upper_bound_satisfiable();
    }

bool does_include_identity(Relation &R) {
   Relation I=Identity(R.n_inp());
   return Must_Be_Subset(I, copy(R));
}

/*****
 * Bill's closure: check if it is possible to calculate transitive closure
 * of the relation using the Bill's algorithm.
 * Return the transitive closure relation if it is possible and null relation
 * otherwise
 ****/

bool Bill_closure(Relation &R, Relation& IterationSpace, Relation & R_plus,
                  Relation & R_star)
    {
#ifdef TC_STATS
     fprintf(statsfile,"start bill closure\n");
#endif

    if (does_include_identity(R))
       return false;

    if (detailedClosureDebug)
	{
	fprintf(DebugFile, "\nApplying Bill's method to calculate transitive closure\n");
	}
 
// get D and AxA
    Relation D=get_D_form(R);


    if (detailedClosureDebug) 
	{
	fprintf(DebugFile,"\n D form for the relation:\n");
	D.print_with_subs(DebugFile);
	}

    Relation AxA=form_region1(R, IterationSpace);



    if (detailedClosureDebug) 
	{ 
	fprintf(DebugFile, "\n AxA for the relation:\n");
	AxA.print_with_subs(DebugFile);
	}


// compute R_+  

    R_plus=Intersection(get_D_plus_form(D), copy(AxA));

    if (detailedClosureDebug) 
	{
	fprintf(DebugFile, "\nR_+= D+ intersection AxA is:\n");
	R_plus.print_with_subs(DebugFile);
	}

// compute R_*
    R_star=Intersection(get_D_star_form(D), form_region(R,IterationSpace));

    if (detailedClosureDebug) 
	{
	fprintf(DebugFile, "\nR_*= D* intersection AxA is:\n");
	R_star.print_with_subs(DebugFile);
	}

/*        Check that R_+ is acyclic. 
          Given the way we constructed R_+, R_+=(R_+)+.
          As a result it's enough to verify that R_+ intersection I = 0,
          to prove that R_+ is acyclic.
 */


        if (does_intersect_with_identity(R_plus)) {
          if (detailedClosureDebug) {
            fprintf(DebugFile,"R_+ is not acyclic.\n");
          } 
          return false;
        }

	//Check R_+ - R is Must_Be_Subset of R o R_+

	if (!Must_Be_Subset(Difference(copy(R_plus), copy(R)),
		    Composition(copy(R), copy(R_plus))))
	    {
#if defined(TC_STATS)
		fprintf(statsfile, "R_+ -R is not a Must_Be_Subset of R o R_+\n");
		fprintf(statsfile, "Bill Method is not applicable\n");
#endif
	    return false;
	    }
	if (detailedClosureDebug) 
	    {
	    fprintf(DebugFile, "R_+ -R is a Must_Be_Subset of R o R_+ - good\n");
	    }
	

// if we are here than all tests worked, and R_+ is transitive closure
// of R.

#if defined(TC_STATS)
	fprintf(statsfile,"\nAll three tests succeeded -- exact closure found\n");
	fprintf(statsfile, "Transitive closure is R_+\n");
	}
#endif
//    assert(isD_OK(R,D,AxA));
    return true;
    } // end Bill_closure




/**********************************************************************
 * print the relation given the bounds on the iteration space
 * If the bounds are unknown (Bounds is Null), then just print relation
 * itself
 ****/

void print_given_bounds( const Relation&  R1, NOT_CONST Relation& input_Bounds)
    {
    Relation & Bounds = (Relation &)input_Bounds;
    Relation r;
    if (Bounds.is_null())
	r=R1;
    else 
	r = Gist(copy(R1),copy(Bounds),1);
    r.print_with_subs(DebugFile);
    }

/**********************************************************************
 * Investigate closure:
 * checks if the copmuted approximation on the Transitive closure
 * is upper and lower bound. If it's both - it's exact.
 * This function doesn't return any value. It's just prints a lot
 * of debug output
 * INPUT:
 *    r  - relation
 *    r_closure - approximation on r+.
 *    F - iteration space
 **********************************************************************/

void InvestigateClosure(Relation r, Relation r_closure, Relation F)
    {
    Relation r3;
    bool LB_res, UB_res;

    if (!F.is_null())
	F=Cross_Product(copy(F),copy(F));

    fprintf(DebugFile, "\n\n--->investigating the closure of the relation:\n");
    print_given_bounds(r,F);


    fprintf(DebugFile, "\nComputed closure is:\n");
    print_given_bounds(r_closure,F);


    r3=Composition(copy(r),copy(r_closure));
    r3.simplify(1,1);

    r3=Union(r3,Composition(copy(r_closure),copy(r)));
    r3.simplify(1,1);


    r3=Union(r3,copy(r));
    r3.simplify(1,1);


    Relation remainder = Difference(copy(r3),copy(r_closure));

    if (!F.is_null())
	{
	r3=Gist(r3,F,1);
	}
    r3.simplify(1,1);


    if (!F.is_null())
	{
	r_closure=Gist(r_closure,F,1);
	}
    r_closure.simplify(1,1);


    LB_res= Must_Be_Subset(copy(r_closure),copy(r3));


    UB_res=Must_Be_Subset(copy(r3),copy(r_closure)); 

    fprintf(DebugFile,"\nThe results of checking closure (gist) are:\n");
    fprintf(DebugFile,"LB - %s, UB - %s\n",
	    LB_res?"YES":"NO",
	    UB_res?"YES":"NO");

    if (!UB_res) 
	{
	remainder.simplify(2,2);
	fprintf(DebugFile,"Dependences not included include:\n");
	print_given_bounds(remainder,F);
	};

    }  



/****
 * Transitive closure of the relation containing single conjunct
 ****/

bool   ConjunctTransitiveClosure (
    NOT_CONST Relation & input_R, Relation & IterationSpace, 
    Relation & R_plus, Relation & R_star)
    {
    Relation R = consume_and_regurgitate(input_R);
    assert(R.has_single_conjunct());

    if (printConjunctClosure) {
	fprintf(DebugFile,"\nTaking closure of the single conjunct: [\n");
	R.print_with_subs(DebugFile);
	}
#ifdef TC_STATS	
    fprintf(statsfile,"start conjuncttransitiveclosure\n");
    singles++;
#endif

    if (is_closure_itself(copy(R))) { 
#ifdef TC_STATS	
	      fprintf(statsfile, "Relation is closure itself\n");
#endif
       int ndim_all, ndim_domain;
        R.dimensions(ndim_all,ndim_domain);
        if (ndim_all == ndim_domain +1)
            {

	      Relation ispace =  Cross_Product(Domain(copy(R)),Range(copy(R)));
	      Relation R_zero = Intersection(copy(ispace),Identity(R.n_inp()));
	      R_star  = Hull(Union(copy(R),R_zero),true,1,ispace);
	      R_plus=R;
	      if (printConjunctClosure) {
		  fprintf(DebugFile, "\n] For this relation R+=R\n");
		  fprintf(DebugFile,"R*:\n");
		  R_star.print_with_subs(DebugFile);
		  }
	       return true;
	    }
        else
            {
            R_star=R;
            R_plus=R;
	    if (printConjunctClosure) {
		  fprintf(DebugFile, "\n] For this relation R+=R, not appropriate for R*\n");
		  }
            return false;
            }

       } 
     else  
     {
        bool done=false;
	if (!IterationSpace.is_null())
	    {
// Bill's closure requires the information about Iteration Space. 
// So if IterationSpace is NULL, i.e. unknown( e.g. when calling from parser,
// we do not do Bill's closure 
           
	    done  = Bill_closure(R, IterationSpace, R_plus, R_star);
#ifdef TC_STATS
	    fprintf(statsfile,"Bill closure is %sapplicable\n",done?"":"not ");
#endif
	    if (printConjunctClosure)   
		{
		if (!done)
		    fprintf(DebugFile, "Bill's closure is not applicable\n");
		else 
		    {
		    fprintf(DebugFile, "Bill's closure is applicable\n");
                    fprintf (DebugFile, " For R:\n");
                    R.print_with_subs(DebugFile);
                    fprintf(DebugFile, "R+ is:\n");
                    R_plus.print_with_subs(DebugFile);
                    fprintf(DebugFile, "R* is:\n");
                    R_star.print_with_subs(DebugFile);
                    fprintf(DebugFile, "\n");
		    InvestigateClosure(R, R_plus, IterationSpace);
		    }	
		 }
	    } 
	    if (done)  
		{
		if (printConjunctClosure) 
		    {
		    fprintf(DebugFile, "]\n");
		    }
		return true;
		}
	    else 
	      {
// do and check approximate closure (several compositions)
  	      R_plus  = approx_closure(copy(R), 2);
#ifdef TC_STATS
	      fprintf(statsfile,"Approximating closure with 2 compositions\n");
#endif
	      if (printConjunctClosure)
	      {
		fprintf(DebugFile, "Doing approximate closure\n");
		InvestigateClosure(R, R_plus, IterationSpace);
	      }
                  
             } //end else (!done after Bill Closure or Iteration space is NULL) 
      
          if (printConjunctClosure) 
	  {
	  fprintf(DebugFile, "]\n");
	  }
	  }
    return false;
    } /* end ConjunctTransitiveClosure */


/*********************************************************************
 * try to get conjunct transitive closure.
 * it we can get it easy get it, return true.
 * if not - return false
 ********************************************************************/


bool   TryConjunctTransitiveClosure (
    NOT_CONST Relation & input_R, Relation & IterationSpace, 
    Relation & R_plus) {

    Relation R = consume_and_regurgitate(input_R);
    assert(R.has_single_conjunct());
#ifdef TC_STATS	
    fprintf(statsfile,"start tryconjuncttransitiveclosure\n");
    singles++;
#endif

    if (printConjunctClosure) {
	fprintf(DebugFile,"\nTrying to take closure of the single conjunct: [\n");
	R.print_with_subs(DebugFile);
	}

    if (is_closure_itself(copy(R))) { 
#ifdef TC_STATS
       fprintf(statsfile, "Relation is closure itself, leave alone (try)\n");
#endif
       if (printConjunctClosure)
         fprintf(DebugFile, "\n ]The relation is closure itself. Leave it alone\n");
       return false;
       } 
     else  
     {
        bool done;
	assert(!IterationSpace.is_null());
        Relation R_star;
	done  = Bill_closure(R, IterationSpace, R_plus, R_star);
#ifdef TC_STATS
       fprintf(statsfile, "Bill closure is %sapplicable (try)\n", done?"":"NOT ");
#endif
	if (printConjunctClosure)   
	    {
	     if (!done)
		    fprintf(DebugFile, "]Bill's closure is not applicable\n");
	     else 
		    {
		    fprintf(DebugFile, "]Bill's closure is applicable\n");
                    fprintf (DebugFile, " For R:\n");
                    R.print_with_subs(DebugFile);
                    fprintf(DebugFile, "R+ is:\n");
                    R_plus.print_with_subs(DebugFile);
                    fprintf(DebugFile, "R* is:\n");
                    R_star.print_with_subs(DebugFile);
                    fprintf(DebugFile, "\n");
		    InvestigateClosure(R, R_plus, IterationSpace);
		    }	
		 }
         return done; 
	   }  
    //return false;
    } /* end ConjunctTransitiveClosure */


bool Equal (const Relation & r1, const Relation & r2)
    {
    bool res=Must_Be_Subset (copy(r1), copy(r2));
    if (!res)
	return false;
    return Must_Be_Subset (copy(r2),copy(r1));
    } /* end Equal */


void appendClausesToList(Simple_List<Relation> &L, Relation &R) 
    {
    R.make_level_carried_to(R.n_inp());
    R.simplify(2,2);
    for(int depth = R.n_inp(); depth >= -1; depth--)
	for (DNF_Iterator d(R.query_DNF()); d.live(); d.next())
	    if (d.curr()->query_guaranteed_leading_0s() == depth)
		{
		L.append(Relation(R, d.curr()));
		}   
    }

void printRelationList(Simple_List<Relation> &L) 
    {
    for (Simple_List_Iterator<Relation> li(L); li.live(); li.next())
	{
	li.curr().print_with_subs(DebugFile);
	}
    }

/****
 * Transitive closure of the relation containing multiple conjuncts
 * New (Bill's) version
 ***/

	    
	    
	

Relation TransitiveClosure0(NOT_CONST Relation &input_r, 
			      int maxExpansion,
			      NOT_CONST Relation & input_IterationSpace)
    {

    Relation r = consume_and_regurgitate(input_r);
    Relation IterationSpace = consume_and_regurgitate(input_IterationSpace);

    if (closure_presburger_debug) 
	fprintf(DebugFile, "\n\n[Transitive closure\n\n");

Relation result;

#ifdef TC_STATS
#define TC_RUNS 1
    int in_conj = copy(r).query_DNF()->length();
    totals++;
    fprintf(statsfile,"%d closure run\n", totals);
    if(is_in_D_form(copy(r))) 
	fprintf(statsfile, "Relation initially in D form\n");
    else
	fprintf(statsfile, "Relation initially NOT in D form\n");
     if(is_lex_forward(copy(r)))
	fprintf(statsfile, "Relation is initially lex forw\n");
    else
	fprintf(statsfile, "Relation is NOT initially lex forw\n");
    start_clock();
    for(int tc_loop = 1; tc_loop <= TC_RUNS; tc_loop++){
    singles = 0;
#endif

    assert(!r.is_null());
    assert(r.n_inp() == r.n_out());

    if (r.max_ufs_arity() > 0)
	{
	assert(r.max_ufs_arity() == 0 &&
	       "Can't take transitive closure with UFS yet.");

	fprintf(stderr,
		"Can't take transitive closure with UFS yet.");
	Exit(1);
	}

    r.simplify(2,2);
    if (!r.is_upper_bound_satisfiable()) {
#ifdef TC_STATS
	int totalTime = clock_diff();
	fprintf(statsfile, "Relation is unsatisfiable\n");
	fprintf(statsfile, "input conj: %d   output conj: %d   #singe conj closures: %d   time: %d\n",
	in_conj, copy(result).query_DNF()->length(),
	singles,
	totalTime/TC_RUNS);
#endif


      if (closure_presburger_debug) 
	fprintf(DebugFile, "]TC : relation is false\n");
      return r;
    }
    IterationSpace = Hull(
			Union(Domain(copy(r)),Range(copy(r))),
				true,1,IterationSpace);

    if (detailedClosureDebug) 
	{
	fprintf(DebugFile, "r is:\n");
	r.print_with_subs(DebugFile);
	fprintf(DebugFile, "IS is:\n");
	IterationSpace.print_with_subs(DebugFile);
	}
    Relation dom = Domain(copy(r));
    dom.simplify(2,1);
    Relation rng = Range(copy(r)); 
    rng.simplify(2,1);
    Relation AC = ConicClosure(Restrict_Range(Restrict_Domain(copy(r),copy(rng)),copy(dom)));
    Relation UB = Union(copy(r),Join(copy(r),Join(AC,copy(r))));
    UB.simplify(2,1);
    
    if (detailedClosureDebug) 
	{
	fprintf(DebugFile, "UB is:\n");
	UB.print_with_subs(DebugFile);
	}
    result = Relation::False(r); 
    Simple_List<Relation> firstChoice,secondChoice;


    r.simplify(2,2);

    Relation test = Difference(copy(r),Composition(copy(r),copy(r)));
    test.simplify(2,2);
    if (r.number_of_conjuncts() > test.number_of_conjuncts())  {
      Relation test2 = Union(copy(test),Composition(copy(test),copy(test)));
      test2.simplify(2,2);
      if (Must_Be_Subset(copy(r),copy(test2))) r = test;
      else if (detailedClosureDebug) {
	fprintf(DebugFile, "Transitive reduction not possible:\n");
	fprintf(DebugFile, "R is:\n");
	r.print_with_subs(DebugFile);
	fprintf(DebugFile, "test2 is:\n");
	test2.print_with_subs(DebugFile);
	}
      }

    r.make_level_carried_to(r.n_inp());
    if (detailedClosureDebug) 
	{
	fprintf(DebugFile, "r is:\n");
	r.print_with_subs(DebugFile);
	}
    for(int depth = r.n_inp(); depth >= -1; depth--)
	for (DNF_Iterator d(r.query_DNF()); d.live(); d.next())
	    if (d.curr()->query_guaranteed_leading_0s() == depth)
		{
		Relation C(r, d.curr());
                firstChoice.append(C);
		}   

    bool first_conj=true; 
    for (Simple_List_Iterator<Relation> sli(firstChoice); sli; sli++)
      {
       if (first_conj)
         first_conj=false;
       else {
         Relation C_plus;
         bool change=TryConjunctTransitiveClosure(
            copy(sli.curr()), IterationSpace, C_plus);
         if (change)
           sli.curr()=C_plus;
       }
     }
      
//compute closure
    int maxClauses = 3+firstChoice.size()*(1+maxExpansion);



    int resultConjuncts = 0;
    int numFails = 0;
    bool resultInexact = false;
    while (!firstChoice.empty() || !secondChoice.empty())
	{
	Relation R_plus, R_star;

	if (detailedClosureDebug) 
	    {
	    fprintf(DebugFile,"Main loop of TC:\n");
	    if (!firstChoice.empty()) 
		{
		fprintf(DebugFile,"First choice:\n");
		printRelationList(firstChoice);
		};
	    if (!secondChoice.empty()) 
		{
		fprintf(DebugFile,"Second choice:\n");
		printRelationList(secondChoice);
		};
	    }
  
	Relation R;
	if (!firstChoice.empty()) 
	    R = firstChoice.remove_front();
	else R = secondChoice.remove_front();

	if (detailedClosureDebug) 
	    {
	    fprintf(DebugFile, "Working with conjunct:\n");
	    R.print_with_subs(DebugFile);
	    }

	bool known=ConjunctTransitiveClosure(copy(R),IterationSpace,
					     R_plus, R_star);

	if (!known && numFails < firstChoice.size())  
	    {
	    numFails++;
	    firstChoice.append(R);
	    if (detailedClosureDebug)
		{
		fprintf(DebugFile, "\nTry another conjunct, R is not suitable\n");
		R.print_with_subs(DebugFile);
		}
	    continue;
	    }


	if (detailedClosureDebug)
	    {
	    fprintf(DebugFile,"\nR+ is:\n");
	    R_plus.print_with_subs(DebugFile);
	    if (known) 
		{
		fprintf(DebugFile, "Known R? is :\n");
		R_star.print_with_subs(DebugFile);
		}
	    else
		fprintf(DebugFile, "The R* for this relation is not calculated\n");
	    }
        
	Relation R_z;
	if (known) 
	    {
	    R_z=Difference(copy(R_star),copy(R_plus));
	    known = R_z.is_upper_bound_satisfiable();
	    if (known) 
		{
		int d = R.single_conjunct()->query_guaranteed_leading_0s();
		R_z.make_level_carried_to(min(R.n_inp(),d+1));
		if (R_z.query_DNF()->length() > 1) known = false;
		if (detailedClosureDebug)
		    {
		    fprintf(DebugFile, "\nForced R_Z to be level carried at level %d\n",min(R.n_inp(),d+1));
		    }
		}
	    if (detailedClosureDebug)
		{
		if (known)  
		    {
		    fprintf(DebugFile, "\nDifference between R? and R+ is:\n");
		    R_z.print_with_subs(DebugFile);
		    }
		else
		    fprintf(DebugFile, "\nR_z is unusable\n");
		}
	    }
	else R_z = Relation::False(r);

	if (!known)
	    numFails++;
	else numFails = 0;
	if (!known && numFails <= firstChoice.size())  
	    {
	    firstChoice.append(R);
	    if (detailedClosureDebug)
		{
		fprintf(DebugFile, "\nTry another conjunct, Rz is avaiable for R:\n");
		R.print_with_subs(DebugFile);
		}
	    continue;
	    }



	//make N empty list
	Relation N = Relation::False(r);
 
	//append R+ to T
	result = Union(result, copy(R_plus));
	resultConjuncts++;

	int expansion = maxClauses 
	    - (resultConjuncts + 2*firstChoice.size() + secondChoice.size());
	if (expansion < 0) expansion = 0;
	if (detailedClosureDebug)
	    {
	    fprintf(DebugFile,"Max clauses = %d\n",maxClauses);
	    fprintf(DebugFile,"result conjuncts =  %d\n",resultConjuncts);
	    fprintf(DebugFile,"firstChoice's =  %d\n",firstChoice.size());
	    fprintf(DebugFile,"secondChoice's =  %d\n",secondChoice.size());
	    fprintf(DebugFile,"Allowed expansion is %d\n",expansion);
	    }


	bool firstPart=true;
	if (!known && expansion == 0) 
	    {
	    if (detailedClosureDebug)
		{
		fprintf(DebugFile,"Expansion = 0, R? unknown, skipping composition\n");
		}
	    if (!resultInexact && detailedClosureDebug) fprintf(DebugFile,"RESULT BECOMES INEXACT 1\n");
	    resultInexact = true;
	    }
	else 
	    for (Simple_List_Iterator<Relation> s(firstChoice); 
		 firstPart?
		     (s.live()?true:
		      (s = Simple_List_Iterator<Relation>(secondChoice),
		       firstPart = false,
		       s.live()))
		     :s.live();
		 s.next())
		{
		assert(s.live());
		Relation C=(s.curr());
		if (detailedClosureDebug)
		    {
		    fprintf(DebugFile, "\nComposing chosen conjunct with C:\n");
		    C.print_with_subs(DebugFile);
		    }


		if (!known)
		    {
		    if (detailedClosureDebug) 
			{
			fprintf(DebugFile, "\nR? is unknown! No debug info here yet\n");
			}
			Relation C1=Composition(copy(C), copy(R_plus));
			if (detailedClosureDebug) 
			    {
			    fprintf(DebugFile, "\nGenerating \n");
			    C1.print_with_subs(DebugFile);
			    }
			C1.simplify();
			Relation newStuff =
				Difference(
					Difference(copy(C1),copy(C)),
					copy(R_plus));
			newStuff.simplify();
			if (detailedClosureDebug) 
			    {
			    fprintf(DebugFile, "New Stuff:\n");
			    newStuff.print_with_subs(DebugFile);
			    }
			bool C1_contains_new_stuff = 
				newStuff.is_upper_bound_satisfiable();
			if (C1_contains_new_stuff)
			  {
			    if (newStuff.has_single_conjunct())
				C1 = newStuff;
			    if (expansion) {
			      N = Union(N,copy(C1));
			      expansion--;
			      }
			    else {
			        if (!resultInexact && detailedClosureDebug) fprintf(DebugFile,"RESULT BECOMES INEXACT 2\n");
				resultInexact = true;
				break;
				}
			  }
			else C1 = Relation::False(C1);

			    Relation C2(Composition(copy(R_plus),copy(C)));
			    if (detailedClosureDebug) 
				{
				fprintf(DebugFile, "\nGenerating \n");
				C2.print_with_subs(DebugFile);
				}
			     newStuff =
				Difference(
				  Difference(
					Difference(copy(C2),copy(C)),
					copy(C1)),
					copy(R_plus));
			     newStuff.simplify();
			     if (detailedClosureDebug) 
			      {
			      fprintf(DebugFile, "New Stuff:\n");
			      newStuff.print_with_subs(DebugFile);
			      }
			    if (newStuff.is_upper_bound_satisfiable())  {
			      if (newStuff.has_single_conjunct())
				C2 = newStuff;
			      if (expansion) {
			        N = Union(N,copy(C2));
			        expansion--;
			        }
			    else {
			        if (!resultInexact && detailedClosureDebug) fprintf(DebugFile,"RESULT BECOMES INEXACT 3\n");
				resultInexact = true;
				break;
				}
			     }
			   else C2 = Relation::False(C2);
			   
			if (C1_contains_new_stuff) { 
			    Relation C3(Composition(copy(R_plus),copy(C1)));
			    if (detailedClosureDebug) 
				{
				fprintf(DebugFile, "\nGenerating \n");
				C3.print_with_subs(DebugFile);
				}
			     newStuff =
				Difference(
				Difference(
				Difference(
					Difference(copy(C3),copy(C)),
					copy(C1)),
					copy(C2)),
					copy(R_plus));
			     newStuff.simplify();
			     if (detailedClosureDebug) 
			      {
			      fprintf(DebugFile, "New Stuff:\n");
			      newStuff.print_with_subs(DebugFile);
			      }
			    if (newStuff.is_upper_bound_satisfiable()) {
			      if (newStuff.has_single_conjunct())
				C3 = newStuff;
			      if (expansion) {
			        N = Union(N,C3);
			        expansion--;
			        }
			    else {
					      if (!resultInexact && detailedClosureDebug) fprintf(DebugFile,"RESULT BECOMES INEXACT 4\n");
				resultInexact = true;
				break;
				}
			     }
				}
			
		  }
		else 
		    {

		    Relation C_Rz(Composition(copy(C),copy(R_z)));
		    if (detailedClosureDebug) 
			{
			fprintf(DebugFile, "C o Rz is:\n");
			C_Rz.print_with_subs(DebugFile);
			}

		    Relation Rz_C_Rz(Composition(copy(R_z),copy(C_Rz)));
		    if (detailedClosureDebug) 
			{
			fprintf(DebugFile, "\nRz o C o Rz is:\n");
			Rz_C_Rz.print_with_subs(DebugFile);
			} 

        
		    if (Equal(C,Rz_C_Rz)) 
			{
#if defined(TC_STATS)
			    fprintf(statsfile,"weak test selects C?\n");
#endif
			Relation tmp = Composition(C,copy(R_star));
			tmp.simplify();
			Relation tmp2 = Composition(copy(R_star),copy(tmp));
			tmp2.simplify();
			if (Must_Be_Subset(copy(tmp2),copy(tmp)))
				*s = tmp;
			else
				*s = tmp2;
			if (detailedClosureDebug) 
			    {
			    fprintf(DebugFile,"\nC is equal to Rz o C o Rz so  R? o C o R? replaces C\n");
			    fprintf(DebugFile, "R? o C o R? is:\n");
			    (*s).print_with_subs(DebugFile);
			    }
			}
		    else 
			{

#if defined(TC_STATS)
			    fprintf(statsfile,"weak test fails\n");
#endif
			if (Equal(C, C_Rz))
			    {
			    *s=Composition(copy(C),copy(R_star));
			    Relation  p(Composition(copy(R_plus), copy(*s)));
			    p.simplify();
			    if (detailedClosureDebug) 
				{
				fprintf(DebugFile, "\nC is equal to C o Rz, so C o Rz replaces C\n");
				fprintf (DebugFile, "C o R? is:\n");
				(*s).print_with_subs(DebugFile);
				fprintf (DebugFile, "R+ o C o R? is added to list N. It's :\n");
				p.print_with_subs(DebugFile);
				}         
			    if (!Is_Obvious_Subset(copy(p),copy(R_plus))
				&& !Is_Obvious_Subset(copy(p),copy(C)))
				if (expansion)  {
                                   p.simplify(2,2); 
				   expansion--;
				   }
			    else {
					      if (!resultInexact && detailedClosureDebug) fprintf(DebugFile,"RESULT BECOMES INEXACT 5\n");
				resultInexact = true;
				break;
				}
			    }
			else
			    {
			    Relation Rz_C(Composition(copy(R_z),copy(C)));

			    if (Equal(C,Rz_C))
				{
				*s=Composition(copy(R_star),copy(C));
				Relation Rstar_C_Rplus(Composition(copy(*s),copy(R_plus)));   
			        Rstar_C_Rplus.simplify();
				if (detailedClosureDebug) 
				    {
				    fprintf(DebugFile, "\nC is equal to Rz o C , so R? o C replaces C\n");
				    fprintf (DebugFile, "R? o C is:\n");
				    (*s).print_with_subs(DebugFile);
				    fprintf (DebugFile, "R+ o C is added to list N. It's :\n");
				    Rstar_C_Rplus.print_with_subs(DebugFile);
				    }  
				if (!Is_Obvious_Subset(copy(Rstar_C_Rplus),copy(R_plus))
				    && !Is_Obvious_Subset(copy(Rstar_C_Rplus),copy(C)))
				    if (expansion) 
					N = Union(N,Rstar_C_Rplus);
				    else {
					      if (!resultInexact && detailedClosureDebug) fprintf(DebugFile,"RESULT BECOMES INEXACT 6\n");
					resultInexact = true;
					break;
					}

				}
			    else
				{
				if (detailedClosureDebug) 
				    {
				    fprintf(DebugFile, "\nHave to handle it the hard way\n");
				    }
				    Relation C1=Composition(copy(C), copy(R_plus));
				    C1.simplify();
				    if (!Is_Obvious_Subset(copy(C1),copy(R_plus))
					&& !Is_Obvious_Subset(copy(C1),copy(C)))
					if (expansion) {
					   N = Union(N,copy(C1));
					   expansion--;
					   }
				        else {
					      if (!resultInexact && detailedClosureDebug) fprintf(DebugFile,"RESULT BECOMES INEXACT 7\n");
					  resultInexact = true;
					  break;
					  }
			
					Relation C2(Composition(copy(R_plus),copy(C)));
					C2.simplify();
					if (!Is_Obvious_Subset(copy(C2),copy(R_plus))
					    && !Is_Obvious_Subset(copy(C2),copy(C)))
					    if (expansion) {
					      N = Union(N,C2);
					      expansion--;
					      }
				            else {
					      if (!resultInexact && detailedClosureDebug)  {
						fprintf(DebugFile,"RESULT BECOMES INEXACT 8\n");
						fprintf(DebugFile,"Have to discard:\n");
					        C2.print_with_subs(DebugFile);
						}
					      resultInexact = true;
					      break;
					      }
					    Relation C3(Composition(copy(R_plus),C1));
					    C3.simplify();
					    if (!Is_Obvious_Subset(copy(C3),copy(R_plus))
						&& !Is_Obvious_Subset(copy(C3),copy(C)))
					    if (expansion) {
					      N = Union(N,C3);
					      expansion--;
					      }
				            else {
					      if (!resultInexact && detailedClosureDebug) fprintf(DebugFile,"RESULT BECOMES INEXACT 9\n");
					      resultInexact = true;
					      break;
					      }
					    }
					}
				}
			    }
		} //for


//now we processed the first conjunct.

	if (detailedClosureDebug) 
	    {
	    N.simplify(2,2);
	    fprintf(DebugFile, "\nNew conjuncts:\n");
	    N.print_with_subs(DebugFile);
	    }
        
        N.simplify(2,2);
	appendClausesToList(secondChoice,N);
                   
	} //while             
                
  
//Did we do all conjuncts? If not, make T be inexact

    result.copy_names(r);

    result.simplify(2,2);

    if (!result.is_exact()) {
	result = Lower_Bound(result);
	resultInexact = true;
	}
    if (resultInexact)
	{
	Relation test(Composition(copy(result),copy(result)));
	test.simplify(2,2);
	if (detailedClosureDebug) 
	    {
	    fprintf(DebugFile, "\nResult is:\n");
	    result.print_with_subs(DebugFile);
	    fprintf(DebugFile, "\nResult composed with itself is:\n");
	    test.print_with_subs(DebugFile);
	    }
	if (!Must_Be_Subset(test,copy(result)))
	    {
	    result = Union(result,Intersection(UB, Relation::Unknown(result)));
	    }
	}


#ifdef TC_STATS
    {
	Relation rcopy = result;
	Relation test2(Composition(copy(rcopy),copy(rcopy)));
	test2.simplify(2,2);
	test2.remove_disjunction_with_unknown();
	rcopy.remove_disjunction_with_unknown();
	if (detailedClosureDebug) 
	    {
	    fprintf(DebugFile, "\nResult is:\n");
	    rcopy.print_with_subs(DebugFile);
	    fprintf(DebugFile, "\nResult composed with itself is:\n");
	    test2.print_with_subs(DebugFile);
	    }
	if (!Must_Be_Subset(test2,copy(rcopy)))
	    {
	    fprintf(statsfile,"multi TC result is inexact\n");
	    }
	else 
	    fprintf(statsfile,"TC result is exact%s\n",
		    (resultInexact || !rcopy.is_exact())?" despite perceived inexactness":"");
    }
#endif



#ifdef TC_STATS
    }
int totalTime = clock_diff();
fprintf(statsfile, "input conj: %d   output conj: %d   #singe conj closures: %d   time: %d\n",
	in_conj, copy(result).query_DNF()->length(),
	singles,
	totalTime/TC_RUNS);
#endif

    if (closure_presburger_debug || detailedClosureDebug)  {
        if (detailedClosureDebug)  {
	  fprintf(DebugFile, "\nThe transitive closure is :\n");
	  result.print_with_subs(DebugFile);
	  }
	fprintf(DebugFile, "\n\n] END Transitive closure\n\n");
	}
    return result;
    } /* end TransitiveClosure */


Relation TransitiveClosure(NOT_CONST Relation &input_r, 
			      int maxExpansion,
			      NOT_CONST Relation & input_IterationSpace) {
    Relation r = consume_and_regurgitate(input_r);
    Relation IterationSpace = consume_and_regurgitate(input_IterationSpace);


    if (closure_presburger_debug) 
	{
	fprintf(DebugFile,"\nComputing Transitive closure of:\n");
	r.print_with_subs(DebugFile);
	fprintf(DebugFile,"\nIteration space is:\n");
	IterationSpace.print_with_subs(DebugFile);
	}

    if (!r.is_upper_bound_satisfiable()) {
      if (closure_presburger_debug) 
	fprintf(DebugFile, "]TC : relation is false\n");
      return r;
    }


    Relation UB = DeltasToRelation(ConicHull(Project_Sym(Deltas(copy(r)))),
		r.n_inp(),r.n_out());
    UB.simplify();

    if (closure_presburger_debug) 
	{
	fprintf(DebugFile,"UB is:\n");
	UB.print_with_subs(DebugFile);
	}
    Relation conditions = Composition(copy(UB),Domain(copy(r)));

    if (closure_presburger_debug) 
	{
	fprintf(DebugFile,"Forward reachable is:\n");
	conditions.print_with_subs(DebugFile);
	}

    conditions = Composition(Inverse(copy(UB)),conditions);

    if (closure_presburger_debug) 
	{
	fprintf(DebugFile,"Backward/forward reachable is:\n");
	conditions.print_with_subs(DebugFile);
	}

    conditions.simplify();
    conditions = VennDiagramForm(conditions);
    conditions.simplify();

    if (conditions.is_obvious_tautology()) {
	return TransitiveClosure0(r, maxExpansion, IterationSpace);
	}
    if (closure_presburger_debug) 
	{
	fprintf(DebugFile,"Condition regions are:\n");
	conditions.print_with_subs(DebugFile);
	}


    Relation answer = Relation::False(r);
    answer.copy_names(r);
     for (DNF_Iterator c(conditions.query_DNF()); c.live(); ) {
	    Relation tmp = Relation(conditions,c.curr());
	    if (closure_presburger_debug) 
		{
		fprintf(DebugFile,"\nComputing Transitive closure:\n");
		fprintf(DebugFile,"\nRegion:\n");
		tmp.prefix_print(DebugFile);
		}
	    Relation tmp3 = Restrict_Domain(copy(r),tmp);
	    tmp3.simplify(2,2);
	    if (closure_presburger_debug) 
		{
		fprintf(DebugFile,"\nRelation:\n");
		tmp3.prefix_print(DebugFile);
		}
	    answer = Union(answer, TransitiveClosure0(tmp3, maxExpansion,copy(IterationSpace)));
            c.next();
	    }
    return answer;
    }

} // end of namespace omega
