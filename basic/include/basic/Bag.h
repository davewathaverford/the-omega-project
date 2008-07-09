#if ! defined _Bag_h
#define _Bag_h 1

#include <basic/bool.h>
#include <stdio.h>
#include <basic/assert.h>
#include <basic/Iterator.h>
#include <basic/Collection.h>
#include <basic/Link.h>

//#include <basic/enter_Bag.h>


namespace omega {

template<class T> class Bag : public Collection<T> { 
public:
virtual ~Bag();
	Bag();
	Bag(const Bag<T>&);
	Bag & operator=(const Bag<T>&);
virtual	void operator |= (const Bag<T> & b); // add elements in b
	Iterator<T> *new_iterator();
	bool empty() const;
	void remove(T);
virtual	void insert(T);
	void clear();
virtual	bool contains(T) const;
	int size() const;
	T extract();
// protected:  breaks g++ 261
	List_Element<T>* contents;
};


template<class T> class Ordered_Bag : public Bag<T> { 
public:
	Ordered_Bag();
// virtual ~Ordered_Bag();
	Ordered_Bag(const Ordered_Bag<T>& B) : Bag<T>(B) {}
	void insert(T);
virtual	void operator |= (const Ordered_Bag<T> & b); // add elements in b
	void operator |= (const Bag<T> & b);
	bool  contains(T) const;
        bool  operator == (const Ordered_Bag<T>&) const;
        bool  operator != (const Ordered_Bag<T>&) const;
        bool  operator <  (const Ordered_Bag<T>&) const;
};

template <class T> class Set : public Ordered_Bag <T> { 
public:
	Set();
// virtual ~Set();
	Set(T);
	Set(const Set<T>& S) : Ordered_Bag<T>(S) {}

        bool  contains (const Set<T>& b) const;
        bool  contains (T t) const { return Ordered_Bag<T>::contains(t); }
	// the above makes "standard" C++ happy

virtual	void operator |= (const Set<T> & b); // add elements in b
	void operator |= (const Ordered_Bag<T> & b);
	void operator |= (const Bag<T> & b);

        void operator -= (const Set<T> & b); // delete items also in b
        void operator &= (const Set<T> & b); // delete items not in b
        bool operator & (const Set<T> &) const; // check for elements in common
};

} // end of namespace omega



#if ! defined DONT_INCLUDE_TEMPLATE_CODE
#include <basic/Bag.c>
#endif

namespace omega {

#define instantiate_Bag(T)		template class Bag<T>; \
					instantiate_List_Element(T);
#define instantiate_Ordered_Bag(T)	template class Ordered_Bag<T>; \
					instantiate_Bag(T)
#define instantiate_Set(T)		template class Set<T>; \
					instantiate_Ordered_Bag(T)
}

#endif
