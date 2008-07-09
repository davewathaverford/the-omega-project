#include <omega/AST.h>
#include <string.h>

namespace omega {

Global_Declaration_Site *globalDecls;	
Declaration_Site *relationDecl;	
tupleDescriptor *currentTupleDescriptor;
functionSymbolList functionOfInput((Variable_Ref *) 0);
functionSymbolList functionOfOutput((Variable_Ref *) 0);

void printExp(Exp *x) {
	printf("%d",x->constantTerm);
	foreach_map(r,Variable_Ref*,c,int,x->coefs,
		printf("+%d%s",c,(const char *)r->name));
	}

Exp::Exp(int c) : coefs(0) {
	constantTerm = c;
	}

AST_constraints::
        AST_constraints(ExpList * f, Rel_Op r, AST_constraints* o)
                { others = o; rel_op = r; first = f; }
AST_constraints::
        AST_constraints(ExpList* f, Rel_Op r, ExpList * s)
                { others = new AST_constraints(s); rel_op = r; first = f; }
AST_constraints::
        AST_constraints(ExpList *f)
                { others = 0; first = f; }
AST_constraints::~AST_constraints()
    {
    foreach(e,Exp*,*first,delete e);
    delete first;
    delete others;
    }

Exp::Exp(Variable_Ref *v) : coefs(0) {
	assert(v != 0);
	constantTerm = 0;
	coefs[v] = 1;
	}

Exp * negate ( Exp * x ) {
	x->constantTerm *= -1;
	foreach_map(r,Variable_Ref*,c,int,x->coefs,x->coefs[r]*=-1);
	return x;
	}

Exp * add ( Exp * x,  Exp * y) {
	x->constantTerm += y->constantTerm;
	foreach_map(r,Variable_Ref*,c,int,y->coefs,x->coefs[r]+=c);
	delete y;
	return x;
	}

Exp * subtract ( Exp * x,  Exp * y) {
	x->constantTerm -= y->constantTerm;
	foreach_map(r,Variable_Ref*,c,int,y->coefs,x->coefs[r]-=c);
	delete y;
	return x;
	}

Exp * multiply (int c, Exp * x) {
	x->constantTerm *= c;
	foreach_map(r,Variable_Ref*,v,int,x->coefs, v *= c);
	return x;
	}

Exp * multiply ( Exp * x,  Exp * y) {
	if (x->coefs.empty()) {
		int c = x->constantTerm;
		delete x;
		return multiply(c,y);
		}
	assert(y->coefs.empty());
	int c = y->constantTerm;
	delete y;
	return multiply(c,x);
	}

void printExpList(ExpList *l) {
	foreachSeparated(e,Exp*,*l,printExp(e),printf(", "));
	}

Declaration_Site *current_Declaration_Site = 0;
       
Declaration_Site::Declaration_Site()  {
	previous = current_Declaration_Site;
	current_Declaration_Site = this;
	}

Declaration_Site::Declaration_Site(VarList *v) {
	previous = current_Declaration_Site;
	current_Declaration_Site = this;
	foreach(s,char*,*v,declarations.insert(new Variable_Ref(s)));
	}

Declaration_Site::~Declaration_Site()
    {
    foreach(v,Variable_Ref*,declarations,delete v);
    }

Variable_Ref::Variable_Ref(char *s, int _arity, Argument_Tuple _of) {
	name = s;
	arity = _arity;
	of = _of;
	anonymous = !strncmp("In_",s,3) || !strncmp("Out_",s,4);
	char *t = s;
	while (*t != '\0') t++;
	t--;
	while (*t == '\'') t--;
	t++;
	*t = '\0';
	stripped_name = s;
	g = 0;
	}

Variable_Ref::Variable_Ref(char *s) {
	name = s;
	arity = 0;
	anonymous = !strncmp("In_",s,3) || !strncmp("Out_",s,4);
	char *t = s;
	while (*t != '\0') t++;
	t--;
	while (*t == '\'') t--;
	t++;
	*t = '\0';
	stripped_name = s;
	g = 0;
	}
	
Variable_Ref::Variable_Ref() {
	name = "#anonymous";
	arity = 0;
	anonymous = 1;
	stripped_name = name;
	g = 0;
	}

Variable_Ref::~Variable_Ref()
    {
    assert(g == 0);
    }

Variable_Ref *lookupScalar(char *s) {
	Declaration_Site * ds;
	for(ds = current_Declaration_Site; ds; ds=ds->previous) 
	   foreach(r,Variable_Ref*,ds->declarations,
		if (r->name == Const_String(s)) return r);
	printf("Can't find declaraction for `%s'\n",s);
	ds = current_Declaration_Site; 
	if (ds) {
	   printf("Names in scope are:\n");
	   for(; ds; ds=ds->previous) 
	      foreach(r,Variable_Ref*,ds->declarations,
		   printf("   %s\n",(const char *)r->name));
	   }
	else printf("No names are defined!\n");
	return 0;
	}
	
Declaration_Site * defined(char *s) {
	Declaration_Site * ds;
	for(ds = current_Declaration_Site; ds; ds=ds->previous) 
	   foreach(r,Variable_Ref*,ds->declarations,
		if (r->name == Const_String(s)) return ds);
	return 0;
	}


void AST_Or::install(Formula *F) {
	if (F->node_type() != Op_Or) 
		F = F->add_or();
	left->install(F);
	right->install(F);
	}

void AST_And::install(Formula *F) {
	if (F->node_type() != Op_And) F = F->add_and();
	left->install(F);
	right->install(F);
	}

void AST_Not::install(Formula *F) {
	child->install(F->add_not());
	}

void AST_exists::install(Formula *F) {
	F_Exists * G = F->add_exists();
	foreach(r,Variable_Ref*,declaredVariables->declarations,
		r->vid = G->declare(r->stripped_name));
	child->install(G);
	}

void AST_forall::install(Formula *F) {
	F_Forall * G = F->add_forall();
	foreach(r,Variable_Ref*,declaredVariables->declarations,
		r->vid = G->declare(r->stripped_name));
	child->install(G);
	}

void AST_constraints::install(Formula *F) {
	if (!others) return;
	F_And * f =  F->and_with();
	foreach (e1,Exp*,*first,
		foreach(e2,Exp*,*others->first,
			{ 
			switch(rel_op) {
				case(lt) : install_gt(f,e2,e1); break;
				case(gt) : install_gt(f,e1,e2); break;
				case(leq) : install_geq(f,e2,e1); break;
				case(geq) : install_geq(f,e1,e2); break;
				case(eq) : install_eq(f,e1,e2); break;
				case(neq) : install_neq(f,e1,e2); break;
				default : assert(0);
				}
			}));
	others->install(f);
	}
  
	
void install_neq(F_And *F, Exp *e1, Exp *e2) {
	F_Or *or1 = F->add_or();
	F_And *and1 = or1->add_and();
	F_And *and2 = or1->add_and();
	install_gt(and1,e1,e2);
	install_gt(and2,e2,e1);
	};

void install_stride(F_And *F, strideConstraint *s) {
	Stride_Handle c = F->add_stride(s->step);
	c.update_const(s->e->constantTerm);
	foreach_map(r,Variable_Ref*,co,int,s->e->coefs,
		c.update_coef(r->id(F->relation()),co));
	c.finalize();
	}

void install_eq(F_And *F, Exp *e1, Exp *e2) {
	EQ_Handle c = F->add_EQ();
	c.update_const(e1->constantTerm);
	if (e2) c.update_const(-e2->constantTerm);
	foreach_map(r,Variable_Ref*,co,int,e1->coefs,
		c.update_coef(r->id(F->relation()),co));
	if (e2) foreach_map(r,Variable_Ref*,co,int,e2->coefs,
		c.update_coef(r->id(F->relation()),-co));
	c.finalize();
	}
		
void install_geq(F_And *F, Exp *e1, Exp *e2) {
	GEQ_Handle c = F->add_GEQ();
	c.update_const(e1->constantTerm);
	if (e2) c.update_const(-e2->constantTerm);
	foreach_map(r,Variable_Ref*,co,int,e1->coefs,
		c.update_coef(r->id(F->relation()),co));
	if (e2) foreach_map(r,Variable_Ref*,co,int,e2->coefs,
		c.update_coef(r->id(F->relation()),-co));
	c.finalize();
	}

void install_gt(F_And *F, Exp *e1, Exp *e2) {
	GEQ_Handle c = F->add_GEQ();
	c.update_const(-1);
	c.update_const(e1->constantTerm);
	if (e2) c.update_const(-e2->constantTerm);
	foreach_map(r,Variable_Ref*,co,int,e1->coefs,
		c.update_coef(r->id(F->relation()),co));
	if (e2) foreach_map(r,Variable_Ref*,co,int,e2->coefs,
		c.update_coef(r->id(F->relation()),-co));
	c.finalize();
	}
		

Global_Declaration_Site::~Global_Declaration_Site()
    {
    // Take care of global variables - since we do that kludge
    // of declaring globals twice if arity > 0, we must take care
    // not to just delete each global once per declaration.

    // Actually, we can't free these, since Relations containing references to
	// this may get freed later

    foreach(v,Variable_Ref*,this->declarations,v->g=0);
    //Set<Free_Var_Decl *> globals;
    //foreach(v,Variable_Ref*,this->declarations,(globals.insert(v->g),v->g=0));
    //foreach(g,Free_Var_Decl*,globals,delete g);
    }

Variable_Ref * Global_Declaration_Site::extend(char *s)
    {
    Variable_Ref *r  = new Variable_Ref(s);
    r->g = new Free_Var_Decl(r->stripped_name);
    declarations.insert(r);
    return r;
    }
	
void Global_Declaration_Site::extend_both_tuples(char *s, int arity)
    {
    if (arity == 0)
	extend(s);
    else
	{
	assert(arity > 0);
	char s1[256], s2[256];
	strcpy(s1,s); strcat(s1,"(in)");
	strcpy(s2,s); strcat(s2,"(out)");
	Const_String name = s;

	Variable_Ref *r1 = new Variable_Ref(s1, arity, Input_Tuple);
	Variable_Ref *r2 = new Variable_Ref(s2, arity, Output_Tuple);
	r1->g = r2->g = new Free_Var_Decl(s,arity);
	
	functionOfInput[name] = r1;
	functionOfOutput[name] = r2;

	declarations.insert(r1);
	declarations.insert(r2);
	}
    }
	

void resetGlobals() {
	foreach(r,Variable_Ref*,globalDecls->declarations,
		r->vid = 0);
	}


Variable_Ref * Declaration_Site::extend(char *s) {
	Variable_Ref *r  = new Variable_Ref(s);
	declarations.insert(r);
	return r;
	}

Variable_Ref * Declaration_Site::extend(char *s, Argument_Tuple of , int pos) {
	Variable_Ref *r  = new Variable_Ref(s);
	declarations.insert(r);
	r->of = of;
	r->pos = pos;
	return r;
	}

Variable_Ref * Declaration_Site::extend() {
	Variable_Ref *r  = new Variable_Ref();
	declarations.insert(r);
	return r;
	}
	
void tupleDescriptor::extend(char * s) {
	Variable_Ref *r  = relationDecl->extend(s);
	size++;
	vars.append(r);
	assert(size == vars.size());
	}

void tupleDescriptor::extend(char * s, Argument_Tuple of, int pos) {
	Variable_Ref *r  = relationDecl->extend(s,of,pos);
	size++;
	vars.append(r);
	assert(size == vars.size());
	}

void tupleDescriptor::extend(Exp * e) {
	Variable_Ref *r  = relationDecl->extend();
	size++;
        vars.append(r);
	assert(size == vars.size());
	Exp * eq = subtract( e, new Exp(r));
	eq_constraints.insert(eq); 
	}

void tupleDescriptor::extend(char *s, Exp * e) {
	Variable_Ref *r  = relationDecl->extend(s);
	size++;
        vars.append(r);
	assert(size == vars.size());
	Exp * eq = subtract( e, new Exp(r));
	eq_constraints.insert(eq); 
	}

void tupleDescriptor::extend() {
	Variable_Ref *r  = relationDecl->extend();
	size++;
        vars.append(r);
	assert(size == vars.size());
	}
void tupleDescriptor::extend(Exp * lb,Exp *ub) {
	Variable_Ref *r  = relationDecl->extend();
	size++;
        vars.append(r);
	assert(size == vars.size());
	Exp * lb_exp = subtract(new Exp(r), lb);
	geq_constraints.insert(lb_exp); 
	Exp * ub_exp = subtract(ub, new Exp(r));
	geq_constraints.insert(ub_exp); 
	}
void tupleDescriptor::extend(Exp * lb,Exp *ub, int stride) {
	Variable_Ref *r  = relationDecl->extend();
	size++;
        vars.append(r);
	Exp * lb_exp = subtract(new Exp(r), new Exp(*lb));
	geq_constraints.insert(lb_exp); 
	Exp * ub_exp = subtract(ub, new Exp(r));
	geq_constraints.insert(ub_exp); 
	strideConstraint *s = new strideConstraint;
	s->e = subtract(lb,new Exp(r));
	s->step = stride;
	stride_constraints.insert(s); 
	}

} // end of namespace omega
