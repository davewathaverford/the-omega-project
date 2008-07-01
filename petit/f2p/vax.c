/**********************************************
 *					      *
 * Hacked to convert FORTRAN-77 to petit by    *
 * Vadim Maslov,  vadik@cs.umd.edu, 09/20/92. *
 *					      *
 **********************************************/

/****************************************************************
Copyright 1990 by AT&T Bell Laboratories and Bellcore.

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the names of AT&T Bell Laboratories or
Bellcore or any of their entities not be used in advertising or
publicity pertaining to distribution of the software without
specific, written prior permission.

AT&T and Bellcore disclaim all warranties with regard to this
software, including all implied warranties of merchantability
and fitness.  In no event shall AT&T or Bellcore be liable for
any special, indirect or consequential damages or any damages
whatsoever resulting from loss of use, data or profits, whether
in an action of contract, negligence or other tortious action,
arising out of or in connection with the use or performance of
this software.
****************************************************************/

#include "defs.h"
#include "pccdefs.h"
#include "output.h"

int regnum[] =  {
	11, 10, 9, 8, 7, 6 };

/* Put out a constant integer */

prconi(fp, n)
FILEP fp;
ftnint n;
{
	fprintf(fp, "\t%ld\n", n);
}



/* Put out a constant address */

prcona(fp, a)
FILEP fp;
ftnint a;
{
	fprintf(fp, "\tL%ld\n", a);
}



prconr(fp, x, k)
 FILEP fp;
 int k;
 Constp x;
{
	char *x0, *x1;
	char cdsbuf0[64], cdsbuf1[64];

	if (k > 1) {
		if (x->vstg) {
			x0 = x->Const.cds[0];
			x1 = x->Const.cds[1];
			}
		else {
			x0 = cds(dtos(x->Const.cd[0]), cdsbuf0);
			x1 = cds(dtos(x->Const.cd[1]), cdsbuf1);
			}
		fprintf(fp, "\t%s %s\n", x0, x1);
		}
	else
		fprintf(fp, "\t%s\n", x->vstg ? x->Const.cds[0]
				: cds(dtos(x->Const.cd[0]), cdsbuf0));
}


char *memname(stg, mem)
 int stg;
 long mem;
{
	static char s[20];

	switch(stg)
	{
	case STGCOMMON:
	case STGEXT:
		sprintf(s, "_%s", extsymtab[mem].cextname);
		break;

	case STGBSS:
	case STGINIT:
		sprintf(s, "v.%ld", mem);
		break;

	case STGCONST:
		sprintf(s, "L%ld", mem);
		break;

	case STGEQUIV:
		sprintf(s, "q.%ld", mem+eqvstart);
		break;

	default:
		badstg("memname", stg);
	}
	return(s);
}

/* make_int_expr -- takes an arbitrary expression, and replaces all
   occurrences of arguments with indirection */

expptr make_int_expr (e)
expptr e;
{
    if (e != ENULL)
	switch (e -> tag) {
	    case TADDR:
	        if (e -> addrblock.vstg == STGARG)
		    e = mkexpr (OPWHATSIN, e, ENULL);
	        break;
	    case TEXPR:
	        e -> exprblock.leftp = make_int_expr (e -> exprblock.leftp);
	        e -> exprblock.rightp = make_int_expr (e -> exprblock.rightp);
	        break;
	    default:
	        break;
	} /* switch */

    return e;
} /* make_int_expr */

