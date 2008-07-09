/* $Id: uniform-interf.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include "petit/uniform-interf.h"
#include "petit/ddnest.h"
#include "petit/Zima.h"
#include "petit/ddodriver.h"
#include <petit/print.h>

namespace omega
{

static void label_stmts(int i, node *n)
    {
    if (n == NULL) return;
 
    if (is_mem_ref(n))
        {
        n->compound_stmt = i+1;
	}

 
    for (node *nn=n->nodechild; nn!=NULL; nn=nn->nodenext)
        label_stmts(i, nn);
    } /* label_stmts */


static void traverse_pgm(AssignIterator &ai, node *n)
    {
    if (is_assign(n) && n->nodechild->nodeop != op_phi)
        {
        ai.list[ai.nr_stmts] = n;
        label_stmts(ai.nr_stmts, n);
        ai.nr_stmts++;
        }
 
    for (node *nn=n->nodechild; nn!=NULL; nn=nn->nodenext)
        traverse_pgm(ai, nn);
    } /* traverse_pgm */


AssignIterator AssignFirst()
    {
    AssignIterator ai;
    ai.nr_stmts = 0;
    traverse_pgm(ai, Entry);
    ai.posn = 0;
    return ai;
    }


VarIterator VarFirst()
    {
    nametabentry *n;
    for (n = NameHead; n!= NULL; n=n->namenext)
        {
	node *dcl = n->namest.symdecl;
        if (dcl!=NULL && dcl->nodeop==op_declare &&
            n->namest.symclass != CLBUILTIN)
            break;
        }
    return n;
    }




int  NrLoops(AssignStmt *stmt)
    {
    assert(stmt!=NULL && is_assign(stmt));
    while (stmt->nodeop!=op_dolimit)
        {
        stmt=stmt->nodeparent;
        if (stmt == NULL) return 0;
        }
    return stmt->nodeparent->nodevalue;
    }


int AssignCommonNest(AssignStmt *stmt1, AssignStmt *stmt2)
    {
    assert(stmt1!=NULL && is_assign(stmt1));
    assert(stmt2!=NULL && is_assign(stmt2));
    unsigned int onest, inest, bnest;
    dd_get_nests(stmt1,stmt2, &onest,&inest,&bnest);
    dd_fix_nests(stmt1,stmt2, onest,inest,bnest);
    return bnest;
    }


int AccessCommonNest(Access *a1, Access *a2)
    {
    assert(a1!=NULL && is_mem_ref(a1));
    assert(a2!=NULL && is_mem_ref(a2));
    unsigned int onest, inest, bnest;
    dd_get_nests(a1,a2, &onest,&inest,&bnest);
    dd_fix_nests(a1,a2, onest,inest,bnest);
    return bnest;
    }


bool AssignIsSum(AssignStmt *stmt)
    {
    assert(stmt!=NULL && is_assign(stmt));
    return stmt->nodeop == op_add_assign;
    }


bool AssignIsProduct(AssignStmt *stmt)
    {
    assert(stmt!=NULL && is_assign(stmt));
    return stmt->nodeop == op_mul_assign;
    }


bool AssignIsMax(AssignStmt *stmt)
    {
    assert(stmt!=NULL && is_assign(stmt));
    return stmt->nodeop == op_max_assign;
    }


bool AssignIsMin(AssignStmt *stmt)
    {
    assert(stmt!=NULL && is_assign(stmt));
    return stmt->nodeop == op_min_assign;
    }


String AssignStmtString(AssignStmt *stmt)
    {
    assert(stmt!=NULL && is_assign(stmt));
    print_template_stmt(stmt);
    return printBuf;
    }


Relation IterationSpace(AssignStmt *stmt)
    {
    assert(stmt!=NULL && is_assign(stmt));
    Relation is(NrLoops(stmt));
    AccessIteration a(stmt, &is, Input_Tuple);
    access_in_bounds(is.add_and(), a, NULL, false);
    return is;
    }


LoopIterator LoopInnerMost(AssignStmt *stmt)
    {
    assert(stmt!=NULL && is_assign(stmt));
    node *n;
    for (n = stmt; n!=NULL && !do_op(n->nodeop); n=n->nodeparent);
    return n;
    }


static void traverse_stmt(AccessIterator &ai, node *n)
    {
    switch(n->nodeop)
        {
        CASE_MEMREF:
            ai.list[ai.nr_accesses++] = n;
            break;
        case op_add:
        case op_subtract:
        case op_multiply:
        case op_divide:
        case op_floor_divide:
        case op_ceiling_divide:
        case op_sqrt:
        case op_exp:
        case op_min:
        case op_max:
        case op_mod:
        case op_lt:
        case op_le:
        case op_eq:
        case op_ne:
        case op_ge:
        case op_gt:
        case op_rtoi:
        CASE_ASSIGN:
        case op_negate:
        case op_call: 
        {
            for (node *child=n->nodechild; child!=NULL; child=child->nodenext)
                traverse_stmt(ai, child);
            break;
	}
        case op_real:
        case op_index:
        case op_constant:
        case op_rand:
        case op_phi:
            break;
        default:
            assert(0);
        }
    }



AccessIterator AccessLeftMost(AssignStmt *stmt)
    {
    assert(stmt!=NULL && is_assign(stmt));
    AccessIterator ai;
    ai.nr_accesses = 0;
    traverse_stmt(ai, stmt->nodechild->nodenext);
    traverse_stmt(ai, stmt->nodechild);
    ai.posn = 0;
    return ai;
    }




Variable *LoopIndex(LoopStmt *loop)
    {
    assert(loop!=NULL && do_op(loop->nodeop));
    return (symtabentry *)loop->nodechild->nodevalue;
    }


Expr*LoopStep(LoopStmt *loop)
    {
    assert(loop!=NULL && do_op(loop->nodeop));
    return loop->nodechild->nodenext->nodenext->nodenext;
    }




bool ExprIsAffine(Expr *exp)
    {
    assert(exp!=NULL);
    return node_is_affine(exp);
    }


TermIterator TermLeftMost(Expr *exp)
    {
    TermIterator ti;
    ti.posn = 1;
    ti.terms = exp->nodeaffine;
    return ti;
    }


bool ExprIsNegOne(Expr *e)
    {
    return (e!=NULL && !(e->nodeop == op_constant && e->nodevalue == 1));
    }




int StmtNr(Access *access)
    {
    assert(access!=NULL && is_mem_ref(access));
    return access->compound_stmt-1;
    }


bool AccessIsUpdate(Access *access)
    {
    assert(access!=NULL && is_mem_ref(access));
    return update_op(access->nodeop);
    }


bool AccessIsEntry(Access *access)
    {
    assert(access!=NULL);
    return access->nodeop == op_entry;
    }


bool AccessIsExit(Access *access)
    {
    assert(access!=NULL);
    return access->nodeop == op_exit;
    }


Variable*AccessVar(Access *access)
    {
    assert(access!=NULL && is_mem_ref(access));
    return get_nodevalue_sym(get_nodevalue_node(access));
    }


SubscriptIterator SubscriptLeftMost(Access *access)
    {
    assert(access!=NULL && is_mem_ref(access));
    return access->nodechild;
    }


DependIterator DependIncoming(Access *access)
    {
    assert(access!=NULL && is_mem_ref(access));
    DependIterator di;
    di.dd = access->nodeddin;
    di.down = 0;
    return di;
    }


DependIterator DependOutgoing(Access *access)
    {
    assert(access!=NULL && is_mem_ref(access));
    DependIterator di;
    di.dd = access->nodeddout;
    di.down = 1;
    return di;
    }




Variable* TermVar(Term *term)
    {
    assert(term != NULL);
    return term->petit_var;
    }


int TermCoef(Term *term)
    {
    assert(term != NULL);
    return term->coefficient;
    }




AccessIterator AccessFirst(Variable *v)
    {
    AccessIterator ai;
    ai.nr_accesses = 0;
    for (node *r1=v->symdecl->nodelink; r1; r1=r1->nodelink)
        {
        if (InDecl(r1) || is_phi_access(r1))
            continue;
        ai.list[ai.nr_accesses++] = r1;
        }
    ai.posn = 0;
    return ai;
    }


String VarName(Variable *v)
    {
    assert(v != NULL);
    return String(v->symname);
    }


String VarDecl(Variable *v)
    {
    assert(v != NULL);
    print_decl_out(v->symdecl);
    return printBuf;
    }


int VarDimension(Variable *v)
    {
    assert(v != NULL);
    return v->symdims;
    }


bool VarIsIndex(Variable *v)
    {
    assert(v != NULL);
    return var_id_is_index(v);
    }


int IndexLoopNr(Variable *v)
    {
    assert(v != NULL);
    assert(var_id_is_index(v));
    return var_id_loop_no(v);
    }




bool DependIsValue(Depend *d)
    {
    assert(d != NULL);
    return ddisValue(d);
    }


bool DependIsFlow(Depend *d)
    {
    assert(d != NULL);
    return d->ddtype == ddflow;
    }


bool DependIsReduction(Depend *d)
    {
    assert(d != NULL);
    return d->ddtype == ddreduce;
    }


Relation *DependRelation(Depend *d)
    {
    assert(d != NULL);
    return d->dd_relation;
    }


Access*DependSrc(Depend *d)
    {
    assert(d != NULL);
    return d->ddpred;
    }


Access*DependDest(Depend *d)
    {
    assert(d != NULL);
    return d->ddsucc;
    }




void AssignNext(AssignIterator &i)
    {
    i.posn++;
    }


bool AssignLast(AssignIterator i)
    {
    return i.posn >= i.nr_stmts;
    }


AssignStmt*AssignCurr(AssignIterator i)
    {
    assert(i.posn < i.nr_stmts);
    return i.list[i.posn];
    }




void VarNext(VarIterator &i)
    {
    for (i = i->namenext; i!= NULL; i=i->namenext)
        {
	node *dcl = i->namest.symdecl;
        if (dcl!=NULL && dcl->nodeop==op_declare &&
            i->namest.symclass != CLBUILTIN)
            break;
        }
    }


bool VarLast(VarIterator i)
    {
    return i == NULL;
    }


Variable* VarCurr(VarIterator i)
    {
    return &(i->namest);
    }




void LoopNext(LoopIterator &i)
    {
    assert(i!=NULL && do_op(i->nodeop));
    for (i = i->nodeparent; i!=NULL && !do_op(i->nodeop); i=i->nodeparent);
    }


bool LoopLast(LoopIterator i)
    {
    return i == NULL;
    }


LoopStmt* LoopCurr(LoopIterator i)
    {
    assert(i!=NULL && do_op(i->nodeop));
    return i;
    }




void AccessNext(AccessIterator &i)
    {
    i.posn++;
    }


bool AccessLast(AccessIterator i)
    {
    return i.posn >= i.nr_accesses;
    }


Access*AccessCurr(AccessIterator i)
    {
    assert(i.posn < i.nr_accesses);
    return i.list[i.posn];
    }




void SubscriptNext(SubscriptIterator &i)
    {
    assert(i!=NULL);
    i = i->nodenext;
    }

bool SubscriptLast(SubscriptIterator i)
    {
    return i == NULL;
    }


Expr*SubscriptCurr(SubscriptIterator i)
    {
    assert(i!= NULL);
    return i;
    }




void TermNext(TermIterator &i)
    {
    i.posn++;
    }


bool TermLast(TermIterator i)
    {
    return i.posn >= i.terms->nterms;
    }


Term* TermCurr(TermIterator i)
    {
    assert(i.posn < i.terms->nterms);
    return &(i.terms->terms[i.posn]);
    }




void DependNext(DependIterator &i)
    {
    if (i.down)
        i.dd = i.dd->ddnextsucc;
    else
        i.dd = i.dd->ddnextpred;
    }


bool DependLast(DependIterator i)
    {
    return i.dd == NULL;
    }


Depend*DependCurr(DependIterator i)
    {
    return i.dd;
    }

} // end namespace omega

