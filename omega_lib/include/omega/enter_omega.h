/*
 * This is a list of re-defines for everything in the Omega Library
 *  that has, so far, been found to conflict with something else.
 * We can add to it in the future if necessary.
 * Conflicts caused by things in the basic types (such as String,
 *  List, etc) are dealt with by individual enter and leave files
 *  in the basic subdirectory.
 */

#if ! defined Relation

//#define Relation Omega_Relation
//#define Formula  Omega_Formula
#define LOWER_BOUND OMEGA_LOWER_BOUND
#define UPPER_BOUND OMEGA_UPPER_BOUND
#define EXACT_BOUND OMEGA_EXACT_BOUND
#define UNSET_BOUND OMEGA_UNSET_BOUND

#endif

