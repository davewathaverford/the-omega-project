#if ! defined _NonCoercible_h
#define _NonCoercible_h

/*   These are non-coercible ints, designed to be used where a constructor 
     needs a single int argument, but you don't want to allow automatic
     conversion from an int to a class object.  So what you have is a 
     constructor that takes a noncoercible int, so you can call it
     with an int, which gets converted to non-coercible int, but you never
     get an accidental conversion from int, since C++ does (should) not 
     apply multiple user-defined conversions in implicit casts.      */


#define TEMPLATIZED 1

namespace omega {

#if defined NONCOERCIBLE_WONT_WORK_AT_ALL

typedef int NonCoercibleInt;
inline int value(const int &i) { return i; }

#else  /* NONCOERCIBLE_WONT_WORK_AT_ALL */

template <class T> class NonCoercible {
public:
    NonCoercible(const T &i): val(i) {}
//    friend T value(const NonCoercible<T> &);

    T val;
};

template<class T> T value(const NonCoercible<T> &nc)
 { return nc.val; }

#define instantiate_NonCoercible(T) template class NonCoercible<T>; \
				    template T value(const NonCoercible<T> &);

#endif /* NONCOERCIBLE_WONT_WORK_AT_ALL */

} // end of namespace omega

#endif /* already included */
