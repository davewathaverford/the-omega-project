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
/* Line 1447 of yacc.c.  */
#line 251 "y.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



