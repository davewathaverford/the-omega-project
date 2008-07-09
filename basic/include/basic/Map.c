
namespace omega {


template<class K, class V> MapElement<K,V>:: MapElement(const MapElement<K,V>& M) {
		if (M.tail) tail = new MapElement<K,V>(*M.tail);
		else tail = 0;
		k = M.k;
		v = M.v;
		}

template<class K, class V> MapElement<K,V> & 
	MapElement<K,V>:: operator=(const MapElement<K,V>& M) {
                if (this != &M) {
		  if (tail) delete tail;
                  if (M.tail) tail = new MapElement<K,V>(*M.tail);
		  else tail = 0;
		k = M.k;
		v = M.v;
                }
	return *this;
	}




#if ! defined linux
template <class K, class V> Map <K,V>::Map(const V &default_value)
#else
template <class K, class V> Map <K,V>::Map(V default_value)
#endif
                : _default_value(default_value)
                {
		    contents = 0;
		}

template <class K, class V> Map <K,V>::~Map()
    {
    delete contents;
    }

template <class K, class V> V Map<K,V>::operator()(K k) const {
		MapElement <K,V> * P = contents;
		while (P) {
			if (P->k == k) return P->v;
			P = P->tail;
			};
		return _default_value;
		}

template <class K, class V> V & Map<K,V>::operator[](K k) {
		MapElement <K,V> * P = contents;
		while (P) {
			if (P->k == k) return P->v;
			P = P->tail;
			};
		P = new MapElement <K,V>;
		P->k = k;
		P->v = _default_value;
		P->tail = contents;
		contents = P;
		return P->v;
		}


} // end of namespace omega

