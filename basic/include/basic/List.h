/*
 *  Linked lists with an interface like a bit of libg++'s SLList class
 */

#if ! defined _List_h
#define _List_h 1

#if 0
#include <basic/assert.h> /* List requires assert which needs Exit which */
#endif                    /* needs List!  just include assert in List.c  */
#include <stdio.h>  // for NULL
#include <basic/Iterator.h>
#include <basic/Collection.h>
#include <basic/Link.h>

//#include <basic/enter_List.h>

namespace omega {

template<class T> class List_Iterator;

//
// indexing of Lists starts at 1, index == 0 means not there
//

template<class T> class List : public Sequence<T> {
public:
    List(const List<T> &l)
    { contents = l.contents ? new List_Element<T>(*l.contents) : 0; }
    List() { contents = 0; }
    virtual ~List() { delete contents; }

    Iterator<T> *new_iterator();
    const T &operator[](int) const;
          T &operator[](int);

         int index(const T &) const;
    
         int size() const;
         int length() const { return size(); }
	 bool empty() const { return size() == 0; }

           T &front() const;

// insertion/deletion on a list invalidates any iterators
// that are on/after the element added/removed

           T remove_front();	       

        void prepend(const T &item);
        void append(const T &item);
        void ins_after(List_Iterator<T> i, const T &item);

        void del_front();
        void del_after(List_Iterator<T> i);
        void clear();

        void join(List<T> &consumed);

private:
    friend class List_Iterator<T>;
    List_Element<T> **end()
    {
    List_Element<T> **e = &contents;
    while (*e)
	e = &((*e)->tail);
    return e;
    }

    List_Element<T> *contents;
};


template<class T> class List_Iterator : public List_Element_Iterator<T> {
public:
    List_Iterator(List<T> &l);
    List_Iterator(const List<T> &l);
    List_Iterator();
private:
    List_Element<T> &element() { return *(this->i); }
    friend class List<T>;
};

} // end of namespace omega

#if ! defined DONT_INCLUDE_TEMPLATE_CODE
#include <basic/List.c>
#endif

namespace omega {
#define instantiate_List(T)	template class List<T>; \
				template class List_Iterator<T>; \
				instantiate_Only_List_Element(T) \
				instantiate_Sequence(T)

}


#endif
