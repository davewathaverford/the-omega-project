/* $Id: */

#ifndef Already_Included_coverlist
#define Already_Included_coverlist

#include <basic/bool.h>

namespace omega {


typedef struct {
    dd_current  dep;
/*    int         usefulness; */
    bool        end_of_group;
    bool        from_update;
/*    Problem    *exposed_elements; */
} coverlist_entry;

typedef struct {
    int size;
    coverlist_entry list[1];
} coverlist;


/* build coverlist, as described at end of comment below */
coverlist *build_coverlist(a_access r);

/* return 1 iff we can prove f1 <=c f2
   return 0 if it is not true or we can't figure it out based only on dddirs */
int approx_leq_c(dd_current f1, dd_current f2);

/* return 1 iff f1 <c! f2 */
int definite_lt_c(dd_current f1, dd_current f2);

/* return true iff f1<t! f2 */
bool definite_lt_t(dd_current f1, dd_current f2);


/* 
   Coverage Definition 1:
   ---------------------

   We say dependence f1 from W_f1 to R is <=c a dependence f2 from
   W_f2 to R if there is no dependence o from W_f1 to W_f2 such that
   we can execute W_f2 within distance(o) of W_f1 and then R within
   distance(f2) of W_f2, all while keeping R within distance(f1) of
   W_f1.  I.e., if if there is no o allowing

	W_f1(I') -o-> W_f2(I) -f2-> R(I'')
	        `-->-----f1----->--' 

   "<=c" is pronounced "covers at least as closely as".  Note that it
   is possible for f1 <=c f2 and f2 <=c f1 (consider two dependences
   of equal length that either (a) come from different control flow
   branches or (b) write different elements of the array).


   Coverage Theorem 1:
   ------------------

   In the context of a kill of f2 by f1, in which we have constraints
   enforcing W_f1(I') sub= R(I''),

     f1 <=c f2 and W_f2(I) << R(I'') and W_f2(I) sub= R(I'')
     implies   W_f2(I) << W_f1(I')

   Proof by contradiction:
      if f1 <=c f2
     and W_f2(I) << R(I'') and W_f2(I) sub= R(I'')
     and W_f1(I') sub= R(I'')
     and not W_f2(I) << W_f1(I')
    then W_f1(I') << W_f2(I) and W_f2(I) sub= W_f1(I')
         this violates the definition of <=c


   Thus, f1 <=c f2, the effect of c during the attempt to kill f2 can
   be computed without inclusion of the constraints that force W_f2 to
   preceed W_f1.


   Coverage Definition 2:
   ---------------------

   We say dependence f1 from W_f1 to R is <c! a dependence f2 from
   W_f2 to R if the maximum dependence distance for f1 must be less
   than the minimum dependence distance for f2.  Distance d1 is
   considered less than d2 if it is lexicographically smaller or they
   are lexicographically equal and within any iteration the source of
   d1 is executed after that of d2.

   "<c!" is pronounced "definitely covers more closely than".  Note
   that f1 <c! f2 and f2 <c! f1 are mutually exclusive, and that
   f1 <c! f2 implies (f1 <=c f2 and not f2 <=c f1).

   Note also that neither it may be the case that neither f1 <c! f2 or
   f2 <c! f1 is true.  In such cases, we know nothing about the <=
   relationships between f1 and f2.

   Coverage Theorem 2: Transitivity of <c!
   ------------------

   If f1 <c! f2, then for any f for which f2 <c! f, f1 <c! f.

   This follows easily from the definition.


   The Cover List
   --------------

   We associate a cover list with each read.  The list is divided into
   groups such that every element of a group is <c! every element of
   every later group, and all elements within a group are not <c! each
   other.

   If some prefix of the list completely covers the read, then any
   dependence f that is >c! every element of that prefix is dead (by
   theorems 1 and 2).

   For any fi, fj in a group, (fi before fj), we say that fi is the
   most useful prefix for fj if forall k<=i fk <=c fj & ~ f(i+1) <=c fj
   The usefulness of fi is the number of other dependences for which it
   is the m.u.p.  Whenever the usefulness is >= 2, we can save work by
   storing the exposed elements beyond this write.

   The elements within each group should be sorted to attempt to max.
   the sum over fi of (useful(fi) ? usefulness(fi) - 1 : 0).
   Currently this is done by trying to order the prefixes such that,
   if f1 <=c f2 and not f2 <=c f1, then f1 precedes f2.

  */

}

#endif
