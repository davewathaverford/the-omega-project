/* $Id: notedd.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/* notedd.c - all functions related to noting dependence into dependence
   graph
   created 05.11.94 by murka
 */

#include <basic/assert.h>
#include <omega/Relations.h>
#include <petit/lang-interf.h>
#include <petit/notedd.h>
#include <petit/debug.h>
#include <petit/petit_args.h>

namespace omega {

/**************************************************
 *  Some things that used to be in ddomega-use.c  *
 **************************************************/

/*
  Store dependence, then do various tests that may need to
  duplicate the original ddnode by cloning and tag the new
  refined version as refined.
 */

static void
flag_and_store_dependence(ddnature nature,
			  a_access from_access, a_access to_access,
			  dir_and_diff_info *d_info) {
  ddi_check(d_info);
  store_dependence(nature, from_access, to_access, d_info);
}

static void dd_to_debug(dir_and_diff_info *d_info)
{
    int j;

    fprintf(debug, "(");
    for(j=1;j<=(int)d_info->nest;j++) {
	if (d_info->differenceKnown[j])
	    fprintf(debug,"%d",d_info->difference[j]);
	else if (ddextract1(d_info->direction,j) == ddall) 
	    fprintf(debug,"*");
	else {
	    if (dddirtest(d_info->direction, ddind, j)) fprintf(debug,"0");
	    if (dddirtest(d_info->direction, ddfwd, j)) fprintf(debug,"+");
	    if (dddirtest(d_info->direction, ddbck, j)) fprintf(debug,"-");
	}
	if (j < (int)d_info->nest) fprintf(debug,",");
    }
    fprintf(debug, ") restraint = (");
    for(j=1;j<=(int)d_info->nest;j++) {
	if (ddextract1(d_info->restraint,j) == ddall) 
	    fprintf(debug,"*");
	else {
	    if (dddirtest(d_info->restraint, ddind, j)) fprintf(debug,"0");
	    if (dddirtest(d_info->restraint, ddfwd, j)) fprintf(debug,"+");
	    if (dddirtest(d_info->restraint, ddbck, j)) fprintf(debug,"-");
	}
	if (j < (int)d_info->nest) fprintf(debug,",");
    }
    fprintf(debug, ")");
}

/* split a set of flow, anti, or output dependencies into
    pieces that are all forward in time

   if   we have a situation with an exposed 0+ followed
        by an exposed -,
   then filterValid will call itself recursively to split
        the 0+ into 0 and + (eliminating the 0...- case)
   else filterValid will store only one dependency
 */

#define SplitFirst 0

static void
filterValid(ddnature nature, a_access from_access, a_access to_access,
	    int commonNesting, dir_and_diff_info *d_info, int allEQallowed)
{
    int less_at, j;

    /*
     * For private variables:            
     * dependencies can not be carried by loops-privatizers
     */
    if (from_access!=Entry && from_access!=ExitNode &&
	to_access!=ExitNode && to_access!=Entry &&
	access_is_private_var(from_access))
    {
	int PrivLev = access_private_var_level(from_access);
	assert(PrivLev<=commonNesting);
	for(j=1; j<=PrivLev; j++) {
	    dddirreset(d_info->direction, ddfwd|ddbck, j);
	    if (!dddirtest(d_info->direction, ddbck|ddfwd|ddind, j))
		return;
	    dddirreset(d_info->restraint, ddfwd|ddbck, j);
	    d_info->difference[j]=0;
	    d_info->differenceKnown[j]=1;
	}
    }

    /* remove all -'s before 1st + */
    for(j = 1; j <= commonNesting; ++j ){
	if (!dddirtest(d_info->direction, ddfwd | ddind, j ))
	    return;  /* Bare - (without 0 or +) */
	if (dddirtest(d_info->direction, ddbck, j)) { /* Contains - */
	    dddirreset(d_info->direction, ddbck, j);
	    d_info_do_eq(d_info,j);
	    dddirreset(d_info->restraint, ddbck, j);
	}
	if (dddirtest(d_info->direction, ddfwd, j)) break; /* contains + */
    }
    
    /* check for all 0's or no common Nesting */
    less_at = j;
    if (less_at > commonNesting) {
	if (allEQallowed) {
	    if (petit_args.dep_analysis_debug >= 2) {
	      fprintf(debug,"storing ");
	      dd_to_debug(d_info);
	      fprintf(debug,"\n");
	    }
	    flag_and_store_dependence(nature, from_access, to_access, d_info);
	}
	return;
    }

    /* if we start with 0+ rather than just +, check for possible all 0's */

    if (dddirtest(d_info->direction, ddind, less_at)) {
#if !SplitFirst
       int forbidden;
#endif

       for(j = less_at+1;
           j <= commonNesting && !dddirtest(d_info->direction, ddfwd|ddbck, j);
           j++) ;
       if (j <= commonNesting && !dddirtest(d_info->direction, ddfwd|ddind, j))
       {
           /* we have some 0's, a 0+, more 0's, then a - so, 0+ -> just + */
           dddirreset(d_info->direction, ddind, less_at);
           dddirreset(d_info->restraint, ddind, less_at);
       }
       else {
           /* we have some 0's, a 0+, more 0's,
              then either 0- or something with a + */

#if !SplitFirst
           forbidden = !allEQallowed;
           for( j = commonNesting; j >= less_at; j-- ) {
               forbidden = dddirtest(d_info->direction,ddbck,j) ||
                           (dddirtest(d_info->direction,ddind,j) && forbidden);
               /* "forbidden" = some loop outside j must
                  force this dependence to go strictly forward in time */
           }

           if (forbidden) 
#endif
		{
               /* split into leading 0 vs. leading + */
               dir_and_diff_info plus, zero;
               plus = zero = *d_info;
               dddirreset(plus.direction, ddind, less_at);
               dddirreset(plus.restraint, ddind, less_at);
               dddirreset(zero.direction, ddfwd, less_at);
               dddirreset(zero.restraint, ddfwd, less_at);
               zero.difference[less_at] = 0;
               zero.differenceKnown[less_at] = 1;
               filterValid(nature, from_access, to_access, commonNesting,
                           &plus, allEQallowed);
               filterValid(nature, from_access, to_access, commonNesting,
                           &zero, allEQallowed);
               return;
           }
       }

    }

    if (petit_args.dep_analysis_debug >= 2) {
      fprintf(debug,"storing ");
      dd_to_debug(d_info);
      fprintf(debug,"\n");
    }
    flag_and_store_dependence(nature, from_access, to_access, d_info);
}


/* fill d_info for backward dependence */

void inverse_d_info (dir_and_diff_info * d_info, 
                     dir_and_diff_info * backward)
{
 dddirection deq,dgt,dlt, req,rgt,rlt;
 int j;

 deq = ddfilter(d_info->direction, ddind);
 dlt = ddfilter(d_info->direction, ddfwd);
 dgt = ddfilter(d_info->direction, ddbck);
 req = ddfilter(d_info->restraint, ddind);
 rlt = ddfilter(d_info->restraint, ddfwd);
 rgt = ddfilter(d_info->restraint, ddbck);

 backward->direction = backward->restraint = 0;
 ddsetfilter(backward->direction, deq, ddind);
 ddsetfilter(backward->direction, dlt, ddbck);
 ddsetfilter(backward->direction, dgt, ddfwd);
 ddsetfilter(backward->restraint, req, ddind);
 ddsetfilter(backward->restraint, rlt, ddbck);
 ddsetfilter(backward->restraint, rgt, ddfwd);

 backward->nest = d_info->nest;
 for(j=1; j<=(int)d_info->nest; j++)
   {
    backward->loop_dir[j] = d_info->loop_dir[j];
    backward->differenceKnown[j] = d_info->differenceKnown[j];
    if (backward->differenceKnown[j]) 
	 backward->difference[j] = -d_info->difference[j];
   }
  
 backward->flags=d_info->flags;
 backward->dd_relation=NULL;
 
} /* end inverse_d_info */
 


/* 
   call filterValid for the dependence from access1 to access2,
   and if they are distinct, for the dependence the other way.
 */

void noteDependence(situation *sit, dir_and_diff_info *d_info)
{
    dir_and_diff_info backward;

#if defined compareOmegaEpsilon
   addAtomCount(d_info);
#endif
 
    if (sit->access1 != sit->access2) 
         inverse_d_info(d_info, &backward);
    
    if (petit_args.dep_analysis_debug >= 2) {
	fprintf(debug,"filtering ");
	dd_to_debug(d_info);
	fprintf(debug,"\n");
    }

    filterValid(sit->oitype, sit->access1, sit->access2,
		sit->commonNesting, d_info,
		access_lexically_preceeds(sit->access1, sit->access2));


    if (sit->access1 != sit->access2) {
	if (petit_args.dep_analysis_debug >= 2) {
	    fprintf(debug,"backward filter ");
	    dd_to_debug(&backward);
	    fprintf(debug,"\n");
	}


        Relation inverse_rel;
        if (d_info->dd_relation!=NULL)
	  {
           inverse_rel=Inverse(copy(*(d_info->dd_relation)));
           backward.dd_relation=&inverse_rel;
	  }
	filterValid(sit->iotype, sit->access2, sit->access1,
		    sit->commonNesting, &backward,
		    access_lexically_preceeds(sit->access2,sit->access1));
    
    }
} /* noteDependence */

} // end omega namespace
