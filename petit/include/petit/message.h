/* message.h,v 1.1.1.2 1992/07/10 02:40:53 davew Exp */
#ifndef Already_Included_Message
#define Already_Included_Message

namespace omega {

#if ! defined BATCH_ONLY_PETIT

extern int In_Uniform;

extern void Message_Startup( void );

extern void Message_Add( char* msg );

extern int Message_Menu( int dummy );

void *build_Message_Menu( void );

#else

#define Message_Startup()

#define Message_Add(X)	printf("%s", X)

#endif

}

#endif
