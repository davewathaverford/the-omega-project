#if ! defined Already_Included_Util
#define Already_Included_Util

#ifdef WIN32
#define LONGLONG _int64
#else
#define LONGLONG long long
#endif

#include <stdio.h>
#include <limits.h>
#if defined(sparc)
#    include <values.h>
#else
#    include <float.h>
#endif

#if !defined(MAXINT)
#    define MAXINT INT_MAX
#endif
#if !defined(MAXLONGLONG)
#ifdef WIN32
#    define MAXLONGLONG (0x7ffffffffffffff)
#else
#    define MAXLONGLONG (0x7ffffffffffffffLL)
#endif
#endif
#if !defined(MAXDOUBLE)
#    define MAXDOUBLE DBL_MAX
#endif


#include <basic/assert.h>

namespace omega {

#ifdef __SUNPRO_CC
template<class T,class S> inline T max(T x, S y) {
	if (x >= y) return x; else return y;
	}
#else
template<class T> inline T max(T x, T y) {
	if (x >= y) return x; else return y;
	}
#endif

template<class T> inline T max(T x, T y, T z) {
	if (x >= y) return max(x,z); else return max(y,z);
	}

template<class T> inline T min(T x, T y) {
	if (x <= y) return x; else return y;
	}
template<class T> inline T min(T x, T y, T z) {
	if (x <= y) return min(x,z); else return min(y,z);
	}

template<class T> inline void set_max(T &m, T x) {
	if (m < x) m = x;
	}
template<class T> inline void set_min(T &m, T x) {
	if (m > x) m = x;
	}

template<class T> inline void swap(T &i, T &j) {
    T tmp;
    tmp = i;
    i = j;
    j = tmp;
}

template<class T> inline T copy(const T &t) { return t; }

inline int
check_pos_mul(int x, int y) {
#if !defined NDEBUG
    if (x != 0)
        assert(((MAXINT)/4) / x > y);
#elif defined STILL_CHECK_MULT
    if (x != 0 && !(((MAXINT)/4) / x > y))
      {
	fprintf(stderr, "Integer overflow during multiplication (util.h)\n");
	Exit(-2);
      }
#endif
    return x * y;
}

inline LONGLONG
check_pos_mul(LONGLONG x, LONGLONG y) {
#if !defined NDEBUG
    if (x != 0)
        assert(((MAXLONGLONG)/4) / x > y);
#elif defined STILL_CHECK_MULT
    if (x != 0 && !(((MAXLONGLONG)/4) / x > y))
      {
	fprintf(stderr, "Integer overflow during multiplication (util.h)\n");
	Exit(-2);
      }
#endif
    return x * y;
}

inline LONGLONG abs(LONGLONG c) { return (c>=0?c:(-c)); } 


template<class T> inline T
check_mul(T x, T y) {
#if defined NDEBUG && ! defined STILL_CHECK_MULT
    return x*y;
#else
    if (x >= 0) {
        if (y >= 0)
            return check_pos_mul(x, y);
        else
            return -check_pos_mul(x, -y);
    } else if (y >= 0)
        return -check_pos_mul(-x, y);
    else
        return check_pos_mul(-x, -y);
#endif
}

template<class T> inline T int_div(T a, T b) {
        T result;
	assert(b > 0);
	if (a>0) result = a/b;
	else     result = -((-a+b-1)/b);
	return result;
	}

template<class T> inline T int_mod(T a, T b) {
	return a-b*int_div(a,b);
	};

template<class T> inline T int_mod_hat(T a, T b) {
	T r;
	assert(b > 0);
	r = a-b*int_div(a,b);
	if (r > -(r-b)) r -= b;
	return r;
	};

template<class T> inline
T gcd(T b, T a) /* First argument is non-negative */
    {
    assert(a >= 0);
    assert(b >= 0);
    if (b == 1)
	return (1);
    while (b != 0) 
	{
	T t = b;
	b = a % b;
	a = t;
	};
    return (a);
    }

template<class T> inline
T lcm(T b, T a) /* First argument is non-negative */
    {
    assert(a >= 0);
    assert(b >= 0);
    return a*b/gcd(a,b);
    }

#define implies(A,B) (A==(A&B))

} // end of namespace omega


#endif
