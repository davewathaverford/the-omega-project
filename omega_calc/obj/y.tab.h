typedef union {
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
#define	VAR	258
#define	INT	259
#define	STRING	260
#define	OPEN_BRACE	261
#define	CLOSE_BRACE	262
#define	SYMBOLIC	263
#define	OR	264
#define	AND	265
#define	NOT	266
#define	ST	267
#define	APPROX	268
#define	IS_ASSIGNED	269
#define	FORALL	270
#define	EXISTS	271
#define	DOMAIN	272
#define	RANGE	273
#define	DIFFERENCE	274
#define	DIFFERENCE_TO_RELATION	275
#define	GIST	276
#define	GIVEN	277
#define	HULL	278
#define	WITHIN	279
#define	MAXIMIZE	280
#define	MINIMIZE	281
#define	AFFINE_HULL	282
#define	VENN	283
#define	CONVEX_COMBINATION	284
#define	POSITIVE_COMBINATION	285
#define	CONVEX_HULL	286
#define	CONIC_HULL	287
#define	LINEAR_HULL	288
#define	PAIRWISE_CHECK	289
#define	CONVEX_CHECK	290
#define	MAXIMIZE_RANGE	291
#define	MINIMIZE_RANGE	292
#define	MAXIMIZE_DOMAIN	293
#define	MINIMIZE_DOMAIN	294
#define	LEQ	295
#define	GEQ	296
#define	NEQ	297
#define	GOES_TO	298
#define	COMPOSE	299
#define	JOIN	300
#define	INVERSE	301
#define	COMPLEMENT	302
#define	IN	303
#define	CARRIED_BY	304
#define	TIME	305
#define	TIMECLOSURE	306
#define	UNION	307
#define	INTERSECTION	308
#define	VERTICAL_BAR	309
#define	SUCH_THAT	310
#define	SUBSET	311
#define	ITERATIONS	312
#define	SPMD	313
#define	CODEGEN	314
#define	DECOUPLED_FARKAS	315
#define	FARKAS	316
#define	TCODEGEN	317
#define	TRANS_IS	318
#define	SET_MMAP	319
#define	UNROLL_IS	320
#define	PEEL_IS	321
#define	MAKE_UPPER_BOUND	322
#define	MAKE_LOWER_BOUND	323
#define	REL_OP	324
#define	RESTRICT_DOMAIN	325
#define	RESTRICT_RANGE	326
#define	SUPERSETOF	327
#define	SUBSETOF	328
#define	SAMPLE	329
#define	SYM_SAMPLE	330
#define	PROJECT_AWAY_SYMBOLS	331
#define	PROJECT_ON_SYMBOLS	332
#define	REACHABLE_FROM	333
#define	REACHABLE_OF	334
#define	ASSERT_UNSAT	335
#define	PARSE_EXPRESSION	336
#define	PARSE_FORMULA	337
#define	PARSE_RELATION	338
#define	p1	339
#define	p2	340
#define	p3	341
#define	p4	342
#define	p5	343
#define	p6	344
#define	p7	345
#define	p8	346
#define	p9	347
#define	p10	348


extern YYSTYPE yylval;
