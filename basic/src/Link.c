#include <basic/Link.h>

#if ListElementFreeList

namespace omega {


  static List_Element<void*> *_kludgy_List_Element_free_list_pointer;
// we rely on the fact that that is initialized to 0 before any
// constructor-based initialization that could call List_Element::new.



  void *kludgy_List_Element_new(size_t size)
    {
    void *mem;
    if (size == sizeof(List_Element<void*>) &&
	_kludgy_List_Element_free_list_pointer)
	{
	List_Element<void*> *it = _kludgy_List_Element_free_list_pointer;
	_kludgy_List_Element_free_list_pointer = it->tail;
	mem = it;
	}
    else
	mem = ::operator new(size);

    return mem;
    }

  void  kludgy_List_Element_delete(void *ptr, size_t size)
    {
    if (ptr)
	if (size == sizeof(List_Element<void*>))
	    {
	    List_Element<void*> *it = (List_Element<void*> *) ptr;
	    it->tail = _kludgy_List_Element_free_list_pointer;
	    _kludgy_List_Element_free_list_pointer = it;
	    }
	else
	    ::operator delete(ptr);
    }

} // end of namespace omega


#endif



