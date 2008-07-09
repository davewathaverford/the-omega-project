/* $Id: eps_print.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/* printing routines for epsilon test */

#include <memory.h>
#include <stdlib.h>
#include <petit/lang-interf.h>
#include <petit/affine.h>
#include <petit/notedd.h>
#include <petit/ddepsilon.h>
#include <petit/classify.h>
#include <petit/ddInf.h>

namespace omega {

void print_subscr(FILE *f, EpsSubscrType * s)
{
 int i;

 fprintf(f,"%d",s->scalar);
 for (i=0; i<s->nterms;i++)
    fprintf (f, " %d %s %d %s'\n", s->terms[i].a,
             var_id_name(s->terms[i].var->var),
             s->terms[i].b,
             var_id_name(s->terms[i].var->var));
 fprintf(f,"last min = %d, las max = %d", s->lastcomvar_min, 
          s->lastcomvar_max);  
 fprintf(f,"\n");
} /* end print_subscr */ 


void print_eps_var(FILE *f, Epsvar_id v)
{
 fprintf(f, "%s - ",  var_id_name(v->var));
 if (v->l_known==VAL_KNOWN)
     fprintf(f, "lower=%d ",v->lower);
 else 
     fprintf(f,"lower %s ", v->l_known?"never known": "yet unknown");
 
 if (v->u_known==1)
     fprintf(f, "upper=%d ",v->upper);
 else 
     fprintf(f,"upper %s ", v->u_known?"never known": "yet unknown");
  
 fprintf(f, "step = %d", v->step);
 
 fprintf(f,"\n");
} /* end print_eps_var */


void print_ban_info(FILE *f, BanerjeeInfoType * bi)
{
 fprintf(f,"used - %d, minval - (%c%d, %c%d, %c%d), maxval - (%c%d,%c%d,%c%d)\n",
         bi->used, bi->min_known[0]?' ':'*', bi->minval[0], 
                   bi->min_known[1]?' ':'*', bi->minval[1],
                   bi->min_known[2]?' ':'*', bi->minval[2],
                   bi->max_known[0]?' ':'*', bi->maxval[0],
                   bi->max_known[1]?' ':'*', bi->maxval[1],
                   bi->max_known[2]?' ':'*', bi->maxval[2]);
} /* end print_ban_info */

void print_comvar(FILE *f, EpsComvarType *cv, int n)
{
 int i;

 fprintf(f, "%s val=%d, done=%d nused=%d coupled=%d\n",
            var_id_name(cv->var->var), cv->val, cv->done, cv->nused, cv->coupled);
 fprintf(f,"     Subscript bounds:\n");
 for (i=0; i<n; i++)
    print_ban_info(f, &(cv->sb[i]));
} /* end print_comvar */


void print_subscr_bounds(FILE * f, SubscrBoundType * s)
{
 if (s->min_known==VAL_KNOWN)
   fprintf(f, "minval = %d, ",   s->minval);
 else if (s->min_known==VAL_NEVER_KNOWN)
   fprintf(f, "minval is never unknownn, ");
 else 
   fprintf(f, "minval is yet unknown, ");
 
 if (s->max_known==VAL_KNOWN)
   fprintf(f, "maxval = %d\n ",   s->maxval);
 else if (s->max_known==VAL_NEVER_KNOWN)
   fprintf(f, "maxval is never unknownn \n");
 else 
   fprintf(f, "maxval is yet unknown\n");

} /* end print_subscr_bounds */

} // end of namespace omega
