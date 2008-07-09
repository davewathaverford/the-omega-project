// Debugging flags.  Can set any of these.

#if !defined(Already_included_debugging)
#define Already_included_debugging

#include <stdio.h>
#include <ctype.h>


namespace omega {

extern FILE *DebugFile;

extern int omega_core_debug;
extern int pres_debug;
extern int relation_debug;
extern int closure_presburger_debug;
extern int hull_debug;
extern int farkas_debug;
extern int code_gen_debug;

enum negation_control { any_negation, one_geq_or_eq, one_geq_or_stride };
extern negation_control pres_legal_negations;

#if defined STUDY_EVACUATIONS
extern int evac_debug;
#endif

inline void all_debugging_off() {
omega_core_debug = 0;
pres_debug = 0;
relation_debug = 0;
closure_presburger_debug = 0;
#if defined STUDY_EVACUATIONS
evac_debug = 0;
#endif
}


inline int process_pres_debugging_flags(char *arg,int &j) {
    char debug_type;
	    while((debug_type=arg[j]) != 0) {
		j++;
		int level;
		if(isdigit(arg[j]))
		    level = (arg[j++]) - '0';
		else
		    if(arg[j] == 0 || isalpha(arg[j]))
			level = 1;
		    else 
			return(0);
		if (level < 0 || level > 4) {
		    fprintf(stderr,"Debug level %c out of range: %d\n",
			    debug_type, level);
		    return(0);
		}
		switch(debug_type) {
		case 'a':
		    omega_core_debug = relation_debug = hull_debug = 
		      pres_debug = closure_presburger_debug =
		      code_gen_debug = level;
		    break;
		case 'g':
		    code_gen_debug = level; break;
		case 'h':
		    hull_debug = level; break;
		case 'c':
		    omega_core_debug = level; break;
		case 'r':
		    relation_debug = level; break;
		case 'p':
		    pres_debug = level; break;
		case 't':
		    closure_presburger_debug = level; break;
#if defined STUDY_EVACUATIONS
		case 'e':
		    evac_debug = level; break;
#endif
		default:
		    return(0);
		}
	    }
    return(1);
}


} // end of namespace omega

#endif
