/* $Id: timer.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Timer
#define Already_Included_Timer

namespace omega {

void start_clock( void );
long clock_diff( void );  /* return user time since start_clock */

}

#endif

