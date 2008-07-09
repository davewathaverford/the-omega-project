/* $Id: ddepsilon.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/* Epsilon test */

#include <memory.h>
#include <stdlib.h>
#include <basic/bool.h>
#include <basic/Exit.h>
#include <petit/lang-interf.h>
#include <petit/affine.h>
#include <petit/notedd.h>
#include <petit/ddepsilon.h>
#include <petit/classify.h>
#include <petit/ddInf.h>
#include <petit/eps_print.h>
#include <petit/debug.h>
#include <petit/dddriver.h>
#include <petit/petit_args.h>

namespace omega {

/* variables */

static EpsSubscrType subscr[maxSubscr]; /* subscripts */
static int nSubscr;                    /* number of subscripts */
static int nActSubscr;                 /* number of active subscripts */
static EpsVarType   var[maxEpsVars];      /* variables */
static int nVar;                       /* number of variables */
static EpsComvarType comvar[maxCommonNest+1]; /* common loop variables */
static SubscrBoundType Ban_bounds[maxSubscr];

static int sharedDepth;                /* Depth of the common loops */
static int dirtab[3]={ddfwd,ddind,ddbck};
 int ntime=0;



/****
 * investigateLoop - get information about loop bounds:
 *                    affinite, symbolic, triangular etc
 * ENTRY:
 *  l   - loop context
 * EXIT:
 *  non-applicability reason
 ***/

int investigateLoop (loop_context l)
{
 int i,j,res;
 bool known=1;
 int s;
 affine_expr *a; 
 Var_Id v;

 if (! node_is_affine(loop_start_node(l)) ||
     ! node_is_affine(loop_end_node(l)))
    return APPL_NONAFFINE_BOUND;
 
 res=APPL_FULL;
 for (a=loop_start(l),i=0; i<2; i++, a=loop_end(l))
 {
   for (j=1; j<a->nterms; j++)
   {
     v=a->terms[j].petit_var;
     if (var_id_is_index(v))
       res=res | APPL_TRIANG_BOUND;
     else if (var_id_is_var(v))
       res=res | APPL_SYMBOLIC_BOUND;
   }
 }

 if (loop_has_step(l)) 
   {
    loop_step(l,&s,&known);
    if (!known)
      res=res |  APPL_NONUNIT_STEP;
  }
 return res;
} /* end investigateLoop */




/****
 * find_var - searches for the variable in the array var,
 *            adds variable if it's not found
 * ENTRY
 *   petit_var - petit var id
 * EXIT
 *   variable id for the epsilon test
 ****/

static Epsvar_id find_var(Var_Id petit_var)
{
int j;  
loop_context l;

for (j=0; j<nVar; j++)
   {
    if (var[j].var==petit_var)
       return &(var[j]);
   } /* for */
/* if here, then we didn't find var, add it */
var[nVar].var=petit_var;
l=var_id_loop_cont(petit_var);

/* check if upper bound is known */
if (!node_is_affine(loop_end_node(l)) || loop_end(l)->nterms>1) 
   var[nVar].u_known=0;
else
  {
   var[nVar].u_known=1;
   var[nVar].upper=loop_end(l)->terms[0].coefficient;
}

/* check if lower bound is known */

if (!node_is_affine(loop_start_node(l)) || loop_start(l)->nterms>1) 
  var[nVar].l_known=0;
else
  {
  var[nVar].l_known=1;
  var[nVar].lower=loop_start(l)->terms[0].coefficient;
}

/* get step */
var[nVar].step=1;
if (loop_has_step(l))
  {
   bool known=1;
   int s;
  
    loop_step(l,&s,&known);
    if (known)
      var[nVar].step=s;
  }  


if (petit_args.DDalgorithm==DDalg_omega_preproc && investigateLoop(l)!=APPL_FULL)
  return NULL;

#if defined compareOmegaEpsilon
  if (storeInf) {
    int tmp; 
    if ((tmp=investigateLoop(l))!=APPL_FULL)
      setApplReason(tmp);
  }
#endif



var[nVar].ddnum=var_id_loop_no(petit_var);
if (var[nVar].ddnum>sharedDepth)
  var[nVar].ddnum=-1;
else 
  { /* fill entry in comvar */
   comvar[var[nVar].ddnum].var=&(var[nVar]);
  }
nVar++;
return &(var[nVar-1]);
} /* end find_var */


/****
 * form_problem - form problem for the epsilon test
 *   - check for linearity
 *   - check for ZIV
 *   - fill subscr and vars 
 * ENTRY
 *  a1,a2 - accesses
 * EXIT
 *   dependence existence: DD_YES,DD_NO, DD_QUESTION
 ****/

static int form_problem(a_access a1, a_access a2)
{
 sub_iterator iter1, iter2;
 affine_expr  *affn1, *affn2;
 EpsSubscrType *s;
 int nt,i,j;
 int res=DD_QUESTION;
 
 nVar=0;
 nSubscr=0;

 iter1=sub_i_for_access(a1);
 /* check that this is array not variable */
 if (sub_i_done(iter1))
   return DD_YES;


 iter2=sub_i_for_access(a2);
 while (!sub_i_done(iter1))
   {
   /* for each access */

   if (!sub_i_cur_is_affine(iter1) || !sub_i_cur_is_affine(iter2))
     /* non-linear */
   {

    if (petit_args.DDalgorithm==DDalg_omega_preproc)
      res=DD_INEXACT;  

#if defined compareOmegaEpsilon
    if (storeInf) {
       setApplReason(APPL_NONAFFINE_SUBSCR);
       setEpsInf(EPSILON_ASSUME);
     }
#endif 
     goto SKIP_SUBSCRIPT;
   }
   affn1=sub_i_cur_affine(iter1);
   affn2=sub_i_cur_affine(iter2);
   
   /* check ZIV */
   if (affn1->nterms==1 && affn2->nterms==1)
    {   
     if ((affn1->terms)[0].coefficient!=(affn2->terms)[0].coefficient)
       return DD_NO;
   }
   else if (petit_args.DDalgorithm==DDalg_omega_preproc && res==DD_INEXACT)
     goto SKIP_SUBSCRIPT;
   else
     { /* include this subscripts into the problem */
      s=&(subscr[nSubscr]);
      s->done=0;
      s->scalar=affn1->terms[0].coefficient-affn2->terms[0].coefficient;
      nt=0; /* will be s->nterms */

      /* go through the vars of affn1 */
      for (i=1; i<affn1->nterms; i++)
	{
         s->terms[nt].a=affn1->terms[i].coefficient;
         s->terms[nt].b=0;
         if (!(var_id_is_index(affn1->terms[i].petit_var)))
	 {

         if (petit_args.DDalgorithm==DDalg_omega_preproc)
           res=DD_INEXACT; 

#if defined compareOmegaEpsilon
         if (storeInf) {
           setApplReason(APPL_SYMBOLIC_SUBSCR);  
           setEpsInf(EPSILON_ASSUME);
	 }
#endif
          goto SKIP_SUBSCRIPT;
	 }
         s->terms[nt].var=find_var(affn1->terms[i].petit_var);

         if (petit_args.DDalgorithm==DDalg_omega_preproc && s->terms[nt].var==NULL)
           {res=DD_INEXACT; goto SKIP_SUBSCRIPT;}
         nt++;
       }

      /* go through the vars of affn2 */
      for (i=1; i<affn2->nterms; i++)
        {
         /* try to find this var among terms */
         for (j=0; j<nt; j++)
           {
           if (s->terms[j].var->var==affn2->terms[i].petit_var)
             {
              s->terms[j].b=affn2->terms[i].coefficient;
              break;
             }
           } /* for */

         if (j>=nt) /* not found */
	   {
            s->terms[nt].a=0;
            s->terms[nt].b=affn2->terms[i].coefficient;
            if (!(var_id_is_index(affn2->terms[i].petit_var)))
	    {
             if (petit_args.DDalgorithm ==DDalg_omega_preproc)
               res=DD_INEXACT;
             
#if defined compareOmegaEpsilon
              if (storeInf) {
                  setApplReason(APPL_SYMBOLIC_SUBSCR);  
                  setEpsInf(EPSILON_ASSUME);
                }
#endif
              goto SKIP_SUBSCRIPT;
	    }
            s->terms[nt].var=find_var(affn2->terms[i].petit_var);
            if (petit_args.DDalgorithm==DDalg_omega_preproc&& s->terms[nt].var==NULL)
               {res= DD_INEXACT; goto SKIP_SUBSCRIPT;}
            nt++;
	  }
       } /* for affn2 */
       s->nterms=nt;
       nSubscr++;
     } /* else */  
     
 SKIP_SUBSCRIPT:
    /* go to the next subscript */
    sub_i_next(iter1);
    sub_i_next(iter2);

   } /* while */

nActSubscr=nSubscr;

/* fill comvar */
for (i=1; i<=sharedDepth; i++)
 comvar[i].done=comvar[i].siv0_done=0;


return res;
} /* end form_problem */


/****
 * check_SIV - check subscript using  strong SIV test
 * ENTRY
 *  s  - subscript
 *  d_info - dependance information
 * EXIT
 *  *dd - dependance difference
 *  dependence existence ( DD_NO, DD_QUESTION)
 ****/

static int check_SIV(EpsSubscrType *s, int * dd,dir_and_diff_info * d_info)
                                                    {
 int n;

 if (s->scalar%s->terms[0].a)
   return DD_NO;
 *dd=s->scalar/s->terms[0].a;
 if (s->terms[0].var->u_known && s->terms[0].var->l_known)
   if (abs(*dd)>abs(s->terms[0].var->upper - s->terms[0].var->lower)) 
   /* assume step is positive */
   return DD_NO;

 /* check for step */
 if (*dd % s->terms[0].var->step)
   return DD_NO;
      
 n=s->terms[0].var->ddnum;
 d_info->difference      [n]=*dd;
 d_info->differenceKnown [n]=1;
 s->done=1;
 if (s->terms[0].var->u_known && s->terms[0].var->l_known)
  { 
   if (*dd>0)
     s->terms[0].var->upper-=*dd;
   else
     s->terms[0].var->lower-=*dd;
 }
 nActSubscr--;

 return DD_QUESTION;
} /* end check_SIV */

/****
 * check_SIV0 - check subscripts using weak-zero SIV test
 * ENTRY
 *  s - subscript
 *  d_info - dependence information
 * EXIT
 *  *val - value of the variable
 *  *k   - 1, i1=*val
 *         2, i2=*val
 *  dependence existence (DD_NO, DD_QUESTION)
 ****/

static int check_SIV0(EpsSubscrType *s, int *val,dir_and_diff_info * d_info,
                      int* k)
{
 int d,c;
 Epsvar_id v;

 v=s->terms[0].var;
 d=s->scalar;
 if (s->terms[0].a)
   {
   c=s->terms[0].a;
   d=-d;
   *k=1;
   } 
 else
   {
   c=s->terms[0].b;
   *k=2;
 }
if (d%c)
  return DD_NO;
*val=d/c;

if (   (v->l_known && *val<v->lower)
    || (v->l_known && ((*val-v->lower)%v->step)) 
    || (v->u_known && *val>v->upper))
   return DD_NO;

s->done=1;
nActSubscr--;
 
 if (v->ddnum>0) /* common loop's index, affects DD vector */
   {
    if (comvar[v->ddnum].siv0_done) /* already know one value */
      {
       d_info->differenceKnown[v->ddnum]=1;
       d_info->difference[v->ddnum]=
            ((*k==1)?(comvar[v->ddnum].val - *val)
	         :(*val - comvar[v->ddnum].val));
        }
    else {
      comvar[v->ddnum].val=*val;
      comvar[v->ddnum].siv0_done=1; 
      if ((v->l_known && v->lower==d && *k==1) 
       || (v->u_known && v->upper==d && *k==2))
          dddirreset (d_info->direction,ddbck,v->ddnum);
      else if ((v->l_known && v->lower==d && *k==2) 
            || (v->u_known && v->upper==d && *k==1))
          dddirreset (d_info->direction, ddfwd, v->ddnum);
    } /* else */
  } /* if */

 return DD_QUESTION;
} /* end check_SIV0 */


/****
 * compress_subscr - eliminate canceled variable from the subscript
 * ENTRY
 *   s - subscript;
 *   k - index of possibly canceled variable
 * EXIT
 *   return - DD_NO, if dependence is impossible
 *            DD_QUESTION, otherwise
 ****/

static int  compress_subscr(EpsSubscrType * s, int k)
{
 int i;

 if (s->terms[k].a!=0 || s->terms[k].b!=0) /* nothing to eleminate */
   return DD_QUESTION;
 
 if (s->nterms==1)
   {
   if (s->scalar!=0)
     return DD_NO;
   else
     { s->done=1; nActSubscr--;}
 }
 else
   { /* delete this var from subscript */
      for (i=k+1; i<s->nterms; i++)
      {
      s->terms[i-1].var=s->terms[i].var;
      s->terms[i-1].a  =s->terms[i].a;
      s->terms[i-1].b  =s->terms[i].b;
      }
    s->nterms--;
    }
return DD_QUESTION;
} /* end compress_subscr */ 


/****
 * promote_SIV - promote information from the strong SIV test
 * ENTRY
 *  v - variable
 *  d - distance
 * EXIT
 *  dependence existence - DD_NO or DD_QUESTION
 ****/

static int promote_SIV(Epsvar_id v, int d)
{
 int i,j;
 EpsSubscrType * s;

 
 for (i=0; i<nSubscr; i++)
  if (!subscr[i].done)
    {
     s=&(subscr[i]);
     /* find var */
     for (j=0; j<s->nterms; j++)
       if (s->terms[j].var==v)
	 {
          s->terms[j].a-=s->terms[j].b;
          s->scalar-=s->terms[j].b*d;
          s->terms[j].b=0;
 
          if (compress_subscr(s,j)==DD_NO)
            return DD_NO;
          break;
        } /* if , for*/
   } /* for */       
return   DD_QUESTION;
} /* end promote_SIV */        
    

/****
 * promote_SIV0 - promote information from the weak-zero SIV test
 * ENTRY
 *  v - variable
 *  c - its value
 *  k - ik=c, i.e. k is number of access
 * EXIT
 *  dependence existence - DD_NO or DD_QUESTION
 ****/

static int promote_SIV0(Epsvar_id v, int c, int k)
{
 int i,j;
 EpsSubscrType * s;
  
 for (i=0; i<nSubscr; i++)
  if (!subscr[i].done)
    {
     s=&(subscr[i]);
     /* find var */
     for (j=0; j<s->nterms; j++)
       if (s->terms[j].var==v)
	 {
          if (k==1) /* first access */
          {
            s->scalar+=c*s->terms[j].a;
            s->terms[j].a=0;
          }
          else /* second access */
	  {
            s->scalar-=c*s->terms[j].b;
            s->terms[j].b=0;
          }  
          if ( compress_subscr(s,j)==DD_NO)
            return DD_NO;
          break;
        } /* if , for*/
   } /* for */       
return   DD_QUESTION;
} /* end promote_SIV0 */        
         

/***** 
 * check_simple - check dependence using SIV tests and their promotions 
 * ENTRY
 * d_info - dependance information
 * EXIT
 *   dependence existence 
 ****/

static int check_simple (dir_and_diff_info * d_info){
int simplified;
int i,c,k;
EpsSubscrType * s;

do {
   simplified=0;
   for (i=0; i<nSubscr;i++)
     {
     s=&(subscr[i]);
     if (!s->done)
       {
        /* check SIV */
        if (s->nterms==1 && s->terms[0].a==s->terms[0].b)
          {
           if (check_SIV(s,&c,d_info)==DD_NO)
              return DD_NO;
           if (promote_SIV(s->terms[0].var,c)==DD_NO)
              return DD_NO;
           simplified=1;
           break;
          }
        /* check SIV0 */
        if (s->nterms==1 && (!s->terms[0].a || !s->terms[0].b))
          {
           if (check_SIV0(s,&c,d_info,&k)==DD_NO)
              return DD_NO;
           if (promote_SIV0(s->terms[0].var,c,k)==DD_NO)
              return DD_NO;
           simplified=1;
           break;
          }
      } /* if */
    } /* for */
  } while (simplified && nActSubscr);
return DD_QUESTION;
} /* end check_simple */


/****
 * eliminate_done_subscr - eliminate subscripts with done flag set
 **/

static void eliminate_done_subscr(){
int i,j;

j=0;
for (i=0;i<nSubscr; i++)
  { 
   if (!subscr[i].done){
     memcpy(subscr+j, subscr+i, sizeof(EpsSubscrType));
     j++;
  }	  
 }
nActSubscr=nSubscr=j;
} /* end eliminate_done_subscr */



/*****
 * fill_directions - fill dependence vector using known 
 *       dependence differences
 * d_info - dependance information
 ***/

static void fill_directions(dir_and_diff_info * d_info)
{
 int i,d;
 dddirection dir;


 for (i=1; i<=(int)d_info->nest; i++)
   {
    if (d_info->differenceKnown[i])
      {
       comvar[i].done=1;
       d=d_info->difference[i];
       if (d>0)
         dir=ddind|ddbck;
       else if (d==0)
         dir=ddfwd|ddbck;
       else
         dir=ddfwd|ddind;
       dddirreset(d_info->direction,dir,i);
     }
  }
} /* end fill_directions */


/***
 * get_bound - get a bound for the banerjee test
 * ENTRY:
 *   a,b - coefficients
 *   v - variable description
 *   cv - description of the common loop variable
 *   k   - BOUND_LOWER or BOUND_UPPER
 *   dir - direction:
 * EXIT:
 *   bound value if available
 *   UNKNOWN_BOUND if not
 * NOTE: step is assumed to be 1.
 */

int get_bound(int a, int b, Epsvar_id v, EpsComvarType * cv, int k, int dir)
{
int s=0;
int cu,cl,c0=0;
int l,u,l_known,u_known,step;

l_known=v->l_known;
u_known=v->u_known;
l=v->lower;
u=v->upper;
step=v->step;

/* update loop bounds in the case of SIV0 variable */
if (cv!=NULL && cv->siv0_done)
  {
    if (dir==ddind)
      {
      l=u=cv->val;
      l_known=u_known=1;
    }
    else if ((a!=0 && dir==ddfwd) || (b!=0 && dir==ddbck))
        {u=cv->val;
         u_known=1;}
    else if ((a!=0 && dir==ddbck) || (b!=0 && dir==ddfwd))
        {l=cv->val;
         l_known=1;}
  }

switch (dir)
  {
  case ddall: /* all directions */
         if (k==BOUND_LOWER) /* lower bound */
           s=(a>0?0:a)-(b>0?b:0);
         else     /* upper bound */
           s=(a>0?a:0)-(b>0?0:b);
         cu=s;
         cl=-s+(a-b);
         break;
  case ddfwd:/* < direction (forward) */
         if (k==BOUND_LOWER) /* lower bound */
	   {
            s=(a>0?0:a)-b;
            s=(s>0?0:s);
	  }
         else /* upper bound */
           {
            s=(a>0?a:0)-b;
            s=(s>0?s:0);
	  }
         cu=s;
         cl=-s+(a-b);
         c0=(-s-b)*step;
         break;
  case ddind: /* = direction */
         s=a-b;
         if (k==BOUND_LOWER) /* lower bound */
          s=(s>0?0:s);
         else /* upper bound */
          s=(s>0?s:0);
         cu=s;
         cl=-s+(a-b);
         break;
  case ddbck: /* > direction (backward) */
         if (k==BOUND_LOWER) /* lower bound */
	   {
            s=a-(b>0?b:0);
            s=(s>0?0:s);
	  }
         else /* upper bound */
	   {
            s=a-(b>0?0:b);
            s=(s>0?s:0);
	  }
	 cu=s;
         cl=-s+(a-b);
         c0=(-s+a)*step;
         break;
  default:
    return UNKNOWN_BOUND; // this shouldn't happen,
			  // but it makes g++ shut up about cu and cl
  } /* switch */
if ((cu!=0 && !u_known) || (cl!=0 && ! l_known))
  return UNKNOWN_BOUND;
s=c0;
if (cu!=0)
 s+=cu*u;
if (cl!=0)
 s+=cl*l;
return s;
} /* end get_bound */


/****
 * form_banerjee_problem - calculate upper and lower bounds for
 *                         all directions 
 * ENTRY:
 *   d_info - DD information
 ***/

static void form_banerjee_problem(dir_and_diff_info * d_info)
{
int kSubscr,kVar,kComvar;
EpsSubscrType *s;
Epsvar_id       v;
EpsComvarType *cv;
EpsTermType    *t;
BanerjeeInfoType *b;
SubscrBoundType * sb;
int i;
int nvar_to_ban;
EpsComvarType * last_cv;  // initialization ok due to nvar_to_ban 1/29/99

/* initialize comvar */
for (kComvar=1; kComvar<=sharedDepth; kComvar++)
  {
   comvar[kComvar].nused=0;
   comvar[kComvar].coupled=0;
   for (kSubscr=0;kSubscr<nSubscr;kSubscr++)
    {
     b=&(comvar[kComvar].sb[kSubscr]);
     for (i=0; i<3; i++)
       {b->minval[i]=b->maxval[i]=0;
        b->min_known[i]=b->max_known[i]=1;
      }
     b->used=0;
     b->min_known[3]=b->max_known[3]=1; 
   }
} /* for */


/* for each subscript */
for (kSubscr=0; kSubscr<nSubscr;kSubscr++)
  {
    s=&(subscr[kSubscr]);
    sb=&(Ban_bounds[kSubscr]);
    sb->min_known=sb->max_known=1;
    sb->minval=sb->maxval=s->scalar;
    s->lastcomvar_min=s->lastcomvar_max=0;
    nvar_to_ban=0;
    /* for each variable */
    for (kVar=0; kVar<s->nterms; kVar++)
      {
       t=&(s->terms[kVar]);
       v=s->terms[kVar].var;
       if (v->ddnum<0) /* not common loop */
	 {
          int res;          
             res=get_bound(t->a,t->b,v,NULL,BOUND_LOWER,ddall);
	     if (res!=UNKNOWN_BOUND)
               sb->minval+=res;
             else
               {sb->min_known=0; s->lastcomvar_min=maxCommonNest+3;}
             res=get_bound(t->a,t->b,v,NULL,BOUND_UPPER,ddall);
      	     if (res!=UNKNOWN_BOUND)
               sb->maxval+=res;
             else
               {sb->max_known=0; s->lastcomvar_max=maxCommonNest+3;}

        }
       else { /* common loop variable */
	 
          kComvar=v->ddnum;
          cv=&(comvar[kComvar]);
          cv->sb[kSubscr].used=1;
          cv->nused++;
          if (d_info->differenceKnown[kComvar])
	    {
             int c,d;
             c=d_info->difference[kComvar]*t->b;
             d=t->a-t->b;
             if (d>0)
	       {
                if (v->l_known) 
                 sb->minval+=v->lower * d +c;
                else
                 {sb->min_known=0; s->lastcomvar_min=maxCommonNest+3;}
                if (v->u_known)   
                 sb->maxval+=v->upper * d +c;
                else
                 {sb->max_known=0; s->lastcomvar_max=maxCommonNest+3;}
	      }
             else {
                if (v->u_known) 
                 sb->minval+=v->upper * d +c;
                else
                 {sb->min_known=0;s->lastcomvar_min=maxCommonNest+3;}
                if (v->l_known)
                 sb->maxval+=v->lower * d +c;
                else
                 {sb->max_known=0;s->lastcomvar_max=maxCommonNest+3;}
              }
             cv->done=1;
          }
         else /* DD difference is unknown */
	   {
            int lb,ub,res;

              if (nvar_to_ban==0) /* first time var to be refined occured */
                last_cv=cv;
              else 
                {
                if (nvar_to_ban==1) /* just found that vars are coupled here */
                  last_cv->coupled=1;
                cv->coupled=1;
                } 
              nvar_to_ban++;
                            
              lb=get_bound(t->a,t->b,v,cv,BOUND_LOWER,ddall);
              if (lb==UNKNOWN_BOUND)    
		{
                 cv->sb[kSubscr].min_known[3]=0;
                 sb->min_known=0;
                 s->lastcomvar_min=omega::max(s->lastcomvar_min,kComvar);
                }
              else
                 sb->minval+=lb;
              ub=get_bound(t->a,t->b,v,cv,BOUND_UPPER,ddall);
              if (ub==UNKNOWN_BOUND)    
		{
                 cv->sb[kSubscr].max_known[3]=0;
                 sb->max_known=0;
                 s->lastcomvar_max=omega::max(s->lastcomvar_max,kComvar);
                }
              else
                 sb->maxval+=ub;

              for (i=0; i<3; i++) /* for each direction */
		{
                 res=get_bound(t->a,t->b,v,cv,BOUND_LOWER,dirtab[i]);
                 if (res==UNKNOWN_BOUND)
                    cv->sb[kSubscr].min_known[i]=0;
                 else if (lb==UNKNOWN_BOUND)
                    cv->sb[kSubscr].minval[i]=res;
                 else
                    cv->sb[kSubscr].minval[i]=res-lb;

                 res=get_bound(t->a,t->b,v,cv,BOUND_UPPER,dirtab[i]);
                 if (res==UNKNOWN_BOUND)
                    cv->sb[kSubscr].max_known[i]=0;
                 else if (ub==UNKNOWN_BOUND)
                    cv->sb[kSubscr].maxval[i]=res;
                 else
                    cv->sb[kSubscr].maxval[i]=res-ub;
	       } /* for  (direction) */
 	  } /* else  (DD is unknown)*/
	} /* else (common loop variable) */
       } /* for (variable) */

     if (s->lastcomvar_min>sharedDepth)
       sb->min_known=VAL_NEVER_KNOWN;
     if (s->lastcomvar_max>sharedDepth)
       sb->max_known=VAL_NEVER_KNOWN;
     if (sb->min_known == VAL_NEVER_KNOWN && sb->max_known ==VAL_NEVER_KNOWN)
       s->done=1;       
     } /* for (subscript) */

for (kComvar=1;kComvar<=sharedDepth; kComvar++)
  if (comvar[kComvar].nused==0)
    comvar[kComvar].done=1;

} /* end form_banerjee_problem */




/*****
 * adjust_bounds - adjust banerjee bounds for all directions for this 
 *                 common var
 * ENTRY
 *   k       - comvar number
 *   kDir    - number of the direction
 *   sb      - subscribt bounds (can be changed)
 *   s_num   - subscribt number
 *  
 * EXIT
 *   return  - decision (DD_NO or DD_QUESTION 
 *   sb      - subscribt bounds are changed
 *****/

int adjust_bounds (int k, int kDir, SubscrBoundType *sb, int s_num)
                    
{
  EpsComvarType    * cv=&(comvar[k]);
  EpsSubscrType    * s = &(subscr[s_num]); 
  BanerjeeInfoType * b= &(cv->sb[s_num]);
    


  if (petit_args.dep_analysis_debug)
    {
     fprintf(debug, "Inside adjust bounds\n");
     fprintf(debug, "Current COMVAR is :\n");
     
     fprintf(debug, "k = %d, lastmin=%d, var_known=%d, gen_known=%d\n",
             k, s->lastcomvar_min,b->min_known[kDir],sb->min_known);
   }


  if (b->min_known[kDir]) {
        sb->minval+=b->minval[kDir];
        if (sb->min_known==VAL_UNKNOWN && k>=s->lastcomvar_min)
          sb->min_known=VAL_KNOWN;
      }
  else
      sb->min_known=VAL_NEVER_KNOWN;

  if (sb->min_known==VAL_KNOWN && sb->minval>0)
        return DD_NO;
 
          /* max */
  if (b->max_known[kDir]) {
      sb->maxval+=b->maxval[kDir];
      if (sb->max_known==VAL_UNKNOWN && k>=s->lastcomvar_max)
          sb->max_known=VAL_KNOWN;
      }
  else
      sb->max_known=VAL_NEVER_KNOWN; 
  
  if (sb->max_known==1 && sb->maxval<0)
      return DD_NO; 

  return DD_QUESTION;
} /* end adjust_bounds */





/*****
 * check_banerjee_not_coupled - refine non-coupled variables
 * ENTRY 
 *   d_info - DD information
 * EXIT
 *   return - DD_NO or DD_QUESTION
 **/

static int check_banerjee_not_coupled(dir_and_diff_info * d_info) {
int i;
int kSubscr;
EpsSubscrType * s;
SubscrBoundType bound;
int ddres[3];
int res=0;
int kDir;
int ntime_done=0;

for (i=1; i<=sharedDepth; i++)
  if (!comvar[i].done && comvar[i].nused==1 && !comvar[i].coupled)
    {
      /* this var is uncoupled */
      if (!ntime_done)
       {ntime++; ntime_done=1;}     

     if (petit_args.dep_analysis_debug)
        {
         fprintf(debug,"Uncoupled common var number %d\n", i);
         print_comvar(debug, &(comvar[i]), nSubscr);
        } 

      for (kDir=0; kDir<3; kDir++)
         ddres[kDir]=DD_QUESTION;

     for (kSubscr=0; kSubscr<nSubscr; kSubscr++)
       if (comvar[i].sb[kSubscr].used)
       {

        if (petit_args.dep_analysis_debug)
           fprintf(debug, "Analyzing subscribt number %d\n",kSubscr);

        s= &(subscr[kSubscr]); 
        for (int kDir=0; kDir<3; kDir++)
         {
           memcpy(&bound, &(Ban_bounds[kSubscr]), sizeof(SubscrBoundType));
           if (ddres[kDir]!=DD_NO)
              ddres[kDir]=adjust_bounds(i,kDir,&bound,kSubscr); 

           if (petit_args.dep_analysis_debug)
            {
            fprintf(debug, "Current values of Banerjee's bounds:\n");
            fprintf (debug, "for %d subscribt %d direction\n ", kSubscr, kDir);
            print_subscr_bounds(debug, &bound);
            }
            
         } 

        s->done=1;
        nActSubscr--;
        comvar[i].done=1;
       } /* end for subscr */


    if (petit_args.dep_analysis_debug)
       {
        fprintf(debug, "Value of d_info->direction is %lx\n",
                  d_info->direction);
      }

     


     if (petit_args.dep_analysis_debug)
       {
        fprintf(debug, "ddres is (%d,%d,%d)\n", ddres[0],ddres[1],ddres[2]);
      }
     if (ddres[0]==DD_NO)
        dddirreset(d_info->direction, ddfwd,i);
     if (ddres[1]==DD_NO)
        dddirreset(d_info->direction, ddind,i);
     if (ddres[2]==DD_NO)
        dddirreset(d_info->direction, ddbck,i);

     res=0;
     for (kDir=0; kDir<3; kDir++)
       if (ddres[kDir]==DD_NO)
        res++;

     if (petit_args.dep_analysis_debug)
       {
        fprintf(debug, "Adjusted value of d_info->direction is %lx\n",
                  d_info->direction);
      }

     if (res==3)
       return DD_NO;

     if (ddres[0]==DD_NO && ddres[2]==DD_NO)
       {
        d_info->differenceKnown[i]=1;
        d_info->difference[i]=0;
       }  
   } /* end for vars */
return DD_QUESTION;
} /* end check_banerjee_not_coupled */


/****
 * refine_direction - refine one direction using Banerjee test
 * ENTRY
 *   minval,maxval - lower and upper bounds for subscript expressions
 *   k             - number of common variable to test
 *   d_info        - DD information
 *   sit           - situation
 * EXIT
 ***/

static void refine_direction(SubscrBoundType *bb,  int k,
        dir_and_diff_info * d_info, situation * sit)
{
 SubscrBoundType bb_ref[3][maxSubscr];
       
 dir_and_diff_info d_info_save;
 int kSubscr,kDir;
 BanerjeeInfoType * b;
 int dd[3];

 for (kDir=0; kDir<3; kDir++)
   dd[kDir]=DD_QUESTION;

 /* find a variable to be really tested */
 while (comvar[k].done && k<=sharedDepth)
   k++;

 if (k>sharedDepth){
   noteDependence(sit,d_info);
   return;
 }
   
/* copy bounds to 3 new arrays*/
for (kDir=0;kDir<3; kDir++)
  memcpy(bb_ref[kDir], bb, sizeof(SubscrBoundType)*nSubscr);


/* adjust bounds on 3 directions */

for (kSubscr=0; kSubscr<nSubscr; kSubscr++)
 if (!subscr[kSubscr].done && comvar[k].sb[kSubscr].used)
   {
    b=&(comvar[k].sb[kSubscr]);
    for (kDir=0; kDir<3;kDir++)
     {
       if (dd[kDir]!=DD_NO)
          dd[kDir]=adjust_bounds(k,kDir,&(bb_ref[kDir][kSubscr]),
                      kSubscr);  
     }
  if (petit_args.dep_analysis_debug)
    {
     fprintf(debug, "Considering comvar number %d\n",k);
     print_comvar(debug, &(comvar[k]), nSubscr);      
     fprintf(debug, "Current values of Banerjee's bounds:\n");
          fprintf (debug, "for %d subscribt\n ", kSubscr);
          for (kDir=0; kDir<3; kDir++)
            {
             fprintf(debug, "direction %d: ",kDir);
             print_subscr_bounds(debug, &(bb_ref[kDir][kSubscr]));
            }
    } /* end print */    
  }

memcpy(&d_info_save,d_info,sizeof(dir_and_diff_info));
  
if (dd[0]!=DD_NO)
  {
  dddirreset(d_info->direction,ddbck|ddind,k);
  refine_direction(bb_ref[0],k+1,d_info,sit);
}

memcpy(d_info,&d_info_save,sizeof(dir_and_diff_info));
if (dd[1]!=DD_NO)
  {
  dddirreset(d_info->direction,ddfwd|ddbck,k);
  d_info->differenceKnown[k]=1;
  d_info->difference[k]=0;
  refine_direction(bb_ref[1],k+1,d_info,sit);
}

memcpy(d_info,&d_info_save,sizeof(dir_and_diff_info));
if (dd[2]!=DD_NO)
  {
  dddirreset(d_info->direction,ddfwd|ddind,k);
  refine_direction(bb_ref[2],k+1,d_info,sit);
}
} /* end refine direction */


/****
 * check_star_bounds - check subscripts bounds for (*,...*) directions
 * EXIT
 *   return - DD_NO or DD_QUESTION
 ****/

int check_star_bounds (){
int kSubscr;
SubscrBoundType* b;

for (kSubscr=0; kSubscr<nSubscr; kSubscr++)
  if (!subscr[kSubscr].done)
    {
      b=&(Ban_bounds[kSubscr]);
      if ( (b->min_known==VAL_KNOWN  && b->minval>0) || 
           (b->max_known==VAL_KNOWN  && b->maxval<0))
        return DD_NO;
    }
return DD_QUESTION;
} /* end check_star_bounds */



             
/****
 * epsilon_test
 *    analyzer of the data dependence a la commercial compiler
 * ENTRY:
 *  a1,a2 - 2 accesses
 *  d_info - dependence information
 ****/

void  epsilon_test(a_access a1, a_access a2,dir_and_diff_info * d_info,
                 situation *sit )
{
int res;
int i;


sharedDepth= d_info->nest;


if (petit_args.dep_analysis_debug)
  {
   fprintf(debug, "Testing pair %p-%p\n", a1, a2);
 }


/* like in Omega, don't check for the accesses not in a common loop */

#if defined compareOmegaEpsilon
  if (storeInf)
     setEpsInf(EPSILON_ZIV);
#endif  

res=form_problem(a1,a2);


if (petit_args.dep_analysis_debug)
  {
   fprintf(debug, "\nProblem description\n");
   fprintf(debug, "  SUBSCRIPTS:\n");
   for (i=0; i<nSubscr; i++)
      print_subscr(debug,&(subscr[i]));
   fprintf(debug, "  VARIABLES:\n");
   for (i=0; i<nVar; i++)
     print_eps_var(debug, &(var[i]));
   fprintf(debug, "  COM VARS:\n");
   for (i=1; i<=sharedDepth; i++)
     print_comvar(debug, &(comvar[i]), nSubscr);
   fprintf(debug, "\n");
 }
 

if (res==DD_NO)
  {

#if defined compareOmegaEpsilon
  if (storeInf)
     setEpsInf(EPSILON_ZIV);
#endif

  if (petit_args.dep_analysis_debug)
    fprintf(debug, "Dependence is disproved\n");

  return;
 }


if (res==DD_YES || nSubscr==0 || sharedDepth==0)
  {noteDependence(sit,d_info);

#if defined compareOmegaEpsilon
   if (storeInf && getEpsInf()!=EPSILON_ASSUME)
     setEpsInf(EPSILON_ZIV);
#endif
   return;
 }


/* proceed to simple tests */

if (petit_args.DDalgorithm!=DDalg_banerjee)
  {
    if (petit_args.dep_analysis_debug)
      fprintf(debug, "Doing simple tests\n");

    if (check_simple(d_info)==DD_NO)
    {
    #if defined compareOmegaEpsilon
      if (storeInf)
        setEpsInf(EPSILON_SIMPLE);
    #endif
      return;
    }
    fill_directions(d_info);

    if (nActSubscr==0)
     {noteDependence(sit,d_info);
      #if defined compareOmegaEpsilon
        if (storeInf)
          setEpsInf(EPSILON_SIMPLE);
      #endif
      return;
    }
  } /* end simple phase */


/* proceed to Banerjee */
#if defined compareOmegaEpsilon
   if (storeInf)
     setEpsInf(EPSILON_BANERJEE);
#endif

eliminate_done_subscr();
form_banerjee_problem(d_info);


if (petit_args.dep_analysis_debug)
  {
   fprintf(debug, "Banerjee's problem just formed \n");
   fprintf(debug,"   BANERJEE BOUNDS:\n");
   for (i=0; i<nSubscr; i++)
     print_subscr_bounds(debug, &(Ban_bounds[i]));
   fprintf(debug, "  COMVAR:\n");
   for (i=1; i<=sharedDepth; i++)
     print_comvar(debug, &(comvar[i]), nSubscr);
   fprintf(debug, "  SUBSCRIPTS:\n");
   for (i=0; i<nSubscr; i++)
      print_subscr(debug,&(subscr[i]));
   fprintf(debug, "\n");    
   
 }    

if (check_star_bounds()==DD_NO)
  {
   if (petit_args.dep_analysis_debug)
     fprintf(debug,"Disproved by checking (*, ..., *) directions\n");
   return;     
  }

  res=check_banerjee_not_coupled(d_info);
  if (res==DD_NO)
  {
   if (petit_args.dep_analysis_debug)
     fprintf(debug, "Disproved by uncoupled check\n");
   return;
  }

if (nActSubscr==0)
  {noteDependence(sit,d_info);
   return;
 }

refine_direction(Ban_bounds,1,d_info,sit);

} /* end epsilon_test */

/*****
 *  epsilon_preproc - can the dependence be easily disproved or not
 *  ENTRY:
 *   a1,a2 - accesses
 *  EXIT:
 *   1 - dependence is disproved
 *   0 - dependence cannot be easily disproved
 *****/

int epsilon_preproc(a_access a1, a_access a2)
{
 int res;
 dir_and_diff_info d_info;
 sharedDepth=access_shared_depth(a1,a2);
 if ((res=form_problem(a1,a2))== DD_INEXACT)
   return 0;

if (res==DD_NO)
  return 1;

if (res==DD_YES || nSubscr==0 || sharedDepth==0)
    return 0;

if (check_simple(&d_info)==DD_NO)
   return 1;

return 0;
}
 
}
 

