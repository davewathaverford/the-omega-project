#if !defined Already_Included_Collection
#define Already_Included_Collection

//#include <basic/enter_Iterator.h>
//#include <basic/enter_Collection.h>


namespace omega {

template<class T> class Iterator;
template<class T> class Any_Iterator;


/*
 * protocol for any kind of collection
 */

template<class T> class Collection {
public:
    Collection() {}
    virtual ~Collection() {}
    virtual Iterator<T> *new_iterator() = 0;
    virtual Any_Iterator<T>   any_iterator()     { return Any_Iterator<T>(new_iterator()); }

    virtual int size() const = 0;
};


/*
 * protocol for collections whose elements are ordered
 * by the way they are entered into the collection, and
 * whose elements can be accessed by "index"
 *
 * note that the implementation need not be a linked list
 */

template<class T> class Sequence : public Collection<T> {
public:
    Sequence() {}
    virtual ~Sequence() {}
    virtual const T &operator[](int) const = 0;
    virtual       T &operator[](int)       = 0;

    virtual int index(const T &) const = 0;  // Y in X --> X[X.index(Y)] == Y
};




#define instantiate_Collection(T)	template class Collection<T>; \
					instantiate_Any_Iterator(T)
#define instantiate_Sequence(T) 	template class Sequence<T>; \
					instantiate_Collection(T)

} // end of namespace omega
#endif 

