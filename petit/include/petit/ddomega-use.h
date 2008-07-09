/* $Id: ddomega-use.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_DDOmega_Use
#define Already_Included_DDOmega_Use

#if 0

/* compute DD vectors, add them to nodes */

void calculateDDVectors(Problem *problemPtr, a_access access1,a_access access2,
			ddnature oitype, ddnature iotype,
			unsigned int nest1, unsigned int nest2, unsigned int bnest, unsigned int nonloops,
			signed char *loop_dirs, dd_flags incomplete_flags);

void generateAllDDVectors(a_access access1,a_access access2,
			  ddnature oitype, ddnature iotype,
			  unsigned int nest1, unsigned int nest2, unsigned int bnest,
			  signed char *loop_dirs, dd_flags incomplete_flags);

#endif

#endif
