#include <code_gen/MMap.h>

namespace omega {

/*
 * Utility functions for dealing with stm_info arrays
 */

/*
 * Transform each iteration space of info[which_stm],
 * updating mmaps and data flow accordingly.
 * Return the "info" argument
 */
Tuple<stm_info> &Trans_IS(Tuple<stm_info> &info, const Relation &Trans);

/*
 * Set the memory mapping for a given statement to new_MMap restrictDomain IS
 *  where IS is the iteration space of the statement (this may not be
 *  easily available when calling Set_MMap).
 * Returns the "info" argument
 */
Tuple<stm_info> &Set_MMap(Tuple<stm_info> &info, int stm, const MMap &new_MMap);

/*
 * Replace iterations of a statement with collections of statements
 *  representing the unrolling of a loop level.
 * The statement to be unrolled must differ from those not to be unrolled
 *  in outer levels.
 * Statement "stm" is replaced with a collection of "factor" statements
 *  for which in the first, IS.input(loop) mod factor = 0, etc.
 *  the statements are added as a new "level" immediately inside the
 *  level being unrolled.  It might be possible to avoid this, but
 *  testing whether on not that is the case is a bit hard.
 * Also adds this new level (made of all -1) to every other statement.
 * Returns the "info" argument
 */
Tuple<stm_info> &Unroll_One_IS(Tuple<stm_info> &info, int stm, int level, int factor);


/*
 * Peel some iterations from info[which_stm],
 * Replacing which_stm with two statements,
 * The first will have iterations (info[which_stm].IS && peel),
 *  and the second the remaining iterations.
 * As in unrolling, there will be a new level added,
 *  but in this case it will be before level "level"
 */
Tuple<stm_info> &Peel_One_IS(Tuple<stm_info> &info, int stm, int level, Relation &peel);

/*
 * In this version of peeling, we produce three statements,
 * like Peel(Peel(info,stm.peel_before), stm+1, Complement peel_after)
 * except that it only adds one level
 */
Tuple<stm_info> &Peel_One_IS(Tuple<stm_info> &info, int stm, int level, Relation &peel_before, Relation &peel_after);



void DoDebug2(char *message, const Tuple<stm_info> &info);
void DoDebug(const char *message, int effort, const Tuple<stm_info> &info, const Relation &known);


extern int code_gen_debug;
#define tcodegen_debug code_gen_debug
static const char *debug_mark_cp = "# tcodegen debug:\t";
static const String debug_mark = debug_mark_cp;

} // end namespace omega
