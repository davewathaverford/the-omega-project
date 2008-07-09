/* restart.h,v 1.1.1.2 1992/07/10 02:41:06 davew Exp */
#ifndef Already_Included_Restart
#define Already_Included_Restart

namespace omega {

/* keep track of an array of restartable subprocesses */

extern int Restart_Save( unsigned int pid, char *cmd, char *arg );

extern int Restart_Handle( char *cmd, char *arg );

extern int Restart_Menu();

}

#endif
