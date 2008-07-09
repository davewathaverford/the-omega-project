/*
 *  Linked lists with an interface like a bit of libg++'s SLSimple_List class
 */

#if ! defined _Simple_List_h
#define _Simple_List_h 1

#include <basic/assert.h>
#include <stdio.h>  // for NULL
#include <basic/Iterator.h>
#include <basic/Collection.h>
#include <basic/Link.h>

#define Simple_List Omega_Simple_List
#define Simple_List_Iterator Omega_Simple_List_Iterator

namespace omega {

template<class T> class Simple_List_Iterator;

// A TEMPORARY HACK - ERROR IF YOU TRY TO USE "INDEX" - FERD

template<class T> class Simple_List : public Sequence<T> {
public:
    Simple_List(const Simple_List<T> &l)
    { contents = l.contents ? new List_Element<T>(*l.contents) : 0; }
    Simple_List() { contents = 0; }
    virtual ~Simple_List() { delete contents; }

    Iterator<T> *new_iterator();
    const T &operator[](int) const;
          T &operator[](int);

    
         int size() const;
         int length() const { return size(); }
	 int empty() const { return size() == 0; }

           T &front() const;

// insertion/deletion on a list invalidates any iterators
// that are on/after the element added/removed

           T remove_front();	       

        void prepend(const T &item);
        void append(const T &item);

        void del_front();
        void clear();

        void join(Simple_List<T> &consumed);

         int index(const T &) const
	    {
		fprintf(stderr, "ILLEGAL SimpleList operation\n");
		assert(0); abort(); return -1;
	    }

private:
    friend class Simple_List_Iterator<T>;
    List_Element<T> **end()
    {
    List_Element<T> **e = &contents;
    while (*e)
	e = &((*e)->tail);
    return e;
    }

    List_Element<T> *contents;
};


template<class T> class Simple_List_Iterator : public List_Element_Iterator<T> {
public:
    Simple_List_Iterator(Simple_List<T> &l);
    Simple_List_Iterator(const Simple_List<T> &l);
    Simple_List_Iterator();
private:
    List_Element<T> &element() { return *(this->i); } ;
    friend class Simple_List<T>;
};

} // end of namespace omega

#if ! defined DONT_INCLUDE_TEMPLATE_CODE
#include <basic/SimpleList.c>
#endif

namespace omega {
#define instantiate_Simple_List(T)	template class Simple_List<T>; \
				template class Simple_List_Iterator<T>; \
				instantiate_Only_List_Element(T) \
				instantiate_Sequence(T)


} // end of namespace omega

#endif
