#if ! defined _Map_h
#define _Map_h 1

#include <basic/bool.h>
#include <basic/Link.h>
#include <stdio.h>  // for NULL

//#include <basic/enter_Map.h>


namespace omega {

#define foreach_map(k,K,v,V,M,A) {for (MapElementIterator<K,V> __M_##k = (M).iterator();__M_##k;__M_##k++) {K & k = *__M_##k; V & v = __M_##k.value(); A;}}

template <class K, class V> class MapElement {
public:
	K k;
	V v;
	MapElement<K,V> *tail;
	MapElement(const MapElement<K,V>&);
	MapElement() {}
	MapElement & operator=(const MapElement<K,V>&);
	~MapElement() {	delete tail; }
};

template<class K, class V> class MapElementIterator {
public:
    MapElementIterator(MapElement<K,V>* j) { i = j;}
    virtual ~MapElementIterator() {}
    virtual const K &  operator*() const   { return i->k; }
    virtual       K &  operator*()         { return i->k;}
    virtual const V &  value() const       { return i->v; }
    virtual	  V &  value()             { return i->v; }
    virtual    void    operator++(int)        { i = i->tail; }
    virtual    void    operator++()           { i = i->tail; }
    virtual    bool    live() const        { return i != NULL; }
	               operator bool() const { return live(); }
protected:
MapElement<K,V> *i;
};

template <class K, class V> class Map {
public:
#if ! defined linux
	Map(const V &default_value);
#else
    //  work around for '386 g++ on Linux
	Map(V default_value);
#endif
	~Map();
	MapElementIterator<K,V> iterator() 
		{return MapElementIterator<K,V>(contents);}
	int empty() const {return contents == NULL;}
	V  operator()(K) const;
	V& operator[](K);
private:
	MapElement<K,V> *  contents;
	V _default_value;
};

} // end of namespace omega

#if ! defined DONT_INCLUDE_TEMPLATE_CODE
#include <basic/Map.c>
#endif

namespace omega {
#define instantiate_Map(T1,T2)	template class Map<T1,T2>; \
				template class MapElement<T1,T2>; \
				template class MapElementIterator<T1,T2>;
#define instantiate_MapElement(T1,T2)		instantiate_Map(T1,T2)
#define instantiate_MapElementIterator(T1,T2)	instantiate_Map(T1,T2)
}


#endif
