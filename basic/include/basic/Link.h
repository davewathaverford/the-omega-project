#if ! defined _Link_h
#define _Link_h 1

#include <basic/bool.h>
#include <basic/Iterator.h>
#include <stddef.h>

// By default, if ndebug is not set, do not do free list

#if ! defined ListElementFreeList
#if ! defined NDEBUG || defined ASSERTIONS_ANYWAY
#define ListElementFreeList 0
#else
#define ListElementFreeList 1
#endif
#endif

namespace omega {


/* 
   List_Element: one item in a list and the pointer to the next.
   Each such object should be pointed to by either exactly one
    other List_Element or by some other pointer(s), exactly one
    of which will delete the List_Element.
   ListElements should ONLY be allocated on the heap.
 */

#if ListElementFreeList
  // g++ 2.5.8 does not allow static data in template classes, so...
  extern void *kludgy_List_Element_new(size_t size);
  extern void  kludgy_List_Element_delete(void *ptr, size_t size);
#endif

template <class T> class List_Element {
public:
#if ListElementFreeList
        void *operator new(size_t size)
	    {
	    return kludgy_List_Element_new(size);
	    }
        void operator delete(void *ptr, size_t size)
	    {
	    kludgy_List_Element_delete(ptr, size);
	    }
#endif

	T head;
	List_Element<T> *tail;

	List_Element() { 
		tail = 0;
		}
	List_Element(T h, List_Element<T> * t) {
		head = h;
		tail = t;
		}
	List_Element(const List_Element<T> & L) {
		 head =  L.head;
		 if (L.tail) tail = new List_Element<T>(*L.tail);
		 else tail = 0;
		}
	List_Element & operator=(const List_Element<T> &L) {
		if (this != &L) {
		  head = L.head;
		  if (tail) delete tail;
		  if (L.tail) tail = new List_Element<T>(*L.tail);
		  else tail = 0;
		}
		return *this;
		}
	virtual ~List_Element() {  // virtual ensures 2nd arg of delete is right
		delete tail;
		}
};



template<class T> class List_Element_Iterator : public Iterator<T> {
public:
    List_Element_Iterator(List_Element<T>* j) { i = j; }
    virtual const T &  operator*() const { return i->head; }
    virtual       T &  operator*()       { return i->head; }
    virtual    void    operator++(int)      { i = i->tail; }
    virtual    void    operator++()         { i = i->tail; }
    virtual    bool    live()      const { return i != 0; }
        Iterator<T> *  new_copy()  const { return new List_Element_Iterator<T>(i);}
    
protected:
    List_Element<T> *i;
};

#define instantiate_Only_List_Element(T) template class List_Element<T>; \
					 template class List_Element_Iterator<T>;
#define instantiate_List_Element(T)	instantiate_Only_List_Element(T)\
					instantiate_Collection(T)


} // end of namespace omega

#endif
