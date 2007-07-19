/* $Id: */
#ifndef Already_Included_Dynamic_Array
#define Already_Included_Dynamic_Array

namespace omega {

template <class T> class Dynamic_Array2;
template <class T> class Dynamic_Array3;
template <class T> class Dynamic_Array4;

template <class T, int d> class Dynamic_Array
    {
    public:
	Dynamic_Array(Dynamic_Array<T,d> &D);
        ~Dynamic_Array();

    protected:
	Dynamic_Array();
	bool partial;
	int *bounds;
	T *elements;

	void do_constr();
	void do_destruct();
    };


template <class T> class Dynamic_Array1 : public Dynamic_Array<T,1>
    {
    public:
	Dynamic_Array1(const char *s0 = 0);
	Dynamic_Array1(int d0);
	void resize(int d0);
        T& operator[](int d);

	friend class Dynamic_Array2<T>;

    private:
	void do_construct(int d0);
    };


template <class T> class Dynamic_Array2 : public Dynamic_Array<T,2>
    {
    public:
	Dynamic_Array2(const char *s0 = 0, const char *s1 = 0);
	Dynamic_Array2(int d0, int d1);
	void resize(int d0, int d1);
  	Dynamic_Array1<T> operator[](int d);

	friend class Dynamic_Array3<T>;

    private:
	void do_construct(int d0, int d1);
    };


template <class T> class Dynamic_Array3 : public Dynamic_Array<T,3>
    {
    public:
	Dynamic_Array3(char *s0 = 0, char *s1 = 0, char *s2 = 0);
	Dynamic_Array3(int d0, int d1, int d2);
	void resize(int d0, int d1, int d2);
  	Dynamic_Array2<T> operator[](int d);

	friend class Dynamic_Array4<T>;

    private:
	void do_construct(int d0, int d1, int d2);
    };

template <class T> class Dynamic_Array4 : public Dynamic_Array<T,4>
    {
    public:
	Dynamic_Array4(char *s0 = 0, char *s1 = 0, char *s2 = 0, char *s3 = 0);
	Dynamic_Array4(int d0, int d1, int d2, int d3);
	void resize(int d0, int d1, int d2, int d3);
  	Dynamic_Array3<T> operator[](int d);

    private:
	void do_construct(int d0, int d1, int d2, int d3);
    };

} // end of namespace omega

#if ! defined DONT_INCLUDE_TEMPLATE_CODE
#include <basic/Dynamic_Array.c>
#endif

namespace omega {

#define instantiate_Dynamic_Array1(T)	template class Dynamic_Array1<T>; \
					template class Dynamic_Array<T,1>;

#define instantiate_Dynamic_Array2(T)	template class Dynamic_Array2<T>;  \
					template class Dynamic_Array<T,2>; \
					instantiate_Dynamic_Array1(T);

#define instantiate_Dynamic_Array3(T)	template class Dynamic_Array3<T>;  \
					template class Dynamic_Array<T,3>; \
					instantiate_Dynamic_Array2(T);

#define instantiate_Dynamic_Array4(T)	template class Dynamic_Array4<T>;  \
					template class Dynamic_Array<T,4>; \
					instantiate_Dynamic_Array3(T);
}

#endif
