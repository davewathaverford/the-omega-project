/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     VAR = 258,
     INT = 259,
     STRING = 260,
     OPEN_BRACE = 261,
     CLOSE_BRACE = 262,
     SYMBOLIC = 263,
     OR = 264,
     AND = 265,
     NOT = 266,
     ST = 267,
     APPROX = 268,
     IS_ASSIGNED = 269,
     FORALL = 270,
     EXISTS = 271,
     DOMAIN = 272,
     RANGE = 273,
     DIFFERENCE = 274,
     DIFFERENCE_TO_RELATION = 275,
     GIST = 276,
     GIVEN = 277,
     HULL = 278,
     WITHIN = 279,
     MAXIMIZE = 280,
     MINIMIZE = 281,
     AFFINE_HULL = 282,
     VENN = 283,
     CONVEX_COMBINATION = 284,
     POSITIVE_COMBINATION = 285,
     CONVEX_HULL = 286,
     CONIC_HULL = 287,
     LINEAR_HULL = 288,
     PAIRWISE_CHECK = 289,
     CONVEX_CHECK = 290,
     MAXIMIZE_RANGE = 291,
     MINIMIZE_RANGE = 292,
     MAXIMIZE_DOMAIN = 293,
     MINIMIZE_DOMAIN = 294,
     LEQ = 295,
     GEQ = 296,
     NEQ = 297,
     GOES_TO = 298,
     COMPOSE = 299,
     JOIN = 300,
     INVERSE = 301,
     COMPLEMENT = 302,
     IN = 303,
     CARRIED_BY = 304,
     TIME = 305,
     TIMECLOSURE = 306,
     UNION = 307,
     INTERSECTION = 308,
     VERTICAL_BAR = 309,
     SUCH_THAT = 310,
     SUBSET = 311,
     ITERATIONS = 312,
     SPMD = 313,
     CODEGEN = 314,
     DECOUPLED_FARKAS = 315,
     FARKAS = 316,
     TCODEGEN = 317,
     TRANS_IS = 318,
     SET_MMAP = 319,
     UNROLL_IS = 320,
     PEEL_IS = 321,
     MAKE_UPPER_BOUND = 322,
     MAKE_LOWER_BOUND = 323,
     REL_OP = 324,
     RESTRICT_DOMAIN = 325,
     RESTRICT_RANGE = 326,
     SUPERSETOF = 327,
     SUBSETOF = 328,
     SAMPLE = 329,
     SYM_SAMPLE = 330,
     PROJECT_AWAY_SYMBOLS = 331,
     PROJECT_ON_SYMBOLS = 332,
     REACHABLE_FROM = 333,
     REACHABLE_OF = 334,
     ASSERT_UNSAT = 335,
     PARSE_EXPRESSION = 336,
     PARSE_FORMULA = 337,
     PARSE_RELATION = 338,
     p1 = 339,
     p2 = 340,
     p3 = 341,
     p4 = 342,
     p5 = 343,
     p6 = 344,
     p7 = 345,
     p8 = 346,
     p9 = 347,
     p10 = 348
   };
#endif
/* Tokens.  */
#define VAR 258
#define INT 259
#define STRING 260
#define OPEN_BRACE 261
#define CLOSE_BRACE 262
#define SYMBOLIC 263
#define OR 264
#define AND 265
#define NOT 266
#define ST 267
#define APPROX 268
#define IS_ASSIGNED 269
#define FORALL 270
#define EXISTS 271
#define DOMAIN 272
#define RANGE 273
#define DIFFERENCE 274
#define DIFFERENCE_TO_RELATION 275
#define GIST 276
#define GIVEN 277
#define HULL 278
#define WITHIN 279
#define MAXIMIZE 280
#define MINIMIZE 281
#define AFFINE_HULL 282
#define VENN 283
#define CONVEX_COMBINATION 284
#define POSITIVE_COMBINATION 285
#define CONVEX_HULL 286
#define CONIC_HULL 287
#define LINEAR_HULL 288
#define PAIRWISE_CHECK 289
#define CONVEX_CHECK 290
#define MAXIMIZE_RANGE 291
#define MINIMIZE_RANGE 292
#define MAXIMIZE_DOMAIN 293
#define MINIMIZE_DOMAIN 294
#define LEQ 295
#define GEQ 296
#define NEQ 297
#define GOES_TO 298
#define COMPOSE 299
#define JOIN 300
#define INVERSE 301
#define COMPLEMENT 302
#define IN 303
#define CARRIED_BY 304
#define TIME 305
#define TIMECLOSURE 306
#define UNION 307
#define INTERSECTION 308
#define VERTICAL_BAR 309
#define SUCH_THAT 310
#define SUBSET 311
#define ITERATIONS 312
#define SPMD 313
#define CODEGEN 314
#define DECOUPLED_FARKAS 315
#define FARKAS 316
#define TCODEGEN 317
#define TRANS_IS 318
#define SET_MMAP 319
#define UNROLL_IS 320
#define PEEL_IS 321
#define MAKE_UPPER_BOUND 322
#define MAKE_LOWER_BOUND 323
#define REL_OP 324
#define RESTRICT_DOMAIN 325
#define RESTRICT_RANGE 326
#define SUPERSETOF 327
#define SUBSETOF 328
#define SAMPLE 329
#define SYM_SAMPLE 330
#define PROJECT_AWAY_SYMBOLS 331
#define PROJECT_ON_SYMBOLS 332
#define REACHABLE_FROM 333
#define REACHABLE_OF 334
#define ASSERT_UNSAT 335
#define PARSE_EXPRESSION 336
#define PARSE_FORMULA 337
#define PARSE_RELATION 338
#define p1 339
#define p2 340
#define p3 341
#define p4 342
#define p5 343
#define p6 344
#define p7 345
#define p8 346
#define p9 347
#define p10 348




/* Copy the first part of user declarations.  */
#line 1 "../src/parser.y"


#define compilingParser
#include <basic/Dynamic_Array.h>
#include <code_gen/code_gen.h>
#include <code_gen/spmd.h>
#include <omega/library_version.h>
#include <omega/AST.h>
#include <omega_calc/yylex.h>
#include <omega/hull.h>
#include <omega/calc_debug.h>
#include <basic/Exit.h>
#include <omega/closure.h>
#include <omega/reach.h>
#include <code_gen/mmap-codegen.h>
#include <code_gen/mmap-util.h>
#ifndef WIN32
#include <sys/time.h>
#include <sys/resource.h>
#endif

#define CALC_VERSION_STRING "Omega Calculator v1.2"

#define DEBUG_FILE_NAME "./oc.out"

/* Can only do the following when "using namespace omega"
   is also put before the inclusion of y.tab.h in parser.l.
*/
using omega::min;
using omega::negate;
using namespace omega;

Map<Const_String,Relation*> relationMap ((Relation *)0);
static int redundant_conj_level;

#if defined BRAIN_DAMAGED_FREE
void free(void *p);
void *realloc(void *p, size_t s);
#endif

namespace omega {
#if !defined(OMIT_GETRUSAGE)
void start_clock( void );
int clock_diff( void );
bool anyTimingDone = false;
#endif

int argCount = 0;
int tuplePos = 0;
Argument_Tuple currentTuple = Input_Tuple;

Relation LexForward(int n);
} // end namespace omega

reachable_information *reachable_info;




/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 130 "../src/parser.y"
typedef union YYSTYPE {
	int INT_VALUE;
	Rel_Op REL_OPERATOR;
	char *VAR_NAME;
	VarList *VAR_LIST;
	Exp *EXP;
	ExpList *EXP_LIST;
	AST *ASTP;
	Argument_Tuple ARGUMENT_TUPLE;
	AST_constraints *ASTCP;
	Declaration_Site * DECLARATION_SITE;
	Relation * RELATION;
	tupleDescriptor * TUPLE_DESCRIPTOR;
	RelTuplePair * REL_TUPLE_PAIR;
        RelTupleTriple * REL_TUPLE_TRIPLE;
	Dynamic_Array2<Relation> * RELATION_ARRAY_2D;
	Dynamic_Array1<Relation> * RELATION_ARRAY_1D;
	Tuple<String> *STRING_TUPLE;
	String *STRING_VALUE;
  	Tuple<stm_info> *STM_INFO_TUPLE;
  	stm_info *STM_INFO;
	Read *READ;
	PartialRead *PREAD;
	MMap *MMAP;
	PartialMMap *PMMAP;
	} YYSTYPE;
/* Line 196 of yacc.c.  */
#line 357 "y.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 219 of yacc.c.  */
#line 369 "y.tab.c"

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T) && (defined (__STDC__) || defined (__cplusplus))
# include <stddef.h> /* INFRINGES ON USER NAME SPACE */
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if defined (__STDC__) || defined (__cplusplus)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     define YYINCLUDED_STDLIB_H
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2005 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM ((YYSIZE_T) -1)
#  endif
#  ifdef __cplusplus
extern "C" {
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if (! defined (malloc) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if (! defined (free) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifdef __cplusplus
}
#  endif
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   874

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  105
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  54
/* YYNRULES -- Number of rules. */
#define YYNRULES  180
/* YYNRULES -- Number of states. */
#define YYNSTATES  372

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   348

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      97,   104,    88,    84,   100,    85,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   101,    99,
       2,     2,     2,     2,    89,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   102,     2,   103,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    86,
      87,    90,    91,    92,    93,    94,    95,    96,    98
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    10,    14,    17,    21,
      26,    29,    33,    37,    42,    48,    54,    59,    62,    70,
      76,    77,    79,    82,    86,    87,    89,    92,    93,    96,
     102,   106,   110,   112,   114,   118,   123,   129,   135,   143,
     145,   149,   159,   167,   171,   173,   180,   184,   188,   190,
     194,   198,   200,   204,   208,   210,   215,   217,   218,   223,
     225,   229,   232,   235,   240,   243,   246,   249,   252,   255,
     258,   261,   264,   267,   270,   273,   276,   279,   282,   285,
     290,   293,   296,   299,   302,   305,   308,   311,   314,   317,
     322,   325,   328,   331,   334,   339,   344,   348,   352,   356,
     360,   364,   368,   372,   376,   380,   383,   386,   389,   392,
     395,   398,   400,   403,   404,   405,   412,   415,   417,   420,
     421,   423,   425,   427,   428,   433,   435,   439,   440,   442,
     444,   446,   450,   456,   460,   462,   464,   466,   470,   474,
     478,   480,   484,   487,   493,   499,   502,   505,   507,   509,
     511,   514,   517,   519,   521,   525,   527,   531,   535,   537,
     538,   544,   548,   552,   554,   556,   559,   561,   564,   568,
     572,   576,   580,   586,   590,   594,   596,   600,   606,   614,
     620
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
     106,     0,    -1,    -1,   107,   108,    -1,   110,    -1,    -1,
     108,   109,   110,    -1,     1,    99,    -1,     8,   125,    99,
      -1,     3,    14,   127,    99,    -1,   127,    99,    -1,    50,
     127,    99,    -1,    51,   127,    99,    -1,   127,    56,   127,
      99,    -1,    59,   113,   115,   114,    99,    -1,    62,   113,
     116,   114,    99,    -1,    58,   112,   111,    99,    -1,   153,
      99,    -1,   111,   100,   127,   101,   127,   101,   127,    -1,
     127,   101,   127,   101,   127,    -1,    -1,     4,    -1,     4,
       4,    -1,     4,     4,     4,    -1,    -1,     4,    -1,    85,
       4,    -1,    -1,    22,   127,    -1,   115,   100,   127,   101,
     127,    -1,   115,   100,   127,    -1,   127,   101,   127,    -1,
     127,    -1,   117,    -1,    63,   127,   116,    -1,    64,     4,
     119,   116,    -1,    65,     4,     4,     4,   116,    -1,    66,
       4,     4,   127,   116,    -1,    66,     4,     4,   127,   100,
     127,   116,    -1,   118,    -1,   117,   100,   118,    -1,   102,
       5,   100,   127,   100,   119,   100,   121,   103,    -1,   102,
       5,   100,   127,   100,   119,   103,    -1,   119,   100,   120,
      -1,   120,    -1,     5,   102,   127,   103,   100,   127,    -1,
       5,   100,   127,    -1,   121,   100,   122,    -1,   122,    -1,
     102,   123,   103,    -1,   123,   100,   124,    -1,   124,    -1,
       4,   100,   127,    -1,   125,   100,   126,    -1,   126,    -1,
       3,    97,     4,   104,    -1,     3,    -1,    -1,     6,   128,
     129,     7,    -1,     3,    -1,    97,   127,   104,    -1,   127,
      84,    -1,   127,    88,    -1,   127,    84,    24,   127,    -1,
      37,   127,    -1,    36,   127,    -1,    39,   127,    -1,    38,
     127,    -1,    25,   127,    -1,    26,   127,    -1,    61,   127,
      -1,    60,   127,    -1,   127,    89,    -1,    76,   127,    -1,
      77,   127,    -1,    19,   127,    -1,    20,   127,    -1,    17,
     127,    -1,    28,   127,    -1,    28,   127,    22,   127,    -1,
      31,   127,    -1,    30,   127,    -1,    29,   127,    -1,    34,
     127,    -1,    35,   127,    -1,    27,   127,    -1,    32,   127,
      -1,    33,   127,    -1,    23,   127,    -1,    23,   127,    22,
     127,    -1,    13,   127,    -1,    18,   127,    -1,    46,   127,
      -1,    47,   127,    -1,    21,   127,    22,   127,    -1,   127,
      97,   127,   104,    -1,   127,    44,   127,    -1,   127,    49,
       4,    -1,   127,    45,   127,    -1,   127,    71,   127,    -1,
     127,    70,   127,    -1,   127,    53,   127,    -1,   127,    85,
     127,    -1,   127,    52,   127,    -1,   127,    88,   127,    -1,
      72,   127,    -1,    73,   127,    -1,    67,   127,    -1,    68,
     127,    -1,    74,   127,    -1,    75,   127,    -1,   154,    -1,
      80,   127,    -1,    -1,    -1,   134,    43,   130,   134,   131,
     132,    -1,   134,   132,    -1,   141,    -1,   133,   141,    -1,
      -1,   101,    -1,    54,    -1,    55,    -1,    -1,   135,   102,
     136,   103,    -1,   137,    -1,   136,   100,   137,    -1,    -1,
       3,    -1,    88,    -1,   152,    -1,   152,   101,   152,    -1,
     152,   101,   152,   101,     4,    -1,   138,   100,     3,    -1,
       3,    -1,   138,    -1,   139,    -1,   102,   139,   103,    -1,
     141,    10,   141,    -1,   141,     9,   141,    -1,   148,    -1,
      97,   141,   104,    -1,    11,   141,    -1,   142,   140,   143,
     141,   146,    -1,   144,   140,   145,   141,   146,    -1,    97,
      16,    -1,    16,    97,    -1,   101,    -1,    54,    -1,    55,
      -1,    97,    15,    -1,    15,    97,    -1,   101,    -1,   104,
      -1,   152,   100,   147,    -1,   152,    -1,   147,    69,   147,
      -1,   147,    69,   148,    -1,     3,    -1,    -1,     3,    97,
     150,   151,   104,    -1,    97,   152,   104,    -1,   151,   100,
       3,    -1,     3,    -1,     4,    -1,     4,   149,    -1,   149,
      -1,    85,   152,    -1,   152,    84,   152,    -1,   152,    85,
     152,    -1,   152,    88,   152,    -1,    78,   155,   157,    -1,
      79,     3,    48,   155,   157,    -1,    97,   156,   104,    -1,
     156,   100,     3,    -1,     3,    -1,     6,   158,     7,    -1,
     158,   100,     3,   101,   127,    -1,   158,   100,     3,    43,
       3,   101,   127,    -1,     3,    43,     3,   101,   127,    -1,
       3,   101,   127,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   161,   161,   161,   165,   166,   166,   171,   183,   186,
     195,   203,   287,   375,   382,   390,   407,   435,   453,   463,
     476,   477,   478,   479,   482,   483,   484,   487,   489,   492,
     502,   511,   522,   534,   541,   545,   549,   551,   555,   562,
     565,   570,   576,   584,   588,   593,   599,   606,   609,   614,
     617,   620,   625,   631,   632,   635,   637,   642,   641,   653,
     663,   664,   669,   676,   682,   691,   700,   709,   718,   728,
     738,   744,   750,   755,   760,   765,   770,   775,   780,   785,
     791,   796,   801,   806,   811,   816,   821,   826,   831,   836,
     842,   847,   852,   857,   862,   868,   874,   880,   886,   892,
     898,   904,   910,   916,   922,   928,   933,   938,   943,   948,
     953,   958,   959,   973,   974,   973,  1000,  1017,  1026,  1027,
    1030,  1031,  1032,  1036,  1036,  1042,  1043,  1044,  1047,  1061,
    1063,  1065,  1067,  1072,  1073,  1077,  1087,  1088,  1091,  1092,
    1093,  1094,  1095,  1096,  1098,  1102,  1103,  1106,  1107,  1108,
    1111,  1112,  1115,  1118,  1122,  1127,  1133,  1135,  1140,  1146,
    1146,  1157,  1163,  1174,  1186,  1187,  1188,  1189,  1190,  1191,
    1192,  1197,  1206,  1221,  1229,  1232,  1238,  1289,  1297,  1308,
    1319
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "VAR", "INT", "STRING", "OPEN_BRACE",
  "CLOSE_BRACE", "SYMBOLIC", "OR", "AND", "NOT", "ST", "APPROX",
  "IS_ASSIGNED", "FORALL", "EXISTS", "DOMAIN", "RANGE", "DIFFERENCE",
  "DIFFERENCE_TO_RELATION", "GIST", "GIVEN", "HULL", "WITHIN", "MAXIMIZE",
  "MINIMIZE", "AFFINE_HULL", "VENN", "CONVEX_COMBINATION",
  "POSITIVE_COMBINATION", "CONVEX_HULL", "CONIC_HULL", "LINEAR_HULL",
  "PAIRWISE_CHECK", "CONVEX_CHECK", "MAXIMIZE_RANGE", "MINIMIZE_RANGE",
  "MAXIMIZE_DOMAIN", "MINIMIZE_DOMAIN", "LEQ", "GEQ", "NEQ", "GOES_TO",
  "COMPOSE", "JOIN", "INVERSE", "COMPLEMENT", "IN", "CARRIED_BY", "TIME",
  "TIMECLOSURE", "UNION", "INTERSECTION", "VERTICAL_BAR", "SUCH_THAT",
  "SUBSET", "ITERATIONS", "SPMD", "CODEGEN", "DECOUPLED_FARKAS", "FARKAS",
  "TCODEGEN", "TRANS_IS", "SET_MMAP", "UNROLL_IS", "PEEL_IS",
  "MAKE_UPPER_BOUND", "MAKE_LOWER_BOUND", "REL_OP", "RESTRICT_DOMAIN",
  "RESTRICT_RANGE", "SUPERSETOF", "SUBSETOF", "SAMPLE", "SYM_SAMPLE",
  "PROJECT_AWAY_SYMBOLS", "PROJECT_ON_SYMBOLS", "REACHABLE_FROM",
  "REACHABLE_OF", "ASSERT_UNSAT", "PARSE_EXPRESSION", "PARSE_FORMULA",
  "PARSE_RELATION", "'+'", "'-'", "p1", "p2", "'*'", "'@'", "p3", "p4",
  "p5", "p6", "p7", "p8", "p9", "'('", "p10", "';'", "','", "':'", "'['",
  "']'", "')'", "$accept", "start", "@1", "inputSequence", "@2",
  "inputItem", "relTripList", "blockAndProcsAndEffort", "effort",
  "context", "relPairList", "statementInfoResult", "statementInfoList",
  "statementInfo", "partialwrites", "partialwrite", "reads", "oneread",
  "partials", "partial", "globVarList", "globVar", "relation", "@3",
  "builtRelation", "@4", "@5", "optionalFormula", "formula_sep",
  "tupleDeclaration", "@6", "optionalTupleVarList", "tupleVar", "varList",
  "varDecl", "varDeclOptBrackets", "formula", "start_exists", "exists_sep",
  "start_forall", "forall_sep", "end_quant", "expList", "constraintChain",
  "simpleExp", "@7", "argumentList", "exp", "reachable", "reachable_of",
  "nodeNameList", "realNodeNameList", "nodeSpecificationList",
  "realNodeSpecificationList", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,    43,    45,   339,   340,    42,    64,
     341,   342,   343,   344,   345,   346,   347,    40,   348,    59,
      44,    58,    91,    93,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   105,   107,   106,   108,   109,   108,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   111,   111,
     112,   112,   112,   112,   113,   113,   113,   114,   114,   115,
     115,   115,   115,   116,   116,   116,   116,   116,   116,   117,
     117,   118,   118,   119,   119,   120,   120,   121,   121,   122,
     123,   123,   124,   125,   125,   126,   126,   128,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   130,   131,   129,   129,   129,   132,   132,
     133,   133,   133,   135,   134,   136,   136,   136,   137,   137,
     137,   137,   137,   138,   138,   139,   140,   140,   141,   141,
     141,   141,   141,   141,   141,   142,   142,   143,   143,   143,
     144,   144,   145,   146,   147,   147,   148,   148,   149,   150,
     149,   149,   151,   151,   152,   152,   152,   152,   152,   152,
     152,   153,   154,   155,   156,   156,   157,   158,   158,   158,
     158
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     1,     0,     3,     2,     3,     4,
       2,     3,     3,     4,     5,     5,     4,     2,     7,     5,
       0,     1,     2,     3,     0,     1,     2,     0,     2,     5,
       3,     3,     1,     1,     3,     4,     5,     5,     7,     1,
       3,     9,     7,     3,     1,     6,     3,     3,     1,     3,
       3,     1,     3,     3,     1,     4,     1,     0,     4,     1,
       3,     2,     2,     4,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     4,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     4,
       2,     2,     2,     2,     4,     4,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     2,
       2,     1,     2,     0,     0,     6,     2,     1,     2,     0,
       1,     1,     1,     0,     4,     1,     3,     0,     1,     1,
       1,     3,     5,     3,     1,     1,     1,     3,     3,     3,
       1,     3,     2,     5,     5,     2,     2,     1,     1,     1,
       2,     2,     1,     1,     3,     1,     3,     3,     1,     0,
       5,     3,     3,     1,     1,     2,     1,     2,     3,     3,
       3,     3,     5,     3,     3,     1,     3,     5,     7,     5,
       3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     0,     0,     1,     0,    59,    57,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    20,    24,     0,     0,    24,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     4,     0,     0,   111,     7,     0,   123,    56,
       0,    54,    59,    90,    77,    91,    75,    76,     0,    88,
      68,    69,    85,    78,    82,    81,    80,    86,    87,    83,
      84,    65,    64,    67,    66,    92,    93,     0,     0,    21,
       0,    25,     0,     0,    71,    70,     0,   107,   108,   105,
     106,   109,   110,    73,    74,     0,     0,     0,   112,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    61,
       0,    62,    72,     0,    10,    17,     0,   158,   164,     0,
       0,     0,     0,     0,     0,   119,     0,   117,     0,     0,
       0,   140,   166,   155,     0,     8,     0,     0,     0,     0,
      11,    12,    22,     0,     0,    26,    27,    32,     0,     0,
       0,     0,     0,    27,    33,    39,   175,     0,     0,   171,
       0,    60,     6,    96,    98,    97,   103,   101,     0,   100,
      99,     0,   102,   104,     0,     9,   159,     0,   165,   142,
     151,   146,   167,   150,   145,     0,   155,    58,   113,   121,
     122,   120,   116,     0,   127,     0,     0,   134,     0,   135,
     136,     0,     0,     0,     0,     0,     0,     0,     0,    53,
      94,    89,    79,    23,    16,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   173,
       0,     0,     0,    13,    63,    95,     0,     0,   141,   161,
     123,   118,   158,   129,     0,   125,   130,   139,   138,     0,
       0,   148,   149,   147,     0,   152,     0,   156,   157,   168,
     169,   170,   154,    55,     0,     0,    28,    30,    14,    31,
      34,     0,     0,    44,     0,     0,     0,    15,    40,   174,
       0,     0,   176,     0,   172,   163,     0,   114,     0,   124,
       0,   137,   133,     0,     0,     0,     0,     0,     0,     0,
       0,    35,     0,     0,     0,     0,   180,     0,     0,   160,
     119,   126,   131,   153,   143,   144,     0,    19,    29,    46,
       0,    43,    36,     0,    37,     0,     0,     0,     0,   162,
     115,     0,     0,     0,     0,     0,   179,     0,   177,   132,
      18,     0,    38,     0,    42,     0,    45,     0,     0,    48,
     178,     0,     0,    51,     0,    41,     0,     0,    49,    47,
      52,    50
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     1,     2,    51,   110,    52,   153,    90,    93,   229,
     156,   163,   164,   165,   282,   283,   358,   359,   362,   363,
      60,    61,    53,    58,   134,   250,   320,   202,   203,   135,
     136,   254,   255,   209,   210,   211,   137,   138,   264,   139,
     266,   324,   140,   141,   142,   246,   296,   143,    54,    55,
     106,   167,   169,   241
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -252
static const short int yypact[] =
{
    -252,    48,   336,  -252,   -36,    75,  -252,    97,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   101,    42,   164,   164,    42,   164,
     164,   164,   164,   164,   164,   164,   164,    19,   125,   164,
     164,   143,  -252,   627,    65,  -252,  -252,   164,    51,    64,
      -1,  -252,  -252,    69,    69,    69,    69,    69,   205,     4,
      69,    69,    69,    35,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,   657,   687,   175,
     164,  -252,   200,   164,    69,    69,   149,    69,    69,   215,
     215,    69,    69,    69,    69,   183,   210,   173,   777,   373,
     336,   164,   164,   224,   164,   164,   164,   164,   164,   206,
     164,   164,  -252,   164,  -252,  -252,   717,   136,    55,    51,
     137,   138,    66,    57,   235,    32,   153,   166,    27,    27,
     193,  -252,  -252,    -7,   259,  -252,    97,   164,   164,   164,
    -252,  -252,   263,   109,   437,  -252,    34,   465,   164,   267,
     268,   275,   276,   258,   187,  -252,  -252,   -24,   285,  -252,
      19,  -252,  -252,    69,    69,  -252,   215,    47,   747,   641,
     641,   164,    47,    47,   375,  -252,  -252,    66,  -252,  -252,
    -252,  -252,  -252,   137,   138,    41,   223,  -252,  -252,  -252,
    -252,  -252,  -252,    51,    61,    51,    51,  -252,   292,   196,
    -252,    29,   212,    66,    66,    66,    66,    66,   213,  -252,
      69,    69,    69,  -252,  -252,   164,   164,   164,   164,   222,
     164,   431,   309,   316,   318,   226,   230,   229,   330,  -252,
      36,    38,   210,  -252,    69,  -252,   332,    84,  -252,  -252,
    -252,   166,   126,  -252,   -29,  -252,   168,   326,  -252,   237,
     335,  -252,  -252,  -252,    51,  -252,    51,   193,  -252,   253,
     253,  -252,  -252,  -252,   499,   519,   777,   553,  -252,   777,
    -252,   -12,   182,  -252,   341,   164,   164,  -252,  -252,  -252,
     343,   164,  -252,   345,  -252,  -252,    14,  -252,    61,  -252,
      66,  -252,  -252,    43,    43,   164,   164,   164,   164,   164,
     309,  -252,   149,   403,   607,   249,   777,    54,   348,  -252,
     105,  -252,   231,  -252,  -252,  -252,   573,   777,   777,   777,
     221,  -252,  -252,   164,  -252,   309,   164,   349,   164,  -252,
    -252,   356,   164,   277,   431,    53,   777,   278,   777,  -252,
     777,   164,  -252,    22,  -252,   164,   777,   372,    62,  -252,
     777,   280,    71,  -252,   279,  -252,   164,   372,  -252,  -252,
     777,  -252
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -252,  -252,  -252,  -252,  -252,   274,  -252,  -252,   340,   225,
    -252,  -187,  -252,   148,    56,  -251,  -252,    25,  -252,    23,
    -252,   246,    -8,  -252,  -252,  -252,  -252,    73,  -252,   150,
    -252,  -252,   103,  -252,   191,   266,   -86,  -252,  -252,  -252,
    -252,    98,   -82,   194,   293,  -252,  -252,   -93,  -252,  -252,
     236,  -252,   181,  -252
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -129
static const short int yytable[] =
{
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,   148,   281,    94,    95,
     207,    97,    98,    99,   100,   101,   102,   103,   104,   192,
     196,   108,   109,   189,   280,   292,    91,   195,     3,   126,
     205,   206,   205,   206,   127,   128,   227,   149,   127,   331,
     127,   128,   129,    56,   252,   128,   130,   131,   129,   127,
     128,   298,   193,   194,   299,   198,   238,   214,   215,   290,
     239,   216,   154,   261,   262,   157,   199,   200,   308,    57,
     309,   111,   112,   217,   247,   311,   113,   337,   145,   146,
      59,   123,   331,   173,   174,    89,   176,   177,   178,   179,
     180,   256,   182,   183,   318,   184,   105,   251,   319,   257,
     258,   269,   270,   271,   357,   332,   334,    92,   107,   208,
     263,   267,   123,   201,   228,   272,   132,   291,   293,   220,
     221,   222,   132,    -3,   123,   248,   132,   323,   133,   253,
     231,   132,   187,   353,   133,   338,   354,   352,   187,   199,
     200,   144,   364,   187,   125,   365,   123,    62,   214,   215,
       6,   367,   216,   244,   368,   205,   206,     8,   303,   152,
     304,     9,    10,    11,    12,    13,   166,    14,   249,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,   155,   256,   201,   322,   224,   225,
      30,    31,   158,   159,   160,   161,   168,   274,   275,   276,
     277,   170,   279,   186,    36,    37,  -128,   147,   175,  -128,
     181,    39,    40,   186,   190,   191,    41,    42,    43,    44,
      45,    46,   197,    48,    49,   158,   159,   160,   161,   111,
     112,   162,   214,   215,   113,   204,   216,   114,   115,   111,
     112,    50,   213,   218,   113,   111,   112,   223,   115,   300,
     113,   232,   233,   114,   115,   117,   118,   313,   314,   234,
     227,   235,   310,   316,   162,   117,   118,   237,   240,   119,
     120,   117,   118,   121,   122,   207,   260,   326,   327,   328,
     329,   330,   123,   121,   122,   119,   120,   214,   215,   121,
     122,   216,   123,   265,   281,   214,   215,   273,   123,   216,
     284,   278,   285,   217,   343,   344,   286,   249,   346,   287,
     348,   162,   341,   289,   350,   295,   206,     4,   302,     5,
     301,   216,     6,   356,     7,   312,   315,   360,   317,     8,
     336,   339,   347,     9,    10,    11,    12,    13,   370,    14,
     349,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,   361,   351,    96,   355,
     366,   357,    30,    31,   172,   288,    32,    33,   236,   369,
     371,   345,   219,   340,    34,    35,    36,    37,    38,   259,
     297,   321,   325,    39,    40,   212,   242,   268,    41,    42,
      43,    44,    45,    46,    47,    48,    49,   111,   112,   111,
     112,   188,   113,   294,   113,   114,   115,   114,   115,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   117,   118,   117,   118,   111,   112,     0,
       0,     0,   113,     0,     0,   114,   115,   119,   120,   119,
     120,   121,   122,   121,   122,     0,   158,   159,   160,   161,
     123,     0,   123,   117,   118,   111,   112,   171,     0,   245,
     113,   111,   112,   114,   115,     0,   113,   119,   120,   114,
     115,   121,   122,     0,   158,   159,   160,   161,     0,     0,
     123,   117,   118,   333,     0,   162,     0,   117,   118,   111,
     112,     0,     0,     0,   113,   119,   120,   114,   115,   121,
     122,   119,   120,     0,     0,   121,   122,     0,   123,     0,
       0,     0,     0,   162,   123,   117,   118,     0,   226,     0,
       0,     0,     0,   111,   112,     0,     0,     0,   113,   119,
     120,   114,   115,   121,   122,     0,     0,     0,     0,     0,
       0,     0,   123,   111,   112,     0,   230,     0,   113,   117,
     118,   114,   115,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   119,   120,     0,     0,   121,   122,   117,
     118,     0,     0,     0,     0,     0,   123,   111,   112,     0,
     305,     0,   113,   119,   120,   114,   115,   121,   122,     0,
       0,     0,     0,     0,     0,     0,   123,   111,   112,     0,
     306,     0,   113,   117,   118,   114,   115,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   119,   120,     0,
       0,   121,   122,   117,   118,     0,     0,     0,     0,     0,
     123,   111,   112,     0,   307,     0,   113,   119,   120,   114,
     115,   121,   122,     0,     0,     0,     0,     0,     0,     0,
     123,   111,   112,     0,   342,     0,   113,   117,   118,   114,
     115,     0,     0,   116,     0,   111,   112,     0,     0,     0,
     113,   119,   120,     0,   115,   121,   122,   117,   118,     0,
       0,   111,   112,     0,   123,     0,   113,   335,     0,   114,
     115,   119,   120,     0,     0,   121,   122,     0,     0,     0,
       0,     0,     0,     0,   123,     0,   124,   117,   118,   121,
     122,   111,   112,     0,     0,     0,   113,     0,   123,   114,
     115,   119,   120,     0,     0,   121,   122,     0,     0,     0,
       0,     0,     0,     0,   123,     0,   150,   117,   118,     0,
       0,   111,   112,     0,     0,     0,   113,     0,     0,   114,
     115,   119,   120,     0,     0,   121,   122,     0,     0,     0,
       0,     0,     0,     0,   123,     0,   151,   117,   118,     0,
       0,   111,   112,     0,     0,     0,   113,     0,     0,   114,
     115,   119,   120,     0,     0,   121,   122,     0,     0,     0,
       0,     0,     0,     0,   123,     0,   185,   117,   118,     0,
       0,   111,   112,     0,     0,     0,   113,     0,     0,   114,
     115,   119,   120,     0,     0,   121,   122,     0,     0,     0,
       0,     0,     0,     0,   123,     0,   243,   117,   118,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   119,   120,     0,     0,   121,   122,     0,     0,     0,
       0,     0,     0,     0,   123
};

static const short int yycheck[] =
{
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    22,     5,    36,    37,
       3,    39,    40,    41,    42,    43,    44,    45,    46,   132,
     133,    49,    50,   129,   231,     7,     4,   133,     0,    57,
       9,    10,     9,    10,     3,     4,    22,    22,     3,   310,
       3,     4,    11,    99,     3,     4,    15,    16,    11,     3,
       4,   100,    15,    16,   103,    43,   100,    84,    85,    43,
     104,    88,    90,    54,    55,    93,    54,    55,   100,    14,
     102,    44,    45,   100,   187,   282,    49,    43,    99,   100,
       3,    97,   353,   111,   112,     4,   114,   115,   116,   117,
     118,   204,   120,   121,   100,   123,    97,   203,   104,   205,
     206,   214,   215,   216,   102,   312,   313,    85,     3,   102,
     101,   213,    97,   101,   100,   217,    85,   101,   100,   147,
     148,   149,    85,     0,    97,   104,    85,   104,    97,    88,
     158,    85,    97,   100,    97,   101,   103,   344,    97,    54,
      55,    97,   100,    97,    99,   103,    97,     3,    84,    85,
       6,   100,    88,   181,   103,     9,    10,    13,   264,     4,
     266,    17,    18,    19,    20,    21,     3,    23,   104,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,     4,   298,   101,   300,    99,   100,
      46,    47,    63,    64,    65,    66,     6,   225,   226,   227,
     228,    48,   230,    97,    60,    61,   100,    22,     4,   103,
      24,    67,    68,    97,    97,    97,    72,    73,    74,    75,
      76,    77,     7,    79,    80,    63,    64,    65,    66,    44,
      45,   102,    84,    85,    49,   102,    88,    52,    53,    44,
      45,    97,    69,     4,    49,    44,    45,     4,    53,   101,
      49,     4,     4,    52,    53,    70,    71,   285,   286,     4,
      22,     5,   100,   291,   102,    70,    71,   100,     3,    84,
      85,    70,    71,    88,    89,     3,   100,   305,   306,   307,
     308,   309,    97,    88,    89,    84,    85,    84,    85,    88,
      89,    88,    97,   101,     5,    84,    85,   104,    97,    88,
       4,    99,     4,   100,   103,   333,   100,   104,   336,    99,
     338,   102,   101,     3,   342,     3,    10,     1,     3,     3,
     103,    88,     6,   351,     8,     4,     3,   355,     3,    13,
     101,     3,     3,    17,    18,    19,    20,    21,   366,    23,
       4,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,     4,   100,    38,   101,
     100,   102,    46,    47,   110,   237,    50,    51,   163,   364,
     367,   335,   146,   320,    58,    59,    60,    61,    62,   208,
     250,   298,   304,    67,    68,   139,   170,   213,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    44,    45,    44,
      45,   128,    49,   242,    49,    52,    53,    52,    53,    -1,
      -1,    -1,    -1,    97,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    71,    70,    71,    44,    45,    -1,
      -1,    -1,    49,    -1,    -1,    52,    53,    84,    85,    84,
      85,    88,    89,    88,    89,    -1,    63,    64,    65,    66,
      97,    -1,    97,    70,    71,    44,    45,   104,    -1,   104,
      49,    44,    45,    52,    53,    -1,    49,    84,    85,    52,
      53,    88,    89,    -1,    63,    64,    65,    66,    -1,    -1,
      97,    70,    71,   100,    -1,   102,    -1,    70,    71,    44,
      45,    -1,    -1,    -1,    49,    84,    85,    52,    53,    88,
      89,    84,    85,    -1,    -1,    88,    89,    -1,    97,    -1,
      -1,    -1,    -1,   102,    97,    70,    71,    -1,   101,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    49,    84,
      85,    52,    53,    88,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    97,    44,    45,    -1,   101,    -1,    49,    70,
      71,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    84,    85,    -1,    -1,    88,    89,    70,
      71,    -1,    -1,    -1,    -1,    -1,    97,    44,    45,    -1,
     101,    -1,    49,    84,    85,    52,    53,    88,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    97,    44,    45,    -1,
     101,    -1,    49,    70,    71,    52,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    84,    85,    -1,
      -1,    88,    89,    70,    71,    -1,    -1,    -1,    -1,    -1,
      97,    44,    45,    -1,   101,    -1,    49,    84,    85,    52,
      53,    88,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      97,    44,    45,    -1,   101,    -1,    49,    70,    71,    52,
      53,    -1,    -1,    56,    -1,    44,    45,    -1,    -1,    -1,
      49,    84,    85,    -1,    53,    88,    89,    70,    71,    -1,
      -1,    44,    45,    -1,    97,    -1,    49,   100,    -1,    52,
      53,    84,    85,    -1,    -1,    88,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    97,    -1,    99,    70,    71,    88,
      89,    44,    45,    -1,    -1,    -1,    49,    -1,    97,    52,
      53,    84,    85,    -1,    -1,    88,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    97,    -1,    99,    70,    71,    -1,
      -1,    44,    45,    -1,    -1,    -1,    49,    -1,    -1,    52,
      53,    84,    85,    -1,    -1,    88,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    97,    -1,    99,    70,    71,    -1,
      -1,    44,    45,    -1,    -1,    -1,    49,    -1,    -1,    52,
      53,    84,    85,    -1,    -1,    88,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    97,    -1,    99,    70,    71,    -1,
      -1,    44,    45,    -1,    -1,    -1,    49,    -1,    -1,    52,
      53,    84,    85,    -1,    -1,    88,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    97,    -1,    99,    70,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    84,    85,    -1,    -1,    88,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    97
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,   106,   107,     0,     1,     3,     6,     8,    13,    17,
      18,    19,    20,    21,    23,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      46,    47,    50,    51,    58,    59,    60,    61,    62,    67,
      68,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      97,   108,   110,   127,   153,   154,    99,    14,   128,     3,
     125,   126,     3,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,     4,
     112,     4,    85,   113,   127,   127,   113,   127,   127,   127,
     127,   127,   127,   127,   127,    97,   155,     3,   127,   127,
     109,    44,    45,    49,    52,    53,    56,    70,    71,    84,
      85,    88,    89,    97,    99,    99,   127,     3,     4,    11,
      15,    16,    85,    97,   129,   134,   135,   141,   142,   144,
     147,   148,   149,   152,    97,    99,   100,    22,    22,    22,
      99,    99,     4,   111,   127,     4,   115,   127,    63,    64,
      65,    66,   102,   116,   117,   118,     3,   156,     6,   157,
      48,   104,   110,   127,   127,     4,   127,   127,   127,   127,
     127,    24,   127,   127,   127,    99,    97,    97,   149,   141,
      97,    97,   152,    15,    16,   141,   152,     7,    43,    54,
      55,   101,   132,   133,   102,     9,    10,     3,   102,   138,
     139,   140,   140,    69,    84,    85,    88,   100,     4,   126,
     127,   127,   127,     4,    99,   100,   101,    22,   100,   114,
     101,   127,     4,     4,     4,     5,   114,   100,   100,   104,
       3,   158,   155,    99,   127,   104,   150,   152,   104,   104,
     130,   141,     3,    88,   136,   137,   152,   141,   141,   139,
     100,    54,    55,   101,   143,   101,   145,   147,   148,   152,
     152,   152,   147,   104,   127,   127,   127,   127,    99,   127,
     116,     5,   119,   120,     4,     4,   100,    99,   118,     3,
      43,   101,     7,   100,   157,     3,   151,   134,   100,   103,
     101,   103,     3,   141,   141,   101,   101,   101,   100,   102,
     100,   116,     4,   127,   127,     3,   127,     3,   100,   104,
     131,   137,   152,   104,   146,   146,   127,   127,   127,   127,
     127,   120,   116,   100,   116,   100,   101,    43,   101,     3,
     132,   101,   101,   103,   127,   119,   127,     3,   127,     4,
     127,   100,   116,   100,   103,   101,   127,   102,   121,   122,
     127,     4,   123,   124,   100,   103,   100,   100,   103,   122,
     127,   124
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (0)
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
              (Loc).first_line, (Loc).first_column,	\
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr,					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname[yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      size_t yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

#endif /* YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()
    ;
#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 161 "../src/parser.y"
    { 
        }
    break;

  case 5:
#line 166 "../src/parser.y"
    { assert( current_Declaration_Site == globalDecls);}
    break;

  case 7:
#line 171 "../src/parser.y"
    {
		flushScanBuffer();
		/* Kill all the local declarations -- ejr */
		Declaration_Site *ds1, *ds2;
		for(ds1 = current_Declaration_Site; ds1 != globalDecls;) {
		    ds2 = ds1;
		    ds1=ds1->previous;
		    delete ds2;
		}
                current_Declaration_Site = globalDecls;
		yyerror("skipping to statement end");
		}
    break;

  case 8:
#line 184 "../src/parser.y"
    { flushScanBuffer();
		}
    break;

  case 9:
#line 187 "../src/parser.y"
    {
			  flushScanBuffer();
			  (yyvsp[-1].RELATION)->simplify(::min(2,redundant_conj_level),4);
			  Relation *r = relationMap((Const_String)(yyvsp[-3].VAR_NAME));
			  if (r) delete r;
			  relationMap[(Const_String)(yyvsp[-3].VAR_NAME)] = (yyvsp[-1].RELATION); 
			  delete (yyvsp[-3].VAR_NAME);
			}
    break;

  case 10:
#line 195 "../src/parser.y"
    { 
			  flushScanBuffer();
			printf("\n"); 
			(yyvsp[-1].RELATION)->simplify(redundant_conj_level,4);
			(yyvsp[-1].RELATION)->print_with_subs(stdout); 
			printf("\n"); 
			delete (yyvsp[-1].RELATION);
			}
    break;

  case 11:
#line 203 "../src/parser.y"
    {

#if defined(OMIT_GETRUSAGE)
	    printf("'time' requires getrusage, but the omega calclator was compiled with OMIT_GETRUSAGE set!\n");

#else

			flushScanBuffer();
			printf("\n");
			int t;
			Relation R;
			bool SKIP_FULL_CHECK = getenv("OC_TIMING_SKIP_FULL_CHECK");
			((yyvsp[-1].RELATION))->and_with_GEQ();
			start_clock();
			for (t=1;t<=100;t++) {
				R = *(yyvsp[-1].RELATION);
				R.finalize();
				}
			int copyTime = clock_diff();
			start_clock();
			for (t=1;t<=100;t++) {
				R = *(yyvsp[-1].RELATION);
				R.finalize();
				R.simplify();
				}
			int simplifyTime = clock_diff() -copyTime;
			Relation R2;
			if (!SKIP_FULL_CHECK)
			  {
			    start_clock();
			    for (t=1;t<=100;t++) {
			      R2 = *(yyvsp[-1].RELATION);
			      R2.finalize();
			      R2.simplify(2,4);
			    }
			  }
			int excessiveTime = clock_diff() - copyTime;
			printf("Times (in microseconds): \n");
			printf("%5d us to copy original set of constraints\n",copyTime/100);
			printf("%5d us to do the default amount of simplification, obtaining: \n\t",
					simplifyTime/100);
			R.print_with_subs(stdout); 
		    printf("\n"); 
			if (!SKIP_FULL_CHECK)
			  {
			    printf("%5d us to do the maximum (i.e., excessive) amount of simplification, obtaining: \n\t",
				   excessiveTime/100);
			    R2.print_with_subs(stdout); 
			    printf("\n");
			  }
			if (!anyTimingDone) {
				bool warn = false;
#ifndef SPEED 
				warn =true;
#endif
#ifndef NDEBUG
				warn = true;
#endif
				if (warn) {
				   printf("WARNING: The Omega calculator was compiled with options that force\n");
				   printf("it to perform additional consistency and error checks\n");
				   printf("that may slow it down substantially\n");
				  printf("\n");
				   }
				printf("NOTE: These times relect the time of the current _implementation_\n");
				printf("of our algorithms. Performance bugs do exist. If you intend to publish or \n");
				printf("report on the performance on the Omega test, we respectfully but strongly \n");
				printf("request that send your test cases to us to allow us to determine if the \n");
				printf("times are appropriate, and if the way you are using the Omega library to \n"); 
				printf("solve your problem is the most effective way.\n");
				printf("\n");

printf("Also, please be aware that over the past two years, we have focused our \n");
printf("efforts on the expressive power of the Omega library, sometimes at the\n");
printf("expensive of raw speed. Our original implementation of the Omega test\n");
printf("was substantially faster on the limited domain it handled.\n");
				printf("\n");
				printf("	Thanks, \n");
				printf("	   the Omega Team \n");	
				}			 
			anyTimingDone = true;
			delete (yyvsp[-1].RELATION);
#endif
			}
    break;

  case 12:
#line 287 "../src/parser.y"
    {

#if defined(OMIT_GETRUSAGE)
	    printf("'timeclosure' requires getrusage, but the omega calclator was compiled with OMIT_GETRUSAGE set!\n");
#else
			flushScanBuffer();
			printf("\n");
			int t;
			Relation R;
			((yyvsp[-1].RELATION))->and_with_GEQ();
			start_clock();
			for (t=1;t<=100;t++) {
				R = *(yyvsp[-1].RELATION);
				R.finalize();
				}
			int copyTime = clock_diff();
			start_clock();
			for (t=1;t<=100;t++) {
				R = *(yyvsp[-1].RELATION);
				R.finalize();
				R.simplify();
				};
			int simplifyTime = clock_diff() -copyTime;
			Relation Rclosed;
			start_clock();
			for (t=1;t<=100;t++) {
				Rclosed = *(yyvsp[-1].RELATION);
				Rclosed.finalize();
				Rclosed = TransitiveClosure(Rclosed, 1,Relation::Null());
				};
			int closureTime = clock_diff() - copyTime;
			Relation R2;
			start_clock();
			for (t=1;t<=100;t++) {
				R2 = *(yyvsp[-1].RELATION);
				R2.finalize();
				R2.simplify(2,4);
				};
			int excessiveTime = clock_diff() - copyTime;
			printf("Times (in microseconds): \n");
			printf("%5d us to copy original set of constraints\n",copyTime/100);
			printf("%5d us to do the default amount of simplification, obtaining: \n\t",
					simplifyTime/100);
			R.print_with_subs(stdout); 
		    printf("\n"); 
			printf("%5d us to do the maximum (i.e., excessive) amount of simplification, obtaining: \n\t",
					excessiveTime/100);
			R2.print_with_subs(stdout); 
			printf("%5d us to do the transitive closure, obtaining: \n\t",
					closureTime/100);
			Rclosed.print_with_subs(stdout);
		        printf("\n");
			if (!anyTimingDone) {
				bool warn = false;
#ifndef SPEED 
				warn =true;
#endif
#ifndef NDEBUG
				warn = true;
#endif
				if (warn) {
				   printf("WARNING: The Omega calculator was compiled with options that force\n");
				   printf("it to perform additional consistency and error checks\n");
				   printf("that may slow it down substantially\n");
				  printf("\n");
				   }
				printf("NOTE: These times relect the time of the current _implementation_\n");
				printf("of our algorithms. Performance bugs do exist. If you intend to publish or \n");
				printf("report on the performance on the Omega test, we respectfully but strongly \n");
				printf("request that send your test cases to us to allow us to determine if the \n");
				printf("times are appropriate, and if the way you are using the Omega library to \n"); 
				printf("solve your problem is the most effective way.\n");
				printf("\n");

printf("Also, please be aware that over the past two years, we have focused our \n");
printf("efforts on the expressive power of the Omega library, sometimes at the\n");
printf("expensive of raw speed. Our original implementation of the Omega test\n");
printf("was substantially faster on the limited domain it handled.\n");
				printf("\n");
				printf("	Thanks, \n");
				printf("	   the Omega Team \n");	
				}			 
			anyTimingDone = true;
			delete (yyvsp[-1].RELATION);
#endif
			}
    break;

  case 13:
#line 375 "../src/parser.y"
    {
			  flushScanBuffer();
	                int c = Must_Be_Subset(*(yyvsp[-3].RELATION), *(yyvsp[-1].RELATION));
			printf("\n%s\n", c ? "True" : "False");
			delete (yyvsp[-3].RELATION);
			delete (yyvsp[-1].RELATION);
			}
    break;

  case 14:
#line 383 "../src/parser.y"
    {
			  flushScanBuffer();
			  String s = MMGenerateCode((yyvsp[-2].REL_TUPLE_PAIR)->mappings, (yyvsp[-2].REL_TUPLE_PAIR)->ispaces,*(yyvsp[-1].RELATION),(yyvsp[-3].INT_VALUE));
			  delete (yyvsp[-1].RELATION);
			  delete (yyvsp[-2].REL_TUPLE_PAIR);
			  printf("%s\n", (const char *) s); 
	               }
    break;

  case 15:
#line 391 "../src/parser.y"
    {
			  flushScanBuffer();
			  String s = tcodegen((yyvsp[-3].INT_VALUE), *((yyvsp[-2].STM_INFO_TUPLE)), *((yyvsp[-1].RELATION)));
			  delete (yyvsp[-1].RELATION);
			  delete (yyvsp[-2].STM_INFO_TUPLE);
			  printf("%s\n", (const char *) s); 
			}
    break;

  case 16:
#line 408 "../src/parser.y"
    {
	    Tuple<Free_Var_Decl*> lowerBounds(0), upperBounds(0), my_procs(0);
            Tuple<spmd_stmt_info *> names(0);

	    flushScanBuffer();
	    int nr_statements = (yyvsp[-1].REL_TUPLE_TRIPLE)->space.size();

	    for (int i = 1; i<= (yyvsp[-1].REL_TUPLE_TRIPLE)->space[1].n_out(); i++)
		{
	        lowerBounds.append(new Free_Var_Decl("lb" + itoS(i)));
	        upperBounds.append(new Free_Var_Decl("ub" + itoS(i)));
	        my_procs.append(new Free_Var_Decl("my_proc" + itoS(i)));
		}

            for (int p = 1; p <= nr_statements; p++)
                names.append(new numbered_stmt_info(p-1, Identity((yyvsp[-1].REL_TUPLE_TRIPLE)->time[p].n_out()),
					            (yyvsp[-1].REL_TUPLE_TRIPLE)->space[p], 
					(char *)(const char *)("s"+itoS(p-1))));

	    String s = SPMD_GenerateCode("", (yyvsp[-1].REL_TUPLE_TRIPLE)->space, (yyvsp[-1].REL_TUPLE_TRIPLE)->time, (yyvsp[-1].REL_TUPLE_TRIPLE)->ispaces, 
					 names,
					 lowerBounds, upperBounds, my_procs,
                                         nr_statements);

	    delete (yyvsp[-1].REL_TUPLE_TRIPLE);
	    printf("%s\n", (const char *) s); 
            }
    break;

  case 17:
#line 436 "../src/parser.y"
    { 	flushScanBuffer();
		Dynamic_Array1<Relation> &final = *(yyvsp[-1].RELATION_ARRAY_1D);
		bool any_sat=false;
		int i,n_nodes = reachable_info->node_names.size();
		for(i = 1; i <= n_nodes; i++) if(final[i].is_upper_bound_satisfiable()) {
		  any_sat = true;
		  fprintf(stdout,"Node %s: ",
			  (const char *) (reachable_info->node_names[i]));
		  final[i].print_with_subs(stdout);
		}
		if(!any_sat)
		  fprintf(stdout,"No nodes reachable.\n");
		delete (yyvsp[-1].RELATION_ARRAY_1D);
		delete reachable_info;
	}
    break;

  case 18:
#line 454 "../src/parser.y"
    {
                (yyvsp[-6].REL_TUPLE_TRIPLE)->space.append(*(yyvsp[-4].RELATION));
                (yyvsp[-6].REL_TUPLE_TRIPLE)->time.append(*(yyvsp[-2].RELATION));
                (yyvsp[-6].REL_TUPLE_TRIPLE)->ispaces.append(*(yyvsp[0].RELATION));
                delete (yyvsp[-4].RELATION);
                delete (yyvsp[-2].RELATION);
                delete (yyvsp[0].RELATION);
                (yyval.REL_TUPLE_TRIPLE) = (yyvsp[-6].REL_TUPLE_TRIPLE);
                }
    break;

  case 19:
#line 464 "../src/parser.y"
    {
                RelTupleTriple *rtt = new RelTupleTriple;
                rtt->space.append(*(yyvsp[-4].RELATION));
                rtt->time.append(*(yyvsp[-2].RELATION));
                rtt->ispaces.append(*(yyvsp[0].RELATION));
                delete (yyvsp[-4].RELATION);
                delete (yyvsp[-2].RELATION);
                delete (yyvsp[0].RELATION);
                (yyval.REL_TUPLE_TRIPLE) = rtt;
                }
    break;

  case 20:
#line 476 "../src/parser.y"
    { Block_Size = 0; Num_Procs = 0; overheadEffort=0; }
    break;

  case 21:
#line 477 "../src/parser.y"
    { Block_Size = (yyvsp[0].INT_VALUE); Num_Procs = 0; overheadEffort=0;}
    break;

  case 22:
#line 478 "../src/parser.y"
    { Block_Size = (yyvsp[-1].INT_VALUE); Num_Procs = (yyvsp[0].INT_VALUE); overheadEffort=0;}
    break;

  case 23:
#line 479 "../src/parser.y"
    { Block_Size = (yyvsp[-2].INT_VALUE); Num_Procs = (yyvsp[-1].INT_VALUE); overheadEffort=(yyvsp[0].INT_VALUE);}
    break;

  case 24:
#line 482 "../src/parser.y"
    { (yyval.INT_VALUE) = 0; }
    break;

  case 25:
#line 483 "../src/parser.y"
    { (yyval.INT_VALUE) = (yyvsp[0].INT_VALUE); }
    break;

  case 26:
#line 484 "../src/parser.y"
    { (yyval.INT_VALUE) = -(yyvsp[0].INT_VALUE); }
    break;

  case 27:
#line 487 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		*(yyval.RELATION) = Relation::Null(); }
    break;

  case 28:
#line 489 "../src/parser.y"
    {(yyval.RELATION) = (yyvsp[0].RELATION); }
    break;

  case 29:
#line 493 "../src/parser.y"
    {
	        (yyvsp[-4].REL_TUPLE_PAIR)->mappings.append(*(yyvsp[-2].RELATION));
		(yyvsp[-4].REL_TUPLE_PAIR)->mappings[(yyvsp[-4].REL_TUPLE_PAIR)->mappings.size()].compress();
		(yyvsp[-4].REL_TUPLE_PAIR)->ispaces.append(*(yyvsp[0].RELATION));
		(yyvsp[-4].REL_TUPLE_PAIR)->ispaces[(yyvsp[-4].REL_TUPLE_PAIR)->ispaces.size()].compress();
		delete (yyvsp[-2].RELATION);
		delete (yyvsp[0].RELATION);
	        (yyval.REL_TUPLE_PAIR) = (yyvsp[-4].REL_TUPLE_PAIR);
                }
    break;

  case 30:
#line 503 "../src/parser.y"
    {
	        (yyvsp[-2].REL_TUPLE_PAIR)->mappings.append(Identity((yyvsp[0].RELATION)->n_set()));
		(yyvsp[-2].REL_TUPLE_PAIR)->mappings[(yyvsp[-2].REL_TUPLE_PAIR)->mappings.size()].compress();
		(yyvsp[-2].REL_TUPLE_PAIR)->ispaces.append(*(yyvsp[0].RELATION));
		(yyvsp[-2].REL_TUPLE_PAIR)->ispaces[(yyvsp[-2].REL_TUPLE_PAIR)->ispaces.size()].compress();
		delete (yyvsp[0].RELATION);
	        (yyval.REL_TUPLE_PAIR) = (yyvsp[-2].REL_TUPLE_PAIR);
                }
    break;

  case 31:
#line 512 "../src/parser.y"
    {
                RelTuplePair *rtp = new RelTuplePair;
	        rtp->mappings.append(*(yyvsp[-2].RELATION));
		rtp->mappings[rtp->mappings.size()].compress();
	        rtp->ispaces.append(*(yyvsp[0].RELATION));
		rtp->ispaces[rtp->ispaces.size()].compress();
		delete (yyvsp[-2].RELATION);
		delete (yyvsp[0].RELATION);
	        (yyval.REL_TUPLE_PAIR) = rtp;
		}
    break;

  case 32:
#line 523 "../src/parser.y"
    {
                RelTuplePair *rtp = new RelTuplePair;
	        rtp->mappings.append(Identity((yyvsp[0].RELATION)->n_set()));
		rtp->mappings[rtp->mappings.size()].compress();
	        rtp->ispaces.append(*(yyvsp[0].RELATION));
		rtp->ispaces[rtp->ispaces.size()].compress();
		delete (yyvsp[0].RELATION);
	        (yyval.REL_TUPLE_PAIR) = rtp;
                }
    break;

  case 33:
#line 535 "../src/parser.y"
    { (yyval.STM_INFO_TUPLE) = (yyvsp[0].STM_INFO_TUPLE); }
    break;

  case 34:
#line 542 "../src/parser.y"
    { (yyval.STM_INFO_TUPLE) = &Trans_IS(*((yyvsp[0].STM_INFO_TUPLE)), *((yyvsp[-1].RELATION)));
		  delete (yyvsp[-1].RELATION);
		}
    break;

  case 35:
#line 546 "../src/parser.y"
    { (yyval.STM_INFO_TUPLE) = &Set_MMap(*((yyvsp[0].STM_INFO_TUPLE)), (yyvsp[-2].INT_VALUE), *((yyvsp[-1].MMAP)));
		  delete (yyvsp[-1].MMAP);
		}
    break;

  case 36:
#line 550 "../src/parser.y"
    { (yyval.STM_INFO_TUPLE) = &Unroll_One_IS(*((yyvsp[0].STM_INFO_TUPLE)), (yyvsp[-3].INT_VALUE), (yyvsp[-2].INT_VALUE), (yyvsp[-1].INT_VALUE));}
    break;

  case 37:
#line 552 "../src/parser.y"
    { (yyval.STM_INFO_TUPLE) = &Peel_One_IS(*((yyvsp[0].STM_INFO_TUPLE)), (yyvsp[-3].INT_VALUE), (yyvsp[-2].INT_VALUE), *((yyvsp[-1].RELATION)));
		  delete (yyvsp[-1].RELATION);
		}
    break;

  case 38:
#line 556 "../src/parser.y"
    { (yyval.STM_INFO_TUPLE) = &Peel_One_IS(*((yyvsp[0].STM_INFO_TUPLE)), (yyvsp[-5].INT_VALUE), (yyvsp[-4].INT_VALUE), *((yyvsp[-3].RELATION)), *((yyvsp[-1].RELATION)));
		  delete (yyvsp[-3].RELATION);
		  delete (yyvsp[-1].RELATION);
		}
    break;

  case 39:
#line 562 "../src/parser.y"
    { (yyval.STM_INFO_TUPLE) = new Tuple<stm_info>;
						  (yyval.STM_INFO_TUPLE)->append(*((yyvsp[0].STM_INFO)));
						  delete (yyvsp[0].STM_INFO); }
    break;

  case 40:
#line 565 "../src/parser.y"
    { (yyval.STM_INFO_TUPLE) = (yyvsp[-2].STM_INFO_TUPLE);
						  (yyval.STM_INFO_TUPLE)->append(*((yyvsp[0].STM_INFO)));
						  delete (yyvsp[0].STM_INFO); }
    break;

  case 41:
#line 571 "../src/parser.y"
    { (yyval.STM_INFO) = (yyvsp[-1].STM_INFO);
		  (yyval.STM_INFO)->stm = *((yyvsp[-7].STRING_VALUE)); delete (yyvsp[-7].STRING_VALUE);
		  (yyval.STM_INFO)->IS  = *((yyvsp[-5].RELATION)); delete (yyvsp[-5].RELATION);
		  (yyval.STM_INFO)->map = *((yyvsp[-3].MMAP)); delete (yyvsp[-3].MMAP);
		  }
    break;

  case 42:
#line 577 "../src/parser.y"
    { (yyval.STM_INFO) = new stm_info;
		  (yyval.STM_INFO)->stm = *((yyvsp[-5].STRING_VALUE)); delete (yyvsp[-5].STRING_VALUE);
		  (yyval.STM_INFO)->IS  = *((yyvsp[-3].RELATION)); delete (yyvsp[-3].RELATION);
		  (yyval.STM_INFO)->map = *((yyvsp[-1].MMAP)); delete (yyvsp[-1].MMAP);
		}
    break;

  case 43:
#line 585 "../src/parser.y"
    { (yyval.MMAP) = (yyvsp[-2].MMAP);
				  (yyval.MMAP)->partials.append(*((yyvsp[0].PMMAP))); delete (yyvsp[0].PMMAP);
				}
    break;

  case 44:
#line 588 "../src/parser.y"
    { (yyval.MMAP) = new MMap;
				  (yyval.MMAP)->partials.append(*((yyvsp[0].PMMAP))); delete (yyvsp[0].PMMAP);
				}
    break;

  case 45:
#line 594 "../src/parser.y"
    { (yyval.PMMAP) = new PartialMMap;
					  (yyval.PMMAP)->mapping = *((yyvsp[0].RELATION)); delete (yyvsp[0].RELATION);
					  (yyval.PMMAP)->bounds  = *((yyvsp[-3].RELATION)); delete (yyvsp[-3].RELATION);
					  (yyval.PMMAP)->var     = *((yyvsp[-5].STRING_VALUE)); delete (yyvsp[-5].STRING_VALUE);
					}
    break;

  case 46:
#line 599 "../src/parser.y"
    { (yyval.PMMAP) = new PartialMMap;
					  (yyval.PMMAP)->mapping = *((yyvsp[0].RELATION)); delete (yyvsp[0].RELATION);
					  (yyval.PMMAP)->bounds  = Relation::True(0);
					  (yyval.PMMAP)->var     = *((yyvsp[-2].STRING_VALUE)); delete (yyvsp[-2].STRING_VALUE);
					}
    break;

  case 47:
#line 606 "../src/parser.y"
    { (yyval.STM_INFO) = (yyvsp[-2].STM_INFO);
				  (yyval.STM_INFO)->read.append(*((yyvsp[0].READ))); delete (yyvsp[0].READ);
				}
    break;

  case 48:
#line 609 "../src/parser.y"
    { (yyval.STM_INFO) = new stm_info;
				  (yyval.STM_INFO)->read.append(*((yyvsp[0].READ))); delete (yyvsp[0].READ);
				}
    break;

  case 49:
#line 614 "../src/parser.y"
    { (yyval.READ) = (yyvsp[-1].READ); }
    break;

  case 50:
#line 617 "../src/parser.y"
    { (yyval.READ) = (yyvsp[-2].READ);
				  (yyval.READ)->partials.append(*((yyvsp[0].PREAD))); delete (yyvsp[0].PREAD);
				}
    break;

  case 51:
#line 620 "../src/parser.y"
    { (yyval.READ) = new Read;
				  (yyval.READ)->partials.append(*((yyvsp[0].PREAD))); delete (yyvsp[0].PREAD);
				}
    break;

  case 52:
#line 625 "../src/parser.y"
    { (yyval.PREAD) = new PartialRead;
				  (yyval.PREAD)->from = (yyvsp[-2].INT_VALUE);
				  (yyval.PREAD)->dataFlow = *((yyvsp[0].RELATION)); delete (yyvsp[0].RELATION);
				}
    break;

  case 55:
#line 636 "../src/parser.y"
    { globalDecls->extend_both_tuples((yyvsp[-3].VAR_NAME), (yyvsp[-1].INT_VALUE)); free((yyvsp[-3].VAR_NAME)); }
    break;

  case 56:
#line 638 "../src/parser.y"
    { globalDecls->extend((yyvsp[0].VAR_NAME)); free((yyvsp[0].VAR_NAME)); }
    break;

  case 57:
#line 642 "../src/parser.y"
    { relationDecl = new Declaration_Site(); }
    break;

  case 58:
#line 645 "../src/parser.y"
    { (yyval.RELATION) = (yyvsp[-1].RELATION); 
		  if (omega_calc_debug) {
			fprintf(DebugFile,"Built relation:\n");
			(yyval.RELATION)->prefix_print(DebugFile);
			};
		  current_Declaration_Site = globalDecls;
		  delete relationDecl;
		}
    break;

  case 59:
#line 653 "../src/parser.y"
    {
		Const_String s = (yyvsp[0].VAR_NAME);
		free((yyvsp[0].VAR_NAME));
		if (relationMap(s) == 0) {
			fprintf(stderr,"Variable %s not declared\n",(yyvsp[0].VAR_NAME));
			YYERROR;
			assert(0);
			};
		(yyval.RELATION) = new Relation(*relationMap(s));
		}
    break;

  case 60:
#line 663 "../src/parser.y"
    {(yyval.RELATION) = (yyvsp[-1].RELATION);}
    break;

  case 61:
#line 665 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = TransitiveClosure(*(yyvsp[-1].RELATION), 1,Relation::Null());
		  delete (yyvsp[-1].RELATION);
		}
    break;

  case 62:
#line 670 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  int vars = (yyvsp[-1].RELATION)->n_inp();
		  *(yyval.RELATION) = Union(Identity(vars),
			      TransitiveClosure(*(yyvsp[-1].RELATION), 1,Relation::Null()));
		  delete (yyvsp[-1].RELATION);
		}
    break;

  case 63:
#line 677 "../src/parser.y"
    {(yyval.RELATION) = new Relation();
                 *(yyval.RELATION)= TransitiveClosure(*(yyvsp[-3].RELATION), 1,*(yyvsp[0].RELATION));
                 delete (yyvsp[-3].RELATION);
                 delete (yyvsp[0].RELATION);
	       }
    break;

  case 64:
#line 683 "../src/parser.y"
    {
		Relation o(*(yyvsp[0].RELATION));
		Relation r(*(yyvsp[0].RELATION));
		r = Join(r,LexForward((yyvsp[0].RELATION)->n_out()));
		(yyval.RELATION) = new Relation();
		*(yyval.RELATION) = Difference(o,r);
		delete (yyvsp[0].RELATION);
		}
    break;

  case 65:
#line 692 "../src/parser.y"
    {
		Relation o(*(yyvsp[0].RELATION));
		Relation r(*(yyvsp[0].RELATION));
		r = Join(r,Inverse(LexForward((yyvsp[0].RELATION)->n_out())));
		(yyval.RELATION) = new Relation();
		*(yyval.RELATION) = Difference(o,r);
		delete (yyvsp[0].RELATION);
		}
    break;

  case 66:
#line 701 "../src/parser.y"
    {
		Relation o(*(yyvsp[0].RELATION));
		Relation r(*(yyvsp[0].RELATION));
		r = Join(LexForward((yyvsp[0].RELATION)->n_inp()),r);
		(yyval.RELATION) = new Relation();
		*(yyval.RELATION) = Difference(o,r);
		delete (yyvsp[0].RELATION);
		}
    break;

  case 67:
#line 710 "../src/parser.y"
    {
		Relation o(*(yyvsp[0].RELATION));
		Relation r(*(yyvsp[0].RELATION));
		r = Join(Inverse(LexForward((yyvsp[0].RELATION)->n_inp())),r);
		(yyval.RELATION) = new Relation();
		*(yyval.RELATION) = Difference(o,r);
		delete (yyvsp[0].RELATION);
		}
    break;

  case 68:
#line 719 "../src/parser.y"
    {
		Relation c(*(yyvsp[0].RELATION));
		Relation r(*(yyvsp[0].RELATION));
		(yyval.RELATION) = new Relation(); 
		*(yyval.RELATION) = Cross_Product(Relation(*(yyvsp[0].RELATION)),c);
		delete (yyvsp[0].RELATION);
		assert((yyval.RELATION)->n_inp() ==(yyval.RELATION)->n_out());
		*(yyval.RELATION) = Difference(r,Domain(Intersection(*(yyval.RELATION),LexForward((yyval.RELATION)->n_inp()))));
		}
    break;

  case 69:
#line 729 "../src/parser.y"
    {
		Relation c(*(yyvsp[0].RELATION));
		Relation r(*(yyvsp[0].RELATION));
		(yyval.RELATION) = new Relation(); 
		*(yyval.RELATION) = Cross_Product(Relation(*(yyvsp[0].RELATION)),c);
		delete (yyvsp[0].RELATION);
		assert((yyval.RELATION)->n_inp() ==(yyval.RELATION)->n_out());
		*(yyval.RELATION) = Difference(r,Range(Intersection(*(yyval.RELATION),LexForward((yyval.RELATION)->n_inp()))));
		}
    break;

  case 70:
#line 739 "../src/parser.y"
    {
		(yyval.RELATION) = new Relation();
		*(yyval.RELATION) = Farkas(*(yyvsp[0].RELATION), Basic_Farkas);
		delete (yyvsp[0].RELATION);
		}
    break;

  case 71:
#line 745 "../src/parser.y"
    {
		(yyval.RELATION) = new Relation();
		*(yyval.RELATION) = Farkas(*(yyvsp[0].RELATION), Decoupled_Farkas);
		delete (yyvsp[0].RELATION);
		}
    break;

  case 72:
#line 751 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = ConicClosure(*(yyvsp[-1].RELATION));
		  delete (yyvsp[-1].RELATION);
		}
    break;

  case 73:
#line 756 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Project_Sym(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 74:
#line 761 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Project_On_Sym(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 75:
#line 766 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Deltas(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 76:
#line 771 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
                  *(yyval.RELATION) = DeltasToRelation(*(yyvsp[0].RELATION),(yyvsp[0].RELATION)->n_set(),(yyvsp[0].RELATION)->n_set());
                  delete (yyvsp[0].RELATION);
                }
    break;

  case 77:
#line 776 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Domain(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 78:
#line 781 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = VennDiagramForm(*(yyvsp[0].RELATION),Relation::True(*(yyvsp[0].RELATION)));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 79:
#line 786 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = VennDiagramForm(*(yyvsp[-2].RELATION),*(yyvsp[0].RELATION));
		  delete (yyvsp[-2].RELATION);
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 80:
#line 792 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = ConvexHull(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 81:
#line 797 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Farkas(*(yyvsp[0].RELATION),Positive_Combination_Farkas);
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 82:
#line 802 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Farkas(*(yyvsp[0].RELATION),Convex_Combination_Farkas);
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 83:
#line 807 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = CheckForConvexRepresentation(CheckForConvexPairs(*(yyvsp[0].RELATION)));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 84:
#line 812 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = CheckForConvexRepresentation(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 85:
#line 817 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = AffineHull(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 86:
#line 822 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = ConicHull(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 87:
#line 827 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = LinearHull(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 88:
#line 832 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Hull(*(yyvsp[0].RELATION),false,1,Null_Relation());
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 89:
#line 837 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Hull(*(yyvsp[-2].RELATION),false,1,*(yyvsp[0].RELATION));
		  delete (yyvsp[-2].RELATION);
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 90:
#line 843 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Approximate(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 91:
#line 848 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Range(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 92:
#line 853 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Inverse(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 93:
#line 858 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Complement(*(yyvsp[0].RELATION));
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 94:
#line 863 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Gist(*(yyvsp[-2].RELATION),*(yyvsp[0].RELATION),1);
		  delete (yyvsp[-2].RELATION);
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 95:
#line 869 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Composition(*(yyvsp[-3].RELATION),*(yyvsp[-1].RELATION));
		  delete (yyvsp[-3].RELATION);
		  delete (yyvsp[-1].RELATION);
		}
    break;

  case 96:
#line 875 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Composition(*(yyvsp[-2].RELATION),*(yyvsp[0].RELATION));
		  delete (yyvsp[-2].RELATION);
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 97:
#line 881 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = After(*(yyvsp[-2].RELATION),(yyvsp[0].INT_VALUE),(yyvsp[0].INT_VALUE));
		  delete (yyvsp[-2].RELATION);
		  (*(yyval.RELATION)).prefix_print(stdout);
		}
    break;

  case 98:
#line 887 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Composition(*(yyvsp[0].RELATION),*(yyvsp[-2].RELATION));
		  delete (yyvsp[-2].RELATION);
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 99:
#line 893 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Restrict_Range(*(yyvsp[-2].RELATION),*(yyvsp[0].RELATION));
		  delete (yyvsp[-2].RELATION);
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 100:
#line 899 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Restrict_Domain(*(yyvsp[-2].RELATION),*(yyvsp[0].RELATION));
		  delete (yyvsp[-2].RELATION);
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 101:
#line 905 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Intersection(*(yyvsp[-2].RELATION),*(yyvsp[0].RELATION));
		  delete (yyvsp[-2].RELATION);
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 102:
#line 911 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
                  *(yyval.RELATION) = Difference(*(yyvsp[-2].RELATION),*(yyvsp[0].RELATION));
                  delete (yyvsp[-2].RELATION);
                  delete (yyvsp[0].RELATION);
                }
    break;

  case 103:
#line 917 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Union(*(yyvsp[-2].RELATION),*(yyvsp[0].RELATION));
		  delete (yyvsp[-2].RELATION);
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 104:
#line 923 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
		  *(yyval.RELATION) = Cross_Product(*(yyvsp[-2].RELATION),*(yyvsp[0].RELATION));
		  delete (yyvsp[-2].RELATION);
		  delete (yyvsp[0].RELATION);
		}
    break;

  case 105:
#line 929 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
                  *(yyval.RELATION) = Union(*(yyvsp[0].RELATION), Relation::Unknown(*(yyvsp[0].RELATION)));
                  delete (yyvsp[0].RELATION);
                }
    break;

  case 106:
#line 934 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
                  *(yyval.RELATION) = Intersection(*(yyvsp[0].RELATION), Relation::Unknown(*(yyvsp[0].RELATION)));
                  delete (yyvsp[0].RELATION);
                }
    break;

  case 107:
#line 939 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
                  *(yyval.RELATION) = Upper_Bound(*(yyvsp[0].RELATION));
                  delete (yyvsp[0].RELATION);
                }
    break;

  case 108:
#line 944 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
                  *(yyval.RELATION) = Lower_Bound(*(yyvsp[0].RELATION));
                  delete (yyvsp[0].RELATION);
                }
    break;

  case 109:
#line 949 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
                  *(yyval.RELATION) = Sample_Solution(*(yyvsp[0].RELATION));
                  delete (yyvsp[0].RELATION);
                }
    break;

  case 110:
#line 954 "../src/parser.y"
    { (yyval.RELATION) = new Relation();
                  *(yyval.RELATION) = Symbolic_Solution(*(yyvsp[0].RELATION));
                  delete (yyvsp[0].RELATION);
                }
    break;

  case 111:
#line 958 "../src/parser.y"
    { (yyval.RELATION) = (yyvsp[0].RELATION); }
    break;

  case 112:
#line 960 "../src/parser.y"
    {
		  if (((yyvsp[0].RELATION))->is_satisfiable())
			  {
			    fprintf(stderr,"assert_unsatisfiable failed on ");
			    ((yyvsp[0].RELATION))->print_with_subs(stderr);
			    Exit(1);
			  }
		  (yyval.RELATION)=(yyvsp[0].RELATION);
		}
    break;

  case 113:
#line 973 "../src/parser.y"
    {currentTuple = Output_Tuple;}
    break;

  case 114:
#line 974 "../src/parser.y"
    {currentTuple = Input_Tuple;}
    break;

  case 115:
#line 974 "../src/parser.y"
    {
		Relation * r = new Relation((yyvsp[-5].TUPLE_DESCRIPTOR)->size,(yyvsp[-2].TUPLE_DESCRIPTOR)->size);
		resetGlobals();
		F_And *f = r->add_and();
		int i;
		for(i=1;i<=(yyvsp[-5].TUPLE_DESCRIPTOR)->size;i++) {	
			(yyvsp[-5].TUPLE_DESCRIPTOR)->vars[i]->vid = r->input_var(i);
			if (!(yyvsp[-5].TUPLE_DESCRIPTOR)->vars[i]->anonymous) 
				r->name_input_var(i,(yyvsp[-5].TUPLE_DESCRIPTOR)->vars[i]->stripped_name);
			};
		for(i=1;i<=(yyvsp[-2].TUPLE_DESCRIPTOR)->size;i++) {
			(yyvsp[-2].TUPLE_DESCRIPTOR)->vars[i]->vid = r->output_var(i);
			if (!(yyvsp[-2].TUPLE_DESCRIPTOR)->vars[i]->anonymous) 
				r->name_output_var(i,(yyvsp[-2].TUPLE_DESCRIPTOR)->vars[i]->stripped_name);
			};
		foreach(e,Exp*,(yyvsp[-5].TUPLE_DESCRIPTOR)->eq_constraints, install_eq(f,e,0));
                foreach(e,Exp*,(yyvsp[-5].TUPLE_DESCRIPTOR)->geq_constraints, install_geq(f,e,0)); 
		foreach(c,strideConstraint*,(yyvsp[-5].TUPLE_DESCRIPTOR)->stride_constraints, install_stride(f,c));
		foreach(e,Exp*,(yyvsp[-2].TUPLE_DESCRIPTOR)->eq_constraints, install_eq(f,e,0));
		foreach(e,Exp*,(yyvsp[-2].TUPLE_DESCRIPTOR)->geq_constraints, install_geq(f,e,0));
		foreach(c,strideConstraint*,(yyvsp[-2].TUPLE_DESCRIPTOR)->stride_constraints, install_stride(f,c));
		if ((yyvsp[0].ASTP)) (yyvsp[0].ASTP)->install(f);
		delete (yyvsp[-5].TUPLE_DESCRIPTOR);
		delete (yyvsp[-2].TUPLE_DESCRIPTOR);
		delete (yyvsp[0].ASTP);
		(yyval.RELATION) = r; }
    break;

  case 116:
#line 1000 "../src/parser.y"
    {
	        Relation * r = new Relation((yyvsp[-1].TUPLE_DESCRIPTOR)->size);
		resetGlobals();
		F_And *f = r->add_and();
		int i;
		for(i=1;i<=(yyvsp[-1].TUPLE_DESCRIPTOR)->size;i++) {	
			(yyvsp[-1].TUPLE_DESCRIPTOR)->vars[i]->vid = r->set_var(i);
			if (!(yyvsp[-1].TUPLE_DESCRIPTOR)->vars[i]->anonymous) 
				r->name_set_var(i,(yyvsp[-1].TUPLE_DESCRIPTOR)->vars[i]->stripped_name);
			};
                foreach(e,Exp*,(yyvsp[-1].TUPLE_DESCRIPTOR)->eq_constraints, install_eq(f,e,0)); 
		foreach(e,Exp*,(yyvsp[-1].TUPLE_DESCRIPTOR)->geq_constraints, install_geq(f,e,0));
		foreach(c,strideConstraint*,(yyvsp[-1].TUPLE_DESCRIPTOR)->stride_constraints, install_stride(f,c));
		if ((yyvsp[0].ASTP)) (yyvsp[0].ASTP)->install(f);
		delete (yyvsp[-1].TUPLE_DESCRIPTOR);
		delete (yyvsp[0].ASTP);
		(yyval.RELATION) = r; }
    break;

  case 117:
#line 1017 "../src/parser.y"
    {
		Relation * r = new Relation(0,0);
		F_And *f = r->add_and();
		(yyvsp[0].ASTP)->install(f);
		delete (yyvsp[0].ASTP);
		(yyval.RELATION) = r;
		}
    break;

  case 118:
#line 1026 "../src/parser.y"
    { (yyval.ASTP) = (yyvsp[0].ASTP); }
    break;

  case 119:
#line 1027 "../src/parser.y"
    {(yyval.ASTP) = 0;}
    break;

  case 123:
#line 1036 "../src/parser.y"
    { currentTupleDescriptor = new tupleDescriptor; tuplePos = 1; }
    break;

  case 124:
#line 1038 "../src/parser.y"
    {(yyval.TUPLE_DESCRIPTOR) = currentTupleDescriptor; }
    break;

  case 128:
#line 1048 "../src/parser.y"
    { Declaration_Site *ds = defined((yyvsp[0].VAR_NAME));
	  if (!ds) currentTupleDescriptor->extend((yyvsp[0].VAR_NAME),currentTuple,tuplePos);
	  else {
	      Variable_Ref * v = lookupScalar((yyvsp[0].VAR_NAME));
	      assert(v);
	      if (ds != globalDecls) 
		currentTupleDescriptor->extend((yyvsp[0].VAR_NAME), new Exp(v));
	      else 
		currentTupleDescriptor->extend(new Exp(v));
	      }
	  free((yyvsp[0].VAR_NAME));
	  tuplePos++;
	}
    break;

  case 129:
#line 1062 "../src/parser.y"
    {currentTupleDescriptor->extend(); tuplePos++; }
    break;

  case 130:
#line 1064 "../src/parser.y"
    {currentTupleDescriptor->extend((yyvsp[0].EXP)); tuplePos++; }
    break;

  case 131:
#line 1066 "../src/parser.y"
    {currentTupleDescriptor->extend((yyvsp[-2].EXP),(yyvsp[0].EXP)); tuplePos++; }
    break;

  case 132:
#line 1068 "../src/parser.y"
    {currentTupleDescriptor->extend((yyvsp[-4].EXP),(yyvsp[-2].EXP),(yyvsp[0].INT_VALUE)); tuplePos++; }
    break;

  case 133:
#line 1072 "../src/parser.y"
    {(yyval.VAR_LIST) = (yyvsp[-2].VAR_LIST); (yyval.VAR_LIST)->insert((yyvsp[0].VAR_NAME)); }
    break;

  case 134:
#line 1073 "../src/parser.y"
    { (yyval.VAR_LIST) = new VarList;
		(yyval.VAR_LIST)->insert((yyvsp[0].VAR_NAME)); }
    break;

  case 135:
#line 1078 "../src/parser.y"
    {
		(yyval.DECLARATION_SITE) = current_Declaration_Site = new Declaration_Site((yyvsp[0].VAR_LIST));
		foreach(s,char *, *(yyvsp[0].VAR_LIST), delete s);
		delete (yyvsp[0].VAR_LIST);
		}
    break;

  case 136:
#line 1087 "../src/parser.y"
    { (yyval.DECLARATION_SITE) = (yyvsp[0].DECLARATION_SITE); }
    break;

  case 137:
#line 1088 "../src/parser.y"
    { (yyval.DECLARATION_SITE) = (yyvsp[-1].DECLARATION_SITE); }
    break;

  case 138:
#line 1091 "../src/parser.y"
    { (yyval.ASTP) = new AST_And((yyvsp[-2].ASTP),(yyvsp[0].ASTP)); }
    break;

  case 139:
#line 1092 "../src/parser.y"
    { (yyval.ASTP) = new AST_Or((yyvsp[-2].ASTP),(yyvsp[0].ASTP)); }
    break;

  case 140:
#line 1093 "../src/parser.y"
    { (yyval.ASTP) = (yyvsp[0].ASTCP); }
    break;

  case 141:
#line 1094 "../src/parser.y"
    { (yyval.ASTP) = (yyvsp[-1].ASTP); }
    break;

  case 142:
#line 1095 "../src/parser.y"
    { (yyval.ASTP) = new AST_Not((yyvsp[0].ASTP)); }
    break;

  case 143:
#line 1097 "../src/parser.y"
    { (yyval.ASTP) = new AST_exists((yyvsp[-3].DECLARATION_SITE),(yyvsp[-1].ASTP)); }
    break;

  case 144:
#line 1099 "../src/parser.y"
    { (yyval.ASTP) = new AST_forall((yyvsp[-3].DECLARATION_SITE),(yyvsp[-1].ASTP)); }
    break;

  case 153:
#line 1119 "../src/parser.y"
    { popScope(); }
    break;

  case 154:
#line 1123 "../src/parser.y"
    {
		(yyval.EXP_LIST) = (yyvsp[0].EXP_LIST); 
		(yyval.EXP_LIST)->insert((yyvsp[-2].EXP));
		}
    break;

  case 155:
#line 1127 "../src/parser.y"
    {
		(yyval.EXP_LIST) = new ExpList;
		(yyval.EXP_LIST)->insert((yyvsp[0].EXP));
		}
    break;

  case 156:
#line 1134 "../src/parser.y"
    { (yyval.ASTCP) = new AST_constraints((yyvsp[-2].EXP_LIST),(yyvsp[-1].REL_OPERATOR),(yyvsp[0].EXP_LIST)); }
    break;

  case 157:
#line 1136 "../src/parser.y"
    { (yyval.ASTCP) = new AST_constraints((yyvsp[-2].EXP_LIST),(yyvsp[-1].REL_OPERATOR),(yyvsp[0].ASTCP)); }
    break;

  case 158:
#line 1141 "../src/parser.y"
    { Variable_Ref * v = lookupScalar((yyvsp[0].VAR_NAME));
		  if (!v) YYERROR;
		  (yyval.EXP) = new Exp(v); 
		  free((yyvsp[0].VAR_NAME)); 
		  }
    break;

  case 159:
#line 1146 "../src/parser.y"
    {argCount = 1;}
    break;

  case 160:
#line 1147 "../src/parser.y"
    {Variable_Ref *v;
		 if ((yyvsp[-1].ARGUMENT_TUPLE) == Input_Tuple) v = functionOfInput[(yyvsp[-4].VAR_NAME)];
		 else v = functionOfOutput[(yyvsp[-4].VAR_NAME)];
		 free((yyvsp[-4].VAR_NAME));
		 if (v == 0) {
			fprintf(stderr,"Function %s(...) not declared\n",(yyvsp[-4].VAR_NAME));
			YYERROR;
			}
		 (yyval.EXP) = new Exp(v);
		}
    break;

  case 161:
#line 1157 "../src/parser.y"
    { (yyval.EXP) = (yyvsp[-1].EXP);}
    break;

  case 162:
#line 1163 "../src/parser.y"
    {
		Variable_Ref * v = lookupScalar((yyvsp[0].VAR_NAME));
		if (!v) YYERROR;
		 free((yyvsp[0].VAR_NAME));
		 if (v->pos != argCount || v->of != (yyvsp[-2].ARGUMENT_TUPLE) || v->of != Input_Tuple && v->of != Output_Tuple) {
			fprintf(stderr,"arguments to function must be prefix of input or output tuple\n");
			YYERROR;
			}
		 (yyval.ARGUMENT_TUPLE) = v->of;
		 argCount++;
		}
    break;

  case 163:
#line 1174 "../src/parser.y"
    { Variable_Ref * v = lookupScalar((yyvsp[0].VAR_NAME));
		if (!v) YYERROR;
		 free((yyvsp[0].VAR_NAME));
		 if (v->pos != argCount || v->of != Input_Tuple && v->of != Output_Tuple) {
			fprintf(stderr,"arguments to function must be prefix of input or output tuple\n");
			YYERROR;
			}
		 (yyval.ARGUMENT_TUPLE) = v->of;
		 argCount++;
		}
    break;

  case 164:
#line 1186 "../src/parser.y"
    {(yyval.EXP) = new Exp((yyvsp[0].INT_VALUE));}
    break;

  case 165:
#line 1187 "../src/parser.y"
    {(yyval.EXP) = multiply((yyvsp[-1].INT_VALUE),(yyvsp[0].EXP));}
    break;

  case 166:
#line 1188 "../src/parser.y"
    { (yyval.EXP) = (yyvsp[0].EXP); }
    break;

  case 167:
#line 1189 "../src/parser.y"
    { (yyval.EXP) = ::negate((yyvsp[0].EXP));}
    break;

  case 168:
#line 1190 "../src/parser.y"
    { (yyval.EXP) = add((yyvsp[-2].EXP),(yyvsp[0].EXP));}
    break;

  case 169:
#line 1191 "../src/parser.y"
    { (yyval.EXP) = subtract((yyvsp[-2].EXP),(yyvsp[0].EXP));}
    break;

  case 170:
#line 1192 "../src/parser.y"
    { (yyval.EXP) = multiply((yyvsp[-2].EXP),(yyvsp[0].EXP));}
    break;

  case 171:
#line 1198 "../src/parser.y"
    {
		  Dynamic_Array1<Relation> *final =
		    Reachable_Nodes(reachable_info);
		  (yyval.RELATION_ARRAY_1D) = final;
		}
    break;

  case 172:
#line 1207 "../src/parser.y"
    {
		  Dynamic_Array1<Relation> *final =
		    Reachable_Nodes(reachable_info);
		  int index = reachable_info->node_names.index(String((yyvsp[-3].VAR_NAME)));
		  assert(index != 0 && "No such node");
		  (yyval.RELATION) = new Relation; 
		  *(yyval.RELATION) = (*final)[index];
		  delete final;
		  delete reachable_info;
		  delete (yyvsp[-3].VAR_NAME);
		}
    break;

  case 173:
#line 1222 "../src/parser.y"
    { int sz = reachable_info->node_names.size();
		reachable_info->node_arity.reallocate(sz);
	  	reachable_info->transitions.resize(sz+1,sz+1);
	  	reachable_info->start_nodes.resize(sz+1);
	      }
    break;

  case 174:
#line 1230 "../src/parser.y"
    { reachable_info->node_names.append(String((yyvsp[0].VAR_NAME)));
		delete (yyvsp[0].VAR_NAME); }
    break;

  case 175:
#line 1232 "../src/parser.y"
    { reachable_info = new reachable_information;
		reachable_info->node_names.append(String((yyvsp[0].VAR_NAME)));
		delete (yyvsp[0].VAR_NAME); }
    break;

  case 176:
#line 1239 "../src/parser.y"
    {  
	   int i,j;
	   int n_nodes = reachable_info->node_names.size();
	   Tuple<int> &arity = reachable_info->node_arity;
	   Dynamic_Array2<Relation> &transitions = reachable_info->transitions;

           /* fixup unspecified transitions to be false */
	   /* find arity */
	   for(i = 1; i <= n_nodes; i++) arity[i] = -1;
	   for(i = 1; i <= n_nodes; i++)
	     for(j = 1; j <= n_nodes; j++)
		if(! transitions[i][j].is_null()) {
		  int in_arity = transitions[i][j].n_inp();
		  int out_arity = transitions[i][j].n_out();
		  if(arity[i] < 0) arity[i] = in_arity;
		  if(arity[j] < 0) arity[j] = out_arity;
		  if(in_arity != arity[i] || out_arity != arity[j]) {
		    fprintf(stderr,
			    "Arity mismatch in node transition: %s -> %s",
			    (const char *) reachable_info->node_names[i],
			    (const char *) reachable_info->node_names[j]);
			assert(0);
		    YYERROR;
		  }
		}
	   for(i = 1; i <= n_nodes; i++) 
	     if(arity[i] < 0) arity[i] = 0;
	   /* Fill in false relations */
	   for(i = 1; i <= n_nodes; i++)
	     for(j = 1; j <= n_nodes; j++)
		if(transitions[i][j].is_null())
		  transitions[i][j] = Relation::False(arity[i],arity[j]);


          /* fixup unused start node positions */
	   Dynamic_Array1<Relation> &nodes = reachable_info->start_nodes;
	   for(i = 1; i <= n_nodes; i++) 
	     if(nodes[i].is_null()) 
		nodes[i] = Relation::False(arity[i]);
	     else
		if(nodes[i].n_set() != arity[i]){
		    fprintf(stderr,"Arity mismatch in start node %s",
			    (const char *) reachable_info->node_names[i]);
		assert(0);
		    YYERROR;
		}
 	}
    break;

  case 177:
#line 1290 "../src/parser.y"
    {  int n_nodes = reachable_info->node_names.size();
	     int index = reachable_info->node_names.index((yyvsp[-2].VAR_NAME));
	     assert(index != 0 && index <= n_nodes);
	     reachable_info->start_nodes[index] = *(yyvsp[0].RELATION);
	     delete (yyvsp[-2].VAR_NAME);
	     delete (yyvsp[0].RELATION);
	  }
    break;

  case 178:
#line 1298 "../src/parser.y"
    {  int n_nodes = reachable_info->node_names.size();
	     int from_index = reachable_info->node_names.index((yyvsp[-4].VAR_NAME));
	     int   to_index = reachable_info->node_names.index((yyvsp[-2].VAR_NAME));
	     assert(from_index != 0 && to_index != 0);
	     assert(from_index <= n_nodes && to_index <= n_nodes);
	     reachable_info->transitions[from_index][to_index] = *(yyvsp[0].RELATION);
	     delete (yyvsp[-4].VAR_NAME);
	     delete (yyvsp[-2].VAR_NAME);
	     delete (yyvsp[0].RELATION);
	  }
    break;

  case 179:
#line 1309 "../src/parser.y"
    {  int n_nodes = reachable_info->node_names.size();
	     int from_index = reachable_info->node_names.index((yyvsp[-4].VAR_NAME));
	     int   to_index = reachable_info->node_names.index((yyvsp[-2].VAR_NAME));
	     assert(from_index != 0 && to_index != 0);
	     assert(from_index <= n_nodes && to_index <= n_nodes);
	     reachable_info->transitions[from_index][to_index] = *(yyvsp[0].RELATION);
	     delete (yyvsp[-4].VAR_NAME);
	     delete (yyvsp[-2].VAR_NAME);
	     delete (yyvsp[0].RELATION);
	  }
    break;

  case 180:
#line 1320 "../src/parser.y"
    {  int n_nodes = reachable_info->node_names.size();
	     int index = reachable_info->node_names.index((yyvsp[-2].VAR_NAME));
	     assert(index != 0 && index <= n_nodes);
	     reachable_info->start_nodes[index] = *(yyvsp[0].RELATION);
	     delete (yyvsp[-2].VAR_NAME);
	     delete (yyvsp[0].RELATION);
	  }
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 3382 "y.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  int yytype = YYTRANSLATE (yychar);
	  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
	  YYSIZE_T yysize = yysize0;
	  YYSIZE_T yysize1;
	  int yysize_overflow = 0;
	  char *yymsg = 0;
#	  define YYERROR_VERBOSE_ARGS_MAXIMUM 5
	  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
	  int yyx;

#if 0
	  /* This is so xgettext sees the translatable formats that are
	     constructed on the fly.  */
	  YY_("syntax error, unexpected %s");
	  YY_("syntax error, unexpected %s, expecting %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
#endif
	  char *yyfmt;
	  char const *yyf;
	  static char const yyunexpected[] = "syntax error, unexpected %s";
	  static char const yyexpecting[] = ", expecting %s";
	  static char const yyor[] = " or %s";
	  char yyformat[sizeof yyunexpected
			+ sizeof yyexpecting - 1
			+ ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
			   * (sizeof yyor - 1))];
	  char const *yyprefix = yyexpecting;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 1;

	  yyarg[0] = yytname[yytype];
	  yyfmt = yystpcpy (yyformat, yyunexpected);

	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
		  {
		    yycount = 1;
		    yysize = yysize0;
		    yyformat[sizeof yyunexpected - 1] = '\0';
		    break;
		  }
		yyarg[yycount++] = yytname[yyx];
		yysize1 = yysize + yytnamerr (0, yytname[yyx]);
		yysize_overflow |= yysize1 < yysize;
		yysize = yysize1;
		yyfmt = yystpcpy (yyfmt, yyprefix);
		yyprefix = yyor;
	      }

	  yyf = YY_(yyformat);
	  yysize1 = yysize + yystrlen (yyf);
	  yysize_overflow |= yysize1 < yysize;
	  yysize = yysize1;

	  if (!yysize_overflow && yysize <= YYSTACK_ALLOC_MAXIMUM)
	    yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg)
	    {
	      /* Avoid sprintf, as that infringes on the user's name space.
		 Don't have undefined behavior even if the translation
		 produced a string with the wrong number of "%s"s.  */
	      char *yyp = yymsg;
	      int yyi = 0;
	      while ((*yyp = *yyf))
		{
		  if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		    {
		      yyp += yytnamerr (yyp, yyarg[yyi++]);
		      yyf += 2;
		    }
		  else
		    {
		      yyp++;
		      yyf++;
		    }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    {
	      yyerror (YY_("syntax error"));
	      goto yyexhaustedlab;
	    }
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (YY_("syntax error"));
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (0)
     goto yyerrorlab;

yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK;
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 1329 "../src/parser.y"


extern FILE *yyin;

#if ! defined(OMIT_GETRUSAGE)
#ifdef __sparc__
extern "C" int getrusage (int, struct rusage*);
#endif


namespace omega {

#if !defined(OMIT_GETRUSAGE)
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

struct rusage start_time;
#endif

#if defined BRAIN_DAMAGED_FREE
void free(void *p)
    {
    free((char *)p);
    }

void *realloc(void *p, size_t s)
    {
    return realloc((malloc_t) p, s);
    }
#endif

void start_clock( void )
    {
    getrusage(RUSAGE_SELF, &start_time);
    }

int clock_diff( void )
    {
    struct rusage current_time;
    getrusage(RUSAGE_SELF, &current_time);
    return (current_time.ru_utime.tv_sec -start_time.ru_utime.tv_sec)*1000000 +
           (current_time.ru_utime.tv_usec-start_time.ru_utime.tv_usec);
    }
#endif

void printUsage(FILE *outf, char **argv) {
    fprintf(outf, "usage: %s {-R} {-D[facility][level]...} infile\n  -R means skip redundant conjunct elimination\n  -D sets debugging level as follows:\n    a = all debugging flags\n    g = code generation\n    l = calculator\n    c = omega core\n    p = presburger functions\n    r = relational operators\n    t = transitive closure\nAll debugging output goes to %s\n",argv[0],DEBUG_FILE_NAME);
}

int omega_calc_debug;

} // end namespace omega

int main(int argc, char **argv){
  redundant_conj_level = 2;
  current_Declaration_Site = globalDecls = new Global_Declaration_Site();
#if YYDEBUG != 0
  yydebug  = 1;
#endif
  int i;
  char * fileName = 0;

  printf("# %s (based on %s, %s):\n",CALC_VERSION_STRING, Omega_Library_Version, Omega_Library_Date);

  calc_all_debugging_off();

#ifdef SPEED
  DebugFile = fopen("/dev/null","w");
  assert(DebugFile);
#else
  DebugFile = fopen(DEBUG_FILE_NAME, "w");
  if (!DebugFile) {
    fprintf(stderr, "Can't open debug file %s\n", DEBUG_FILE_NAME);
    DebugFile = stderr;
  }
  setbuf(DebugFile,0);
#endif

  closure_presburger_debug = 0;

  setOutputFile(DebugFile);

  // Process flags
  for(i=1; i<argc; i++) {
    if(argv[i][0] == '-') {
      int j = 1, c;
      while((c=argv[i][j++]) != 0) {
	switch(c) {
	case 'D':
	    if (! process_calc_debugging_flags(argv[i],j)) {
		printUsage(stderr,argv);
		Exit(1);
	    }
	    break;
	case 'G':
	    { 
	      fprintf(stderr,"Note: specifying number of GEQ's is no longer useful.\n");
	      while(argv[i][j] != 0) j++;
	    }
	    break;
	case 'E':
	    {
	      fprintf(stderr,"Note: specifying number of EQ's is no longer useful.\n");
	      while(argv[i][j] != 0) j++;
	    }
	    break;
	case 'R':
	    redundant_conj_level = 1;
	    break;
        // Other future options go here
	default:
	  fprintf(stderr, "\nUnknown flag -%c\n", c);
	  printUsage(stderr,argv);
	  Exit(1);
	}
      }
    } 
   else {
     // Make sure this is a file name
     if (fileName) {
	fprintf(stderr,"\nCan only handle a single input file\n");
	printUsage(stderr,argv);
	Exit(1);
	};
     fileName = argv[i];
     yyin = fopen(fileName, "r");
     if (!yyin) {
	    fprintf(stderr, "\nCan't open input file %s\n",fileName);
	    printUsage(stderr,argv);
	    Exit(1);
	    };
     }
   }


  initializeOmega();
  initializeScanBuffer();

  yyparse();

  delete globalDecls;
  foreach_map(cs,Const_String,r,Relation *,relationMap,
	      {delete r; relationMap[cs]=0;});

  return(0);
} /* end main */

namespace omega {

Relation LexForward(int n) {
  Relation r(n,n);
  F_Or *f = r.add_or();
  for (int i=1; i <= n; i++) {
	F_And *g = f->add_and();
	for(int j=1;j<i;j++) {
	   EQ_Handle e = g->add_EQ();
	   e.update_coef(r.input_var(j),-1);
	   e.update_coef(r.output_var(j),1);
	   e.finalize();
	   }
	GEQ_Handle e = g->add_GEQ();
	e.update_coef(r.input_var(i),-1);
	e.update_coef(r.output_var(i),1);
	e.update_const(-1);
	e.finalize();
	}
  r.finalize();
  return r;
  }

} // end of namespace omega


