
/*  A Bison parser, made from ../src/petity.y
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	TCOMMENT	258
#define	TASSERT	259
#define	TBY	260
#define	TCEILING	261
#define	TCOLON	262
#define	TCOMMA	263
#define	TCONST	264
#define	TDO	265
#define	TDOANY	266
#define	TELSE	267
#define	TENDFOR	268
#define	TENDIF	269
#define	TEQ	270
#define	TEQUAL	271
#define	TEXP	272
#define	TFLOAT	273
#define	TFLOOR	274
#define	TFOR	275
#define	TGE	276
#define	TGT	277
#define	TRAND	278
#define	TID	279
#define	TIF	280
#define	TINT	281
#define	TINTEGER	282
#define	TLE	283
#define	TLT	284
#define	TLP	285
#define	TMAX	286
#define	TMIN	287
#define	TMINUS	288
#define	TMOD	289
#define	TNE	290
#define	TAND	291
#define	TOR	292
#define	TNOT	293
#define	TPARFOR	294
#define	TPLUS	295
#define	TREAL	296
#define	TRP	297
#define	TSEMI	298
#define	TSLASH	299
#define	TSQRT	300
#define	TSTAR	301
#define	TTHEN	302
#define	TTO	303
#define	TVECFOR	304
#define	TLCB	305
#define	TRCB	306
#define	TFORMAL	307
#define	TCOMMON	308
#define	TSTATIC	309
#define	TAUTO	310
#define	TPRIVATE	311
#define	TBUILTIN	312
#define	TIN	313
#define	TOUT	314
#define	TINOUT	315
#define	TRETURN	316
#define	TPLUSEQUAL	317
#define	TSTAREQUAL	318
#define	TMAXEQUAL	319
#define	TMINEQUAL	320

#line 4 "../src/petity.y"

#include <string.h>
#include <stdlib.h>
#include <basic/assert.h>
#include <basic/Exit.h>
#include <petit/lang-interf.h>
#include <petit/make.h>
#include <petit/tree.h>
#include <petit/vutil.h>
#include <petit/petity.h>
#ifdef WIN32
#include <io.h>
#define isatty _isatty
#include <malloc.h>
#define alloca _alloca
#endif

#define AllowComments 1

static void parse_die();
static int parse_intval();
static void parse_linknode( node *new_node );
static void parse_unlinknode();
static void parse_linknodeto( node *old, node *new_node );
static void parse_pushscope( node *n );
static void parse_popscope();
static node *parse_binaryop( node *left, optype op, node *right );
static void parse_forloop(optype op, symtabentry *index,
			       node *lower, node *upper, node *inc);
static void parse_endforloop();

static void parse_stmt_index( node *n );
static void parse_check_stmt_number();
static void parse_init();
static void yyerror( char *s);
symtabentry *add_symbol(const char *name, int new_int,
			symboltype symtype, exprtype exptype,
			nametabentry *onam);
static void CheckEnd();
static symtabentry *parse_doid_symbol();

/* global variables: */
/* line number */
unsigned long int Lines;

/* Entry node */
node *Entry;

/* Exit node */
node *ExitNode;

/* List of assertions */
node *Assertions = NULL;

/* Current head of name table */
nametabentry *NameHead;

/* communication from lexer */
/* name of ID */
char P_IDname[MAXIDLENGTH], P_INTname[MAXIDLENGTH];

/* line number of next token */
unsigned long int P_Lines;

#ifdef WIN32
#include <io.h>
#define isatty _isatty
#endif
#include "petit/petitl.c"

/* current nest level in loops */
unsigned long int P_LoopNest;

#include <petit/ddutil.h>
char *loopsyms[maxnest];

/* current statement number */
unsigned long int P_StmtNumber;

/* A node pointer */
node *P_Node;

/* A symbol table entry pointer */
symtabentry *P_Sym, *P_Sym1, *P_DeclSym;

/* type of symbol being declared */
symboltype P_SaveType;

var_class SaveClass;

/* What to use for 'prev' of next node, 'parent' of next node */
node *P_NextPrev, *P_NextParent;

/* To allow undeclared variables appear in array bounds */
int NoNewOldCheck = 0;

/* to prohibit RETURN statements from appearing not at the end */
int ShouldBeEnd;




/* end of declarations section */

#line 109 "../src/petity.y"
typedef union {
    int iv;
    node *np;
    symtabentry *stp;
    optype op;
    } YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		264
#define	YYFLAG		-32768
#define	YYNTBASE	66

#define YYTRANSLATE(x) ((unsigned)(x) <= 320 ? yytranslate[x] : 123)

static const char yytranslate[] = {     0,
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
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,    64,    65
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     6,    10,    11,    13,    15,    17,    19,
    21,    23,    25,    27,    29,    31,    33,    35,    37,    40,
    45,    47,    51,    55,    57,    59,    61,    63,    65,    66,
    72,    75,    77,    81,    85,    86,    97,    98,   110,   111,
   123,   124,   127,   128,   140,   141,   153,   154,   166,   168,
   170,   172,   174,   176,   178,   179,   187,   188,   192,   193,
   194,   199,   200,   202,   204,   206,   208,   210,   212,   214,
   216,   218,   220,   222,   224,   228,   229,   233,   234,   235,
   236,   237,   247,   248,   251,   256,   258,   262,   264,   265,
   267,   271,   273,   277,   279,   281,   283,   285,   289,   293,
   296,   299,   303,   307,   309,   311,   312,   318,   320,   322,
   326,   330,   334,   337,   340,   344,   348,   352,   356,   360,
   364,   368,   372,   376,   380,   384,   387,   391,   395,   399,
   404,   408,   415,   422,   423,   429,   431,   433,   435
};

static const short yyrhs[] = {    -1,
    67,    68,     0,    69,     0,    68,    43,    69,     0,     0,
    71,     0,    72,     0,    73,     0,    74,     0,    78,     0,
    81,     0,    86,     0,    94,     0,    88,     0,    90,     0,
    98,     0,    70,     0,    61,     0,    61,   118,     0,     4,
    30,   118,    42,     0,     3,     0,    76,    16,   118,     0,
    76,    75,   118,     0,    62,     0,    63,     0,    64,     0,
    65,     0,    24,     0,     0,    24,    77,    30,   122,    42,
     0,     9,    79,     0,    80,     0,    79,     8,    80,     0,
   114,    16,   116,     0,     0,    10,   115,    16,   118,    92,
   118,    85,    43,    82,    69,     0,     0,    10,   115,    16,
   118,    92,   118,    85,    83,    50,    68,    51,     0,     0,
    20,   115,    16,   118,    92,   118,    85,    84,    10,    68,
    13,     0,     0,    93,   118,     0,     0,    49,   115,    16,
   118,    92,   118,    85,    87,    10,    68,    13,     0,     0,
    39,   115,    16,   118,    92,   118,    85,    89,    10,    68,
    13,     0,     0,    11,   115,    16,   118,    92,   118,    85,
    91,    10,    68,    13,     0,    48,     0,     8,     0,     7,
     0,     5,     0,     8,     0,     7,     0,     0,    25,   118,
    95,    47,    68,    96,    14,     0,     0,    12,    97,    68,
     0,     0,     0,   100,   101,    99,   102,     0,     0,    55,
     0,    53,     0,    54,     0,    52,     0,    58,     0,    59,
     0,    60,     0,    56,     0,    57,     0,    27,     0,    41,
     0,   103,     0,   102,     8,   103,     0,     0,   114,   104,
   109,     0,     0,     0,     0,     0,   114,   105,    30,   106,
   112,   107,    42,   108,   109,     0,     0,    16,   111,     0,
    16,    50,   110,    51,     0,   111,     0,   110,     8,   111,
     0,   118,     0,     0,   113,     0,   112,     8,   113,     0,
   118,     0,   118,     7,   118,     0,    24,     0,    24,     0,
    24,     0,    26,     0,   116,    40,   116,     0,   116,    33,
   116,     0,    33,   116,     0,    40,   116,     0,   116,    46,
   116,     0,    30,   116,    42,     0,    24,     0,   117,     0,
     0,   117,   119,    30,   122,    42,     0,    26,     0,    18,
     0,    30,   118,    42,     0,   118,    40,   118,     0,   118,
    33,   118,     0,    40,   118,     0,    33,   118,     0,   118,
    46,   118,     0,   118,    44,   118,     0,   118,    17,   118,
     0,   118,    29,   118,     0,   118,    28,   118,     0,   118,
    15,   118,     0,   118,    35,   118,     0,   118,    21,   118,
     0,   118,    22,   118,     0,   118,    36,   118,     0,   118,
    37,   118,     0,    38,   118,     0,   118,    34,   118,     0,
   118,    31,   118,     0,   118,    32,   118,     0,    45,    30,
   118,    42,     0,    23,    30,    42,     0,    19,    30,   118,
    44,   118,    42,     0,     6,    30,   118,    44,   118,    42,
     0,     0,   121,   120,    30,   122,    42,     0,    31,     0,
    32,     0,   118,     0,   122,     8,   118,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   164,   170,   178,   180,   184,   185,   186,   187,   188,   189,
   190,   191,   192,   193,   194,   195,   196,   199,   200,   203,
   216,   231,   255,   283,   284,   285,   286,   290,   317,   342,
   365,   369,   371,   375,   391,   395,   396,   399,   401,   404,
   409,   410,   414,   419,   425,   430,   436,   441,   447,   449,
   451,   455,   457,   459,   463,   476,   486,   494,   494,   497,
   498,   500,   501,   502,   503,   504,   505,   506,   507,   508,
   509,   512,   513,   517,   519,   523,   549,   549,   562,   562,
   563,   587,   590,   591,   592,   595,   596,   598,   602,   604,
   605,   608,   623,   639,   644,   649,   660,   663,   666,   669,
   672,   675,   678,   683,   688,   705,   721,   746,   754,   763,
   766,   769,   772,   775,   778,   781,   784,   787,   790,   793,
   796,   799,   802,   805,   808,   811,   814,   817,   820,   823,
   826,   831,   836,   841,   847,   864,   867,   872,   875
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","TCOMMENT",
"TASSERT","TBY","TCEILING","TCOLON","TCOMMA","TCONST","TDO","TDOANY","TELSE",
"TENDFOR","TENDIF","TEQ","TEQUAL","TEXP","TFLOAT","TFLOOR","TFOR","TGE","TGT",
"TRAND","TID","TIF","TINT","TINTEGER","TLE","TLT","TLP","TMAX","TMIN","TMINUS",
"TMOD","TNE","TAND","TOR","TNOT","TPARFOR","TPLUS","TREAL","TRP","TSEMI","TSLASH",
"TSQRT","TSTAR","TTHEN","TTO","TVECFOR","TLCB","TRCB","TFORMAL","TCOMMON","TSTATIC",
"TAUTO","TPRIVATE","TBUILTIN","TIN","TOUT","TINOUT","TRETURN","TPLUSEQUAL","TSTAREQUAL",
"TMAXEQUAL","TMINEQUAL","pgm","@1","stlist","stmt","streturn","stassert","stcomment",
"stasgn","stasgnoper","asgnoper","lhs","@2","stconst","constlist","constdecl",
"stfor","@3","@4","@5","optforby","stforall","@6","stparfor","@7","stdoany",
"@8","forto","forby","stif","@9","elsepart","@10","stdecl","@11","class","type",
"decllist","decl","@12","@13","@14","@15","@16","init","initlist","init1","boundlist",
"bound","newid","doid","cexpr","oldid","expr","@17","@18","assocop","list", NULL
};
#endif

static const short yyr1[] = {     0,
    67,    66,    68,    68,    69,    69,    69,    69,    69,    69,
    69,    69,    69,    69,    69,    69,    69,    70,    70,    71,
    72,    73,    74,    75,    75,    75,    75,    76,    77,    76,
    78,    79,    79,    80,    82,    81,    83,    81,    84,    81,
    85,    85,    87,    86,    89,    88,    91,    90,    92,    92,
    92,    93,    93,    93,    95,    94,    97,    96,    96,    99,
    98,   100,   100,   100,   100,   100,   100,   100,   100,   100,
   100,   101,   101,   102,   102,   104,   103,   105,   106,   107,
   108,   103,   109,   109,   109,   110,   110,   111,   112,   112,
   112,   113,   113,   114,   115,   116,   116,   116,   116,   116,
   116,   116,   116,   117,   118,   119,   118,   118,   118,   118,
   118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
   118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
   118,   118,   118,   120,   118,   121,   121,   122,   122
};

static const short yyr2[] = {     0,
     0,     2,     1,     3,     0,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     2,     4,
     1,     3,     3,     1,     1,     1,     1,     1,     0,     5,
     2,     1,     3,     3,     0,    10,     0,    11,     0,    11,
     0,     2,     0,    11,     0,    11,     0,    11,     1,     1,
     1,     1,     1,     1,     0,     7,     0,     3,     0,     0,
     4,     0,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     3,     0,     3,     0,     0,     0,
     0,     9,     0,     2,     4,     1,     3,     1,     0,     1,
     3,     1,     3,     1,     1,     1,     1,     3,     3,     2,
     2,     3,     3,     1,     1,     0,     5,     1,     1,     3,
     3,     3,     2,     2,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     2,     3,     3,     3,     4,
     3,     6,     6,     0,     5,     1,     1,     1,     3
};

static const short yydefact[] = {     1,
     5,    21,     0,     0,     0,     0,     0,    28,     0,     0,
     0,    66,    64,    65,    63,    70,    71,    67,    68,    69,
    18,     2,     3,    17,     6,     7,     8,     9,     0,    10,
    11,    12,    14,    15,    13,    16,     0,     0,    94,    31,
    32,     0,    95,     0,     0,     0,     0,     0,   109,     0,
     0,   104,   108,     0,   136,   137,     0,     0,     0,     0,
   105,    55,   134,     0,     0,    19,     5,     0,    24,    25,
    26,    27,     0,    72,    73,    60,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   114,   126,   113,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     4,    22,    23,     0,    20,    33,    96,    97,
     0,     0,     0,    34,     0,     0,     0,   138,     0,     0,
     0,   131,   110,     0,     0,   120,   117,   122,   123,   119,
   118,   128,   129,   112,   127,   121,   124,   125,   111,   116,
   115,     5,     0,     0,     0,    61,    74,    76,     0,   100,
   101,     0,     0,     0,    51,    50,    49,     0,     0,     0,
     0,    30,     0,     0,   130,     0,    59,     0,     0,     0,
     0,    83,     0,   103,    99,    98,   102,    41,    41,    41,
   139,   116,   116,   107,    57,     0,   135,    41,    41,    75,
     0,    77,    79,    52,    54,    53,    37,     0,    47,    39,
   133,   132,     5,    56,    45,    43,     0,    84,    88,    89,
    35,     0,    42,     0,     0,    58,     0,     0,     0,    86,
    80,    90,    92,     5,     5,     5,     5,     5,     5,     0,
    85,     0,     0,     0,    36,     0,     0,     0,     0,     0,
    87,    91,    81,    93,    38,    48,    40,    46,    44,    83,
    82,     0,     0,     0
};

static const short yydefgoto[] = {   262,
     1,    22,    23,    24,    25,    26,    27,    28,    73,    29,
    47,    30,    40,    41,    31,   234,   222,   225,   207,    32,
   228,    33,   227,    34,   224,   168,   208,    35,   109,   196,
   213,    36,   116,    37,    76,   156,   157,   182,   183,   220,
   243,   260,   202,   229,   218,   231,   232,    42,    44,   124,
    61,   128,    92,   110,    63,   129
};

static const short yypact[] = {-32768,
   128,-32768,   -17,    44,    50,    50,    50,   -13,     2,    50,
    50,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
     2,   -24,-32768,-32768,-32768,-32768,-32768,-32768,   -11,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    38,     2,-32768,    15,
-32768,    54,-32768,    94,    99,   100,    81,    90,-32768,    92,
    93,-32768,-32768,     2,-32768,-32768,     2,     2,     2,   104,
   106,   358,-32768,   108,   114,   358,   128,     2,-32768,-32768,
-32768,-32768,     2,-32768,-32768,-32768,   280,    44,   174,     2,
     2,     2,     2,     2,     2,   105,   306,   112,   462,   112,
     2,   111,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,   102,   124,
     2,     2,-32768,   358,   358,    44,-32768,-32768,-32768,-32768,
   174,   174,   174,   -22,   188,   188,   188,   358,    -4,   384,
   410,-32768,-32768,   332,     2,   483,   125,   483,   483,   483,
   483,    -7,    -7,   112,    83,   483,   462,   436,   112,    46,
    46,   128,     2,   188,   188,   149,-32768,   133,   232,   120,
   120,   174,   174,   174,-32768,-32768,-32768,     2,     2,     2,
     2,-32768,     2,     2,-32768,    -1,    -6,     1,     2,     2,
    44,   156,   143,-32768,   120,   120,-32768,   222,   222,   222,
   358,   159,   490,-32768,-32768,   160,-32768,   222,   222,-32768,
    95,-32768,-32768,-32768,-32768,-32768,   132,     2,-32768,-32768,
-32768,-32768,   128,-32768,-32768,-32768,     2,-32768,   358,     2,
-32768,   144,   358,   187,   192,   -24,   196,   202,    -5,-32768,
   205,-32768,   254,   128,   128,   128,   128,   128,   128,     2,
-32768,     2,   173,     2,-32768,    61,    17,    18,    23,    26,
-32768,-32768,-32768,   358,-32768,-32768,-32768,-32768,-32768,   156,
-32768,   218,   226,-32768
};

static const short yypgoto[] = {-32768,
-32768,  -130,   -66,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   162,-32768,-32768,-32768,-32768,   336,-32768,
-32768,-32768,-32768,-32768,-32768,    24,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    57,-32768,-32768,-32768,
-32768,-32768,   -12,-32768,  -159,-32768,     5,  -114,   235,  -107,
-32768,    -9,-32768,-32768,-32768,   -91
};


#define	YYLAST		535


static const short yytable[] = {    62,
   113,   158,   240,   171,    68,   195,   171,    48,   171,    94,
   162,    66,    38,   159,   160,   161,   -29,   163,    67,    49,
    50,   177,    78,   164,    51,    52,   102,    53,    77,   256,
   257,    54,    55,    56,    57,   258,    67,   172,   259,    58,
   194,    59,   197,   176,    87,   241,    60,    88,    89,    90,
    69,    70,    71,    72,   185,   186,   187,   230,   114,    67,
    67,   178,    94,   115,    74,    67,   158,    39,    67,    79,
   125,   126,   127,    43,   130,   131,    99,   100,    75,   102,
   251,   134,   226,   136,   137,   138,   139,   140,   141,   142,
   143,   144,   145,   146,   147,   148,   149,   150,   151,    94,
    48,   154,   155,    67,   246,   247,   248,   249,   250,    80,
    83,   255,    49,    50,    81,    82,-32768,    51,    52,    84,
    53,    85,    86,   111,    54,    55,    56,    57,    94,   112,
     2,     3,    58,    91,    59,  -106,     4,     5,     6,    60,
   135,    94,    99,   100,   217,   102,   132,     7,   152,   169,
   170,     8,     9,   153,   -62,   107,   181,   108,   188,   189,
   190,   191,   -78,   192,   193,   164,    10,   245,   -62,   198,
   199,   201,   203,   214,   221,    94,    11,   179,   180,    12,
    13,    14,    15,    16,    17,    18,    19,    20,    21,    99,
   100,   219,   102,   235,   165,   166,   236,   119,   223,   120,
   211,   237,    93,   121,    94,   238,   122,   219,    95,    96,
   233,   239,   242,   123,   253,    97,    98,   263,    99,   100,
   101,   102,   103,   104,   105,   264,   204,   106,   205,   206,
   219,   107,   233,   108,   254,   167,    93,   200,    94,   118,
    45,    46,    95,    96,    64,    65,   252,   261,     0,    97,
    98,     0,    99,   100,   101,   102,   103,   104,   105,     0,
   244,   106,     0,     0,   162,   107,     0,   108,    93,     0,
    94,   163,     0,   184,    95,    96,     0,   164,     0,     0,
     0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
   105,     0,     0,   106,    93,     0,    94,   107,     0,   108,
    95,    96,     0,     0,     0,     0,     0,    97,    98,     0,
    99,   100,   101,   102,   103,   104,   105,     0,     0,   106,
    93,   117,    94,   107,     0,   108,    95,    96,     0,     0,
     0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
   103,   104,   105,     0,     0,   106,    93,   133,    94,   107,
     0,   108,    95,    96,     0,     0,     0,     0,     0,    97,
    98,     0,    99,   100,   101,   102,   103,   104,   105,     0,
     0,   106,    93,   175,    94,   107,     0,   108,    95,    96,
     0,     0,     0,     0,     0,    97,    98,     0,    99,   100,
   101,   102,   103,   104,   105,     0,     0,   106,    93,     0,
    94,   107,     0,   108,    95,    96,     0,     0,     0,     0,
     0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
   105,     0,     0,   106,    93,     0,    94,   173,     0,   108,
    95,    96,     0,     0,     0,     0,     0,    97,    98,     0,
    99,   100,   101,   102,   103,   104,   105,     0,     0,   106,
    93,     0,    94,   174,     0,   108,    95,    96,     0,     0,
     0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
   103,   104,     0,     0,     0,   106,    93,     0,    94,   107,
     0,   108,    95,    96,     0,     0,     0,     0,     0,    97,
    98,     0,    99,   100,   101,   102,   103,-32768,     0,    94,
     0,   106,     0,-32768,-32768,   107,    94,   108,     0,     0,
-32768,-32768,     0,    99,   100,   101,   102,-32768,     0,     0,
    99,   100,   106,   102,   209,   210,   107,     0,   108,     0,
     0,   212,     0,   215,   216
};

static const short yycheck[] = {     9,
    67,   116,     8,     8,    16,    12,     8,     6,     8,    17,
    33,    21,    30,   121,   122,   123,    30,    40,    43,    18,
    19,   152,     8,    46,    23,    24,    34,    26,    38,    13,
    13,    30,    31,    32,    33,    13,    43,    42,    13,    38,
    42,    40,    42,   135,    54,    51,    45,    57,    58,    59,
    62,    63,    64,    65,   162,   163,   164,   217,    68,    43,
    43,   153,    17,    73,    27,    43,   181,    24,    43,    16,
    80,    81,    82,    24,    84,    85,    31,    32,    41,    34,
   240,    91,   213,    93,    94,    95,    96,    97,    98,    99,
   100,   101,   102,   103,   104,   105,   106,   107,   108,    17,
     6,   111,   112,    43,   235,   236,   237,   238,   239,    16,
    30,    51,    18,    19,    16,    16,    34,    23,    24,    30,
    26,    30,    30,    16,    30,    31,    32,    33,    17,    16,
     3,     4,    38,    30,    40,    30,     9,    10,    11,    45,
    30,    17,    31,    32,    50,    34,    42,    20,    47,   126,
   127,    24,    25,    30,    27,    44,     8,    46,   168,   169,
   170,   171,    30,   173,   174,    46,    39,   234,    41,   179,
   180,    16,    30,    14,    43,    17,    49,   154,   155,    52,
    53,    54,    55,    56,    57,    58,    59,    60,    61,    31,
    32,   201,    34,    50,     7,     8,    10,    24,   208,    26,
    42,    10,    15,    30,    17,    10,    33,   217,    21,    22,
   220,    10,     8,    40,    42,    28,    29,     0,    31,    32,
    33,    34,    35,    36,    37,     0,     5,    40,     7,     8,
   240,    44,   242,    46,   244,    48,    15,   181,    17,    78,
     6,     7,    21,    22,    10,    11,   242,   260,    -1,    28,
    29,    -1,    31,    32,    33,    34,    35,    36,    37,    -1,
     7,    40,    -1,    -1,    33,    44,    -1,    46,    15,    -1,
    17,    40,    -1,    42,    21,    22,    -1,    46,    -1,    -1,
    -1,    28,    29,    -1,    31,    32,    33,    34,    35,    36,
    37,    -1,    -1,    40,    15,    -1,    17,    44,    -1,    46,
    21,    22,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,
    31,    32,    33,    34,    35,    36,    37,    -1,    -1,    40,
    15,    42,    17,    44,    -1,    46,    21,    22,    -1,    -1,
    -1,    -1,    -1,    28,    29,    -1,    31,    32,    33,    34,
    35,    36,    37,    -1,    -1,    40,    15,    42,    17,    44,
    -1,    46,    21,    22,    -1,    -1,    -1,    -1,    -1,    28,
    29,    -1,    31,    32,    33,    34,    35,    36,    37,    -1,
    -1,    40,    15,    42,    17,    44,    -1,    46,    21,    22,
    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    31,    32,
    33,    34,    35,    36,    37,    -1,    -1,    40,    15,    -1,
    17,    44,    -1,    46,    21,    22,    -1,    -1,    -1,    -1,
    -1,    28,    29,    -1,    31,    32,    33,    34,    35,    36,
    37,    -1,    -1,    40,    15,    -1,    17,    44,    -1,    46,
    21,    22,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,
    31,    32,    33,    34,    35,    36,    37,    -1,    -1,    40,
    15,    -1,    17,    44,    -1,    46,    21,    22,    -1,    -1,
    -1,    -1,    -1,    28,    29,    -1,    31,    32,    33,    34,
    35,    36,    -1,    -1,    -1,    40,    15,    -1,    17,    44,
    -1,    46,    21,    22,    -1,    -1,    -1,    -1,    -1,    28,
    29,    -1,    31,    32,    33,    34,    35,    15,    -1,    17,
    -1,    40,    -1,    21,    22,    44,    17,    46,    -1,    -1,
    28,    29,    -1,    31,    32,    33,    34,    35,    -1,    -1,
    31,    32,    40,    34,   189,   190,    44,    -1,    46,    -1,
    -1,    42,    -1,   198,   199
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
#line 165 "../src/petity.y"
{
	  parse_init();
	  ShouldBeEnd = 0;
	;
    break;}
case 2:
#line 170 "../src/petity.y"
{
	  P_Node = make_node(op_exit);
	  ExitNode = P_Node;
	  parse_linknode(P_Node);
	;
    break;}
case 8:
#line 187 "../src/petity.y"
{CheckEnd();;
    break;}
case 9:
#line 188 "../src/petity.y"
{CheckEnd();;
    break;}
case 11:
#line 190 "../src/petity.y"
{CheckEnd();;
    break;}
case 12:
#line 191 "../src/petity.y"
{CheckEnd();;
    break;}
case 13:
#line 192 "../src/petity.y"
{CheckEnd();;
    break;}
case 14:
#line 193 "../src/petity.y"
{CheckEnd();;
    break;}
case 15:
#line 194 "../src/petity.y"
{CheckEnd();;
    break;}
case 17:
#line 196 "../src/petity.y"
{ShouldBeEnd=1;;
    break;}
case 18:
#line 199 "../src/petity.y"
{;
    break;}
case 19:
#line 200 "../src/petity.y"
{free_node(yyvsp[0].np);;
    break;}
case 20:
#line 204 "../src/petity.y"
{
		    parse_check_stmt_number();   /* what the heck... */
		    P_Node = make_node( op_assert );
		    P_Node->nodelink = Assertions;
		    Assertions = P_Node;
		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );
		    parse_linknode( yyvsp[-1].np );
		    parse_popscope();
		;
    break;}
case 21:
#line 216 "../src/petity.y"
{
                if(1) {
		    P_Sym = parse_symbol(-1);
		    P_Node = make_node( op_comment );
		    P_Node->nodevalue = (unsigned long int)P_Sym;
		    parse_linknode( P_Node );
		    P_Sym->symtype = symcomment;
		    P_Sym->symdims = (unsigned) -1;
		    P_Sym->symdecl = P_Node;

		    P_Sym->exptype = exprint; /* cexpr's are only integers */
		  }
		;
    break;}
case 22:
#line 233 "../src/petity.y"
{
		  node *rp = yyvsp[0].np;

		  parse_check_stmt_number();
		  P_Node = make_node( op_assign );
		  
		  /* make sure types are compatible */
		  /* note: refreal can accept any type */
		  if( yyvsp[-2].np->exptype == exprrefint && rp->exptype != exprint ) {
		    rp = make_node(op_rtoi);
		    rp->exptype = exprrefint;
		    rp->nodechild = yyvsp[0].np;
		    (yyvsp[0].np)->nodeparent = rp;
		  }
		  
		  parse_linknodeto( P_Node, rp );
		  parse_linknodeto( P_Node, yyvsp[-2].np );
		  parse_linknode( P_Node );
		;
    break;}
case 23:
#line 256 "../src/petity.y"
{
		  optype *pop;
		  
		  parse_check_stmt_number();
		  P_Node = make_node(yyvsp[-1].op);
		  
		  /* make sure types are compatible */
		  /* note: refreal can accept any type */
		  if( yyvsp[-2].np->exptype == exprrefint && yyvsp[0].np->exptype != exprint ) {
		    fprintf(stderr, "*** can't assign real to integer\n");
		    parse_die();
		  }

		  parse_linknodeto( P_Node, yyvsp[0].np );
		  parse_linknodeto( P_Node, yyvsp[-2].np );
		  parse_linknode( P_Node );

		  pop = &P_Node->nodechild->nodenext->nodeop;
		  if(*pop==op_store) {
		    *pop = op_update;
		  } else if(*pop==op_store_array) {
		    *pop = op_update_array;
		  }
                ;
    break;}
case 24:
#line 283 "../src/petity.y"
{yyval.op = op_add_assign;;
    break;}
case 25:
#line 284 "../src/petity.y"
{yyval.op = op_mul_assign;;
    break;}
case 26:
#line 285 "../src/petity.y"
{yyval.op = op_max_assign;;
    break;}
case 27:
#line 286 "../src/petity.y"
{yyval.op = op_min_assign;;
    break;}
case 28:
#line 292 "../src/petity.y"
{
		    P_Sym = parse_symbol(0);
		    if( P_Sym->symdims > 0 ){
			fprintf(stderr, "*** lhs array must have %d subscripts: %s\n", 
				P_Sym->symdims, P_Sym->symname );
			parse_die();
		    }
		    if( P_Sym->symtype == symconst ){
			fprintf(stderr, "*** lhs must be variable: %s\n",
				P_Sym->symname );
			parse_die();
		    }
		    if( P_Sym->symtype == symindex ){
			fprintf(stderr, "*** lhs cannot be loop index: %s\n",
				P_Sym->symname );
			parse_die();
		    }

		    P_Node = make_node( op_store );
		    P_Node->nodevalue = (unsigned long int)P_Sym->symdecl;

		    P_Node->exptype = P_Sym->exptype; /* refint or refreal */

		    yyval.np = P_Node;
		;
    break;}
case 29:
#line 319 "../src/petity.y"
{
		    P_Sym = parse_symbol(0);
		    if( P_Sym->symdims <= 0 ){
			fprintf(stderr, "*** not an array: %s\n", P_Sym->symname );
			parse_die();
		    }
		    P_Node = make_node( op_store_array );
		    P_Node->nodevalue = (unsigned long int)P_Sym->symdecl;

		    if( P_Sym->exptype == exprintarray )
			P_Node->exptype = exprrefint;
		    else
		    if( P_Sym->exptype == exprrealarray )
			P_Node->exptype = exprrefreal;
		    else
			P_Node->exptype = exprunknown;

		    parse_linknode( P_Node );
		    parse_pushscope( P_Node ); 
		    yyval.np = P_Node;
		    /* save it because it can be reassigned down there */
		    P_Sym1 = P_Sym; 
		;
    break;}
case 30:
#line 343 "../src/petity.y"
{
		    /* make sure all subscripts are integer expressions */
		    int count = 0;
		    for( P_Node = (yyvsp[-3].np)->nodechild; P_Node != NULL;
				      P_Node = P_Node->nodenext ){
		        count++;
			if( P_Node->exptype != exprint ) {
			    fprintf(stderr, "*** All subscripts must be integer\n" );
			    parse_die();
			}
		    }
		    if(count != (int)P_Sym1->symdims) {
			fprintf(stderr, "*** Wrong number of subscripts in %s (store)\n", P_Sym1->symname );
			parse_die();
		    }

		    parse_popscope();
		    parse_unlinknode();
		    yyval.np = yyvsp[-3].np;
		;
    break;}
case 31:
#line 366 "../src/petity.y"
{;
    break;}
case 34:
#line 377 "../src/petity.y"
{
		    P_Sym = yyvsp[-2].stp;
		    P_Node = make_node( op_declare );
		    P_Node->nodevalue = (unsigned long int)P_Sym;
		    parse_linknode( P_Node );
		    P_Sym->symtype = symconst;
		    P_Sym->symdims = (unsigned) -1;
		    P_Sym->symvalue = yyvsp[0].iv;
		    P_Sym->symdecl = P_Node;

		    P_Sym->exptype = exprint; /* cexpr's are only integers */
		;
    break;}
case 35:
#line 393 "../src/petity.y"
{ parse_forloop( op_do, yyvsp[-6].stp, yyvsp[-4].np, yyvsp[-2].np, yyvsp[-1].np ); ;
    break;}
case 36:
#line 395 "../src/petity.y"
{ parse_endforloop(); ;
    break;}
case 37:
#line 398 "../src/petity.y"
{ parse_forloop( op_do, yyvsp[-5].stp, yyvsp[-3].np, yyvsp[-1].np, yyvsp[0].np ); ;
    break;}
case 38:
#line 400 "../src/petity.y"
{ parse_endforloop(); ;
    break;}
case 39:
#line 403 "../src/petity.y"
{ parse_forloop( op_do, yyvsp[-5].stp, yyvsp[-3].np, yyvsp[-1].np, yyvsp[0].np ); ;
    break;}
case 40:
#line 405 "../src/petity.y"
{ parse_endforloop(); ;
    break;}
case 41:
#line 409 "../src/petity.y"
{yyval.np = NULL;;
    break;}
case 42:
#line 410 "../src/petity.y"
{yyval.np = yyvsp[0].np;;
    break;}
case 43:
#line 416 "../src/petity.y"
{
		    parse_forloop( op_dovec, yyvsp[-5].stp, yyvsp[-3].np, yyvsp[-1].np, yyvsp[0].np );
		;
    break;}
case 44:
#line 420 "../src/petity.y"
{
		    parse_endforloop();
		;
    break;}
case 45:
#line 427 "../src/petity.y"
{
		    parse_forloop( op_dopar, yyvsp[-5].stp, yyvsp[-3].np, yyvsp[-1].np, yyvsp[0].np );
		;
    break;}
case 46:
#line 431 "../src/petity.y"
{
		    parse_endforloop();
		;
    break;}
case 47:
#line 438 "../src/petity.y"
{
		    parse_forloop( op_doany, yyvsp[-5].stp, yyvsp[-3].np, yyvsp[-1].np, yyvsp[0].np );
		;
    break;}
case 48:
#line 442 "../src/petity.y"
{
		    parse_endforloop();
		;
    break;}
case 49:
#line 448 "../src/petity.y"
{;
    break;}
case 50:
#line 450 "../src/petity.y"
{;
    break;}
case 51:
#line 452 "../src/petity.y"
{;
    break;}
case 52:
#line 456 "../src/petity.y"
{;
    break;}
case 53:
#line 458 "../src/petity.y"
{;
    break;}
case 54:
#line 460 "../src/petity.y"
{;
    break;}
case 55:
#line 465 "../src/petity.y"
{
		    parse_check_stmt_number();
		    P_Node = make_node( op_if );

		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );
		    parse_linknode( yyvsp[0].np );
		    P_Node = make_node( op_then );
		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );
		;
    break;}
case 56:
#line 479 "../src/petity.y"
{
		    parse_popscope();
		    parse_popscope();
		;
    break;}
case 57:
#line 488 "../src/petity.y"
{   parse_popscope();
		    P_Node = make_node( op_else );
		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );
		;
    break;}
case 60:
#line 497 "../src/petity.y"
{SaveClass=(var_class) yyvsp[-1].iv; P_SaveType=(symboltype) yyvsp[0].iv;;
    break;}
case 62:
#line 500 "../src/petity.y"
{yyval.iv = CLunknown;;
    break;}
case 63:
#line 501 "../src/petity.y"
{yyval.iv = CLAUTO;;
    break;}
case 64:
#line 502 "../src/petity.y"
{yyval.iv = CLCOMMON;;
    break;}
case 65:
#line 503 "../src/petity.y"
{yyval.iv = CLSTATIC;;
    break;}
case 66:
#line 504 "../src/petity.y"
{yyval.iv = CLFORMAL;;
    break;}
case 67:
#line 505 "../src/petity.y"
{yyval.iv = CLIN;;
    break;}
case 68:
#line 506 "../src/petity.y"
{yyval.iv = CLOUT;;
    break;}
case 69:
#line 507 "../src/petity.y"
{yyval.iv = CLINOUT;;
    break;}
case 70:
#line 508 "../src/petity.y"
{yyval.iv = CLPRIVATE;;
    break;}
case 71:
#line 509 "../src/petity.y"
{yyval.iv = CLBUILTIN;;
    break;}
case 72:
#line 512 "../src/petity.y"
{yyval.iv=syminteger;;
    break;}
case 73:
#line 513 "../src/petity.y"
{yyval.iv=symreal;;
    break;}
case 76:
#line 525 "../src/petity.y"
{
		    P_DeclSym = yyvsp[0].stp;
		    P_Node = make_node( op_declare );
		    P_Node->nodevalue = (unsigned long int)P_DeclSym;
		    parse_linknode( P_Node );
		    P_DeclSym->symtype = P_SaveType;
		    P_DeclSym->symdims = 0;
		    P_DeclSym->symdecl = P_Node;
		    if(SaveClass == CLBUILTIN) {
		      fprintf(stderr, "Not function %s\n", yyvsp[0].stp->symname);
		      parse_die();
		    }
		    P_DeclSym->symclass =
		      SaveClass==CLunknown ? CLAUTO : SaveClass;

		    /* determine the type based on declaration */
		    if( P_SaveType == syminteger )
			P_DeclSym->exptype = exprrefint;
		    else if( P_SaveType == symreal )
			P_DeclSym->exptype = exprrefreal;
		    else
			P_DeclSym->exptype = exprunknown;
		;
    break;}
case 78:
#line 551 "../src/petity.y"
{
		    P_DeclSym = yyvsp[0].stp;
		    P_Node = make_node( op_declare );
		    P_Node->nodevalue = (unsigned long int)P_DeclSym;
		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );
		    P_DeclSym->symtype = P_SaveType;
		    P_DeclSym->symdecl = P_Node;
		    P_DeclSym->symclass = SaveClass==CLunknown ?
		      (1 ? CLINOUT : CLAUTO) : SaveClass;
		;
    break;}
case 79:
#line 562 "../src/petity.y"
{NoNewOldCheck=1;;
    break;}
case 80:
#line 562 "../src/petity.y"
{NoNewOldCheck=0;;
    break;}
case 81:
#line 563 "../src/petity.y"
{
		    /* determine the type based on declaration */
		    if(P_DeclSym->symdims!=DIMSFUN || SaveClass!=CLBUILTIN) {
		      if(P_DeclSym->symdims==DIMSFUN) {
			P_DeclSym->symdims = 1;
		      }
		      if( P_SaveType == syminteger )
			P_DeclSym->exptype = exprintarray;
		      else if( P_SaveType == symreal )
			P_DeclSym->exptype = exprrealarray;
		      else
			P_DeclSym->exptype = exprunknown;
		    } else {
		      if( P_SaveType == syminteger )
			P_DeclSym->exptype = exprintfun;
		      else if( P_SaveType == symreal )
			P_DeclSym->exptype = exprrealfun;
		      else
			P_DeclSym->exptype = exprunknown;
		    }

		    parse_popscope();
		;
    break;}
case 84:
#line 591 "../src/petity.y"
{;
    break;}
case 85:
#line 592 "../src/petity.y"
{;
    break;}
case 88:
#line 598 "../src/petity.y"
{free_node(yyvsp[0].np);;
    break;}
case 89:
#line 603 "../src/petity.y"
{   P_DeclSym->symdims = DIMSFUN; ;
    break;}
case 90:
#line 604 "../src/petity.y"
{   P_DeclSym->symdims = 1; ;
    break;}
case 91:
#line 605 "../src/petity.y"
{   P_DeclSym->symdims++; ;
    break;}
case 92:
#line 610 "../src/petity.y"
{
		    P_Node = make_node( op_bounds );
		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );

		    P_Node = make_node( op_constant );
		    P_Node->nodevalue = 1;
		    parse_linknode( P_Node );

		    P_Node = yyvsp[0].np;
		    parse_linknode( P_Node );
		    parse_popscope();
		;
    break;}
case 93:
#line 625 "../src/petity.y"
{
		    P_Node = make_node( op_bounds );
		    parse_linknode( P_Node );
		    parse_pushscope( P_Node );

		    P_Node = yyvsp[-2].np;
		    parse_linknode( P_Node );

		    P_Node = yyvsp[0].np;
		    parse_linknode( P_Node );
		    parse_popscope();
		;
    break;}
case 94:
#line 641 "../src/petity.y"
{   yyval.stp = parse_symbol(1); ;
    break;}
case 95:
#line 646 "../src/petity.y"
{   yyval.stp = parse_doid_symbol(); ;
    break;}
case 96:
#line 651 "../src/petity.y"
{   
		    P_Sym = parse_symbol(0);
		    yyval.iv = P_Sym->symvalue;
		    /* don't allow variables in constant expressions */
		    if( P_Sym->symtype != symconst ){
			fprintf(stderr, "*** not a constant: %s\n", P_Sym->symname );
			parse_die();
		    }
		;
    break;}
case 97:
#line 662 "../src/petity.y"
{   yyval.iv = parse_intval(); ;
    break;}
case 98:
#line 665 "../src/petity.y"
{   yyval.iv = yyvsp[-2].iv + yyvsp[0].iv; ;
    break;}
case 99:
#line 668 "../src/petity.y"
{   yyval.iv = yyvsp[-2].iv - yyvsp[0].iv; ;
    break;}
case 100:
#line 671 "../src/petity.y"
{   yyval.iv = - yyvsp[0].iv; ;
    break;}
case 101:
#line 674 "../src/petity.y"
{   yyval.iv = yyvsp[0].iv; ;
    break;}
case 102:
#line 677 "../src/petity.y"
{   yyval.iv = yyvsp[-2].iv * yyvsp[0].iv; ;
    break;}
case 103:
#line 680 "../src/petity.y"
{   yyval.iv = yyvsp[-1].iv; ;
    break;}
case 104:
#line 685 "../src/petity.y"
{   yyval.np = parse_symbolnode(); ;
    break;}
case 105:
#line 690 "../src/petity.y"
{
		    yyval.np = yyvsp[0].np;
		    if(yyvsp[0].np->nodeop == op_fetch_array) {
		        P_Node = (node*)yyvsp[0].np->nodevalue;
			P_Sym = (symtabentry*)P_Node->nodevalue;
			fprintf(stderr,
				"*** array must have %d subscripts: %s\n",
				P_Sym->symdims, P_Sym->symname );
			parse_die();
		    }
		    if(yyvsp[0].np->nodevalue == 0) {
		        fprintf(stderr,	"*** You use variable (loop index) which is not completely declared ***\n");
			parse_die();
		    }
		;
    break;}
case 106:
#line 707 "../src/petity.y"
{
	            P_Node = (node*)yyvsp[0].np->nodevalue;
		    P_Sym = (symtabentry*)P_Node->nodevalue;
		    if( yyvsp[0].np->nodeop == op_fetch ){
			fprintf(stderr, "*** not an array or function call: %s\n", P_Sym->symname );
			parse_die();
		    }
		    else if( yyvsp[0].np->nodeop == op_index ){
			fprintf(stderr, "*** not an array or function call: %s\n", P_Sym->symname );
			parse_die();
		    }
		    parse_linknode( yyvsp[0].np );
		    parse_pushscope( yyvsp[0].np );
		;
    break;}
case 107:
#line 722 "../src/petity.y"
{   parse_popscope();
		    parse_unlinknode();

	            P_Node = (node*)yyvsp[-4].np->nodevalue;
		    P_Sym = (symtabentry*)P_Node->nodevalue;
		    if(yyvsp[-4].np->nodeop != op_call) {
		      int count = 0;
		      /* make sure all subscripts are integer expressions */
		      for( P_Node = yyvsp[-4].np->nodechild; P_Node != NULL;
			  P_Node = P_Node->nodenext ) {
			count++;
			if( P_Node->exptype != exprint ) {
			  fprintf(stderr, "*** All subscripts must be integer\n" );
			  parse_die();
			}
		      }
		      if(count != (int)P_Sym->symdims) {
			fprintf(stderr, "*** Wrong number of subscripts in %s (load)\n", P_Sym->symname );
			parse_die();
		      }
		    }

		    yyval.np = yyvsp[-4].np;
		;
    break;}
case 108:
#line 748 "../src/petity.y"
{
		    P_Node = make_node( op_constant );
		    P_Node->nodevalue = parse_intval();
		    P_Node->exptype = exprint;
		    yyval.np = P_Node;
		;
    break;}
case 109:
#line 756 "../src/petity.y"
{
		    P_Sym = parse_symbol(-1);
		    P_Node = make_node( op_real );
		    P_Node->nodevalue = (unsigned long int)P_Sym;
		    P_Node->exptype = exprreal;
		    yyval.np = P_Node;
		;
    break;}
case 110:
#line 765 "../src/petity.y"
{   yyval.np = yyvsp[-1].np; ;
    break;}
case 111:
#line 768 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_add,yyvsp[0].np); ;
    break;}
case 112:
#line 771 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_subtract,yyvsp[0].np); ;
    break;}
case 113:
#line 774 "../src/petity.y"
{   yyval.np = yyvsp[0].np; ;
    break;}
case 114:
#line 777 "../src/petity.y"
{   yyval.np = parse_binaryop(make_const(0),op_subtract,yyvsp[0].np); ;
    break;}
case 115:
#line 780 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_multiply,yyvsp[0].np); ;
    break;}
case 116:
#line 783 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_divide,yyvsp[0].np); ;
    break;}
case 117:
#line 786 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_exp,yyvsp[0].np); ;
    break;}
case 118:
#line 789 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_lt,yyvsp[0].np); ;
    break;}
case 119:
#line 792 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_le,yyvsp[0].np); ;
    break;}
case 120:
#line 795 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_eq,yyvsp[0].np); ;
    break;}
case 121:
#line 798 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_ne,yyvsp[0].np); ;
    break;}
case 122:
#line 801 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_ge,yyvsp[0].np); ;
    break;}
case 123:
#line 804 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_gt,yyvsp[0].np); ;
    break;}
case 124:
#line 807 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_and,yyvsp[0].np); ;
    break;}
case 125:
#line 810 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_or,yyvsp[0].np); ;
    break;}
case 126:
#line 813 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[0].np,op_not,NULL); ;
    break;}
case 127:
#line 816 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_mod,yyvsp[0].np); ;
    break;}
case 128:
#line 819 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_max,yyvsp[0].np); ;
    break;}
case 129:
#line 822 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-2].np,op_min,yyvsp[0].np); ;
    break;}
case 130:
#line 825 "../src/petity.y"
{   yyval.np = parse_binaryop(NULL,op_sqrt,yyvsp[-1].np); ;
    break;}
case 131:
#line 828 "../src/petity.y"
{   yyval.np = make_node( op_rand );
                    yyval.np->exptype = exprint;
                ;
    break;}
case 132:
#line 833 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-3].np,op_floor_divide,yyvsp[-1].np);
		    yyval.np->exptype = exprint;
		;
    break;}
case 133:
#line 838 "../src/petity.y"
{   yyval.np = parse_binaryop(yyvsp[-3].np,op_ceiling_divide,yyvsp[-1].np);
		    yyval.np->exptype = exprint;
		;
    break;}
case 134:
#line 843 "../src/petity.y"
{
		    parse_linknode( yyvsp[0].np );
		    parse_pushscope( yyvsp[0].np );
		;
    break;}
case 135:
#line 848 "../src/petity.y"
{
		    parse_popscope();
		    parse_unlinknode();

		    /* if any of the list's expressions is real, so is this */
		    yyvsp[-4].np->exptype = exprint;
		    for( P_Node = yyvsp[-4].np->nodechild;
			 P_Node != NULL; P_Node = P_Node->nodenext ) {
			if( P_Node->exptype != exprint ) {
			    yyvsp[-4].np->exptype = exprreal;
			}
		    }
		    yyval.np = yyvsp[-4].np;
		;
    break;}
case 136:
#line 866 "../src/petity.y"
{ yyval.np = make_node( op_max ); ;
    break;}
case 137:
#line 869 "../src/petity.y"
{ yyval.np = make_node( op_min ); ;
    break;}
case 138:
#line 874 "../src/petity.y"
{   parse_linknode( yyvsp[0].np ); ;
    break;}
case 139:
#line 877 "../src/petity.y"
{   parse_linknode( yyvsp[0].np ); ;
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
#line 880 "../src/petity.y"

/* program section */


static 
void CheckEnd() {
  if(ShouldBeEnd) {
    Error("RETURN statement at the middle of a program");
  }
}


/* parse_die */
static void parse_die()
{
    fprintf(stderr, "Ended on line %ld at %s\n", Lines, yytext );
    Exit(1);
}/* parse_die */

/* Utility routine ... get the integer */
static int parse_intval()
{
int val, p;
char ch;

    p = 0;
    val = 0;
    while( (ch=P_INTname[p]) >= '0' && ch <= '9' ){
	val = val * 10 + (ch - '0');
	p = p + 1;
    }
    return val;
}/* parse_intval */

/* Link in a new node at this point. */
static void parse_linknode( node *new_node )
{
    new_node->nodeprev = P_NextPrev;
    new_node->nodeparent = P_NextParent;

    if( P_NextPrev != NULL )
	P_NextPrev->nodenext = new_node;
    if( P_NextParent != NULL && P_NextParent->nodechild == NULL )
	P_NextParent->nodechild = new_node;

    P_NextPrev = new_node;
}/* parse_linknode */

/* unlink node, when it doesn't really belong */
static void parse_unlinknode()
{
node *old;
    old = P_NextPrev;

    P_NextPrev = old->nodeprev;

    if( P_NextParent != NULL && P_NextParent->nodechild == old )
	P_NextParent->nodechild = NULL;
    if( P_NextPrev != NULL )
	P_NextPrev->nodenext = NULL;

    old->nodeprev = NULL;
    old->nodeparent = NULL;
}/* parse_unlinknode */

static void parse_linknodeto( node *old, node *new_node )
{
node *last;
    for(last = old->nodechild;
	last != NULL && last->nodenext != NULL;
	last = last->nodenext ) ;

    new_node->nodeprev = last;
    new_node->nodeparent = old;

    if( last != NULL )
	last->nodenext = new_node;
    if( old->nodechild == NULL )
	old->nodechild = new_node;
}/* parse_linknodeto */

/* Push and Pop scopes */
static void parse_pushscope( node *n )
{
    if( n == 0 ){
	P_NextParent = P_NextPrev;
    } else {
	P_NextParent = n;
    }
    P_NextPrev = NULL;
}/* parse_pushscope */

static void parse_popscope()
{
    P_NextPrev = P_NextParent;
    P_NextParent = P_NextPrev->nodeparent;
}/* parse_popscope */


static exprtype expression_type( node *left, optype op, node *right )
{
exprtype result;

    /* assume the expression's type is integer until proven otherwise */
    result = exprint;

    switch( op ) {
    case op_add:
    case op_subtract:
    case op_multiply:
    case op_divide:
    case op_exp:
    case op_mod:
    case op_max:
    case op_min:
    case op_floor_divide:
    case op_ceiling_divide:
      /* if either expression is real, the result is real */
      if( left != NULL ) {
	  if( left->exptype != exprint && left->exptype != exprrefint ) {
	      result = exprreal;
	  }
      }
      if( right != NULL ) {
	  if( right->exptype != exprint && right->exptype != exprrefint ) {
	      result = exprreal;
	  }
      }
      break;

    case op_lt:
    case op_le:
    case op_eq:
    case op_ne:
    case op_ge:
    case op_gt:
      /* type already set to integer, which these are */
      break;

    case op_sqrt:
      /* type is always real in this case */
      result = exprreal;
      break;

    default:
      /* default is unknown, which had better not happen! */
      result = exprunknown;
      break;
    }

    return( result );
}/* expression_type */

static node *parse_binaryop( node *left, optype op, node *right )
{
node *n;

    n = make_node( op );

    /* get the expression's type */
    n->exptype = expression_type( left, op, right );

    if( left  != NULL ) parse_linknodeto( n, left  );
    if( right != NULL ) parse_linknodeto( n, right );
    return n;
}/* parse_binaryop */

static void parse_forloop(optype op, symtabentry *index,
			node *lower, node *upper, node *inc) {
    node *donode, *doindex;
    int i;

    if(!(P_LoopNest<maxnest))
      ErrAssert("parse_forloop: too deep loop nesting level");
    for(i=0; i<(int)P_LoopNest; i++) {
      if(strcmp(loopsyms[i],index->symname)==0) {
	fprintf(stderr,
		"Loop parameter `%s' is already used in outside loop\n",
		index->symname);
	parse_die();
      }
    }
    loopsyms[P_LoopNest]=index->symname;

    donode = make_node( op );
    parse_linknode( donode );
    ++P_LoopNest;
    donode->nodevalue = P_LoopNest;
    parse_pushscope( donode );

    doindex = make_node( op_dolimit );
    doindex->nodevalue = (unsigned long int)index;
    parse_linknode( doindex );

    parse_linknode( lower );
    parse_linknode( upper );
    if( inc != NULL ) parse_linknode( inc );

    parse_pushscope( doindex );

    index->symtype = symindex;
    index->symdecl = doindex;
    index->exptype = exprrefint; /* we only allow integer index variables */

    /* make sure the lower and upper loop bounds are integer expressions */
    if( lower->exptype != exprint ) {
	fprintf(stderr, "*** lower bound not integer expression\n" );
	parse_die();
    }
    else
    if( upper->exptype != exprint ) {
	fprintf(stderr, "*** upper bound not integer expression\n" );
	parse_die();
    }
    else
    if( inc != NULL ) {
	if( inc->exptype != exprint ) {
	    fprintf(stderr, "*** increment not integer expression\n" );
	    parse_die();
	}
    }
}/* parse_forloop */

static void parse_endforloop()
{
symtabentry *index;

    index = (symtabentry*)P_NextParent->nodevalue;
    index->symtype = symoldindex;
    parse_popscope();
    parse_popscope();
    --P_LoopNest;
}/* parse_endforloop */

nametabentry *parse_findsym()
{
nametabentry *f;

    for( f = NameHead; f != NULL; f = f->namenext ){
	if( strcmp( f->namest.symname, P_IDname ) == 0 &&
	    f->namest.symtype != symoldindex ){
	     return f;
	}
    }
    return NULL;
}/* parse_findsym */

symtabentry *add_symbol(const char *name, int new_int,
			symboltype symtype, exprtype exptype,
			nametabentry *onam)
{
  nametabentry *nam;

#if 0  
  fprintf(stderr, "At line %ld, adding symbol %s to name table\n", Lines, name);
  for (nam = NameHead; nam; nam = nam->namenext)
  {
      fprintf(stderr, "   %s\n", nam->namest.symname);
  }
#endif

  nam = new nametabentry;
  if (!nam) out_of_memory();
  nam->namenext = NameHead;
  NameHead = nam;
  nam->namest.symtype = symtype;
  nam->namest.symdims = 0;
  nam->namest.symdecl = NULL;
  nam->namest.def_node = NULL;
  nam->namest.symname = (char*)calloc( strlen( name )+1, 1 );
  if (!nam->namest.symname) out_of_memory();
  strcpy( nam->namest.symname, name );
  nam->namest.nte = nam;
  nam->namest.original = NULL;
  nam->namest.exptype = exptype;
  nam->namest.symoffset = 0;	  /* set later if needs memory */
  nam->namest.symtag = UNTAGGED;
  nam->namest.omega_vars = 0;
  if(new_int==0 && onam==NULL) {
    node *n = make_node( op_declare );
    n->nodevalue = (unsigned long int)(&nam->namest);
    nam->namest.symdecl = n;
    nam->namest.symclass = CLFORMAL;
    PlaceNodeBefore(n,Entry->nodechild);
  }
  return &(nam->namest);
}

/*
 * Find/create symbol table entry.
 * new_int : 0 - name should already exist or you'll get error message,
 *       1 - name should not exist or get error message,
 *       2 - create new name even if the entry with this name exists already
 */
symtabentry *parse_symbol(int new_int) {
  nametabentry *nam = parse_findsym();
  nametabentry *onam = nam;
  
  if(NoNewOldCheck) {
    if(nam)
      return &(nam->namest);
  } else {
    if( new_int == 0 ){
      if( nam == NULL ){
	fprintf(stderr, "*** `%s' is undeclared variable ***\n", P_IDname);
	parse_die();
      }
      return &(nam->namest);
    }
    if( new_int == 1 ) {
      if( nam != NULL ){
	fprintf(stderr, "Warning: variable `%s' declared twice, 2nd declaration on line %ld\n", P_IDname, Lines);
	return &(nam->namest);
      }
    }
  }

  return add_symbol(P_IDname, new_int, syminteger, exprrefint, onam);
       /* we'll set the type later */
}/* parse_symbol */

/*
 * create symbol table entry for loop, and for loop step count
 * (the latter is needed so that we can have a var_id for the omega test)
 */

static symtabentry *parse_doid_symbol()
{
    symtabentry *id, *stepsym;
    char loop_step_name[256];
    sprintf(loop_step_name, "<%s-trip>", P_IDname);
    stepsym = add_symbol(loop_step_name, 1, symloopstep, exprrefint, 0);
    id = parse_symbol(2);
    assert(&(id->nte->namenext->namest) == stepsym);
    return id;
}
							      
/*
 * Make node referencing already existing variable
 */
node *parse_symbolnode()
{
node *nod;
symtabentry *sym;

    sym = parse_symbol( 0 );

    if( sym->symtype == symindex ){
	nod = make_node( op_index );
	nod->nodevalue = (unsigned long int)sym->symdecl;
	nod->exptype = exprint;	       /* all index expressions are integer */
    } else if( sym->symtype == symconst ) {
	nod = make_node( op_constant );
	nod->nodevalue = (unsigned long int)sym->symvalue;
	nod->exptype = exprint;	       /* all constants are integers */
    } else if( sym->symdims == 0 ) {   /* determine type of scalar var */
	nod = make_node( op_fetch );
	nod->nodevalue = (unsigned long int)sym->symdecl;
	if( sym->exptype == exprrefint )
	    nod->exptype = exprint;
	else if( sym->exptype == exprrefreal )
	    nod->exptype = exprreal;
	else
	    nod->exptype = exprunknown;
    } else if(sym->symdims == DIMSFUN) {   /* builtin function */
	nod = make_node( op_call );
	nod->nodevalue = (unsigned long int)sym->symdecl;
	if( sym->exptype == exprintfun )
	    nod->exptype = exprint;
	else if( sym->exptype == exprrealfun )
	    nod->exptype = exprreal;
	else
	    nod->exptype = exprunknown;
    } else if( sym->symdims > 0 ) {
	nod = make_node( op_fetch_array );
	nod->nodevalue = (unsigned long int)sym->symdecl;
	if( sym->exptype == exprintarray )
	    nod->exptype = exprint;     /* if int array, then just int */
	else
	if( sym->exptype == exprrealarray )
	    nod->exptype = exprreal;    /* if real array, then just real */
	else
	    nod->exptype = exprunknown;
    } else {
	fprintf(stderr, "unknown symbol %s\n", sym->symname );
	parse_die();
	nod = 0;  // make g++ calm down about initialization
    }
    return nod;
}/* parse_symbolnode */

static void parse_stmt_index( node *n )
{
node *index;

    if( n == NULL ) return;
    parse_stmt_index( n->nodeparent );
    if( n->nodeop == op_dolimit ){
	index = make_node( op_index );
	index->nodevalue = (unsigned long int)n;
	parse_linknode( index );
    }
}/* parse_stmt_index */

static void parse_check_stmt_number()
/* Put an op_stmt_number here. */
{
node *stmt_number;

    if( P_NextParent != NULL && P_NextParent->nodeop == op_dolimit ){
	++P_StmtNumber;
	stmt_number = make_node( op_stmt_number );
	stmt_number->nodevalue = P_StmtNumber;
	parse_linknode( stmt_number );
	parse_pushscope( stmt_number );
	parse_stmt_index( stmt_number->nodeparent );
	parse_popscope();
    }
}/* parse_check_stmt_number */

/* Initialize everything. */
static void parse_init()
{
    Lines = 1;
    P_Lines = 1;
    P_LoopNest = 0;
    P_StmtNumber = 0;

    /* Initialize entry point. */
    Entry = make_node( op_entry );

    /* Initialize node links. */
    P_NextPrev = NULL;
    P_NextParent = Entry;

    NameHead = NULL;
}/* parse_init */

void yyerror( char *s)
{
    fprintf(stderr, "%s\n", s );
    parse_die();
}/* yyerror */
