#if !defined _Omega_String_h

#define _Omega_String_h 1

#include <basic/bool.h>
#include <basic/util.h>

#include <stdio.h>
//#include <cstdio>

#include <string.h>
//#include <string>

//#include <iostream.h>
#include <iostream>
using namespace std;


//#include <basic/enter_String.h>

namespace omega {


class String {
public:

  inline String()
    { s=new char[1];  len=0; s[0] = 0; }
  inline String(const String& x)
    { len=x.len; s=new char[len+1]; strcpy(s, x.s); } 
  inline String(const char c)
    { len=1; s=new char[2]; s[0] = c; s[1] = '\0'; } 
  inline String(const char* t) 
    { len=strlen(t); s=new char[len+1]; strcpy(s, t); }

  inline ~String()
    { delete s; }

  void operator =  (const String& y)
    { delete s; len=y.len; s=new char[len+1]; strcpy(s,y.s); }
  void operator =  (const char* y)
    { delete s; len=strlen(y); s=new char[len+1]; strcpy(s,y); }

  inline const char &operator [] (int x) const
    { return s[x];
    }
  inline       char &operator [] (int x)
    { return s[x];
    }
  void operator += (const String& y)
    { char *t=new char[len+y.len+1]; strcpy(t,s); strcpy(t+len,y.s); 
      delete s; s=t; len+=y.len;
    }
  void operator += (const char* y)
    { int ylen=strlen(y); 
      char *t=new char[len+ylen+1]; strcpy(t,s); strcpy(t+len,y); 
      delete s; s=t; len+=ylen;
    }
  inline friend String operator +(const String& x, const String& y)
    { String S;
      delete S.s;
      S.s=new char[x.len+y.len+1]; strcpy(S.s,x.s); strcpy(S.s+x.len,y.s); 
      S.len=x.len+y.len;
      return S;
    }
    
  inline friend ostream &operator<<(ostream &o, const String &S)
    { return o << S.s; }

  inline operator const char *() const 
    { return s; }

  inline int  length()  const
    { return strlen(s); }

friend	inline bool	operator==(const String &x, const String &y);
friend	inline bool	operator!=(const String &x, const String &y);
friend	inline bool	operator<(const String &x, const String &y);
friend	inline bool	operator>(const String &x, const String &y);
private:
  char *s;
  int   len;
};

inline	bool	operator==(const String &x, const String &y)
{    return (strcmp(x.s,y.s) == 0); }
inline	bool	operator!=(const String &x, const String &y)
{    return (strcmp(x.s,y.s) != 0); }
inline	bool	operator<(const String &x, const String &y)
{    return (strcmp(x.s,y.s) < 0); }
inline	bool	operator>(const String &x, const String &y)
{    return (strcmp(x.s,y.s) > 0); }

// Defines the itoS function for converting ints to strings.
inline String itoS(int i) 
  { char t[32]; sprintf(t,"%d",i); return t;}
inline String itoS(LONGLONG i) 
  { char t[65]; sprintf(t,"%lld",i); return t;}

#if defined SCREWED_UP_CASTING_RULES
static int	operator==(const String &x, const char *y)
		    { return x == (String) y; }
static int	operator!=(const String &x, const char *y)
		    { return x != (String) y; }
#endif

} // end of namespace omega

#endif
