%{

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


%}

%token <VAR_NAME> VAR 
%token <INT_VALUE> INT
%token <STRING_VALUE> STRING
%token OPEN_BRACE CLOSE_BRACE
%token SYMBOLIC
%token OR AND NOT
%token ST APPROX
%token IS_ASSIGNED
%token FORALL EXISTS
%token DOMAIN RANGE
%token DIFFERENCE DIFFERENCE_TO_RELATION
%token GIST GIVEN HULL WITHIN MAXIMIZE MINIMIZE 
%token AFFINE_HULL VENN CONVEX_COMBINATION POSITIVE_COMBINATION CONVEX_HULL CONIC_HULL LINEAR_HULL PAIRWISE_CHECK CONVEX_CHECK
%token MAXIMIZE_RANGE MINIMIZE_RANGE
%token MAXIMIZE_DOMAIN MINIMIZE_DOMAIN
%token LEQ GEQ NEQ
%token GOES_TO
%token COMPOSE JOIN INVERSE COMPLEMENT IN CARRIED_BY TIME TIMECLOSURE
%token UNION INTERSECTION
%token VERTICAL_BAR SUCH_THAT
%token SUBSET ITERATIONS SPMD CODEGEN DECOUPLED_FARKAS FARKAS
%token TCODEGEN TRANS_IS SET_MMAP UNROLL_IS PEEL_IS
%token MAKE_UPPER_BOUND MAKE_LOWER_BOUND
%token <REL_OPERATOR> REL_OP
%token RESTRICT_DOMAIN RESTRICT_RANGE
%token SUPERSETOF SUBSETOF SAMPLE SYM_SAMPLE
%token PROJECT_AWAY_SYMBOLS PROJECT_ON_SYMBOLS REACHABLE_FROM REACHABLE_OF
%token ASSERT_UNSAT

%token PARSE_EXPRESSION PARSE_FORMULA PARSE_RELATION

%nonassoc ASSERT_UNSAT
%left UNION p1 '+' '-'
%nonassoc  SUPERSETOF SUBSETOF
%left 		p2 RESTRICT_DOMAIN RESTRICT_RANGE
%left INTERSECTION p3 '*' '@' 
%left 		p4
%left OR    	p5
%left AND   	p6 
%left COMPOSE JOIN CARRIED_BY
%right NOT APPROX DOMAIN RANGE HULL PROJECT_AWAY_SYMBOLS PROJECT_ON_SYMBOLS DIFFERENCE DIFFERENCE_TO_RELATION INVERSE COMPLEMENT FARKAS SAMPLE SYM_SAMPLE MAKE_UPPER_BOUND MAKE_LOWER_BOUND p7
%left p8
%nonassoc GIVEN
%left p9
%left '('	p10


%type <INT_VALUE> effort 
%type <EXP> exp simpleExp 
%type <EXP_LIST> expList 
%type <VAR_LIST> varList
%type <ARGUMENT_TUPLE> argumentList 
%type <ASTP> formula optionalFormula
%type <ASTCP> constraintChain
%type <TUPLE_DESCRIPTOR> tupleDeclaration
%type <DECLARATION_SITE> varDecl varDeclOptBrackets
%type <RELATION> relation builtRelation context
%type <RELATION> reachable_of
%type <REL_TUPLE_PAIR> relPairList
%type <REL_TUPLE_TRIPLE> relTripList
%type <RELATION_ARRAY_1D> reachable
%type <STM_INFO_TUPLE> statementInfoList statementInfoResult
%type <STM_INFO> statementInfo
%type <STM_INFO> reads
%type <READ>  oneread
%type <READ>  partials
%type <PREAD> partial
%type <MMAP>  partialwrites
%type <PMMAP> partialwrite

%union {
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
	};


%%


start : { 
        }
        inputSequence ;

inputSequence : inputItem
	| inputSequence { assert( current_Declaration_Site == globalDecls);}
		inputItem
	;

inputItem :
	error ';' {
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
	| SYMBOLIC globVarList ';' 
		{ flushScanBuffer();
		}
	| VAR IS_ASSIGNED relation ';' 
			{
			  flushScanBuffer();
			  $3->simplify(::min(2,redundant_conj_level),4);
			  Relation *r = relationMap((Const_String)$1);
			  if (r) delete r;
			  relationMap[(Const_String)$1] = $3; 
			  delete $1;
			}
	| relation ';' { 
			  flushScanBuffer();
			printf("\n"); 
			$1->simplify(redundant_conj_level,4);
			$1->print_with_subs(stdout); 
			printf("\n"); 
			delete $1;
			} 
	| TIME relation ';' {

#if defined(OMIT_GETRUSAGE)
	    printf("'time' requires getrusage, but the omega calclator was compiled with OMIT_GETRUSAGE set!\n");

#else

			flushScanBuffer();
			printf("\n");
			int t;
			Relation R;
			bool SKIP_FULL_CHECK = getenv("OC_TIMING_SKIP_FULL_CHECK");
			($2)->and_with_GEQ();
			start_clock();
			for (t=1;t<=100;t++) {
				R = *$2;
				R.finalize();
				}
			int copyTime = clock_diff();
			start_clock();
			for (t=1;t<=100;t++) {
				R = *$2;
				R.finalize();
				R.simplify();
				}
			int simplifyTime = clock_diff() -copyTime;
			Relation R2;
			if (!SKIP_FULL_CHECK)
			  {
			    start_clock();
			    for (t=1;t<=100;t++) {
			      R2 = *$2;
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
			delete $2;
#endif
			}
	| TIMECLOSURE relation ';' {

#if defined(OMIT_GETRUSAGE)
	    printf("'timeclosure' requires getrusage, but the omega calclator was compiled with OMIT_GETRUSAGE set!\n");
#else
			flushScanBuffer();
			printf("\n");
			int t;
			Relation R;
			($2)->and_with_GEQ();
			start_clock();
			for (t=1;t<=100;t++) {
				R = *$2;
				R.finalize();
				}
			int copyTime = clock_diff();
			start_clock();
			for (t=1;t<=100;t++) {
				R = *$2;
				R.finalize();
				R.simplify();
				};
			int simplifyTime = clock_diff() -copyTime;
			Relation Rclosed;
			start_clock();
			for (t=1;t<=100;t++) {
				Rclosed = *$2;
				Rclosed.finalize();
				Rclosed = TransitiveClosure(Rclosed, 1,Relation::Null());
				};
			int closureTime = clock_diff() - copyTime;
			Relation R2;
			start_clock();
			for (t=1;t<=100;t++) {
				R2 = *$2;
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
			delete $2;
#endif
			}


	| relation SUBSET relation ';' {
			  flushScanBuffer();
	                int c = Must_Be_Subset(*$1, *$3);
			printf("\n%s\n", c ? "True" : "False");
			delete $1;
			delete $3;
			} 
        | CODEGEN effort relPairList context';' 
                       {
			  flushScanBuffer();
			  String s = MMGenerateCode($3->mappings, $3->ispaces,*$4,$2);
			  delete $4;
			  delete $3;
			  printf("%s\n", (const char *) s); 
	               }
        | TCODEGEN effort statementInfoResult context';'
			{
			  flushScanBuffer();
			  String s = tcodegen($2, *($3), *($4));
			  delete $4;
			  delete $3;
			  printf("%s\n", (const char *) s); 
			}
/*        | TCODEGEN NOT effort statementInfoResult context';'
 *			{
 *			  flushScanBuffer();
 *			  String s = tcodegen($3, *($4), *($5), false);
 *			  delete $5;
 *			  delete $4;
 *			  printf("%s\n", (const char *) s); 
 *			}
 */
        | SPMD blockAndProcsAndEffort relTripList';'
            {
	    Tuple<Free_Var_Decl*> lowerBounds(0), upperBounds(0), my_procs(0);
            Tuple<spmd_stmt_info *> names(0);

	    flushScanBuffer();
	    int nr_statements = $3->space.size();

	    for (int i = 1; i<= $3->space[1].n_out(); i++)
		{
	        lowerBounds.append(new Free_Var_Decl("lb" + itoS(i)));
	        upperBounds.append(new Free_Var_Decl("ub" + itoS(i)));
	        my_procs.append(new Free_Var_Decl("my_proc" + itoS(i)));
		}

            for (int p = 1; p <= nr_statements; p++)
                names.append(new numbered_stmt_info(p-1, Identity($3->time[p].n_out()),
					            $3->space[p], 
					(char *)(const char *)("s"+itoS(p-1))));

	    String s = SPMD_GenerateCode("", $3->space, $3->time, $3->ispaces, 
					 names,
					 lowerBounds, upperBounds, my_procs,
                                         nr_statements);

	    delete $3;
	    printf("%s\n", (const char *) s); 
            }
        | reachable ';' 
	{ 	flushScanBuffer();
		Dynamic_Array1<Relation> &final = *$1;
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
		delete $1;
		delete reachable_info;
	}
	;

relTripList: relTripList ',' relation ':' relation ':' relation
                {
                $1->space.append(*$3);
                $1->time.append(*$5);
                $1->ispaces.append(*$7);
                delete $3;
                delete $5;
                delete $7;
                $$ = $1;
                }
        | relation ':' relation ':' relation
                {
                RelTupleTriple *rtt = new RelTupleTriple;
                rtt->space.append(*$1);
                rtt->time.append(*$3);
                rtt->ispaces.append(*$5);
                delete $1;
                delete $3;
                delete $5;
                $$ = rtt;
                }
		;

blockAndProcsAndEffort  : { Block_Size = 0; Num_Procs = 0; overheadEffort=0; }
	       | INT { Block_Size = $1; Num_Procs = 0; overheadEffort=0;}
	       | INT INT { Block_Size = $1; Num_Procs = $2; overheadEffort=0;}
	       | INT INT INT { Block_Size = $1; Num_Procs = $2; overheadEffort=$3;}
	       ;

effort : { $$ = 0; }
	| INT { $$ = $1; }
	| '-' INT { $$ = -$2; }
	;

context : { $$ = new Relation();
		*$$ = Relation::Null(); }
	| GIVEN relation {$$ = $2; }
	;

relPairList: relPairList ',' relation ':' relation 
                {
	        $1->mappings.append(*$3);
		$1->mappings[$1->mappings.size()].compress();
		$1->ispaces.append(*$5);
		$1->ispaces[$1->ispaces.size()].compress();
		delete $3;
		delete $5;
	        $$ = $1;
                }
	| relPairList ',' relation 
                {
	        $1->mappings.append(Identity($3->n_set()));
		$1->mappings[$1->mappings.size()].compress();
		$1->ispaces.append(*$3);
		$1->ispaces[$1->ispaces.size()].compress();
		delete $3;
	        $$ = $1;
                }
        | relation ':' relation
        	{
                RelTuplePair *rtp = new RelTuplePair;
	        rtp->mappings.append(*$1);
		rtp->mappings[rtp->mappings.size()].compress();
	        rtp->ispaces.append(*$3);
		rtp->ispaces[rtp->ispaces.size()].compress();
		delete $1;
		delete $3;
	        $$ = rtp;
		}
        | relation
        	{
                RelTuplePair *rtp = new RelTuplePair;
	        rtp->mappings.append(Identity($1->n_set()));
		rtp->mappings[rtp->mappings.size()].compress();
	        rtp->ispaces.append(*$1);
		rtp->ispaces[rtp->ispaces.size()].compress();
		delete $1;
	        $$ = rtp;
                }
        ;

statementInfoResult : statementInfoList
		{ $$ = $1; }
/*	| ASSERT_UNSAT statementInfoResult
 *		{ $$ = ($2);
 *		  DoDebug2("Debug info requested in input", *($2));
 *		}
 */
	| TRANS_IS relation statementInfoResult
		{ $$ = &Trans_IS(*($3), *($2));
		  delete $2;
		}
	| SET_MMAP INT partialwrites statementInfoResult
		{ $$ = &Set_MMap(*($4), $2, *($3));
		  delete $3;
		}
	| UNROLL_IS INT INT INT statementInfoResult
		{ $$ = &Unroll_One_IS(*($5), $2, $3, $4);}
	| PEEL_IS INT INT relation statementInfoResult
		{ $$ = &Peel_One_IS(*($5), $2, $3, *($4));
		  delete $4;
		}
	| PEEL_IS INT INT relation ',' relation statementInfoResult
		{ $$ = &Peel_One_IS(*($7), $2, $3, *($4), *($6));
		  delete $4;
		  delete $6;
		}
	;

statementInfoList : statementInfo		{ $$ = new Tuple<stm_info>;
						  $$->append(*($1));
						  delete $1; }
	| statementInfoList ',' statementInfo	{ $$ = $1;
						  $$->append(*($3));
						  delete $3; }
	;

statementInfo : '[' STRING ',' relation ',' partialwrites ',' reads ']'
		{ $$ = $8;
		  $$->stm = *($2); delete $2;
		  $$->IS  = *($4); delete $4;
		  $$->map = *($6); delete $6;
		  }
	     |  '[' STRING ',' relation ',' partialwrites ']'
		{ $$ = new stm_info;
		  $$->stm = *($2); delete $2;
		  $$->IS  = *($4); delete $4;
		  $$->map = *($6); delete $6;
		}
	;

partialwrites	: partialwrites ',' partialwrite
				{ $$ = $1;
				  $$->partials.append(*($3)); delete $3;
				}
		| partialwrite	{ $$ = new MMap;
				  $$->partials.append(*($1)); delete $1;
				}
	;

partialwrite : STRING '[' relation ']' ',' relation
                                	{ $$ = new PartialMMap;
					  $$->mapping = *($6); delete $6;
					  $$->bounds  = *($3); delete $3;
					  $$->var     = *($1); delete $1;
					}
              | STRING ',' relation	{ $$ = new PartialMMap;
					  $$->mapping = *($3); delete $3;
					  $$->bounds  = Relation::True(0);
					  $$->var     = *($1); delete $1;
					}
	;

reads :	reads ',' oneread	{ $$ = $1;
				  $$->read.append(*($3)); delete $3;
				}
	| oneread		{ $$ = new stm_info;
				  $$->read.append(*($1)); delete $1;
				}
	;

oneread :  '[' partials ']' { $$ = $2; }
	;

partials : partials ',' partial { $$ = $1;
				  $$->partials.append(*($3)); delete $3;
				}
	| partial		{ $$ = new Read;
				  $$->partials.append(*($1)); delete $1;
				}
	;

partial : INT ',' relation	{ $$ = new PartialRead;
				  $$->from = $1;
				  $$->dataFlow = *($3); delete $3;
				}
	;

globVarList: globVarList ',' globVar
	| globVar
	;

globVar:  VAR '(' INT ')'
		{ globalDecls->extend_both_tuples($1, $3); free($1); }
	| VAR
		{ globalDecls->extend($1); free($1); }
	;

relation : OPEN_BRACE 
		{ relationDecl = new Declaration_Site(); }
		  builtRelation 
		CLOSE_BRACE
		{ $$ = $3; 
		  if (omega_calc_debug) {
			fprintf(DebugFile,"Built relation:\n");
			$$->prefix_print(DebugFile);
			};
		  current_Declaration_Site = globalDecls;
		  delete relationDecl;
		}
	 | VAR {
		Const_String s = $1;
		free($1);
		if (relationMap(s) == 0) {
			fprintf(stderr,"Variable %s not declared\n",$1);
			YYERROR;
			assert(0);
			};
		$$ = new Relation(*relationMap(s));
		}
	 | '(' relation ')'	{$$ = $2;}
	 | relation '+' 	%prec p9
		{ $$ = new Relation();
		  *$$ = TransitiveClosure(*$1, 1,Relation::Null());
		  delete $1;
		}
	 | relation '*' 	%prec p9
		{ $$ = new Relation();
		  int vars = $1->n_inp();
		  *$$ = Union(Identity(vars),
			      TransitiveClosure(*$1, 1,Relation::Null()));
		  delete $1;
		}
         | relation '+' WITHIN relation %prec p9
                {$$ = new Relation();
                 *$$= TransitiveClosure(*$1, 1,*$4);
                 delete $1;
                 delete $4;
	       }
	 | MINIMIZE_RANGE relation %prec p8
		{
		Relation o(*$2);
		Relation r(*$2);
		r = Join(r,LexForward($2->n_out()));
		$$ = new Relation();
		*$$ = Difference(o,r);
		delete $2;
		}
	 | MAXIMIZE_RANGE relation %prec p8
		{
		Relation o(*$2);
		Relation r(*$2);
		r = Join(r,Inverse(LexForward($2->n_out())));
		$$ = new Relation();
		*$$ = Difference(o,r);
		delete $2;
		}
	 | MINIMIZE_DOMAIN relation %prec p8
		{
		Relation o(*$2);
		Relation r(*$2);
		r = Join(LexForward($2->n_inp()),r);
		$$ = new Relation();
		*$$ = Difference(o,r);
		delete $2;
		}
	 | MAXIMIZE_DOMAIN relation %prec p8
		{
		Relation o(*$2);
		Relation r(*$2);
		r = Join(Inverse(LexForward($2->n_inp())),r);
		$$ = new Relation();
		*$$ = Difference(o,r);
		delete $2;
		}
	 | MAXIMIZE relation %prec p8
		{
		Relation c(*$2);
		Relation r(*$2);
		$$ = new Relation(); 
		*$$ = Cross_Product(Relation(*$2),c);
		delete $2;
		assert($$->n_inp() ==$$->n_out());
		*$$ = Difference(r,Domain(Intersection(*$$,LexForward($$->n_inp()))));
		}
	 | MINIMIZE relation %prec p8
		{
		Relation c(*$2);
		Relation r(*$2);
		$$ = new Relation(); 
		*$$ = Cross_Product(Relation(*$2),c);
		delete $2;
		assert($$->n_inp() ==$$->n_out());
		*$$ = Difference(r,Range(Intersection(*$$,LexForward($$->n_inp()))));
		}
         | FARKAS relation   %prec p8
		{
		$$ = new Relation();
		*$$ = Farkas(*$2, Basic_Farkas);
		delete $2;
		}
         | DECOUPLED_FARKAS relation   %prec p8
		{
		$$ = new Relation();
		*$$ = Farkas(*$2, Decoupled_Farkas);
		delete $2;
		}
	 | relation '@' 	%prec p9
		{ $$ = new Relation();
		  *$$ = ConicClosure(*$1);
		  delete $1;
		}
	 | PROJECT_AWAY_SYMBOLS relation   %prec p8
		{ $$ = new Relation();
		  *$$ = Project_Sym(*$2);
		  delete $2;
		}
	 | PROJECT_ON_SYMBOLS relation   %prec p8
		{ $$ = new Relation();
		  *$$ = Project_On_Sym(*$2);
		  delete $2;
		}
	 | DIFFERENCE relation   %prec p8
		{ $$ = new Relation();
		  *$$ = Deltas(*$2);
		  delete $2;
		}
         | DIFFERENCE_TO_RELATION relation   %prec p8
                { $$ = new Relation();
                  *$$ = DeltasToRelation(*$2,$2->n_set(),$2->n_set());
                  delete $2;
                }
	 | DOMAIN relation   %prec p8
		{ $$ = new Relation();
		  *$$ = Domain(*$2);
		  delete $2;
		}
	 | VENN relation   %prec p8
		{ $$ = new Relation();
		  *$$ = VennDiagramForm(*$2,Relation::True(*$2));
		  delete $2;
		}
	 | VENN relation GIVEN  relation  %prec p8
		{ $$ = new Relation();
		  *$$ = VennDiagramForm(*$2,*$4);
		  delete $2;
		  delete $4;
		}
	 | CONVEX_HULL relation   %prec p8
		{ $$ = new Relation();
		  *$$ = ConvexHull(*$2);
		  delete $2;
		}
	 | POSITIVE_COMBINATION relation   %prec p8
		{ $$ = new Relation();
		  *$$ = Farkas(*$2,Positive_Combination_Farkas);
		  delete $2;
		}
	 | CONVEX_COMBINATION relation   %prec p8
		{ $$ = new Relation();
		  *$$ = Farkas(*$2,Convex_Combination_Farkas);
		  delete $2;
		}
	 | PAIRWISE_CHECK relation   %prec p8
		{ $$ = new Relation();
		  *$$ = CheckForConvexRepresentation(CheckForConvexPairs(*$2));
		  delete $2;
		}
	 | CONVEX_CHECK relation   %prec p8
		{ $$ = new Relation();
		  *$$ = CheckForConvexRepresentation(*$2);
		  delete $2;
		}
	 | AFFINE_HULL relation   %prec p8
		{ $$ = new Relation();
		  *$$ = AffineHull(*$2);
		  delete $2;
		}
	 | CONIC_HULL relation   %prec p8
		{ $$ = new Relation();
		  *$$ = ConicHull(*$2);
		  delete $2;
		}
	 | LINEAR_HULL relation   %prec p8
		{ $$ = new Relation();
		  *$$ = LinearHull(*$2);
		  delete $2;
		}
	 | HULL relation   %prec p8
		{ $$ = new Relation();
		  *$$ = Hull(*$2,false,1,Null_Relation());
		  delete $2;
		}
	 | HULL relation GIVEN relation  %prec p8
		{ $$ = new Relation();
		  *$$ = Hull(*$2,false,1,*$4);
		  delete $2;
		  delete $4;
		}
	 | APPROX relation   %prec p8
		{ $$ = new Relation();
		  *$$ = Approximate(*$2);
		  delete $2;
		}
	 | RANGE relation   %prec p8
		{ $$ = new Relation();
		  *$$ = Range(*$2);
		  delete $2;
		}
	 | INVERSE relation   %prec p8
		{ $$ = new Relation();
		  *$$ = Inverse(*$2);
		  delete $2;
		}
	 | COMPLEMENT relation   %prec p8
		{ $$ = new Relation();
		  *$$ = Complement(*$2);
		  delete $2;
		}
	 | GIST relation GIVEN relation %prec p8
		{ $$ = new Relation();
		  *$$ = Gist(*$2,*$4,1);
		  delete $2;
		  delete $4;
		}
	 | relation '(' relation ')'
		{ $$ = new Relation();
		  *$$ = Composition(*$1,*$3);
		  delete $1;
		  delete $3;
		}
	 | relation COMPOSE relation 
		{ $$ = new Relation();
		  *$$ = Composition(*$1,*$3);
		  delete $1;
		  delete $3;
		}
	 | relation CARRIED_BY INT 
		{ $$ = new Relation();
		  *$$ = After(*$1,$3,$3);
		  delete $1;
		  (*$$).prefix_print(stdout);
		}
	 | relation JOIN relation 
		{ $$ = new Relation();
		  *$$ = Composition(*$3,*$1);
		  delete $1;
		  delete $3;
		}
	 | relation RESTRICT_RANGE relation 
		{ $$ = new Relation();
		  *$$ = Restrict_Range(*$1,*$3);
		  delete $1;
		  delete $3;
		}
	 | relation RESTRICT_DOMAIN relation 
		{ $$ = new Relation();
		  *$$ = Restrict_Domain(*$1,*$3);
		  delete $1;
		  delete $3;
		}
	 | relation INTERSECTION relation 
		{ $$ = new Relation();
		  *$$ = Intersection(*$1,*$3);
		  delete $1;
		  delete $3;
		}
         | relation '-' relation %prec INTERSECTION
                { $$ = new Relation();
                  *$$ = Difference(*$1,*$3);
                  delete $1;
                  delete $3;
                }
	 | relation UNION relation 
		{ $$ = new Relation();
		  *$$ = Union(*$1,*$3);
		  delete $1;
		  delete $3;
		}
	 | relation '*' relation 
		{ $$ = new Relation();
		  *$$ = Cross_Product(*$1,*$3);
		  delete $1;
		  delete $3;
		}
         | SUPERSETOF relation
                { $$ = new Relation();
                  *$$ = Union(*$2, Relation::Unknown(*$2));
                  delete $2;
                } 
         | SUBSETOF relation
                { $$ = new Relation();
                  *$$ = Intersection(*$2, Relation::Unknown(*$2));
                  delete $2;
                }
         | MAKE_UPPER_BOUND relation %prec p8
                { $$ = new Relation();
                  *$$ = Upper_Bound(*$2);
                  delete $2;
                } 
         | MAKE_LOWER_BOUND relation %prec p8
                { $$ = new Relation();
                  *$$ = Lower_Bound(*$2);
                  delete $2;
                }
         | SAMPLE relation
                { $$ = new Relation();
                  *$$ = Sample_Solution(*$2);
                  delete $2;
                }
         | SYM_SAMPLE relation
                { $$ = new Relation();
                  *$$ = Symbolic_Solution(*$2);
                  delete $2;
                }
	 | reachable_of { $$ = $1; }
	 | ASSERT_UNSAT relation
		{
		  if (($2)->is_satisfiable())
			  {
			    fprintf(stderr,"assert_unsatisfiable failed on ");
			    ($2)->print_with_subs(stderr);
			    Exit(1);
			  }
		  $$=$2;
		}

	;

builtRelation :
	tupleDeclaration GOES_TO {currentTuple = Output_Tuple;} 
			tupleDeclaration {currentTuple = Input_Tuple;} optionalFormula {
		Relation * r = new Relation($1->size,$4->size);
		resetGlobals();
		F_And *f = r->add_and();
		int i;
		for(i=1;i<=$1->size;i++) {	
			$1->vars[i]->vid = r->input_var(i);
			if (!$1->vars[i]->anonymous) 
				r->name_input_var(i,$1->vars[i]->stripped_name);
			};
		for(i=1;i<=$4->size;i++) {
			$4->vars[i]->vid = r->output_var(i);
			if (!$4->vars[i]->anonymous) 
				r->name_output_var(i,$4->vars[i]->stripped_name);
			};
		foreach(e,Exp*,$1->eq_constraints, install_eq(f,e,0));
                foreach(e,Exp*,$1->geq_constraints, install_geq(f,e,0)); 
		foreach(c,strideConstraint*,$1->stride_constraints, install_stride(f,c));
		foreach(e,Exp*,$4->eq_constraints, install_eq(f,e,0));
		foreach(e,Exp*,$4->geq_constraints, install_geq(f,e,0));
		foreach(c,strideConstraint*,$4->stride_constraints, install_stride(f,c));
		if ($6) $6->install(f);
		delete $1;
		delete $4;
		delete $6;
		$$ = r; }
	| tupleDeclaration optionalFormula {
	        Relation * r = new Relation($1->size);
		resetGlobals();
		F_And *f = r->add_and();
		int i;
		for(i=1;i<=$1->size;i++) {	
			$1->vars[i]->vid = r->set_var(i);
			if (!$1->vars[i]->anonymous) 
				r->name_set_var(i,$1->vars[i]->stripped_name);
			};
                foreach(e,Exp*,$1->eq_constraints, install_eq(f,e,0)); 
		foreach(e,Exp*,$1->geq_constraints, install_geq(f,e,0));
		foreach(c,strideConstraint*,$1->stride_constraints, install_stride(f,c));
		if ($2) $2->install(f);
		delete $1;
		delete $2;
		$$ = r; }
	| formula {
		Relation * r = new Relation(0,0);
		F_And *f = r->add_and();
		$1->install(f);
		delete $1;
		$$ = r;
		}
	;

optionalFormula : formula_sep formula { $$ = $2; }
	| {$$ = 0;}  
	;
		  
formula_sep : ':'
	| VERTICAL_BAR
	| SUCH_THAT
	;

tupleDeclaration :
	{ currentTupleDescriptor = new tupleDescriptor; tuplePos = 1; }
	'[' optionalTupleVarList ']' 
	{$$ = currentTupleDescriptor; }
	;

optionalTupleVarList : 
          tupleVar 
	| optionalTupleVarList ',' tupleVar 
	|
	;

tupleVar : VAR %prec p10
	{ Declaration_Site *ds = defined($1);
	  if (!ds) currentTupleDescriptor->extend($1,currentTuple,tuplePos);
	  else {
	      Variable_Ref * v = lookupScalar($1);
	      assert(v);
	      if (ds != globalDecls) 
		currentTupleDescriptor->extend($1, new Exp(v));
	      else 
		currentTupleDescriptor->extend(new Exp(v));
	      }
	  free($1);
	  tuplePos++;
	}
	| '*'
	{currentTupleDescriptor->extend(); tuplePos++; }
	| exp %prec p1
	{currentTupleDescriptor->extend($1); tuplePos++; }
	| exp ':' exp %prec p1
	{currentTupleDescriptor->extend($1,$3); tuplePos++; }
	| exp ':' exp ':' INT %prec p1
	{currentTupleDescriptor->extend($1,$3,$5); tuplePos++; }
	;


varList: varList ',' VAR {$$ = $1; $$->insert($3); }
	| VAR { $$ = new VarList;
		$$->insert($1); }
	;

varDecl : varList
		{
		$$ = current_Declaration_Site = new Declaration_Site($1);
		foreach(s,char *, *$1, delete s);
		delete $1;
		}
	;

/* variable declaration with optional brackets */

varDeclOptBrackets : varDecl	{ $$ = $1; }
	| '[' varDecl ']'	{ $$ = $2; }
	;

formula : formula AND formula	{ $$ = new AST_And($1,$3); }
	| formula OR formula	{ $$ = new AST_Or($1,$3); }
	| constraintChain 	{ $$ = $1; }
	| '(' formula ')'  	{ $$ = $2; }
	| NOT formula 		{ $$ = new AST_Not($2); }
	| start_exists varDeclOptBrackets exists_sep formula end_quant
				{ $$ = new AST_exists($2,$4); }
	| start_forall varDeclOptBrackets forall_sep formula end_quant
				{ $$ = new AST_forall($2,$4); }
	;

start_exists : '(' EXISTS
	| EXISTS '('
	;

exists_sep : ':'
	| VERTICAL_BAR
	| SUCH_THAT
	;

start_forall : '(' FORALL
	| FORALL '('
	;

forall_sep : ':'
	;

end_quant : ')'
	{ popScope(); }
	;

expList : exp ',' expList 
		{
		$$ = $3; 
		$$->insert($1);
		}
	| exp {
		$$ = new ExpList;
		$$->insert($1);
		}
	;

constraintChain : expList REL_OP expList 
			{ $$ = new AST_constraints($1,$2,$3); }
		| expList REL_OP constraintChain 
			{ $$ = new AST_constraints($1,$2,$3); }
		;

simpleExp : 
	VAR	%prec p9 	
		{ Variable_Ref * v = lookupScalar($1);
		  if (!v) YYERROR;
		  $$ = new Exp(v); 
		  free($1); 
		  }
	| VAR '(' {argCount = 1;}  argumentList ')' %prec p9 	
		{Variable_Ref *v;
		 if ($4 == Input_Tuple) v = functionOfInput[$1];
		 else v = functionOfOutput[$1];
		 free($1);
		 if (v == 0) {
			fprintf(stderr,"Function %s(...) not declared\n",$1);
			YYERROR;
			}
		 $$ = new Exp(v);
		}
	| '(' exp ')'  { $$ = $2;}
	;



argumentList :
	argumentList ',' VAR {
		Variable_Ref * v = lookupScalar($3);
		if (!v) YYERROR;
		 free($3);
		 if (v->pos != argCount || v->of != $1 || v->of != Input_Tuple && v->of != Output_Tuple) {
			fprintf(stderr,"arguments to function must be prefix of input or output tuple\n");
			YYERROR;
			}
		 $$ = v->of;
		 argCount++;
		}
	| VAR { Variable_Ref * v = lookupScalar($1);
		if (!v) YYERROR;
		 free($1);
		 if (v->pos != argCount || v->of != Input_Tuple && v->of != Output_Tuple) {
			fprintf(stderr,"arguments to function must be prefix of input or output tuple\n");
			YYERROR;
			}
		 $$ = v->of;
		 argCount++;
		}
	;
	
exp : INT 		{$$ = new Exp($1);}
	| INT simpleExp  %prec '*' {$$ = multiply($1,$2);}
	| simpleExp	{ $$ = $1; }
	| '-' exp %prec '*'   { $$ = ::negate($2);}
	| exp '+' exp  { $$ = add($1,$3);}
	| exp '-' exp  { $$ = subtract($1,$3);}
	| exp '*' exp  { $$ = multiply($1,$3);}
	;


reachable : 
	REACHABLE_FROM nodeNameList nodeSpecificationList
		{
		  Dynamic_Array1<Relation> *final =
		    Reachable_Nodes(reachable_info);
		  $$ = final;
		}
	;

reachable_of : 
	REACHABLE_OF VAR IN nodeNameList nodeSpecificationList 
		{
		  Dynamic_Array1<Relation> *final =
		    Reachable_Nodes(reachable_info);
		  int index = reachable_info->node_names.index(String($2));
		  assert(index != 0 && "No such node");
		  $$ = new Relation; 
		  *$$ = (*final)[index];
		  delete final;
		  delete reachable_info;
		  delete $2;
		}
	;


nodeNameList: '(' realNodeNameList ')'
              { int sz = reachable_info->node_names.size();
		reachable_info->node_arity.reallocate(sz);
	  	reachable_info->transitions.resize(sz+1,sz+1);
	  	reachable_info->start_nodes.resize(sz+1);
	      }
	;

realNodeNameList: realNodeNameList ',' VAR 
              { reachable_info->node_names.append(String($3));
		delete $3; }
	| VAR { reachable_info = new reachable_information;
		reachable_info->node_names.append(String($1));
		delete $1; }
	;


nodeSpecificationList: OPEN_BRACE realNodeSpecificationList CLOSE_BRACE
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
	;

realNodeSpecificationList: 
	realNodeSpecificationList ',' VAR ':' relation
	  {  int n_nodes = reachable_info->node_names.size();
	     int index = reachable_info->node_names.index($3);
	     assert(index != 0 && index <= n_nodes);
	     reachable_info->start_nodes[index] = *$5;
	     delete $3;
	     delete $5;
	  }
	| realNodeSpecificationList ',' VAR GOES_TO VAR ':' relation
	  {  int n_nodes = reachable_info->node_names.size();
	     int from_index = reachable_info->node_names.index($3);
	     int   to_index = reachable_info->node_names.index($5);
	     assert(from_index != 0 && to_index != 0);
	     assert(from_index <= n_nodes && to_index <= n_nodes);
	     reachable_info->transitions[from_index][to_index] = *$7;
	     delete $3;
	     delete $5;
	     delete $7;
	  }
        | VAR GOES_TO VAR ':' relation
          {  int n_nodes = reachable_info->node_names.size();
	     int from_index = reachable_info->node_names.index($1);
	     int   to_index = reachable_info->node_names.index($3);
	     assert(from_index != 0 && to_index != 0);
	     assert(from_index <= n_nodes && to_index <= n_nodes);
	     reachable_info->transitions[from_index][to_index] = *$5;
	     delete $1;
	     delete $3;
	     delete $5;
	  }
        | VAR ':' relation
          {  int n_nodes = reachable_info->node_names.size();
	     int index = reachable_info->node_names.index($1);
	     assert(index != 0 && index <= n_nodes);
	     reachable_info->start_nodes[index] = *$3;
	     delete $1;
	     delete $3;
	  }
	;

%%

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

