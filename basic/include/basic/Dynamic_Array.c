#include <basic/assert.h>
#include <basic/Dynamic_Array.h>


template<class T, int d> void omega::Dynamic_Array<T,d>::do_constr()
    {
#if ! defined SHUT_UP_ABOUT_STATEMENT_WITH_NO_EFFECT_IN_DYNAMIC_ARRAY_CREATION
    assert(d > 0);
#endif
    this->bounds = NULL;
    this->elements = NULL;
    this->partial = false;
    }


template<class T> void omega::Dynamic_Array1<T>::do_construct(int d0)
    {
    this->bounds = new int[1];
    this->bounds[0] = d0;
    this->elements = new T [d0];
    this->partial = false;
    }

template<class T> void omega::Dynamic_Array2<T>::do_construct(int d0, int d1)
    {
    this->bounds = new int[2];
    this->bounds[0] = d0;
    this->bounds[1] = d1;
    this->elements = new T [d0 * d1];
    this->partial = false;
    }

template<class T> void omega::Dynamic_Array3<T>::do_construct(int d0,int d1,int d2)
    {
    this->bounds = new int[3];
    this->bounds[0] = d0;
    this->bounds[1] = d1;
    this->bounds[2] = d2;
    this->elements = new T [d0 * d1 * d2];
    this->partial = false;
    }

template<class T> void omega::Dynamic_Array4<T>::do_construct(int d0,int d1,int d2,int d3)
    {
    this->bounds = new int[4];
    this->bounds[0] = d0;
    this->bounds[1] = d1;
    this->bounds[2] = d2;
    this->bounds[3] = d3;
    this->elements = new T [d0 * d1 * d2 * d3];
    this->partial = false;
    }

template<class T, int d> omega::Dynamic_Array<T,d>::Dynamic_Array()
    {
    do_constr();
    }

template<class T> omega::Dynamic_Array1<T>::Dynamic_Array1(const char *)
    {
    this->do_constr();
    }

template<class T> omega::Dynamic_Array2<T>::Dynamic_Array2(const char *, const char *)
    {
    this->do_constr();
    }

template<class T> omega::Dynamic_Array3<T>::Dynamic_Array3(char *,char *,char *)
    {
    this->do_constr();
    }

template<class T> omega::Dynamic_Array4<T>::Dynamic_Array4(char *,char *,char *,char *)
    {
    this->do_constr();
    }

template<class T> omega::Dynamic_Array1<T>::Dynamic_Array1(int d0)
    {
    this->do_construct(d0);
    } 

template<class T> omega::Dynamic_Array2<T>::Dynamic_Array2(int d0, int d1)
    {
    this->do_construct(d0, d1);
    }

template<class T> omega::Dynamic_Array3<T>::Dynamic_Array3(int d0,int d1,int d2)
    {
    this->do_construct(d0, d1, d2);
    }

template<class T> omega::Dynamic_Array4<T>::Dynamic_Array4(int d0,int d1,int d2,int d3)
    {
    this->do_construct(d0, d1, d2, d3);
    }


template<class T, int d> void omega::Dynamic_Array<T,d>::do_destruct()
    {
    if (! partial)
	{
        delete [] bounds;
        delete [] elements;
	}
    }


template<class T, int d> omega::Dynamic_Array<T,d>::~Dynamic_Array()
    {
    do_destruct();
    }


template<class T> void omega::Dynamic_Array1<T>::resize(int d0)
    {
    assert(!this->partial);
    this->do_destruct();
    if (d0 == 0)
        this->do_constr();
    else
        this->do_construct(d0);
    } 

template<class T> void omega::Dynamic_Array2<T>::resize(int d0, int d1)
    {
    assert(!this->partial);
    this->do_destruct();
    if (d0 == 0 && d1 == 0)
        this->do_constr();
    else
        this->do_construct(d0, d1);
    }

template<class T> void omega::Dynamic_Array3<T>::resize(int d0, int d1, int d2)
    {
    assert(!this->partial);
    this->do_destruct();
    if (d0 == 0 && d1 == 0 && d2 == 0)
        this->do_constr();
    else
        this->do_construct(d0, d1, d2);
    }

template<class T> void omega::Dynamic_Array4<T>::resize(int d0, int d1, int d2, int d3)
    {
    assert(!this->partial);
    this->do_destruct();
    if (d0 == 0 && d1 == 0 && d2 == 0 && d3 == 0)
        this->do_constr();
    else
        this->do_construct(d0, d1, d2, d3);
    }


template<class T> T& omega::Dynamic_Array1<T>::operator[](int d0)
    { 
#if !defined (NDEBUG)
    assert(this->elements != NULL && "Trying to dereference undefined array");
    assert(0 <= d0 && d0 < this->bounds[0] && "Array subscript out of bounds");
#endif

    return this->elements[d0];
    }

template<class T>  omega::Dynamic_Array1<T> omega::Dynamic_Array2<T>::operator[](int d0)
    { 
#if !defined (NDEBUG)
    assert(this->elements != NULL && "Trying to dereference undefined array");
    assert(0 <= d0 && d0 < this->bounds[0] && "Array subscript out of bounds");
#endif

    Dynamic_Array1<T> result;
    result.bounds = this->bounds+1;
    result.elements = this->elements + this->bounds[1] * d0;
    result.partial = true;
    return result;
    }

template<class T>  omega::Dynamic_Array2<T> omega::Dynamic_Array3<T>::operator[](int d0)
    { 
#if !defined (NDEBUG)
    assert(this->elements != NULL && "Trying to dereference undefined array");
    assert(0 <= d0 && d0 < this->bounds[0] && "Array subscript out of bounds");
#endif
    Dynamic_Array2<T> result;
    result.bounds = this->bounds+1;
    result.elements = this->elements + this->bounds[1] * this->bounds[2] * d0;
    result.partial = true;
    return result;
    } 

template<class T>  omega::Dynamic_Array3<T> omega::Dynamic_Array4<T>::operator[](int d0)
    { 
#if !defined (NDEBUG)
    assert(this->elements != NULL && "Trying to dereference undefined array");
    assert(0 <= d0 && d0 < this->bounds[0] && "Array subscript out of bounds");
#endif

    Dynamic_Array3<T> result;
    result.bounds = this->bounds+1;
    result.elements = this->elements + this->bounds[1] * this->bounds[2] 
                      * this->bounds[3] * d0;
    result.partial = true;
    return result;
    } 


template<class T, int d> 
    omega::Dynamic_Array<T,d>::Dynamic_Array(Dynamic_Array<T,d> &D)
    {
    assert(D.elements != NULL && "Trying to copy an undefined array");
    partial = true;
    bounds = D.bounds;
    elements = D.elements;
    }

