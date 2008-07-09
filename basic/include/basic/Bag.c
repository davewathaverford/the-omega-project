/****************************************************************
 *								*
 *  Collection constructors, desctructors, assignments		*
 *								*
 ****************************************************************/

#include <basic/bool.h>


namespace omega {


template<class T> Bag<T>::Bag() {
	 this->contents = new List_Element <T>;
	 this->contents->tail = 0;
	}
template<class T> Bag<T>::~Bag() {
	 delete this->contents;
	}

template<class T> Ordered_Bag<T>::Ordered_Bag() {}

template<class T> Set<T>::Set() {}

template<class T> Bag<T>::Bag(const Bag<T> &L) {
		this->contents = new List_Element<T>(*L.contents);
		}

template<class T> Bag<T> & Bag<T>::operator=(const Bag<T> &L) {
                if (this != &L) {
		  delete this->contents;
                  this->contents = new List_Element<T>(*L.contents);
                }
                return *this;
                }



template<class T> Set<T>::Set(T e) {
	 assert(this->contents);
	 this->contents->tail = new List_Element<T>(e, 0);
	}
	

/****************************************************************
 *								*
 *  Misc. simple Collection operations 				*
 *								*
 ****************************************************************/

template<class T> bool Bag<T>::empty() const {
	 return this->contents->tail == 0;
	}

template<class T> Iterator<T> *Bag<T>::new_iterator()
		{
		return new List_Element_Iterator<T>(this->contents->tail);
		}


template<class T> void Bag<T>::clear() {
		if (this->contents->tail) delete this->contents->tail;
		this->contents->tail = 0;
		}

template<class T> int Bag<T>::size() const {
		int i = 0;
		List_Element<T> * p = this->contents->tail;
		while (p) {
			p = p->tail;
			i++;
			};
		return i;
		}


/****************************************************************
 *								*
 *  Collection/Element operations (e.g. insert, contains)	*
 *								*
 ****************************************************************/

template<class T> void Bag<T>::remove(T e) {
		List_Element<T> * p = this->contents;
		while (p->tail && p->tail->head != e) p = p->tail;
		if (p->tail && p->tail->head == e) {
			List_Element<T> * q = p->tail;
			p->tail = q->tail;
			q->tail = 0;
			delete q;
			}
		}

template<class T> T Bag<T>::extract() {
		List_Element<T> * p = this->contents->tail;
		T e = p->head;
		this->contents->tail = p->tail;
		p->tail = 0;
		delete p;
		return e;
		}


template<class T> void Bag<T>::insert(T e) {
		List_Element<T> * q = new List_Element<T>(e,this->contents->tail);
		this->contents->tail = q;
		}

template<class T> void Ordered_Bag<T>::insert(T e) {
		List_Element<T> * p = this->contents;
		while (p->tail && p->tail->head < e) p = p->tail;
		if (!p->tail || p->tail->head != e) {
			List_Element<T> * q = new List_Element<T>(e,p->tail);
			p->tail = q;
			}
		}


template<class T> bool Bag<T>::contains(T e) const {
		List_Element<T> * p = this->contents;
		while (p->tail && p->tail->head != e) p = p->tail;
		return (p->tail && p->tail->head == e);
		}

template<class T> bool Ordered_Bag<T>::contains(T e) const {
		List_Element<T> * p = this->contents;
		while (p->tail && p->tail->head < e) p = p->tail;
		return (p->tail && p->tail->head == e);
		}


template<class T> bool Set<T>::contains (const Set<T>& b) const {
		List_Element<T> * p = this->contents;
		List_Element<T> * q = b.contents;
		do {
		  /* consume matched elements in p and q */
		  p = p->tail;
		  q = q->tail;
		  if (!q) return 1; /* no more elements to match */
		  if (!p) return 0; /* nothing left in p to match with */
 		  if (q->head < p->head) {
				/* nothing smaller than
				p->head left in p, so q->head
				can't be matched */
				return 0;
				};
		  while (p && p->head < q->head) {
			/* toss away some elements from p */
			p = p->tail;
			}
		  if (!p || q->head < p->head) return 0;
		  } while (q);
		    
		  return 1;
		}
		


/****************************************************************
 *								*
 *  Collection/Collection operations (e.g. |=)			*
 *								*
 ****************************************************************/

template<class T> void Bag<T>::operator |= (const Bag<T> & b) {
		assert(this != &b);
		List_Element<T> * q = b.contents->tail;

		while (q) {
		  List_Element<T> * r = new List_Element<T>(q->head,this->contents->tail);
		  this->contents->tail = r;
		  q = q->tail;
		  }
		}

template<class T> void Ordered_Bag<T>::operator |= (const Ordered_Bag<T> & b) {
		if (this == &b) return;
		List_Element<T> * p = this->contents;
		List_Element<T> * q = b.contents->tail;

		while (q) {
		  while (p->tail && p->tail->head < q->head) p = p->tail;
		  List_Element<T> * r = new List_Element<T>(q->head,p->tail);
		  p->tail = r;
		  q = q->tail;
		  }
		}

template<class T> void Ordered_Bag<T>::operator |= (const Bag<T> & b) {
		Ordered_Bag<T> tmp;
		for (List_Element<T> *p = b.contents; p; p=p->tail) {
		  tmp.insert(p->head);
		}
		*this |= tmp;
}

template<class T> void Set<T>::operator |= (const Set<T> & b) {
		if (this == &b) return;
		List_Element<T> * p = this->contents;
		List_Element<T> * q = b.contents->tail;

		while (q) {
		  while (p->tail && p->tail->head < q->head) p = p->tail;
		  if (!p->tail || p->tail->head != q->head) {
			List_Element<T> * r = new List_Element<T>(q->head,p->tail);
			p->tail = r;
			}
		  q = q->tail;
		  }
		}

template<class T> void Set<T>::operator |= (const Ordered_Bag<T> & b) {
		Set<T> tmp;
		for (List_Element<T> *p = b.contents; p; p=p->tail) {
		  tmp.insert(p->head);
		}
		*this |= tmp;
}

template<class T> void Set<T>::operator |= (const Bag<T> & b) {
		Set<T> tmp;
		for (List_Element<T> *p = b.contents; p; p=p->tail) {
		  tmp.insert(p->head);
		}
		*this |= tmp;
}



// delete items also in b
template<class T> void Set<T>::operator -= (const Set<T> & b) {
		if (this == &b) {
			this->clear();
			return;
			}
		List_Element<T> * p = this->contents;
		List_Element<T> * q = b.contents->tail;

		while (q) {
		  while (p->tail && p->tail->head < q->head) p = p->tail;
		  if (p->tail && p->tail->head == q->head) {
			List_Element<T> * r = p->tail;
			p->tail = r->tail;
			r->tail = 0;
			delete r;
			}
		  q = q->tail;
		  }
		}


// delete items not in b
template<class T> void Set<T>::operator &= (const Set<T> & b)
       {
		if (this == &b) return;
                List_Element<T> * p = this->contents;
                List_Element<T> * q = b.contents->tail;

                while (q) {
                  while (p->tail && p->tail->head < q->head) {
                        List_Element<T> * r = p->tail;
                        p->tail = r->tail;
                        r->tail = 0;
                        delete r;
                        };
                  if (p->tail && p->tail->head == q->head) {
                        /* allow p->tail->head into the result */
                        p  = p->tail;
                        }
                  /* q->head has matched anything it is going to match */
                  q = q->tail;
                  }
		if (p->tail) {
			delete p->tail;
			p->tail = 0;
			};

        }


template<class T> bool Set<T>::operator & (const Set<T>& b) const {
		List_Element<T> * p = this->contents;
		List_Element<T> * q = b.contents;
		do {
		  p = p->tail;
		  q = q->tail;
		  while (p && q && p->head != q->head) {
		     while (p && p->head < q->head) p = p->tail;
		     while (p && q && q->head < p->head) q = q->tail;
		     };
		  if (p && q && p->head == q->head) return 1;
		  } while (p && q);
		    
		  return 0;
		}


template<class T> bool Ordered_Bag<T>::operator == (const Ordered_Bag<T>& b) const {
		List_Element<T> * p = this->contents;
		List_Element<T> * q = b.contents;
		while (1) {
		  p = p->tail;
		  q = q->tail;
		  if (!p && !q) return 1;
		  if (!p || !q) return 0;
		  if (p->head != q->head) return 0;
		  };
		    
		}

template<class T> bool Ordered_Bag<T>::operator != (const Ordered_Bag<T>& b) const {
              return !(*this == b);
              }

template<class T> bool Ordered_Bag<T>::operator < (const Ordered_Bag<T>& b) const {
		List_Element<T> * p = this->contents;
		List_Element<T> * q = b.contents;
		while (1) {
		  p = p->tail;
		  q = q->tail;
		  if (!p && !q) return 0;
		  if (!p) return 1;
		  if (!q) return 0;
		  if (p->head < q->head) return 1;
		  if (q->head < p->head) return 0;
		  };
		    
		  return 1;
		  }


} // end of namespace omega

