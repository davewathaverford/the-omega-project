#if !defined _Already_defined_tuple
#define _Already_defined_tuple

#include <basic/bool.h>
#include <basic/assert.h>
#include <stdio.h>

#include <basic/Collection.h>
#include <basic/Iterator.h>
#include <basic/util.h>

//#include <basic/enter_Tuple.h>

namespace omega {

template<class T> class Tuple_Iterator;

// TUPLES ARE INDEXED STARTING AT 1
// index\(i\) == 0 MEANS i IS NOT IN THE TUPLE

template <class T> class Tuple : public Sequence<T> {  
public:
    Tuple();
    Tuple(int size);
    Tuple (const Tuple<T>& tpl);
    virtual ~Tuple();
    Tuple<T>& operator=(const Tuple<T>& tpl);
    int size()  const { return sz; }
    int length()  const { return sz; }
    bool operator==(const Tuple<T> &b) const;
    void reallocate(const int);
    void delete_last(); 
    void append(const Tuple<T> &v); 
    void append(const T &v); 
    void join(Tuple<T> &v);
    void clear();
    int  empty() const;

    Iterator<T> *new_iterator();

    virtual T &operator[](int index);
    virtual const T &operator[](int index) const;

    int index(const T &) const;

    friend class Tuple_Iterator<T>;

private:
    int prealloc_size(const int req_size) 
         { return max(req_size+prealloc_pad,prealloc_min); }
    int realloc_size(const int oldsize) { return 2*oldsize; }


    int sz, alloc_sz;  // Number of elements, size of allocated array
    int prealloc_min,prealloc_pad; // These should be static, but that 
                                // causes portability prob. for initialization

protected:
    T * data;
};

template <class T> class Tuple_Iterator : public Iterator <T> {
public:
        Tuple_Iterator(const Tuple<T> &tpl);
  const T & operator*() const;
        T & operator*();
        void set_position(const int req_pos);
        void operator++(int);
        void operator++();
        void operator--(int);
        void operator--();
        void set_to_last();
        void set_to_first();
//        void set_position(const int req_pos); Don't do this, compiler bug
        bool live() const;
        Iterator<T> *new_copy() const;

private:
    Tuple_Iterator(T * cr, T * frst, T *lst, int insz);
    T  * current, * lastptr, *firstptr;
    int sz;
}; 
} // end of namespace omega

#if ! defined DONT_INCLUDE_TEMPLATE_CODE
#include <basic/Tuple.c>
#endif

namespace omega {
#define instantiate_Tuple(T)	template class Tuple<T>; \
				template class Tuple_Iterator<T>; \
				instantiate_Sequence(T)

} // end of namespace omega
   
#endif
