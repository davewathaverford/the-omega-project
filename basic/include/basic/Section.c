#include <basic/bool.h>

using namespace omega;

template <class T> Section<T>::Section(Sequence<T> *s, int start, int length)
    {
    assert(s->size() >= start-1 + length);
    it = s;
    _start = start;
    _length = length;
    }

template <class T> Iterator<T> *Section<T>::new_iterator()
    {
    return new Section_Iterator<T>(*this);
    }

template <class T> const T &Section<T>::operator[](int i) const
    {
    assert(1 <= i && i <= size());
    return (*it)[i+(_start-1)];
    }

template <class T> T &Section<T>::operator[](int i)
    {
    assert(1 <= i && i <= size());
    return (*it)[i+(_start-1)];
    }

template <class T> int Section<T>::index(const T &var) const
    {
    int i;
    for (i=1; i<=size(); i++)
	if ((*this)[i] == var)
	    return i;
    return 0;
    }

template <class T> int Section<T>::size() const
    {
    return _length;
    }


template <class T> Section_Iterator<T>::Section_Iterator(Section<T> &sec)
    {
    it = sec.it->new_iterator();
    for (int i = 1; i < sec._start; i++)
	(*it)++;
    remaining = sec.size();
    }


template <class T> Section_Iterator<T>::Section_Iterator(const Section_Iterator<T> &si) : it(si.it), remaining(si.remaining) {}


template <class T> void Section_Iterator<T>::operator++()
    {  this->operator++(0);  }

template <class T> void Section_Iterator<T>::operator++(int)
    {
    if (remaining > 0)
	{
	(*it)++;
	remaining--;
	}
    }

template <class T> bool Section_Iterator<T>::live() const
    {
    return (remaining > 0);
    }

template <class T> Iterator<T> *Section_Iterator<T>::new_copy() const
    {
    return new Section_Iterator<T>(*this);
    }


