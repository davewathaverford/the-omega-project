/* $Id: depend_rel2.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <omega/Relations.h>
#include <uniform/depend_rel.h>
#include <uniform/select.h>
#include <uniform/uniform_misc.h>

namespace omega {


void dump_graph(depend_graph &graph, FILE *f)
    {
    for (int i=0; i<nr_statements; i++)
	{
	fprintf(f,"%d->%d:\n", i, i);
	graph[i+1].self_depend.print_with_subs(f);
        for (rel_edge *e1=graph[i+1].forw_link;e1!=NULL;e1=e1->next_forw_link)
	    {
	    fprintf(f,"%d->%d:\n", e1->parent_index, e1->child_index);
	    e1->rel.print_with_subs(f);
	    }
	}
    }

void copy_depends_graph(depend_graph &from, depend_graph &to)
    {
    int i;
    rel_edge *e1, *e2;

    to.reallocate(nr_statements);

    for (i=0; i<nr_statements; i++)
        {
        to[i+1].forw_link = NULL;
        to[i+1].back_link = NULL;

        to[i+1].self_depend = from[i+1].self_depend;
        to[i+1].constraints = from[i+1].constraints;
	}

    for (i=0; i<nr_statements; i++)
        for (e1=from[i+1].forw_link; e1!=NULL; e1=e1->next_forw_link)
            {
            e2 = new rel_edge;

            e2->child_index = e1->child_index;
            e2->parent_index = e1->parent_index;

	    e2->rel = e1->rel;
	    e2->constraints = e1->constraints;

            e2->next_forw_link = to[e1->parent_index+1].forw_link;
            e2->next_back_link = to[e1->child_index+1].back_link;

            to[e1->parent_index+1].forw_link = e2;
            to[e1->child_index+1].back_link = e2;
            }
    } /* copy_depends_graph */


void copy_depends_graph2(depend_graph &from, depend_graph &to)
    {
    int i;
    rel_edge *e1, *e2;

    to.reallocate(nr_statements);

    for (i=0; i<nr_statements; i++)
        {
        to[i+1].forw_link = NULL;
        to[i+1].back_link = NULL;

        to[i+1].self_depend = from[i+1].self_depend;
        to[i+1].constraints = from[i+1].constraints;
	}

    for (i=0; i<nr_statements; i++)
        for (e1=from[i+1].forw_link; e1!=NULL; e1=e1->next_forw_link)
            {
            e2 = new rel_edge;

            e2->child_index = e1->child_index;
            e2->parent_index = e1->parent_index;

	    e2->rel = e1->rel;
	    e2->constraints = e1->constraints;

            e2->next_forw_link = to[e1->parent_index+1].forw_link;
            e2->next_back_link = to[e1->child_index+1].back_link;

            to[e1->parent_index+1].forw_link = e2;
            to[e1->child_index+1].back_link = e2;
            }
    } /* copy_depends_graph2 */




static void add_relation_to_graph(Depend *dd, depend_graph &current_rel)
    {
    int p1, p2;
    rel_edge *e;

    p1 = StmtNr(DependSrc(dd));
    p2 = StmtNr(DependDest(dd));
    if (p1 == -1 || p2 == -1)
        UniformError("contains control dependences");

    Relation new_node(*DependRelation(dd));

    if (p1 == p2)
	{
	current_rel[p1+1].self_depend = 
	    Union(current_rel[p1+1].self_depend, new_node);
        return;
	}

    for (e = current_rel[p1+1].forw_link;
	 e!=NULL && e->child_index!=p2;
	 e = e->next_forw_link);

    if (e != NULL)
	{
	e->rel = Union(e->rel, new_node); 
	} 
    else 
	{
	e = new rel_edge;
	e->rel = new_node;
	e->constraints = Relation::False(0);
	e->parent_index = p1;
	e->child_index = p2;

	e->next_forw_link = current_rel[p1+1].forw_link;
	current_rel[p1+1].forw_link = e;

	e->next_back_link = current_rel[p2+1].back_link;
	current_rel[p2+1].back_link = e;
	}
    } /* add_relation_to_graph */



int build_depend_relation_graph(void)
    {
    for (int i = 1; i<= nr_statements; i++)
        {
        for (AccessIterator ai = AccessLeftMost(stmt_info[i].stmt); 
	     !AccessLast(ai);
             AccessNext(ai))
            {
	    Access *n = AccessCurr(ai);
	    for(DependIterator di = DependOutgoing(n);
		!DependLast(di);
		DependNext(di))
	        {
		Depend *dd = DependCurr(di);

                if (AccessIsEntry(DependSrc(dd)) ||
                    AccessIsExit(DependDest(dd))) 
                    continue;
                    
		if (!DependIsValue(dd) && DependIsFlow(dd))
		    continue;

                if (DependIsReduction(dd))
		    continue;

		add_relation_to_graph(dd, all_depends);

		if (DependIsFlow(dd))
		    add_relation_to_graph(dd, val_flow_depends);
                }
	    }
        }

    {   // extra braces apparently avoid Visual C++ bug
    for (int i=0; i<nr_statements; i++)
	{
	all_depends[i+1].self_depend.simplify();
        for (rel_edge *e2=all_depends[i+1].forw_link; 
	     e2!=NULL; 
	     e2=e2->next_forw_link)
	     e2->rel.simplify();
	}
    }

    {
    for (int i=0; i<nr_statements; i++)
	{
	val_flow_depends[i+1].self_depend.simplify();
        for (rel_edge *e2=val_flow_depends[i+1].forw_link; 
	     e2!=NULL; 
	     e2=e2->next_forw_link)
	     e2->rel.simplify();
	}
    }

    return 1;
    } /* build_depend_relation_graph */

}
