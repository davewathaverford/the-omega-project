/* range.h,v 1.1.1.1 1992/07/10 02:41:06 davew Exp */
#ifndef Already_Included_Range
#define Already_Included_Range

namespace omega {

typedef struct {
    unsigned int _first;
    unsigned int _length;
} range;

#define r_first(r)   ((r)->_first)
#define r_last(r)    ((r)->_first + (r)->_length - 1)
#define r_length(r)  ((r)->_length)
#define r_in(r, i)   ((i) >= r_first(r) && (i) <= r_last(r))
/* #define r_grow(r)    (++(r)->_length) 
   grow is no longer allowed, as variables after the last region are
   used for iteration number counts */

}

#endif
