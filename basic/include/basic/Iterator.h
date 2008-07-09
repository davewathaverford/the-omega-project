/*
 * Base classes for iterators, generators
 *
 * These don't really work yet for constant collections.
 * I'm not sure how to make that happen.
 */

#if ! defined _Iterator_h
#define _Iterator_h 1

#include <basic/bool.h>

#include <basic/Collection.h>

#define foreach(x,T,S,A) do {for (Any_Iterator<T> __P_##x = (S).any_iterator();__P_##x;__P_##x++) {T & x = *__P_##x; A;}} while (0)

#define foreachSeparated(x,T,S,A,B) do {for (Any_Iterator<T> __P_##x = (S).any_iterator();__P_##x;) {T & x = *__P_##x; A; __P_##x++; if (__P_##x) B;}} while (0)

//#include <basic/enter_Iterator.h>


namespace omega {

/*
 * Abstract base class Iterator<type>
 * Supports two styles of iteration:
 *
 *    for ( ... initialize i (typically i = collection) ...  ; i ; i++ )
 *        operate_on(*i)
 *
 * or
 *
 *    for ( ... initialize i ... ; i.live() ; i.next() )
 *        operate_on(i.curr())
 *
 *   >>> IF THE COLLECTION IS CHANGED, THE ITERATOR IS NO LONGER VALID <<<
 *
 * For collections that are not "Sequence"s, the order in
 *  which the elements are returned may not be consistent.
 */

template<class T> class Iterator {
public:
    Iterator() {}
    virtual ~Iterator() {}
    virtual const T &  operator*() const = 0;
    virtual       T &  operator*() = 0;

    virtual    void    operator++(int) = 0;
    virtual    void    operator++() = 0;
    
    virtual    bool    live() const = 0;
                       operator bool() const { return live(); }

            const T &  curr() const { return *(*this); }
                  T &  curr() { return *(*this); }
               void    next() { (*this)++; }

  virtual Iterator<T> *new_copy() const = 0;
};


// A generator is like an iterator but it gives out values,
// which may or may not exist in some writable collection

template<class T> class Generator {
public:
    Generator() {}
    virtual ~Generator() {}
    virtual       T    operator*() const = 0;

    virtual    void    operator++(int) = 0;
    virtual    void    operator++() = 0;
    
    virtual     int    live() const = 0;
                       operator int() const { return live(); }

            const T    curr() const { return *(*this); }
                  T    curr() { return *(*this); }
               void    next() { (*this)++; }
};



// Delegate to any kind of iterator (on the heap)
// If created via a reference, become a copy of the iterator
// If created via a pointer, manipulate that pointer and free *p when this dies
//
// Mostly useful for Collection::iterator
// Iterator::Iterator(Collection)


template<class T> class Any_Iterator : public Iterator<T> {
public:
    Any_Iterator(Collection<T> &c);
    Any_Iterator(const Iterator<T> &i);  // copy of i

    virtual ~Any_Iterator() { delete me; }

    Any_Iterator<T> &operator=(const Any_Iterator<T> &rhs)
    { delete me; me = rhs.me->new_copy(); return *this; }
    
    const T &  operator*() const	{ return *(*me); }
          T &  operator*()		{ return *(*me); }
       void    operator++(int)		{ (*me)++; }
       void    operator++()		{ ++(*me); }
       bool    live() const		{ return (*me).live(); }

    Iterator<T> *new_copy() const	{ return new Any_Iterator<T>((*me).new_copy()); }

private:
    Any_Iterator(Iterator<T> *p)  // take over *p, *p MUST BE ON THE HEAP
    { me = p; }
    friend class Collection<T>;
#if 0  
    // Couldn't make this work with g++258
    friend Any_Iterator<T> Collection<T>::any_iterator();
#endif    
    Iterator<T> *me;
};

template <class T> inline Any_Iterator<T>::Any_Iterator(Collection<T> &c)
    {
    me = c.new_iterator();
    }

template <class T> inline Any_Iterator<T>::Any_Iterator(const Iterator<T> &i)
    {
    me = i.new_copy();
    }

#define instantiate_Iterator(T) 	template class Iterator<T>;
#define instantiate_Generator(T) 	template class Generator<T>;
#define instantiate_Any_Iterator(T)	template class Any_Iterator<T>; \
					instantiate_Iterator(T)

} // end of namespace omega


#endif
