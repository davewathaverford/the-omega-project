typedef union {
    int iv;
    node *np;
    symtabentry *stp;
    optype op;
    } YYSTYPE;
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


extern YYSTYPE yylval;
