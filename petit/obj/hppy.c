
/*  A Bison parser, made from ../src/hppy.y
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse hpp_yyparse
#define yylex hpp_yylex
#define yyerror hpp_yyerror
#define yylval hpp_yylval
#define yychar hpp_yychar
#define yydebug hpp_yydebug
#define yynerrs hpp_yynerrs
#define	p1	258
#define	p2	259
#define	INTERSECTION	260
#define	p3	261
#define	p4	262
#define	p5	263
#define	p6	264
#define	NOT	265
#define	p7	266
#define	p8	267
#define	p9	268
#define	p10	269
#define	p11	270
#define	VAR	271
#define	INT	272
#define	ALIGN	273
#define	DISTRIBUTE	274
#define	PROCESSORS	275
#define	TEMPLATE	276
#define	WITH	277
#define	ONTO	278
#define	CYCLIC	279
#define	BLOCK	280
#define	STAR	281
#define	LPAREN	282
#define	RPAREN	283
#define	COMMA	284

#line 1 "../src/hppy.y"


/* 
to do to get HPP:

H   write function to create space mapping from access function
M+  figure out how to handle different distributions 
L   come up with interesting time mappings if independent?
M   allow code generation to execute things in any order (for indep. loops)
M   generation of local addresses, then interchange loops
M   generation of communication statements

*/

#define hpp_yywrap()	1

#include <omega.h>
#include <stdio.h>
#include <string.h>
#include <basic/Map.h>
#include <omega/AST.h>
#include <math.h>
#include <petit/tree.h>
#include <petit/hpp.h>
#include <petit/petit_args.h>
#ifdef WIN32
#include <io.h>
#define isatty _isatty
#include <malloc.h>
#define alloca _alloca
#endif

extern int hpp_yylex(void);
extern int hpp_yyerror(char * );

typedef List<int> IntList;
List<template_info *> templates;
Map<String,IntList*> proc_map ((IntList *)0);
Map<String,Relation*> alignment_map ((Relation *)0);
Map<String,Relation*> distribute_map ((Relation *)0);
Map<String,Relation*> whole_map ((Relation *)0);

// array -> templ name
Map<String,String> tname_map("");
//templ -> proc name
Map<String,String> pname_map("");

FILE *hpp_debug_file;

int pickles=0;

void hpp_error_callback(const String &s) {
    fprintf(stderr, "%s\n", (const char *) s);
    exit(1);
}



#line 83 "../src/hppy.y"
typedef union {
    List<int> *INTEGER_LIST;
    char *VAR_NAME;
    int INT_VALUE;
    List<Variable_ID> *VAR_LIST;
    List<dist_info> *DISTLIST;
    dist_info *DISTINFO;
    Relation * RELATION;
    Exp *EXP;
    ExpList *EXP_LIST;
    tupleDescriptor * TUPLE_DESCRIPTOR;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		76
#define	YYFLAG		-32768
#define	YYNTBASE	37

#define YYTRANSLATE(x) ((unsigned)(x) <= 284 ? yytranslate[x] : 55)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    18,
    36,     9,     4,     2,     5,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    35,     2,     2,
     2,     2,     2,    10,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     6,     7,
     8,    11,    12,    13,    14,    15,    16,    17,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
    31,    32,    33,    34
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     6,     9,    11,    13,    15,    17,    23,
    29,    30,    38,    46,    50,    52,    54,    56,    61,    63,
    64,    69,    71,    75,    77,    79,    81,    85,    91,    93,
    97,    99,   103,   105,   108,   110,   113,   117,   121
};

static const short yyrhs[] = {    -1,
    38,    39,     0,    40,     0,    40,    39,     0,    41,     0,
    42,     0,    43,     0,    45,     0,    26,    21,    32,    52,
    33,     0,    25,    21,    32,    52,    33,     0,     0,    44,
    23,    21,    48,    27,    21,    48,     0,    24,    21,    32,
    46,    33,    28,    21,     0,    46,    34,    47,     0,    47,
     0,    30,     0,    29,     0,    29,    32,    22,    33,     0,
    31,     0,     0,    49,    32,    50,    33,     0,    51,     0,
    50,    34,    51,     0,    21,     0,     9,     0,    54,     0,
    54,    35,    54,     0,    54,    35,    54,    35,    22,     0,
    22,     0,    52,    34,    22,     0,    21,     0,    18,    54,
    36,     0,    22,     0,    22,    53,     0,    53,     0,     5,
    54,     0,    54,     4,    54,     0,    54,     5,    54,     0,
    54,     9,    54,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    98,   106,   109,   111,   113,   113,   113,   113,   115,   124,
   135,   137,   203,   233,   234,   238,   239,   240,   241,   245,
   247,   251,   253,   256,   269,   271,   278,   280,   285,   291,
   298,   315,   318,   319,   320,   321,   322,   323,   324
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","p1","'+'",
"'-'","p2","INTERSECTION","p3","'*'","'@'","p4","p5","p6","NOT","p7","p8","p9",
"'('","p10","p11","VAR","INT","ALIGN","DISTRIBUTE","PROCESSORS","TEMPLATE","WITH",
"ONTO","CYCLIC","BLOCK","STAR","LPAREN","RPAREN","COMMA","':'","')'","start",
"@1","stmt_list","stmt","template_decl","proc_decl","alignment","@2","distribution",
"distlist","distinfo","tupleDeclaration","@3","tupleVarList","tupleVar","intlist",
"simpleExp","exp", NULL
};
#endif

static const short yyr1[] = {     0,
    38,    37,    39,    39,    40,    40,    40,    40,    41,    42,
    44,    43,    45,    46,    46,    47,    47,    47,    47,    49,
    48,    50,    50,    51,    51,    51,    51,    51,    52,    52,
    53,    53,    54,    54,    54,    54,    54,    54,    54
};

static const short yyr2[] = {     0,
     0,     2,     1,     2,     1,     1,     1,     1,     5,     5,
     0,     7,     7,     3,     1,     1,     1,     4,     1,     0,
     4,     1,     3,     1,     1,     1,     3,     5,     1,     3,
     1,     3,     1,     2,     1,     2,     3,     3,     3
};

static const short yydefact[] = {     1,
    11,     0,     0,     0,     2,     3,     5,     6,     7,     0,
     8,     0,     0,     0,     4,     0,     0,     0,     0,    20,
    17,    16,    19,     0,    15,    29,     0,     0,     0,     0,
     0,     0,     0,    10,     0,     9,     0,     0,     0,     0,
    14,    30,    20,     0,    25,     0,    31,    33,     0,    22,
    35,    26,    18,    13,    12,    31,    36,     0,    34,    21,
     0,     0,     0,     0,     0,    32,    23,    37,    38,    39,
    27,     0,    28,     0,     0,     0
};

static const short yydefgoto[] = {    74,
     1,     5,     6,     7,     8,     9,    10,    11,    24,    25,
    29,    30,    49,    50,    27,    51,    52
};

static const short yypact[] = {-32768,
    20,     5,    29,    30,-32768,    17,-32768,-32768,-32768,    31,
-32768,    21,    23,    24,-32768,    36,    18,    37,    37,-32768,
    26,-32768,-32768,   -23,-32768,-32768,   -17,   -14,    25,    28,
    39,    34,    18,-32768,    41,-32768,    43,     9,    32,    45,
-32768,-32768,-32768,    16,-32768,    16,    -5,    -6,     2,-32768,
-32768,    -2,-32768,-32768,-32768,-32768,-32768,    -4,-32768,-32768,
     9,    16,    16,    16,    16,-32768,-32768,    58,    58,-32768,
     4,    46,-32768,    69,    70,-32768
};

static const short yypgoto[] = {-32768,
-32768,    65,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    40,
    33,-32768,-32768,    11,    55,    27,   -40
};


#define	YYLAST		76


static const short yytable[] = {    62,
    63,    62,    63,    57,    64,    58,    64,    62,    63,    32,
    33,    46,    64,    44,    56,    34,    35,    45,    36,    35,
    44,    68,    69,    70,    71,    12,    46,   -24,   -24,    47,
    48,    66,    65,    46,    60,    61,    56,    48,    72,   -11,
     2,     3,     4,     2,     3,     4,    21,    22,    23,    13,
    14,    37,    17,    16,    18,    19,    20,    31,    26,    38,
    39,    40,    42,    43,    53,    54,    64,    73,    75,    76,
    15,    67,    41,    28,    59,    55
};

static const short yycheck[] = {     4,
     5,     4,     5,    44,     9,    46,     9,     4,     5,    33,
    34,    18,     9,     5,    21,    33,    34,     9,    33,    34,
     5,    62,    63,    64,    65,    21,    18,    33,    34,    21,
    22,    36,    35,    18,    33,    34,    21,    22,    35,    23,
    24,    25,    26,    24,    25,    26,    29,    30,    31,    21,
    21,    27,    32,    23,    32,    32,    21,    32,    22,    32,
    22,    28,    22,    21,    33,    21,     9,    22,     0,     0,
     6,    61,    33,    19,    48,    43
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

#ifndef YYPARSE_RETURN_TYPE
#define YYPARSE_RETURN_TYPE int
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
YYPARSE_RETURN_TYPE yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 196 "/usr/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

YYPARSE_RETURN_TYPE
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 98 "../src/hppy.y"
{ current_Declaration_Site = 0;
         extern FILE *DebugFile;
         hpp_debug_file = DebugFile;
         current_Declaration_Site = globalDecls = 
	     new Global_Declaration_Site();
         petit_args.hpp_mode = true;
       ;
    break;}
case 2:
#line 106 "../src/hppy.y"
{ delete globalDecls; ;
    break;}
case 9:
#line 116 "../src/hppy.y"
{  
       List<int> *l = yyvsp[-1].INTEGER_LIST;
       add_template(yyvsp[-3].VAR_NAME, l);
       fprintf(hpp_debug_file,"Template %s is: ",yyvsp[-3].VAR_NAME);
       make_bounds_relation(l).print_with_subs(hpp_debug_file,true);
   ;
    break;}
case 10:
#line 125 "../src/hppy.y"
{  
       add_proc(yyvsp[-3].VAR_NAME,yyvsp[-1].INTEGER_LIST);
       assert(yyvsp[-1].INTEGER_LIST->length() == 1 && "Only handle 1-dim processors");
#if 0
       fprintf(hpp_debug_file,"Processor declaration %s is: ",yyvsp[-3].VAR_NAME);
       make_bounds_relation(lookup_proc(yyvsp[-3].VAR_NAME)).print_with_subs(hpp_debug_file,true);
#endif
   ;
    break;}
case 11:
#line 136 "../src/hppy.y"
{ relationDecl = new Declaration_Site(); ;
    break;}
case 12:
#line 138 "../src/hppy.y"
{

    symtabentry *s = lookup_symbol(yyvsp[-4].VAR_NAME);
    s->template_name = yyvsp[-1].VAR_NAME;

/* This chunk of code from parser.y constructs a relation from the 
   expression lists in the alignment statement */

    Relation * r = new Relation(yyvsp[-3].TUPLE_DESCRIPTOR->size,yyvsp[0].TUPLE_DESCRIPTOR->size);
    resetGlobals();
    F_And *f = r->add_and();
    int i;
    for(i=1;i<=yyvsp[-3].TUPLE_DESCRIPTOR->size;i++) {	
	yyvsp[-3].TUPLE_DESCRIPTOR->vars[i]->vid = r->input_var(i);
	if (!yyvsp[-3].TUPLE_DESCRIPTOR->vars[i]->anonymous) 
	    r->name_input_var(i,yyvsp[-3].TUPLE_DESCRIPTOR->vars[i]->stripped_name);
    };
    for(i=1;i<=yyvsp[0].TUPLE_DESCRIPTOR->size;i++) {
	yyvsp[0].TUPLE_DESCRIPTOR->vars[i]->vid = r->output_var(i);
	if (!yyvsp[0].TUPLE_DESCRIPTOR->vars[i]->anonymous) 
	    r->name_output_var(i,yyvsp[0].TUPLE_DESCRIPTOR->vars[i]->stripped_name);
    };
    foreach(e,Exp*,yyvsp[-3].TUPLE_DESCRIPTOR->eq_constraints, install_eq(f,e,0));
    
    /*   foreach(e,Exp*,$4->geq_constraints, install_geq(f,e,0)); */
    do 
	{
	    for (Any_Iterator<Exp*> __P_x = (yyvsp[-3].TUPLE_DESCRIPTOR->geq_constraints).any_iterator();
		 __P_x;
		 __P_x++) 
		{
		    Exp* & e = *__P_x; 
		    install_geq(f,e,0);
		}
	} while (0);
    
    foreach(c,strideConstraint*,yyvsp[-3].TUPLE_DESCRIPTOR->stride_constraints, install_stride(f,c));
    foreach(e,Exp*,yyvsp[0].TUPLE_DESCRIPTOR->eq_constraints, install_eq(f,e,0));
    foreach(e,Exp*,yyvsp[0].TUPLE_DESCRIPTOR->geq_constraints, install_geq(f,e,0));
    foreach(c,strideConstraint*,yyvsp[0].TUPLE_DESCRIPTOR->stride_constraints, install_stride(f,c));
    delete yyvsp[-3].TUPLE_DESCRIPTOR;
    delete yyvsp[0].TUPLE_DESCRIPTOR;
#if 0
    fprintf(hpp_debug_file,"Alignment relation is: ");
    r->print_with_subs(hpp_debug_file,true);
#endif
    
    current_Declaration_Site = globalDecls;
    delete relationDecl; 
    
/* Finished building relation*/
    r->finalize();
    s->alignment = r; // store it

/* 
		Relation *w = new Relation;
		Relation *ddd = distribute_map[$6];
		assert(ddd && "Distribute the template before aligning anything with it");
		*w = Join(copy(*r),copy(*ddd));
		whole_map[$3] = w;
*/
		free(yyvsp[-4].VAR_NAME);
		free(yyvsp[-1].VAR_NAME);
;
    break;}
case 13:
#line 203 "../src/hppy.y"
{

    template_info *ti = lookup_template(yyvsp[-5].VAR_NAME);
    ti->proc_name = yyvsp[0].VAR_NAME;

// create a relation that maps template locations to processor locations;
    List<dist_info> *dl = yyvsp[-3].DISTLIST;
    String template_name(yyvsp[-5].VAR_NAME),proc_name(yyvsp[0].VAR_NAME);

    List<int> *tl = ti->bounds_list; 
    Relation tem = make_bounds_relation(tl);
    List<int> *pl = lookup_proc(proc_name);
    Relation proc = make_bounds_relation(pl);
    assert(tem.n_set() == dl->size());


#if 0
    Relation d = make_distribution_relation(*tl,*pl,*dl);

    d = Restrict_Domain(d, copy(tem));
    d = Restrict_Range(d, copy(proc));
    fprintf((hpp_debug_file),"map from template locations to processors: ");
    d.print_with_subs(hpp_debug_file);
#endif

    ti->dist_list = dl;
;
    break;}
case 14:
#line 233 "../src/hppy.y"
{ yyvsp[-2].DISTLIST->append(*yyvsp[0].DISTINFO); delete yyvsp[0].DISTINFO; yyval.DISTLIST = yyvsp[-2].DISTLIST; ;
    break;}
case 15:
#line 234 "../src/hppy.y"
{ distribution_list *dl = new distribution_list;
	             dl->append(*yyvsp[0].DISTINFO); delete yyvsp[0].DISTINFO; yyval.DISTLIST = dl; ;
    break;}
case 16:
#line 238 "../src/hppy.y"
{ yyval.DISTINFO = new dist_info(block); ;
    break;}
case 17:
#line 239 "../src/hppy.y"
{ yyval.DISTINFO =  new dist_info(cyclic,1); ;
    break;}
case 18:
#line 240 "../src/hppy.y"
{ yyval.DISTINFO =  new dist_info(cyclic, (int)yyvsp[-1].INT_VALUE); ;
    break;}
case 19:
#line 241 "../src/hppy.y"
{ yyval.DISTINFO =  new dist_info(star); ;
    break;}
case 20:
#line 246 "../src/hppy.y"
{ currentTupleDescriptor = new tupleDescriptor; ;
    break;}
case 21:
#line 248 "../src/hppy.y"
{yyval.TUPLE_DESCRIPTOR = currentTupleDescriptor; ;
    break;}
case 24:
#line 258 "../src/hppy.y"
{         Declaration_Site *ds = defined(yyvsp[0].VAR_NAME);
	  if (!ds) 
            currentTupleDescriptor->extend(yyvsp[0].VAR_NAME);
	  else 
            currentTupleDescriptor->extend(yyvsp[0].VAR_NAME, new Exp(lookupScalar(yyvsp[0].VAR_NAME)));

	  free(yyvsp[0].VAR_NAME);

/*currentTupleDescriptor->extend($1);*/

 ;
    break;}
case 25:
#line 270 "../src/hppy.y"
{currentTupleDescriptor->extend(); ;
    break;}
case 26:
#line 272 "../src/hppy.y"
{
//	    if (!pickles)
		currentTupleDescriptor->extend(yyvsp[0].EXP); 
	    pickles=0;
;
    break;}
case 27:
#line 279 "../src/hppy.y"
{currentTupleDescriptor->extend(yyvsp[-2].EXP,yyvsp[0].EXP); ;
    break;}
case 28:
#line 281 "../src/hppy.y"
{currentTupleDescriptor->extend(yyvsp[-4].EXP,yyvsp[-2].EXP,yyvsp[0].INT_VALUE); ;
    break;}
case 29:
#line 286 "../src/hppy.y"
{ 
	List<int> *l = new List<int>;
	l->append(yyvsp[0].INT_VALUE);
	yyval.INTEGER_LIST = l;
    ;
    break;}
case 30:
#line 292 "../src/hppy.y"
{
		 yyvsp[-2].INTEGER_LIST->append(yyvsp[0].INT_VALUE);
		 yyval.INTEGER_LIST = yyvsp[-2].INTEGER_LIST;
	     ;
    break;}
case 31:
#line 300 "../src/hppy.y"
{ 
//fprintf(hpp_debug_file,"variable %s seen",$1);

//	    Declaration_Site *ds = defined($1);
//            if (!ds) {
//fprintf(hpp_debug_file,"...not declared, declaring",$1);

//	      currentTupleDescriptor->extend($1);
//	  }
//fprintf(hpp_debug_file,"...variable ref = %x\n",lookupScalar($1));
	  yyval.EXP = new Exp(lookupScalar(yyvsp[0].VAR_NAME));
	  free(yyvsp[0].VAR_NAME);
	    pickles=1;

	;
    break;}
case 32:
#line 315 "../src/hppy.y"
{ yyval.EXP = yyvsp[-1].EXP;;
    break;}
case 33:
#line 318 "../src/hppy.y"
{yyval.EXP = new Exp(yyvsp[0].INT_VALUE);;
    break;}
case 34:
#line 319 "../src/hppy.y"
{yyval.EXP = multiply(yyvsp[-1].INT_VALUE,yyvsp[0].EXP);;
    break;}
case 35:
#line 320 "../src/hppy.y"
{ yyval.EXP = yyvsp[0].EXP; ;
    break;}
case 36:
#line 321 "../src/hppy.y"
{ yyval.EXP = negate(yyvsp[0].EXP);;
    break;}
case 37:
#line 322 "../src/hppy.y"
{ yyval.EXP = add(yyvsp[-2].EXP,yyvsp[0].EXP);;
    break;}
case 38:
#line 323 "../src/hppy.y"
{ yyval.EXP = subtract(yyvsp[-2].EXP,yyvsp[0].EXP);;
    break;}
case 39:
#line 324 "../src/hppy.y"
{ yyval.EXP = multiply(yyvsp[-2].EXP,yyvsp[0].EXP);;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 498 "/usr/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 329 "../src/hppy.y"



#include <hppl.c>

symtabentry *lookup_symbol(const String &name) {

    for(nametabentry *n =NameHead; n != 0; n = n->namenext)
	if(strcmp(n->namest.symname, name) == 0)
	    return &(n->namest);
    fprintf(stderr,"\nAlignment directive found before symbol's declaration\n");
    assert(0 && "Symbol declaration not found");
    return 0;
}

template_info *lookup_template(const String &name) {
    for(List_Iterator<template_info *> i = templates; i; i++)
	if(name == (*i)->name) return (*i);
    assert(0 && "Template declaration not found");
    return 0;
}


template_info *add_template(const String &name, List<int> *l)
{ 
    template_info *ti = new template_info(name, l);
    templates.append(ti);
    return ti;
}


List<int> *lookup_proc(const String &name) {
    List<int> *l = proc_map(name);
    assert(l && "Processor declaration not found");
    return l;
}

List<int> *add_proc(const String &name, List<int> *l) {
    assert(l && "adding null processors?");
    proc_map[name] = l;
    return l;
}

Relation make_bounds_relation(List<int> *l)
{
       Relation template_rel(l->size());
       template_rel.add_and(); // allow and_with...
       for(int i = 1; i <= l->size(); i++) {
	   GEQ_Handle g = template_rel.and_with_GEQ();
	   Variable_ID v = template_rel.set_var(i);
	   g.update_coef(v,1);
	   g.update_const(-1);
	   g = template_rel.and_with_GEQ();
	   g.update_coef(v,-1);
	   g.update_const((*l)[i]);
       }
       return template_rel;
}


// Template list, processor list, distribution list
Relation make_distribution_relation(List<int> &tl, List<int> &pl,
				    distribution_list &dl) {


    Relation d(tl.length(), pl.length());
    F_Exists *ex = d.and_with()->add_exists();
    F_And *a = ex->add_and();

    int current_pdim = 1;

// Note that the following code does not really work for things like 
// block,block
    for(int dim = 1; dim <= dl.size(); dim++) {
	switch(dl[dim].type) {
	case star: continue;
	case block:
	    {
		assert(dl[dim].blocksize==0);
		int tsize = tl[dim]; // get size of template
                int psize = pl[current_pdim]; // get size of processors
		assert(tsize >= psize);
		int bsize = (int) ceil(((float)tsize)/psize);
		fprintf(hpp_debug_file,"dim %d block size: %d, tsize %d, psize %d\n",dim, bsize,tsize,psize);
		// Now constrain local elements to myproc*bsize...(mp+1)*bsize
		Variable_ID v = input_var(dim), u = output_var(current_pdim);
		
		GEQ_Handle g = a->add_GEQ();
//		Free_Var_Decl mp(String("myproc_")+itoS(dim));
		g.update_const(-1+bsize);
		g.update_coef(v,1);
		g.update_coef(u, -bsize);
//		g.update_coef(d.get_local(&mp), -bsize);
		g = a->add_GEQ();
		g.update_coef(v,-1);
		g.update_const(0);
		g.update_coef(u,bsize);
//		g.update_coef(d.get_local(&mp),bsize);
		g.finalize();
		current_pdim++;
	    }
	    break;
	case cyclic:
	    {
//		stride is  nprocs, lb is myproc, ub is ub
		int tsize = tl[dim]; // get size of template
                int psize = pl[dim]; // get size of processors
		int bsize = dl[dim].blocksize;  // get size of block
		assert(tsize >= psize);

		fprintf(hpp_debug_file,"dim %d: tsize %d, psize %d\n",dim, tsize,psize);

		GEQ_Handle g = a->add_GEQ();
		// stride: inp = outp*bsize + (bsize*nprocs)*alpha
		g.update_coef(ex->declare(), -(psize*bsize));
		g.update_coef(output_var(dim), -1);
		g.update_coef(input_var(dim), 1);
		g = a->add_GEQ();
		g.update_coef(ex->declare(), (psize*bsize));
		g.update_coef(output_var(dim), 1);
		g.update_coef(input_var(dim), -1);
		g.update_const(bsize-1);
		g = a->add_GEQ();
		g.update_coef(input_var(dim), -1);
		g.update_const(tsize);
		current_pdim++;
	    }
	}
    }
    d.finalize();
    return d;
}
