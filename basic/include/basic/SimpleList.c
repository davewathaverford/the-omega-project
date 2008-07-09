
namespace omega {



template<class T> Simple_List_Iterator<T>::Simple_List_Iterator(Simple_List<T> &l) 
: List_Element_Iterator<T>(l.contents) {}

template<class T> Simple_List_Iterator<T>::Simple_List_Iterator(const Simple_List<T> &l) 
: List_Element_Iterator<T>(l.contents) {}

template<class T> Simple_List_Iterator<T>::Simple_List_Iterator()
: List_Element_Iterator<T>(0) {}

template<class T> Iterator<T> *Simple_List<T>::new_iterator()
{
    return new Simple_List_Iterator<T>(*this);
}

template<class T> const T &Simple_List<T>::operator[](int i) const
{
    Simple_List_Iterator<T> p(*this);

    while(--i > 0 && p)
	p++;

    if (p)
	return *p;
    else
	return *((T *)0);
}

template<class T>       T &Simple_List<T>::operator[](int i)
{
    Simple_List_Iterator<T> p(*this);

    while(--i > 0 && p)
	p++;

    if (p)
	return *p;
    else
	return *((T *)0);
}


template<class T> int Simple_List<T>::size() const
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

template<class T> T &Simple_List<T>::front() const
    {
    return contents->head;
    }

template<class T> T Simple_List<T>::remove_front()
    {
    List_Element<T> *frunt = contents;
    contents = contents->tail;
    T fruntT = frunt->head;
    frunt->tail = 0;
    delete frunt;
    return fruntT;
    }

template<class T> void Simple_List<T>::prepend(const T &item)
    {
    contents = new List_Element<T>(item, contents);
    }


template<class T> void Simple_List<T>::append(const T &item)
    {
    *(end()) = new List_Element<T>(item, 0);
    }


template<class T> void Simple_List<T>::del_front()
    {
    List_Element<T> *e = contents;
    contents = contents->tail;
    e->tail = 0;
    delete e;
    }


template<class T> void Simple_List<T>::clear()
    {
    delete contents;
    contents = 0;
    }

template<class T> void Simple_List<T>::join(Simple_List<T> &consumed)
    {
    List_Element<T> *e = consumed.contents;
    consumed.contents = 0;
    *(end()) = e;
    }

} // end of namespace omega

