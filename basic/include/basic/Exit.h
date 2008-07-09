#if ! defined _Exit_h
#define _Exit_h

#include <basic/List.h>


namespace omega {

typedef void (*exit_func)(int);
extern List<exit_func> Exit_hooks;

// Exit calls all functions on the "Exit_hooks" list in order,
// then calls "exit"
extern void Exit(int);


} // end of namespace omega

#endif
