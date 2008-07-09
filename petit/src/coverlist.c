/* $Id: coverlist.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdlib.h>
#include <basic/bool.h>
#include <basic/assert.h>
#include <omega/Relations.h>
#include <petit/lang-interf.h>
#include <petit/debug.h>
#include <petit/coverlist.h>
#include <petit/kill.h>
#include <petit/omega2flags.h>
#include <petit/petit_args.h>

namespace omega {

#define abs(x) ((x)>=0?(x):-(x))

#define USE_DEFINITE 1


/* find the maximum possible distance - works only if loop step > 0 FERD */

static int max_dir(dd_current dd, int j)
{
    if (dd_current_diff_known(dd,j))
    {
//	assert(dd->loop_dir[j] > 0);
	return dd_current_diff(dd)[j]* (dd_current_loop_dir(dd))[j];
    } else {
	dddirection dddir = ddextract1(dd_current_dir(dd),j);
	if (dddir & ddfwd) return posInfinity;
	if (dddir & ddind) return 0;
	if (dddir & ddbck) return -1; /* should be loop step size */
    }
    assert(0); return negInfinity;
}

static int min_dir(dd_current dd, int j)
{
    if (dd_current_diff_known(dd,j))
    {
//	assert(dd->loop_dir[j] > 0);
	return dd_current_diff(dd)[j]*(dd_current_loop_dir(dd))[j];
    } else {
	dddirection dddir = ddextract1(dd_current_dir(dd),j);
	if (dddir & ddbck) return negInfinity;
	if (dddir & ddind) return 0;
	if (dddir & ddfwd) return 1;  /* should be loop step size */
    }
    assert(0); return posInfinity;
}

/* return 1 iff we can prove dd1 <=c dd2
   return 0 if it is not true or we can't figure it out based only on dddirs */


/* order by <c!
   return 1 if f2 <c! f1, -1 if f1 <c! f2, 0 if neither */

static int definite_lt_c_order(dd_current f1, dd_current f2)
{
    int j;

    if (petit_args.dep_analysis_debug > 3) {
	fprintf(debug,"Checking to see if we can prove that\n");
	fprintf(debug," %s\n",dd_current_as_string(f1));
	fprintf(debug," is tighter than\n");
	fprintf(debug," %s\n",dd_current_as_string(f2));
	}
    for(j=1; j <= (int)std::min(dd_current_nest(f1),dd_current_nest(f2)); j++) {
	if (dd_current_diff_known(f1,j) && dd_current_diff_known(f2,j))
	{ 
	    if (dd_current_diff(f1)[j] < dd_current_diff(f2)[j])
		return -1;  /* f1 <c! f2 */
	    if (dd_current_diff(f2)[j] < dd_current_diff(f1)[j])
		return 1;   /* f2 <c! f1 */
	} else {
	    if (max_dir(f1,j) < min_dir(f2,j)) return -1;  /* f1 <c! f2 */
	    if (max_dir(f2,j) < min_dir(f1,j)) return 1;   /* f2 <c! f1 */

	    return 0;                    /* can\'t tell */
	}
    }

    // f1 and f2 have equal dep. distances up to common nest

    if ((int)dd_current_nest(f1) < access_shared_depth(dd_current_src(f1),dd_current_src(f2)))
	    return 0; 
       /* handle the case where the following is not correct if the sources of
	f1 and f2 share more loops than either of f1 and f2.
	For example, cholsky.t - 
	  flow    71: b(i,l,k+jj)     -->  84: b(i,l,n-kk)     (0)
	  flow    67: b(i,l,k)        -->  84: b(i,l,n-kk)     (0)
     */

    /* dependences are known to be equal in difference */
    
    if (access_lexically_preceeds(dd_current_src(f2),dd_current_src(f1)))
	return -1;  /* f1 <c! f2 */
    if (access_lexically_preceeds(dd_current_src(f1),dd_current_src(f2)))
	return 1;   /* f2 <c! f1 */
    
    return 0;
}


int definite_lt_c(dd_current f1, dd_current f2)
{
    return definite_lt_c_order(f1,f2) < 0;
}

/* For f1 and f2 that have the same source and different sinks
   try to prove that f1 is tighter than f2 or 
   f2 is tighter than f1
   return 1 if f2 is tighter than f1, 
         -1 if f1 is tighter than f2,
          0 if none, or cannot prove
 */

static int definite_lt_t_order(dd_current f1, dd_current f2)
{
    int j;

    if (petit_args.dep_analysis_debug > 3) {
	fprintf(debug,"Checking to see if we can prove that\n");
	fprintf(debug," %s\n",dd_current_as_string(f1));
	fprintf(debug," is tighter than\n");
	fprintf(debug," %s\n",dd_current_as_string(f2));
	}
    for(j=1; j <= (int)min(dd_current_nest(f1),dd_current_nest(f2)); j++) {
	if (dd_current_diff_known(f1,j) && dd_current_diff_known(f2,j))
	{ 
	    if (dd_current_diff(f1)[j] < dd_current_diff(f2)[j])
		return -1;  /* f1 <t! f2 */
	    if (dd_current_diff(f2)[j] < dd_current_diff(f1)[j])
		return 1;   /* f2 <t! f1 */
	} else {
	    if (max_dir(f1,j) < min_dir(f2,j)) return -1;  /* f1 <t! f2 */
	    if (max_dir(f2,j) < min_dir(f1,j)) return 1;   /* f2 <t! f1 */

	    return 0;                    /* can\'t tell */
	}
    }

    // f1 and f2 have equal dep. distances up to common nest

    if ((int)dd_current_nest(f1) < access_shared_depth(dd_current_dest(f1),dd_current_dest(f2)))
	    return 0; 
 
    /* dependences are known to be equal in difference */
    
    if (access_lexically_preceeds(dd_current_dest(f1),dd_current_dest(f2)))
	return -1;  /* f1 <t! f2 */
    if (access_lexically_preceeds(dd_current_dest(f2),dd_current_dest(f1)))
	return 1;   /* f2 <t! f1 */
    
    return 0;
}

bool definite_lt_t(dd_current f1, dd_current f2)
{
    return definite_lt_t_order(f1,f2) < 0;
}


int approx_leq_c(dd_current dd1, dd_current dd2)
{
    dd_in_iterator o;
    int possible = 1;

    assert(dd_current_is_flow(dd2)); /* not true if there are updates */
    assert(dd_current_is_flow(dd1)); /* same as above */
    assert(dd_current_dest(dd2) == dd_current_dest(dd1));
    if (petit_args.dep_analysis_debug > 3) {
	fprintf(debug,"Checking to see if we can prove that\n");
	fprintf(debug," %s\n",dd_current_as_string(dd1));
	fprintf(debug," is tighter than\n");
	fprintf(debug," %s\n",dd_current_as_string(dd2));
    };

    if (definite_lt_c(dd1,dd2)) return true;

    for (o = dd_i_i_for_access(dd_current_src(dd2));
	 !dd_i_i_done(o);
	 dd_i_i_next(o) )
    {
	if (dd_current_src(o) == dd_current_src(dd1)
	     && (dd_current_is_output(o) || dd_current_is_reduction(o)))
	{
	    int commonDepth = std::min(dd_current_nest(dd2),
                std::min(dd_current_nest(dd1),
				      dd_current_nest(dd_i_i_current(o))));
	    assert(dd_current_is_output(o) || dd_current_is_reduction(o));
	    if (kill_possible(dd1, dd2, o, commonDepth, 1))
	    {
		possible = 0;
		break;
	    }
	}
    }

    if (petit_args.dep_analysis_debug > 3 && possible)
	fprintf(debug,"    Partial Cover possible\n");

    return possible;
}
/* return -1 if c1 should come before c2 in the coverlist,
   1 if the other way, 0 if can't tell */

static int cover_order(const void *bad, const void *brain)
{
    const coverlist_entry *c1 = (const coverlist_entry *) bad, 
                          *c2 = (const coverlist_entry *) brain;
    int i,d;
#if !defined NDEBUG
    char buf1[80],buf2[80];

    sprintf(buf1,"%s\n",dd_current_as_string(c1->dep));
    sprintf(buf2,"%s\n",dd_current_as_string(c2->dep));

#endif 

#if USE_DEFINITE
    i = definite_lt_c_order(c1->dep, c2->dep);
    if (i)
	return i;
#endif
  
    /* i > 0 --> f2 <c! f1; */

    if (dd_current_nest(c1->dep) != 0 && dd_current_nest(c2->dep) != 0) {
      i = approx_leq_c(c2->dep, c1->dep) - approx_leq_c(c1->dep, c2->dep);
      if (i != 0) return i;

      for(i=1;i<=(int)std::min(dd_current_nest(c1->dep),dd_current_nest(c2->dep)); i++) {
          d = max_dir(c1->dep,i) - max_dir(c2->dep,i);
          if (d != 0) return d;
	  }
	
      i = dd_current_nest(c2->dep) - dd_current_nest(c1->dep);
      if (i != 0) return i;
	
    }

    if (access_lexically_preceeds(dd_current_src(c2->dep),
				  dd_current_src(c1->dep)))
	return -1;
    if (access_lexically_preceeds(dd_current_src(c1->dep),
				  dd_current_src(c2->dep)))
	return 1;

    return 0;
}

coverlist *build_coverlist(a_access r)
{
    dd_in_iterator f;
    coverlist     *cl;
    int n, last_group;

    assert(!is_phi_access(r));
    assert(!access_is_store(r));

    if (petit_args.dep_analysis_debug >= 2)
    {
        fprintf(debug, "\n\nConstructing coverlist for %d: %s\n",
		access_lineno(r),access_as_string(r));
    }

    /* count # of deps */
    n = 0;
    f = dd_i_i_for_access(r);
    while (!dd_i_i_done(f))
    {
	if (dd_i_i_cur_is_flow(f))
	  {
            if (!petit_args.omitTopLevel || dd_current_nest(dd_i_i_current(f))!=0)
      	       n++;
          } 
	dd_i_i_next(f);
    }

    /* build the list */

    cl = (coverlist *) malloc(sizeof(coverlist) +
			      n*sizeof(coverlist_entry) +
			      sizeof(double) /* in case of alignment */);

    assert(cl);  // should really use "new" here
    cl->size = n;
    coverlist * cl2 = (coverlist *) malloc(sizeof(coverlist) +
			      n*sizeof(coverlist_entry) +
			      sizeof(double) /* in case of alignment */);
    assert(cl2);  // should really use "new" here
    cl2->size = n;

    n = 0;
    f = dd_i_i_for_access(r);
    while (!dd_i_i_done(f))
    {
	if (dd_i_i_cur_is_flow(f))
	{
            if (!petit_args.omitTopLevel || dd_current_nest(dd_i_i_current(f))!=0)                     {
   	      cl->list[n].end_of_group = 0;
	      cl->list[n].dep = dd_i_i_current(f);
#if ! defined DAVEW_THESIS_REDUCTIONS
              cl->list[n].from_update = false;
              if (access_is_update(dd_i_i_cur_src(f))) {
                for (dd_in_iterator red=dd_i_i_for_access(dd_i_i_cur_src(f));
                     !dd_i_i_done(red);dd_i_i_next(red)) {
                   if (dd_i_i_cur_is_reduction(red)) {
                     cl->list[n].from_update = true;
                     break;
                   }
		 }
	      }
#endif
              n++;
                
	      assert(! (dd_current_flags(dd_i_i_current(f)) & ddIncompKill) );
	    }
            else //just set the dependence to be value-based
	      {
               dd_current_flags(dd_i_i_current(f))|=ddval;
	     }
	}
	dd_i_i_next(f);
    }
    assert(n == cl->size);

    /* sort list, break into groups */
    int lo,hi;
    int *ready = new int[cl->size];
    int num_ready = 0;
    int *num_pred = new int[cl->size];
    for(hi=0; hi<cl->size; hi++) {
      num_pred[hi] = 0;
      for(lo=0; lo<cl->size; lo++) if (lo != hi 
		&& cover_order(&cl->list[lo],&cl->list[hi]) < 0) {
	num_pred[hi]++;
#if 0
	fprintf(DebugFile,"%d < %d\n",lo,hi);
#endif
	}
      if (!num_pred[hi]) ready[num_ready++] = hi;
#if 0 
	fprintf(DebugFile," # less than %d: %d\n",hi, num_pred[hi]);
#endif
      }


    int num_done = 0;

    while (num_done < cl->size) {
      assert(num_ready > 0);
      int next =  ready[--num_ready];
#if 0
	fprintf(DebugFile,"%d\n",next);
#endif
      for(hi=0; hi<cl->size; hi++) if (hi != next && cover_order(&cl->list[next],&cl->list[hi]) < 0) {
	num_pred[hi]--;
        if (!num_pred[hi]) ready[num_ready++] = hi;
	}

      cl2->list[num_done++] = cl->list[next];
      }
      
     
    free(cl);
    cl = cl2; 
 
    delete ready;
    delete num_pred;

    // qsort(cl->list, cl->size, sizeof(coverlist_entry), cover_order);

    last_group = -1;
    for(n=0; n<cl->size-1; n++)
    {
#if USE_DEFINITE
	assert(!definite_lt_c(cl->list[n+1].dep,cl->list[n].dep));
	if (definite_lt_c(cl->list[n].dep,cl->list[n+1].dep))
	    cl->list[n].end_of_group = 1;
	else
	    cl->list[n].end_of_group = 0;
#else
	/* test to see if n is the end of a group:
	   are all deps from last_group+1 to n <= all deps after n? */

	int lo, hi;
	for(lo=last_group+1; lo<=n; lo++)
	    for(hi=n+1; hi<cl->size; hi++)
		if(!approx_leq_c(cl->list[lo].dep,cl->list[hi].dep))
		    goto not_group;  /* continue 3 */

	cl->list[n].end_of_group = 1;
	last_group = n;
    not_group:
	;
#endif
    }

    if (cl->size > 0)			      
	cl->list[cl->size-1].end_of_group = 1; /* unnecessary if ! USE_DEFINITE? */

    if (petit_args.dep_analysis_debug >= 3)
    {
	int i;

	for(i=0; i<cl->size ;i++)
	{
	    int j, first;
	    for (j=0, first=1 ; j < i; j++)
	    {
		if (!approx_leq_c(cl->list[j].dep,cl->list[i].dep))
		{
		    if (first)
		    {
			fprintf(debug, "        ** note - dependences ");
			first = 0;
		    }
		    else
			fprintf(debug, ", ");
		    fprintf(debug, "%d", j);
		}
	    }
	    if (!first)
		fprintf(debug, " are not <=c dependence %d.\n", i);
	
	    fprintf(debug, "   %3d: %s\n", i,
		    dd_current_as_string(cl->list[i].dep));
	    if (cl->list[i].end_of_group)
		fprintf(debug, "        --------  end of group  --------\n");
	}
    }

#if ! defined NDEBUG
    for(lo=0; lo<cl->size-1; lo++)
        assert(cover_order(&cl->list[lo],&cl->list[lo+1]) <= 0);

    for(lo=0; lo<cl->size; lo++)
	for(hi=lo+1; hi<cl->size; hi++)
          assert(cover_order(&cl->list[lo],&cl->list[hi]) <= 0);

    for(n=0; n<cl->size-1; n++)
	if (cl->list[n].end_of_group)
	{
	    for(lo=0; lo<=n; lo++)
		for(hi=n+1; hi<cl->size; hi++)
		{
		    /* lo <c! hi, but we may not be able to prove it */
		/* assert(definite_lt_c(cl->list[lo].dep,cl->list[hi].dep)); */
		    if(!approx_leq_c(cl->list[lo].dep,cl->list[hi].dep)) {
			fprintf(debug,"ERROR in coverlist: %d not <= %d \n",
				lo,hi);
		         fprintf(debug, "   %3d: %s\n", lo,
		          dd_current_as_string(cl->list[lo].dep));
		         fprintf(debug, "   %3d: %s\n", hi,
		          dd_current_as_string(cl->list[hi].dep));
			  }
		    assert(approx_leq_c(cl->list[lo].dep,cl->list[hi].dep));
		}
	}
#endif

    return cl;
}

} // end of omega namespace
