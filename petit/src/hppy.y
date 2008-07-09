%{

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

using omega::min;
using omega::negate;

extern int hpp_yylex(void);
extern int hpp_yyerror(char * );


namespace omega {

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
extern FILE *DebugFile;
}


int pickles=0;

void hpp_error_callback(const String &s) {
    fprintf(stderr, "%s\n", (const char *) s);
    exit(1);
}

using namespace omega;

%}

%left p1 '+' '-'
%left 		p2 
%left INTERSECTION p3 '*' '@' 
%left 		p4
%left     	p5
%left    	p6 
%nonassoc NOT  	p7
%left p8
%left p9
%left '('	p10
%left 	p11

%token <VAR_NAME> VAR 
%token <INT_VALUE> INT
%token ALIGN DISTRIBUTE PROCESSORS TEMPLATE WITH ONTO CYCLIC BLOCK STAR
%token LPAREN RPAREN COMMA

%type <INTEGER_LIST> intlist
%type <TUPLE_DESCRIPTOR> tupleDeclaration
%type <EXP> exp simpleExp
%type <DISTLIST> distlist
%type <DISTINFO> distinfo

%union {
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
};

%%

start: { current_Declaration_Site = 0;
         hpp_debug_file = omega::DebugFile;
         current_Declaration_Site = globalDecls = 
	     new Global_Declaration_Site();
         petit_args.hpp_mode = true;
       }
        stmt_list
        { delete globalDecls; }
        ;

stmt_list:

   stmt | stmt stmt_list;

stmt: template_decl | proc_decl | alignment | distribution;

template_decl: TEMPLATE VAR LPAREN intlist RPAREN
   {  
       List<int> *l = $4;
       add_template($2, l);
       fprintf(hpp_debug_file,"Template %s is: ",$2);
       make_bounds_relation(l).print_with_subs(hpp_debug_file,true);
   }

    
proc_decl: PROCESSORS VAR LPAREN intlist RPAREN
   {  
       add_proc($2,$4);
       assert($4->length() == 1 && "Only handle 1-dim processors");
#if 0
       fprintf(hpp_debug_file,"Processor declaration %s is: ",$2);
       make_bounds_relation(lookup_proc($2)).print_with_subs(hpp_debug_file,true);
#endif
   }


alignment: 
{ relationDecl = new Declaration_Site(); }
ALIGN VAR tupleDeclaration WITH VAR tupleDeclaration
{

    symtabentry *s = lookup_symbol($3);
    s->template_name = $6;

/* This chunk of code from parser.y constructs a relation from the 
   expression lists in the alignment statement */

    Relation * r = new Relation($4->size,$7->size);
    resetGlobals();
    F_And *f = r->add_and();
    int i;
    for(i=1;i<=$4->size;i++) {	
	$4->vars[i]->vid = r->input_var(i);
	if (!$4->vars[i]->anonymous) 
	    r->name_input_var(i,$4->vars[i]->stripped_name);
    };
    for(i=1;i<=$7->size;i++) {
	$7->vars[i]->vid = r->output_var(i);
	if (!$7->vars[i]->anonymous) 
	    r->name_output_var(i,$7->vars[i]->stripped_name);
    };
    foreach(e,Exp*,$4->eq_constraints, install_eq(f,e,0));
    
    /*   foreach(e,Exp*,$4->geq_constraints, install_geq(f,e,0)); */
    do 
	{
	    for (Any_Iterator<Exp*> __P_x = ($4->geq_constraints).any_iterator();
		 __P_x;
		 __P_x++) 
		{
		    Exp* & e = *__P_x; 
		    install_geq(f,e,0);
		}
	} while (0);
    
    foreach(c,strideConstraint*,$4->stride_constraints, install_stride(f,c));
    foreach(e,Exp*,$7->eq_constraints, install_eq(f,e,0));
    foreach(e,Exp*,$7->geq_constraints, install_geq(f,e,0));
    foreach(c,strideConstraint*,$7->stride_constraints, install_stride(f,c));
    delete $4;
    delete $7;
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
		free($3);
		free($6);
}

distribution: DISTRIBUTE  VAR LPAREN distlist RPAREN ONTO VAR {

    template_info *ti = lookup_template($2);
    ti->proc_name = $7;

// create a relation that maps template locations to processor locations;
    List<dist_info> *dl = $4;
    String template_name($2),proc_name($7);

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
}
;

// Have to use pointers to dist_info; classes w/constructors can't go in unions
distlist: distlist COMMA distinfo { $1->append(*$3); delete $3; $$ = $1; }
        | distinfo { distribution_list *dl = new distribution_list;
	             dl->append(*$1); delete $1; $$ = dl; }
        ;

distinfo: BLOCK { $$ = new dist_info(block); }
        | CYCLIC { $$ =  new dist_info(cyclic,1); }
        | CYCLIC LPAREN INT RPAREN { $$ =  new dist_info(cyclic, (int)$3); }
        | STAR { $$ =  new dist_info(star); }
        ;


tupleDeclaration:
{ currentTupleDescriptor = new tupleDescriptor; }
	LPAREN tupleVarList RPAREN
{$$ = currentTupleDescriptor; }
	;

tupleVarList : 
          tupleVar
	| tupleVarList COMMA tupleVar 
	;

tupleVar :
             VAR %prec p10
{         Declaration_Site *ds = defined($1);
	  if (!ds) 
            currentTupleDescriptor->extend($1);
	  else 
            currentTupleDescriptor->extend($1, new Exp(lookupScalar($1)));

	  free($1);

/*currentTupleDescriptor->extend($1);*/

 }
|	  '*' %prec p10
	{currentTupleDescriptor->extend(); }
	| exp %prec p11
	{
//	    if (!pickles)
		currentTupleDescriptor->extend($1); 
	    pickles=0;
}

	| exp ':' exp %prec p11
	{currentTupleDescriptor->extend($1,$3); }
	| exp ':' exp ':' INT %prec p11
	{currentTupleDescriptor->extend($1,$3,$5); }
	;


intlist: INT 
    { 
	List<int> *l = new List<int>;
	l->append($1);
	$$ = l;
    }
         | intlist COMMA INT
             {
		 $1->append($3);
		 $$ = $1;
	     }


simpleExp : 
	VAR
	{ 
//fprintf(hpp_debug_file,"variable %s seen",$1);

//	    Declaration_Site *ds = defined($1);
//            if (!ds) {
//fprintf(hpp_debug_file,"...not declared, declaring",$1);

//	      currentTupleDescriptor->extend($1);
//	  }
//fprintf(hpp_debug_file,"...variable ref = %x\n",lookupScalar($1));
	  $$ = new Exp(lookupScalar($1));
	  free($1);
	    pickles=1;

	}
	| '(' exp ')'  { $$ = $2;}
	;
	
exp : INT 		{$$ = new Exp($1);}
	| INT simpleExp  %prec '*' {$$ = multiply($1,$2);}
	| simpleExp	{ $$ = $1; }
	| '-' exp %prec '*'   { $$ = ::negate($2);}
	| exp '+' exp  { $$ = add($1,$3);}
	| exp '-' exp  { $$ = subtract($1,$3);}
	| exp '*' exp  { $$ = multiply($1,$3);}
	;



%%


#include <hppl.c>

namespace omega {

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

} // end of namespace omega
