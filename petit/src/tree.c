/* tree.c,v 1.1.1.2 1992/07/10 02:44:06 davew Exp */

#include <petit/tree.h>
#include <petit/pres-interf.h>

namespace omega {

struct OPinforec OPinfo[] = {
#include "op.val"
    };

struct EXPRinforec EXPRinfo[] = {
#include "exprtype.val"
    };

struct SYMinforec SYMinfo[] = {
#include "symtype.val"
    };

struct DDinforec DDinfo[] = {
#include "ddtype.val"
    };

char *DDdirname[] = {
	"...",
	"<",
	"=",
	"<=",
	">",
	"<>",
	">=",
	"*",
	"R",
	"<R",
	"=R",
	"<=R",
	">R",
	"<>R",
	">=R",
	"*R" };

symtabentry::symtabentry() :
        symtype((symboltype)0), symclass((classes)0), symdims(0), symdecl(0),
        symname(0),
        symvalue(0), exptype((exprtype)0), sym_con_var(false),  symoffset(0),
        def_node(0),
        nte(0), original(0), symcopy(0), symtag(0), omega_vars(0), alignment(0)
    {};


symtabentry::~symtabentry()
    {
//    if(omega_vars)
//	for (List_Iterator<Omega_Var *> li(*omega_vars); li; li++)
//	    delete *li;
    delete omega_vars;
    }

} // end omega namespace
