/* evaluate.c,v 1.1.1.2 1992/07/10 02:42:56 davew Exp */

#include <stdio.h>
#include <basic/bool.h>
#include <petit/tree.h>
#include <petit/evaluate.h>

namespace omega {

bool EVdebug = 0;


void evaluate( node *n, int *val, bool *known )
{
node *ch;
int chval;

    if( EVdebug ){
	printf( "evaluate node %p\n", n );
    }
    if( n == NULL ) return;
    switch(n->nodeop){
    case op_constant :
	/* constant term */
	*val = n->nodevalue;
	break;

    case op_add :
	ch = n->nodechild;
	evaluate( ch, val, known );
	for( ch = ch->nodenext; ch != NULL && *known; ch = ch->nodenext ){
	    evaluate( ch, &chval, known );
	    if( *known ) *val += chval;
	}
	break;

    case op_subtract :
	ch = n->nodechild;
	evaluate( ch, val, known );
	for( ch = ch->nodenext; ch != NULL && *known; ch = ch->nodenext ){
	    evaluate( ch, &chval, known );
	    if( *known ) *val -= chval;
	}
	break;

    case op_multiply :
	ch = n->nodechild;
	evaluate( ch, val, known );
	for( ch = ch->nodenext; ch != NULL && *known; ch = ch->nodenext ){
	    evaluate( ch, &chval, known );
	    if( *known ) *val = *val * chval;
	}
	break;

    case op_mod :
	ch = n->nodechild;
	evaluate( ch, val, known );
	for( ch = ch->nodenext; ch != NULL && *known; ch = ch->nodenext ){
	    evaluate( ch, &chval, known );
	    if( *known ){
		if( chval < 0 ) chval = -chval;
		if( *val == 0 ){
		    *val = 0;
		}else if( *val > 0 ){
		    *val = *val % chval;
		}else{
		    *val = chval - ((-*val) % chval);
		}
	    }
	}
	break;

    case op_min :
	ch = n->nodechild;
	evaluate( ch, val, known );
	for( ch = ch->nodenext; ch != NULL && *known; ch = ch->nodenext ){
	    evaluate( ch, &chval, known );
	    if( *known ) *val = ( chval < *val ? chval : *val );
	}
	break;

    case op_max :
	ch = n->nodechild;
	evaluate( ch, val, known );
	for( ch = ch->nodenext; ch != NULL && *known; ch = ch->nodenext ){
	    evaluate( ch, &chval, known );
	    if( *known ) *val = ( chval > *val ? chval : *val );
	}
	break;

    default :
	/* anything else is treated as nonlinear */
	*known = 0;
    }

    if( EVdebug ){
	printf( "results of evaluating %p are %d (known=%ld)\n", 
			n, *val, (unsigned long int)(*known) );
    }
}/* evaluate */


void evaluate_simple( node *n, int *val, node **rest )
{
node *ch, *chrest;
int chval;

    if( EVdebug ){
	printf( "evaluate_simple %p\n", n );
    }
    *rest = NULL;
    *val = 0;
    if( n == NULL ) return;
    switch(n->nodeop){
    case op_constant :
	/* constant term */
	*val = n->nodevalue;
	break;

    case op_add :
	ch = n->nodechild;
	evaluate_simple( ch, val, rest );
	for( ch = ch->nodenext; *rest != n && ch != NULL; ch = ch->nodenext ){
	    evaluate_simple( ch, &chval, &chrest );
	    if( chrest == NULL ){
		*val += chval;
	    }else if( *rest == NULL ){
		*val += chval;
		*rest = chrest;
	    }else{
		*rest = n;
		*val = 0;
	    }
	}
	break;

    case op_subtract :
	ch = n->nodechild;
	evaluate_simple( ch, val, rest );
	for( ch = ch->nodenext; *rest != n && ch != NULL; ch = ch->nodenext ){
	    evaluate_simple( ch, &chval, &chrest );
	    if( chrest == NULL ){
		*val -= chval;
	    }else{
		*rest = n;
		*val = 0;
	    }
	}
	break;

    case op_multiply :
	ch = n->nodechild;
	evaluate_simple( ch, val, rest );
	for( ch = ch->nodenext; *rest != n && ch != NULL; ch = ch->nodenext ){
	    evaluate_simple( ch, &chval, &chrest );
	    if( chrest == NULL && rest == NULL ){
		*val *= chval;
	    }else{
		*rest = n;
		*val = 0;
	    }
	}
	break;

    default :
	*rest = n;
	*val = 0;
    }
    if( EVdebug ){
	printf( "results of evaluate_simpling %p are %d +%p\n",
			n, *val, *rest );
    }
}/* evaluate_simple */

} // end omega namespace
