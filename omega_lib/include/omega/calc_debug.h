// Debugging flags.  Can set any of these.

#if !defined(Already_included_calc_debug)
#define Already_included_calc_debug

#include <omega/omega_core/debugging.h>
#include <ctype.h>

namespace omega {

extern int omega_calc_debug;
extern int code_gen_debug;


inline void calc_all_debugging_off() {
all_debugging_off();
omega_calc_debug = 0;
}

inline int process_calc_debugging_flags(char *arg,int &j) {
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
		      closure_presburger_debug = 
			farkas_debug = 
		      pres_debug = omega_calc_debug = code_gen_debug = level;
		    break;
		case 'g':
		    code_gen_debug = level; break;
		case 'f':
		    farkas_debug = level; break;
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
		case 'l':
		    omega_calc_debug = level; break;
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
