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


extern YYSTYPE hpp_yylval;
