/* $Id: */

#if ! defined _Const_String_h
#define _Const_String_h 1

#include <basic/String.h>

namespace omega {



// should be inside Const_String, but I can't get it to
// compile the hashTable when it is: hashTable can't be
// global, but if it and its size are static to Const_String,
// the compiler still doesn't seem to like the definition,
// or the declaration either for that matter.

class ConstStringRep {
public:
	const char * name;
	int count;
	ConstStringRep *nextInBucket;
	ConstStringRep(const char *t);
};

class Const_String {
private:
	ConstStringRep *rep;
	void buildRep(const char *t);

public:
	Const_String();
	Const_String(const String &s);
	Const_String(const char* t);
	Const_String(const Const_String & t) {rep = t.rep;}

//	~Const_String();
//	void	operator =  (const Const_String & y);
//	void	operator =  (const char* y);

	    	operator int() const;
	int	null() const;

	    	operator const char*() const;
	    	operator String() const;
	int	operator++(int);
	int	operator++();
	int	operator--(int);
	int	operator--();
friend	int	operator==(const Const_String &x, const Const_String &y);
friend	int	operator!=(const Const_String &x, const Const_String &y);
friend	int	operator<(const Const_String &x, const Const_String &y);
friend  int     operator >(const Const_String &x, const Const_String &y);

	};

#if defined SCREWED_UP_CASTING_RULES
static int	operator==(const Const_String &x, const char *y)
		    { return x == (Const_String) y; }
static int	operator!=(const Const_String &x, const char *y)
		    { return x != (Const_String) y; }
#endif

} // end of namespace omega


#endif
