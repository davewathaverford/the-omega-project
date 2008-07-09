/* ddutil.c,v 1.1.1.2 1992/07/10 02:42:49 davew Exp */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <basic/bool.h>
#include <petit/Exit.h>
#include <petit/tree.h>
#include <petit/ddutil.h>
#include <petit/ops.h>
#include <petit/petit_args.h>

namespace omega {

bool DDdebug;

/* dump routine */


void dump_eq_part( int coef, unsigned int index, bool *any, char ch )
{
int c;

    if( coef != 0 || (index == 0 && *any == 0) ){
	c = coef;
	if( coef > 0 && *any ){
	    printf("+");
	}else if( coef < 0 ){
	    printf("-");
	    c = -c;
	}
	if( c > 1 || index == 0 ){
	    printf("%d", c );
	}
	if( index != 0 ){
	    if( c > 1 ){
		printf("*");
	    }
	    printf("%c%d", ch, index);
	}
	*any = 1;
    }
}/* dump_eq_part */

void dumpeq( unsigned int nest, Nvector ocoef, Nvector icoef )
{
bool a;
unsigned int n;

    /* dump equation */
    printf(" Dependence equation is " );
    a=0;
    for( n=1; n<=nest; ++n ){
	dump_eq_part( ocoef[n], n, &a, 'i' );
    }
    dump_eq_part( ocoef[0], 0, &a, ' ' );
    printf(" = ");
    a=0;
    for( n=1; n<=nest; ++n ){
	dump_eq_part( icoef[n], n, &a, 'j' );
    }
    dump_eq_part( icoef[0], 0, &a, ' ' );
    printf("\n");
}/* dumpeq */

void dumpcoef( unsigned int nest, Nvector coef )
{
bool a;
unsigned int n;

    /* dump equation */
    a=0;
    for( n=1; n<=nest; ++n ){
	dump_eq_part( coef[n], n, &a, 'i' );
    }
    dump_eq_part( coef[0], 0, &a, ' ' );
}/* dumpcoef */

static char dirstr[PETITBUFSIZ];

char *dir_vec( dddirection dddir, unsigned int nest )
{
unsigned int i;
unsigned int index;
char ch;

    dirstr[0] = 0;
    if( nest == 0 ) return dirstr;
    ch = '(';
    for( i = 0; i < nest; i++ ){
	index = dddir & 0xf;
	sprintf( dirstr,"%s%c%s", dirstr, ch, DDdirname[index] );
	ch = ',';
	dddir = dddir >> 4;
    }
    sprintf( dirstr, "%s)", dirstr );
    return( dirstr );
}/* dir_vec */

char *dir_diff_vec( dddirection dddir, DDdifference dddiff, unsigned int nest )
{
unsigned int n;
char ch;

    dirstr[0] = 0;
    if( nest == 0 ) return dirstr;
    dir_vec( dddir, nest );

    if( dddiff != 0 ){
	sprintf( dirstr, "%s  ", dirstr );
	ch = '(';
	for( n=1; n<=nest; ++n ){
	    if( (unsigned)dddiff[n] != ddunknown ){
	        sprintf( dirstr, "%s%c%d", dirstr, ch, dddiff[n] );
	    }else{
	        sprintf( dirstr, "%s%c*", dirstr, ch );
	    }
	    ch = ',';
	}
    }
    sprintf( dirstr, "%s)", dirstr);
    return dirstr;
}/* dir_diff_vec */



int SSfirst;
int SSnloops;
Nvector SSnextloop;
node *DOptr[maxnest+1];
Nvector DOinc;
Nvector DOlower;
Nvector DOupper;
NBvector DOlowerknown;
NBvector DOupperknown;
Narray DOlowercoef;
Narray DOuppercoef;

void dd_find_limit( node *n, int *val, bool *known )
{
node *ch;
int chval;

    switch(n->nodeop){
    case op_constant :
	/* constant term */
	*val = n->nodevalue;
	break;

    case op_add :
	*val = 0;
	for( ch = n->nodechild; ch != NULL && *known; ch = ch->nodenext ){
	    dd_find_limit( ch, &chval, known );
	    if( !*known ) return;
	    *val -= chval;
	}
	break;

    case op_subtract :
	ch = n->nodechild;
	dd_find_limit( ch, val, known );
	for( ch = ch->nodenext; ch != NULL && *known; ch = ch->nodenext ){
	    dd_find_limit( ch, &chval, known );
	    if( !*known ) return;
	    *val -= chval;
	}
	break;

    case op_multiply :
	*val = 1;
	for( ch = n->nodechild; ch != NULL && *known; ch = ch->nodenext ){
	    dd_find_limit( ch, &chval, known );
	    if( !*known ) return;
	    *val = *val * chval;
	}
	break;

    default :
	/* anything else is treated as nonlinear */
	*known = 0;
    }
}/* dd_find_limit */


unsigned int dd_linearity( node *ss )
{
node *ch, *donode;
unsigned int lin, l, n;

    switch(ss->nodeop){
    case op_constant :
	/* constant term */
	return 0;

    case op_index :
	/* remember that we saw this index */
	donode = (node*)ss->nodevalue;
	donode = donode->nodeparent;
	n = donode->nodevalue;
	if( SSnextloop[n] == 0 ){
	    /* linked list of indices which we have seen */
	    SSnextloop[n] = SSfirst;
	    DOptr[n] = donode;
	    SSfirst = n;
	    ++SSnloops;
	}
	/* linear */
	return 1;

    case op_add :
    case op_subtract :
	/* max of dd_linearity of addends */
	lin = 0;
	for( ch = ss->nodechild; ch != NULL; ch = ch->nodenext ){
	    l = dd_linearity(ch);
	    if( l > lin ) lin = l;
	}
	return lin;

    case op_multiply :
	/* two more more indices multiplied is nonlinear */
	lin = 0;
	for( ch = ss->nodechild; ch != NULL; ch = ch->nodenext ){
	    lin += dd_linearity(ch);
	}
	return lin;

    default :
	/* anything else is treated as nonlinear */
	if( DDdebug ) printf("nonlinear element at %p\n", ss );
	return 2;
    }
}/* dd_linearity */

int dd_find_coef( node *ss, unsigned int n )
{
node *ch, *donode;
int val, mpy;

    switch(ss->nodeop){
    case op_constant :
	/* constant term */
	return ss->nodevalue;

    case op_index :
	/* loop index, return either 1 or zero */
	donode = (node*)ss->nodevalue;
	donode = donode->nodeparent;
	if( donode->nodevalue == n ) return 1;
	return 0;

    case op_add :
	/* sum up */
	val = 0;
	for( ch = ss->nodechild; ch != NULL; ch = ch->nodenext ){
	    val = val + dd_find_coef( ch, n );
	}
	return val;

    case op_subtract :
	/* sum up */
	val = 0;
	mpy = 1;
	for( ch = ss->nodechild; ch != NULL; ch = ch->nodenext ){
	    val = val + mpy*dd_find_coef( ch, n );
	    mpy = -1; /* negate other operands */
	}
	return val;

    case op_multiply :
	val = 1;
	for( ch = ss->nodechild; ch != NULL; ch = ch->nodenext ){
	    val = val * dd_find_coef( ch, n );
	}
	return val;

    default :
	/* anything else should not happen */
	fprintf( stderr, "reached unknown operand in subscript\n" );
	Exit(2);
    }
    return 0;
}/* dd_find_coef */

void dd_fill_coef( node *ss, NNvector coef )
{
int n;
int c0;

    c0 = dd_find_coef( ss, 0 );
    coef[0] = c0;
    for( n=SSfirst; n > 0; n=SSnextloop[n] ){
	coef[n] = dd_find_coef( ss, n ) - c0;
    }
}/* dd_fill_coef */

void dd_find_do_coefs( unsigned int n )
{
node *lower, *upper;

    lower = DOptr[n]->nodechild->nodenext;
    upper = lower->nodenext;
    if( DOinc[n] == 0 ){
	DOlowerknown[n] = 0;
	DOupperknown[n] = 0;
	return;
    }

    if( DOinc[n] < 0 ){
	lower = upper;
	upper = DOptr[n]->nodechild->nodenext;
    }
    if( dd_linearity( lower ) <= 1 ){
	DOlowerknown[n] = 1;
	dd_fill_coef( lower, DOlowercoef[n] );
	if( DDdebug ){
	    printf( "At nest %d, lower limit is: ", n );
	    dumpcoef( n, DOlowercoef[n] );
	    printf( "\n" );
	}
    }else{
	DOlowerknown[n] = 0;
	if( DDdebug ) printf ( "At nest %d, lower limit is unknown\n", n );
    }
    if( dd_linearity( upper ) <= 1 ){
	DOupperknown[n] = 1;
	dd_fill_coef( upper, DOuppercoef[n] );
	if( DDdebug ){
	    printf( "At nest %d, upper limit is: ", n );
	    dumpcoef( n, DOuppercoef[n] );
	    printf( "\n" );
	}
    }else{
	DOupperknown[n] = 0;
	if( DDdebug ) printf ( "At nest %d, upper limit is unknown\n", n );
    }
}/* dd_find_do_coefs */

int dd_gcd( int a, int b )
{
int g, r;
int g0, g1;

    if( a == 0 ){
	g = ABS(b);
    }else if( b == 0 ){
	g = ABS(a);
    }else{
	g0 = ABS(a);
	g1 = ABS(b);
	r = g0 % g1;
	while( r != 0 ){
	    g0 = g1; g1 = r;
	    r = g0 % g1;
	}
	g = g1;
    }
    return g;
}/* dd_gcd */


void dd_kirch( int a, int b, int *g, int *x, int *y )
{
int q, r;
int g0, g1;
int a0, a1, a2, b0, b1, b2;

    if( a == 0 && b == 0 ){
	*g = 0; *x = 0; *y = 0;
    }else if( a == 0 ){
	*g = ABS(b); *x = 0; *y = -b/(*g);
    }else if( b == 0 ){
	*g = ABS(a); *x = a/(*g); *y = 0;
    }else{
	a0 = 1; a1 = 0; b0 = 0; b1 = 1; g0 = ABS(a); g1 = ABS(b);
	q = g0/g1; r = g0 % g1;
	while( r != 0 ){
	    a2 = a0-q*a1; a0 = a1; a1 = a2;
	    b2 = b0-q*b1; b0 = b1; b1 = b2;
	    g0 = g1; g1 = r;
	    q = g0/g1; r = g0 % g1;
	}
	*g = g1; *x = a1; *y = -b1;
	if( a < 0 ) *x = -*x;
	if( b < 0 ) *y = -*y;
    }
}/* dd_kirch */
 
} // end namespace omega
