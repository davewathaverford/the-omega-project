/* $Id: hppl.l,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */


%{

#include <stdio.h>
#include <string.h>

#if 0
#define POSSIBLY_ECHO ECHO
#else
#define POSSIBLY_ECHO
#endif


int hpp_yyerror(char *s) {
    fprintf(stderr,"%s\n",s);
    fprintf(stderr,"last token was %s\n",yytext);
    exit(1);
    return 0;
    }

%}

%% 

[ \t]+	      { POSSIBLY_ECHO; }
\*           { POSSIBLY_ECHO; return STAR; }
\(           { POSSIBLY_ECHO; return LPAREN; }
\)           { POSSIBLY_ECHO; return RPAREN; }
\,           { POSSIBLY_ECHO; return COMMA; }
"align"	{ POSSIBLY_ECHO; return ALIGN; }
"distribute"	{ POSSIBLY_ECHO; return DISTRIBUTE; }
"processors"	{ POSSIBLY_ECHO; return PROCESSORS; }
"template"	{ POSSIBLY_ECHO; return TEMPLATE; }
"block"	        { POSSIBLY_ECHO; return BLOCK; }
"cyclic"	{ POSSIBLY_ECHO; return CYCLIC; }
"BLOCK"	        { POSSIBLY_ECHO; return BLOCK; }
"CYCLIC"	{ POSSIBLY_ECHO; return CYCLIC; }
"onto"          { POSSIBLY_ECHO; return ONTO; }
"with"          { POSSIBLY_ECHO; return WITH; }
[a-zA-Z][a-zA-Z0-9_]*	{ POSSIBLY_ECHO; yylval.VAR_NAME = (char *) malloc(1+yyleng);
			  strcpy(yylval.VAR_NAME,yytext);
			  return VAR;
                        }
[0-9]+		{ POSSIBLY_ECHO; yylval.INT_VALUE = atoi(yytext);
		  return INT;
                 }
.		{ return yytext[0]; }

