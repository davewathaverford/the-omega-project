/* $Id: omega2flags.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
/* compile time and run time flags that control the behavior of the
   code that eliminates dead dependences */
#ifndef Already_Included_Omega2flags
#define Already_Included_Omega2flags

namespace omega {

#if defined SKIP_OMEGA4
#define skipping_omega4 1
#else
extern int skipping_omega4;
#endif

#if defined SKIP_OMEGA5
#define skipping_omega5 1
#else
extern int skipping_omega5;
#endif

#if defined SKIP_ZAPPABLE
#define skipping_zappable 1
#else
extern int skipping_zappable;
#endif


#if defined ONLY_CHANGE_FLOW_LEVEL
#define skipping_plus_refinement 1
#else
#define skipping_plus_refinement 0
#endif

#if defined ONLY_CHANGE_FLOW_LEVEL || OMEGA2_FOR_FLOW_ONLY
/* skip cover test for output deps and termination test for anti deps */
#define skipping_some_cover_and_term  1
#else
#define skipping_some_cover_and_term  0
#endif

#if ! defined skipping_all_tightening
#define skipping_all_tightening  0
#endif

#if ! defined skipping_bailout
#define skipping_bailout	 0
#endif

#if defined EXTRAVAGANT
#define doing_all_accurate_kills 1
/* do refinement of all kinds of dds:
   "skipping_plus_refinement" still works
   "only_worry_about_cycles" should too
   */
#endif

/* There are several ways to do covers and terminators:
   test dependences containing all 0's to see if the cover,
     if so, try to tighten them as much as possible
   test all dependences, try to tighten them
   test dependences containing all 0's to see if 0,0,0,0 covers
     this will miss some covers but requres no tightening
   to get these three options, set the flags below to 1,0; 0,0; or 1,1
*/
             
#if ! defined cover_test_only_with_zeros
#define cover_test_only_with_zeros 1
#endif
#if ! defined cover_test_the_zeros
#define cover_test_the_zeros 0
#endif

/* kill everything the hard way - no covers or terminatiors */
#if ! defined naive_kills
#define naive_kills 0
#endif

/* Only do refinement & accurate kills for dependences in cycles,
   and only test for cover/termination if nest > 0 */
#if ! defined only_worry_about_cycles
#define only_worry_about_cycles 0
#endif

#if defined SKIP_MONICA_TESTS
#define do_monica_tests 0
#else
#define do_monica_tests 1
#endif

#if only_worry_about_cycles && defined OMIT_DDS_FOR_SCALARS
I guess you did not read the makefile sufficiently carefully.
Those options do not go together.  This will not compile.
#endif

}

#endif

namespace omega {

extern int number_of_easy_assulters;
extern int number_of_easy_assults;
extern int number_of_hard_assults;

}

