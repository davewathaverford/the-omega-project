/* petity.h,v 1.1.1.2 1992/07/10 02:41:36 davew Exp */
#if ! defined Already_Included_Petity
#define Already_Included_Petity

#define MAXIDLENGTH 2048

#include <petit/tree.h>

// can't put these in namespace omega because parser generator won't
extern int yyparse (void);
extern FILE *yyin;

namespace omega {

extern char P_IDname[MAXIDLENGTH];
extern char P_INTname[MAXIDLENGTH];


extern nametabentry *parse_findsym();
extern symtabentry *parse_symbol(int new_int);
extern node *parse_symbolnode();
extern nametabentry *parse_findsym();

extern symtabentry *add_symbol(const char *name,
                               int new_int,
                               symboltype symtype,
                               exprtype exptype,
                               nametabentry *onam);

}

#endif
