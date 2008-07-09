#include <basic/bool.h>

using namespace omega;

/* class Tuple */



// THESE FIRST TWO REALLY SHOULD BE INLINE BUT IT BREAKS CFRONT:

template<class T>  T& Tuple<T>::operator[](int index)
    {
    assert(1 <= index && index <= sz); return data[index-1];
    }

template<class T>  const T& Tuple<T>::operator[](int index) const
    {
    assert(1 <= index && index <= sz); return data[index-1];
    }


template<class T> Tuple<T>::~Tuple()
    {
    if (data)
	delete [] data;
    }

template<class T> Tuple<T>::Tuple()  : sz(0), alloc_sz(0),
 prealloc_min(20),prealloc_pad(5), data(0)
{
	// nothing needs be done
	}

template<class T> Tuple<T>::Tuple(int size)  : sz(size),
 prealloc_min(20),prealloc_pad(5)
{
    if (sz > 0)
	{
	    alloc_sz = prealloc_size(sz);
	    data = new T[alloc_sz];
	    assert(alloc_sz >= sz);
	    //Need some handling for out of memory.
	    assert (data!=0);
      } 
    else {
      alloc_sz = 0;
      data = 0;
    }
}


template<class T> Tuple<T>::Tuple(const Tuple<T>& t) 
    : sz(t.sz), alloc_sz(t.alloc_sz),  prealloc_min(20),prealloc_pad(5)
{
    if (sz > 0) {
	data = new T[alloc_sz];
	assert (data!=0);
	assert (alloc_sz >= sz);
	for (int i=0; i<sz; i++)
	    data[i] = t.data[i];
    } else {
	data = 0;   
	alloc_sz = 0;  // THis might not be 0 if it was a "clear"ed Tuple
//        assert(alloc_sz == 0);
    }
}


template<class T> Tuple<T>& Tuple<T>::operator=(const Tuple<T>& t)
{
    if (this != &t) {                 // Delete this
	if (data)
	    delete [] data;
	sz = t.sz;
	alloc_sz = t.alloc_sz;
	assert(alloc_sz >= sz);
	if (sz > 0) {                 // Copy old
	    data = new T[alloc_sz];
	    assert (data!=0);
	    for (int i=0; i<sz; i++)
		data[i] = t.data[i];
	} else {
	    data=0;
	    alloc_sz = 0;   // THis might not be 0 if it was a "clear"ed Tuple
//	    assert(alloc_sz == 0);
	}
    }   
    return *this;
}


template<class T> void Tuple<T>::reallocate(const int req_size) 
{
    if (alloc_sz >= req_size) { // if (sz >= req_size), does this.
	sz = req_size;
	return;
    }
    alloc_sz = prealloc_size(req_size);
    T* tmp_data = new T[alloc_sz];
    for(int i=0;i<sz;i++)
	tmp_data[i] = data[i];
    delete [] data;
    data = tmp_data;
    sz = req_size;
    assert(alloc_sz >= req_size);
}

template<class T> void Tuple<T>::delete_last()
{
assert(sz > 0);
sz --;
}

template<class T> void Tuple<T>::append(const T &v)
{ 
    // Check if reallocation is necessary.
    if (sz == 0)  {                 // Empty Tuple
	assert(alloc_sz >= 0);      // May be nonzero for cleared tuple

	if(alloc_sz == 0) {         // If it's > 1 no allocation is necessary
	    alloc_sz = prealloc_size(1);
	    data = new T[alloc_sz];
	}
	assert (alloc_sz > 0 && data != 0);
    } else {
	if(sz == alloc_sz)  {       // Requires new allocation
	    alloc_sz = realloc_size(alloc_sz);
	    T * data_tmp = new T[alloc_sz];
	    assert (data_tmp!=0);
	    assert (alloc_sz > sz);
	    for (int i=0; i<sz; i++)
		data_tmp[i] = data[i];
	    delete [] data;
	    data=data_tmp; 
	}              // Otherwise big enough, no reallocation necessary
    }
    // Make assignment
    assert(alloc_sz >= sz);
    data[sz++] = v; 
}

template<class T> void Tuple<T>::append(const Tuple<T>& t) {
    int old_sz = sz;
    reallocate(t.size()+size());
    assert(alloc_sz >= sz);
    for(int i=0; i<t.sz; i++)
	data[i+old_sz] = t.data[i];
}

template<class T> void Tuple<T>::join(Tuple<T>& t) {
    int old_sz = sz;
    reallocate(t.size()+size());
    assert(alloc_sz >= sz);
    for(int i=0; i<t.sz; i++)
	data[i+old_sz] = t.data[i];
    t.clear();
}

template<class T> void Tuple<T>::clear() { if (sz) delete [] data; data = 0; alloc_sz = 0; sz = 0; }

template<class T> int  Tuple<T>::empty() const { return (sz == 0); }

template<class T> Iterator<T> *Tuple<T>::new_iterator() 
{
    return new Tuple_Iterator<T>(*this);
}

template<class T> int Tuple<T>::index(const T & var) const
/* returns index or 0 if var isn't in the tuple */
{
 int i;
 for (i=0; i<sz; i++)
   if (data[i]== var)
    return i+1;
 return 0;  
}

template<class T> bool Tuple<T>::operator == (const Tuple<T>& b) const 
{
 int i;
 if (sz != b.size()) return false;
 for (i=0; i<sz; i++)
   if (!(data[i] == b[i+1])) return false;
 return true;  
}

/* class Tuple_Iterator */

template<class T> Tuple_Iterator<T>::Tuple_Iterator(const Tuple<T> &tpl) : 
current(tpl.data), lastptr(tpl.data+tpl.sz-1), firstptr(tpl.data), sz(tpl.sz)
{
}					     

template<class T> Tuple_Iterator<T>::Tuple_Iterator(T * cr, T *frst, T * lst,
						  int insz) 
    : current(cr), lastptr(lst), firstptr(frst), sz(insz)
{
}

template<class T> const T & Tuple_Iterator<T>::operator*() const
{
    assert (current<=lastptr && current>=firstptr);
    return *current;
}

template<class T> T & Tuple_Iterator<T>::operator*()
{
    assert (current<=lastptr && current >=firstptr);
    return *current;
}

template<class T> void Tuple_Iterator<T>::operator++(int)
{
    current++;
}

template<class T> void Tuple_Iterator<T>::operator++()
{
    current++;
}

template<class T> void Tuple_Iterator<T>::operator--(int)
{
    current--;
}

template<class T> void Tuple_Iterator<T>::operator--()
{
    current--;
}

template<class T> void Tuple_Iterator<T>::set_to_last()
{
    current = lastptr;
}

template<class T> void Tuple_Iterator<T>::set_to_first()
{
    current = firstptr;
}

template<class T> void Tuple_Iterator<T>::set_position(const int req_pos)
{
    assert(req_pos <= sz && 1 <= req_pos);
    current = firstptr + (req_pos - 1);
}


template<class T> bool Tuple_Iterator<T>::live() const
{
    return (current !=0 &&  current<=lastptr && current >= firstptr);
}

template<class T> Iterator<T> *Tuple_Iterator<T>::new_copy() const {
    return new Tuple_Iterator<T>(current, firstptr, lastptr, sz); 
}

