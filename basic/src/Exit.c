#include <stdlib.h>
#include <basic/Exit.h>

List<exit_func> Exit_hooks;

void Exit(int e) 
    {
    foreach(func, exit_func, Exit_hooks, (*func)(e));

    exit(e);
    }
