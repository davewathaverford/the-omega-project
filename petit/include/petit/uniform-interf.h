/* $Id: uniform-interf.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#ifndef Already_Included_Uniform_Interf
#define Already_Included_Uniform_Interf 1

#include "petit/tree.h"
#include "petit/affine.h"

namespace omega {

typedef node 		AssignStmt;
typedef node 		LoopStmt;
typedef node 		Expr;
typedef node 		Access;
typedef affine_term 	Term;
typedef symtabentry 	Variable;
typedef ddnode 		Depend;

 
typedef node* 		LoopIterator;

typedef node* 		SubscriptIterator;

typedef nametabentry* 	VarIterator;

typedef struct 
    {
    int posn;
    int nr_stmts;
    node* list[200];
    } AssignIterator;

typedef struct
    {
    int posn;
    int nr_accesses;
    node* list[200];
    } AccessIterator;

typedef struct
    {
    int posn;
    affine_expr* terms;
    } TermIterator;

typedef struct
    {
    ddnode* dd;
    bool down;
    } DependIterator;


extern AssignIterator	 AssignFirst();
extern VarIterator	 VarFirst();

extern int 	 	 NrLoops(AssignStmt *stmt);
extern int		 AssignCommonNest(AssignStmt *a1, AssignStmt *a2);
extern bool		 AssignIsSum(AssignStmt *stmt);
extern bool		 AssignIsProduct(AssignStmt *stmt);
extern bool		 AssignIsMax(AssignStmt *stmt);
extern bool		 AssignIsMin(AssignStmt *stmt);
extern String 		 AssignStmtString(AssignStmt *stmt);
extern Relation		 IterationSpace(AssignStmt *stmt);
extern LoopIterator	 LoopInnerMost(AssignStmt *stmt);
extern AccessIterator	 AccessLeftMost(AssignStmt *stmt);

extern Variable		*LoopIndex(LoopStmt *loop);
extern Expr		*LoopStep(LoopStmt *loop);

extern bool		 ExprIsAffine(Expr *exp);
extern TermIterator	 TermLeftMost(Expr *exp);
extern bool		 ExprIsNegOne(Expr *e);

extern int		 StmtNr(Access *access);
extern int		 AccessCommonNest(Access *a1, Access *a2);
extern bool		 AccessIsUpdate(Access *access);
extern bool		 AccessIsEntry(Access *access);
extern bool		 AccessIsExit(Access *access);
extern Variable		*AccessVar(Access *access);
extern SubscriptIterator SubscriptLeftMost(Access *access);
extern DependIterator	 DependIncoming(Access *access);
extern DependIterator	 DependOutgoing(Access *access);

extern Variable		*TermVar(Term *term);
extern int		 TermCoef(Term *term);

extern AccessIterator	 AccessFirst(Variable *stmt);
extern String		 VarName(Variable *v);
extern String 		 VarDecl(Variable *v);
extern int		 VarDimension(Variable *v);
extern bool		 VarIsIndex(Variable *v);
extern int		 IndexLoopNr(Variable *v);

extern bool		 DependIsValue(Depend *d);
extern bool		 DependIsFlow(Depend *d);
extern bool		 DependIsReduction(Depend *d);
extern Relation	 	*DependRelation(Depend *d);
extern Access		*DependSrc(Depend *d);
extern Access		*DependDest(Depend *d);


extern void		 AssignNext(AssignIterator &i);
extern bool		 AssignLast(AssignIterator i);
extern AssignStmt	*AssignCurr(AssignIterator i);

extern void		 LoopNext(LoopIterator &i);
extern bool		 LoopLast(LoopIterator i);
extern LoopStmt		*LoopCurr(LoopIterator i);

extern void		 AccessNext(AccessIterator &i);
extern bool		 AccessLast(AccessIterator i);
extern Access		*AccessCurr(AccessIterator i);

extern void		 SubscriptNext(SubscriptIterator &i);
extern bool	 	 SubscriptLast(SubscriptIterator i);
extern Expr		*SubscriptCurr(SubscriptIterator i);

extern void		 TermNext(TermIterator &i);
extern bool 		 TermLast(TermIterator i);
extern Term		*TermCurr(TermIterator i);

extern void		 DependNext(DependIterator &i);
extern bool	 	 DependLast(DependIterator i);
extern Depend		*DependCurr(DependIterator i);

extern void		 VarNext(VarIterator &i);
extern bool		 VarLast(VarIterator i);
extern Variable		*VarCurr(VarIterator i);

}

#endif
