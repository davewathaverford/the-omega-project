/* $Id: select.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <uniform/uniform_misc.h>
#include <uniform/select.h>

namespace omega {


static void initialize_stmt(AssignStmt *n)
    {
    int nest = NrLoops(n);

    depend_info dep;
    dep.forw_link = NULL;
    dep.back_link = NULL;
    dep.constraints= Relation::False(0);
    dep.self_depend = Relation::False(nest,nest);

    val_flow_depends.append(dep);
    all_depends.append(dep);

    sinfo stmt;
    stmt.nest = nest;
    stmt.stmt = n;

    Access *lhs = AccessCurr(AccessLeftMost(n));
    
    for (int i = 1; i<=nest; i++)
        stmt.distributed[i] = 0;

    for (SubscriptIterator si = SubscriptLeftMost(lhs);
         !SubscriptLast(si);
         SubscriptNext(si))
        {
        Expr *s = SubscriptCurr(si);
        if (ExprIsAffine(s))
            {
            int loop = 0, coef = 0;
	    int nr_indexes = 0;
            for (TermIterator ti = TermLeftMost(s);
		 !TermLast(ti);
		 TermNext(ti))
		{
		Term *t = TermCurr(ti);
		if (VarIsIndex(TermVar(t)))
                    {
	            coef = TermCoef(t);
                    loop = IndexLoopNr(TermVar(t));
		    nr_indexes++;
                    }
		}
            if (nr_indexes == 1 && abs(coef) == 1)
	        stmt.distributed[loop] = 1;
            }
        }

    stmt.bounds = IterationSpace(n);

    if (stmt.bounds.unknown_uses() != no_u)
        UniformError("iteration spaces are not affine");

    if (!stmt.bounds.is_upper_bound_satisfiable())
        return;
    
    stmt_info.append(stmt);

#if 0
    fprintf(uniform_debug, "\nstmt %d\n", nr_statements);
    rel.print_with_subs(uniform_debug);
    for (i = 1; i<=nest; i++)
	fprintf(uniform_debug, "loop %d, distributed = %d\n",
	       i, stmt.distributed[i]);
#endif

    nr_statements++;
    } /* initialize_stmt */


void traverse()
    {
    for (AssignIterator ai = AssignFirst(); !AssignLast(ai); AssignNext(ai))
        initialize_stmt(AssignCurr(ai));
    } /* traverse */

}
