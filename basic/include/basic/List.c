#include <basic/assert.h>
#include "List.h"

using namespace omega;

template<class T> List_Iterator<T>::List_Iterator(List<T> &l) 
: List_Element_Iterator<T>(l.contents) {}

template<class T> List_Iterator<T>::List_Iterator(const List<T> &l) 
: List_Element_Iterator<T>(l.contents) {}

template<class T> List_Iterator<T>::List_Iterator()
: List_Element_Iterator<T>(0) {}


template<class T> Iterator<T> *List<T>::new_iterator()
{
    return new List_Iterator<T>(*this);
}

template<class T> const T &List<T>::operator[](int i) const
{
    assert(i > 0 && "Subscript out of bounds");
    List_Iterator<T> p(*this);

    while(--i > 0 && p)
	p++;

    if (p)
	return *p;
    else
	return *((T *)0);
}

template<class T>       T &List<T>::operator[](int i)
{
    assert(i > 0 && "Subscript out of bounds");
    List_Iterator<T> p(*this);

    while(--i > 0 && p)
	p++;

    if (p)
	return *p;
    else
	return *((T *)0);
}

template<class T>      int List<T>::index(const T &item) const
{
    List_Iterator<T> p(*this);
    int i = 1;

    while(p && *p != item)
    {
	p++;
	i++;
    }

    if (p)
	return i;
    else
	return 0;
}

template<class T> int List<T>::size() const
    {
    int i = 0;
    List_Element<T> * p = contents;
    while (p)
	{
	p = p->tail;
	i++;
	}
    return i;
    }

template<class T> T &List<T>::front() const
    {
    return contents->head;
    }

template<class T> T List<T>::remove_front()
    {
    List_Element<T> *frunt = contents;
    contents = contents->tail;
    T fruntT = frunt->head;
    frunt->tail = 0;
    delete frunt;
    return fruntT;
    }

template<class T> void List<T>::prepend(const T &item)
    {
    contents = new List_Element<T>(item, contents);
    }


template<class T> void List<T>::append(const T &item)
    {
    *(end()) = new List_Element<T>(item, 0);
    }

template<class T> void List<T>::ins_after(List_Iterator<T> i,
					     const T &item)
    {
#if ! defined NDEBUG
    for (List_Element<T> *e = contents; e != &(i.element()); e=e->tail)
	{
	assert(e);
	}
#endif
    i.element().tail = new List_Element<T>(item, i.element().tail);
    }

template<class T> void List<T>::del_front()
    {
    List_Element<T> *e = contents;
    contents = contents->tail;
    e->tail = 0;
    delete e;
    }

template<class T> void List<T>::del_after(List_Iterator<T> i)
    {
#if ! defined NDEBUG
    for (List_Element<T> *e0 = contents; e0 != &(i.element()); e0=e0->tail)
	{
	assert(e0);
	}
#endif
    List_Element<T> *e = i.element().tail;
    i.element().tail = e->tail;
    e->tail = 0;
    delete e;
    }

template<class T> void List<T>::clear()
    {
    delete contents;
    contents = 0;
    }

template<class T> void List<T>::join(List<T> &consumed)
    {
    List_Element<T> *e = consumed.contents;
    consumed.contents = 0;
    *(end()) = e;
    }

     
