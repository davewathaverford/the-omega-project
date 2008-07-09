/* $Id: petitl.l,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

%{
#define yywrap()	1

YY_BUFFER_STATE hpp_yy_scan_string(yyconst char *);
int hpp_yyparse(void);


#define free(x)         free((char *)(x))

namespace omega {
    extern FILE *hpp_debug_file;
};
using namespace omega;

%}
%%


"!"[hH][pP][pPFf]\$[^\n]*$      { YY_BUFFER_STATE hppbuf;
			  	  hpp_debug_file = DebugFile;
			          hppbuf = hpp_yy_scan_string(yytext+5);
				  hpp_yyparse();
/*				  yy_delete_buffer(hppbuf);*/
	                          Lines=P_Lines; 
                                  strncpy(P_IDname,&(yytext[1]),MAXIDLENGTH);
                                  return(TCOMMENT); 
				}
":"				{ Lines=P_Lines; return(TCOLON); }
","				{ Lines=P_Lines; return(TCOMMA); }
"="				{ Lines=P_Lines; return(TEQUAL); }
"**"				{ Lines=P_Lines; return(TEXP); }
[0-9]+"."[0-9]*			{ Lines=P_Lines;
                                  strncpy(P_IDname,yytext,MAXIDLENGTH); 
                                  return(TFLOAT); }
"."[0-9]+			{ Lines=P_Lines;
                                  strncpy(P_IDname,yytext,MAXIDLENGTH); 
                                  return(TFLOAT); }
[0-9]+"."[0-9]*[eE][+-]?[0-9]+	{ Lines=P_Lines;
                                  strncpy(P_IDname,yytext,MAXIDLENGTH); 
                                  return(TFLOAT); }
[0-9]+[eE][+-]?[0-9]+		{ Lines=P_Lines;
                                  strncpy(P_IDname,yytext,MAXIDLENGTH); 
                                  return(TFLOAT); }
"."[0-9]+[eE][+-]?[0-9]+	{ Lines=P_Lines;
                                  strncpy(P_IDname,yytext,MAXIDLENGTH); 
                                  return(TFLOAT); }
"=="				{ Lines=P_Lines; return(TEQ); }
"<>"				{ Lines=P_Lines; return(TNE); }
"!="				{ Lines=P_Lines; return(TNE); }
">="				{ Lines=P_Lines; return(TGE); }
">"				{ Lines=P_Lines; return(TGT); }
"<="				{ Lines=P_Lines; return(TLE); }
"<"				{ Lines=P_Lines; return(TLT); }
[0-9]+				{ Lines=P_Lines; 
                                  strncpy(P_INTname,yytext,MAXIDLENGTH); 
                                  return(TINT); }
"{"				{ Lines=P_Lines; return(TLCB); }
"}"				{ Lines=P_Lines; return(TRCB); }
"("				{ Lines=P_Lines; return(TLP); }
"["				{ Lines=P_Lines; return(TLP); }
"-"				{ Lines=P_Lines; return(TMINUS); }
"+"				{ Lines=P_Lines; return(TPLUS); }
")"				{ Lines=P_Lines; return(TRP); }
"]"				{ Lines=P_Lines; return(TRP); }
";"				{ Lines=P_Lines; return(TSEMI); }
"!"[^=\n][^\n]*                       { Lines=P_Lines; 
                                  strncpy(P_IDname,&(yytext[1]),MAXIDLENGTH);
                                  return(TCOMMENT); }
"!"/"\n"                       { Lines=P_Lines; 
                                  strncpy(P_IDname,&(yytext[1]),MAXIDLENGTH);
                                  return(TCOMMENT); }
"/"				{ Lines=P_Lines; return(TSLASH); }
"*"				{ Lines=P_Lines; return(TSTAR); }

"+="				{ Lines=P_Lines; return(TPLUSEQUAL); }
"*="				{ Lines=P_Lines; return(TSTAREQUAL); }
"max="				{ Lines=P_Lines; return(TMAXEQUAL); }
"min="				{ Lines=P_Lines; return(TMINEQUAL); }

[aA][sS][sS][eE][rR][tT]	{ Lines=P_Lines; return(TASSERT); }
[bB][yY]			{ Lines=P_Lines; return(TBY); }
[cC][eE][iI][lL][iI][nN][gG]	{ Lines=P_Lines; return(TCEILING); }
[cC][oO][nN][sS][tT]		{ Lines=P_Lines; return(TCONST); }
[dD][oO]			{ Lines=P_Lines; return(TDO); }
[dD][oO][aA][lL][lL]		{ Lines=P_Lines; return(TPARFOR); }
[dD][oO][aA][nN][yY]		{ Lines=P_Lines; return(TDOANY); }
[eE][lL][sE][eE]		{ Lines=P_Lines; return(TELSE); }
[eE][nN][dD][fF][oO][rR]	{ Lines=P_Lines; return(TENDFOR); }
[eE][nN][dD][iI][fF]		{ Lines=P_Lines; return(TENDIF); }
[fF][lL][oO][oO][rR]		{ Lines=P_Lines; return(TFLOOR); }
[fF][oO][rR]			{ Lines=P_Lines; return(TFOR); }
[fF][oO][rR][aA][lL][lL]	{ Lines=P_Lines; return(TVECFOR); }
[iI][fF]			{ Lines=P_Lines; return(TIF); }
[iI][nN][tT][eE][gG][eE][rR]	{ Lines=P_Lines; return(TINTEGER); }
[mM][aA][xX]			{ Lines=P_Lines; return(TMAX); }
[mM][iI][nN]			{ Lines=P_Lines; return(TMIN); }
[mM][oO][dD]			{ Lines=P_Lines; return(TMOD); }
[rR][eE][aA][lL]		{ Lines=P_Lines; return(TREAL); }
[sS][qQ][rR][tT]		{ Lines=P_Lines; return(TSQRT); }
[tT][hH][eE][nN]		{ Lines=P_Lines; return(TTHEN); }
[tT][oO]			{ Lines=P_Lines; return(TTO); }

[rR][eE][tT][uU][rR][nN]	{ Lines=P_Lines; return(TRETURN); }
[cC][oO][mM][mM][oO][nN]	{ Lines=P_Lines; return(TCOMMON); }
[sS][tT][aA][tT][iI][cC]	{ Lines=P_Lines; return(TSTATIC); }
[aA][uU][tT][oO]        	{ Lines=P_Lines; return(TAUTO); }
[fF][oO][rR][mM][aA][lL]	{ Lines=P_Lines; return(TFORMAL); }
[iI][nN]		       	{ Lines=P_Lines; return(TIN); }
[oO][uU][tT]		       	{ Lines=P_Lines; return(TOUT); }
[iI][nN][oO][uU][tT]	       	{ Lines=P_Lines; return(TINOUT); }
[bB][uU][iI][lL][tT][iI][nN]	{ Lines=P_Lines; return(TBUILTIN); }
[pP][rR][iI][vV][aA][tT][eE]	{ Lines=P_Lines; return(TPRIVATE); }
[aA][nN][dD]                    { Lines=P_Lines; return(TAND); }
[oO][rR]                        { Lines=P_Lines; return(TOR); }
[nN][oO][tT]                    { Lines=P_Lines; return(TNOT); }
[rR][aA][nN][dD]		{ Lines=P_Lines; return(TRAND); }

[\n\r]				{ Lines=P_Lines; ++P_Lines; return(TSEMI); }

[a-zA-Z][a-zA-Z0-9_$#]*		{ Lines=P_Lines; 
                                  strncpy(P_IDname,yytext,MAXIDLENGTH); 
                                  return(TID); }
[\t" "]				{ Lines=P_Lines; ; }
.                               { sprintf(P_IDname, "Unknown character '%s' on line %ld", yytext, P_Lines);
                                  Error(P_IDname);
}
%%


