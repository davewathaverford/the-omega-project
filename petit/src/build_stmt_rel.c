/* $Id: build_stmt_rel.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <basic/assert.h>
#include <basic/bool.h>
#include <basic/Exit.h>
#include <omega/Relations.h>
#include <omega/closure.h>
#include <petit/lang-interf.h>
#include <petit/debug.h>
#include <petit/ops.h>
#include <petit/browse.h>
#include <petit/Zima.h>
#include <petit/omega2flags.h>
#include <petit/build_stmt_rel.h>
#include <petit/petit_args.h>

namespace omega {

#define printClosure    (closureDebugLevel & 0x1)
#define printBuildInfo  (closureDebugLevel & 0x2)

#define NOT_ASSIGNMENT_STATEMENT -1

static StmtDep stmt_deps[myMaxStmts][myMaxStmts];
static Stmt    statements[myMaxStmts];
static int nStmts;
static int currentStatement;
static int closureDebugLevel;



static void init_stmt_deps_graph(void)
{
 int i,j;
 for (i=0; i<nStmts; i++) {
   for (j=0; j<nStmts; j++)
       {stmt_deps[i][j].exists=0;
        stmt_deps[i][j].is_closure=0;
      }
   statements[i].access=NULL;
  } /* end for i */  
}

static void print_line_header(int from_index, int to_index)
{
         fprintf(debug,"Dependence from  line %2d to line %2d:\n",
                      statements[from_index].tree_node->nodesequence,
                      statements[to_index].tree_node->nodesequence);
} /* end print_line_header */

static int find_assignment_stmts(void)
{
    node *n;

    nStmts = 0;
    for(n = br_go_to_another(Entry); n!=Entry; n = br_go_to_another(n))
	{
        if (is_assign(n) && n->nodechild->nodeop != op_phi)
	    {
	    if (nStmts >= myMaxStmts)
		{
                fprintf(debug,
			"We cannot handle more than %d statements\n",
			myMaxStmts);
		Exit(1);
		}
            n->nodevalue=nStmts;                        
            statements[nStmts].tree_node=n;  
            nStmts++;
	    }
        }
    return true;
    } /* find_assignment_stmts */

static node *parent_stmt(node *n)
    {
    while (n!=NULL && !is_assign(n))
	n=n->nodeparent;
    return n;
    } /* parent_stmt */


/****
 ** UGLY - the array access can occur not only in the assignment statement,
   but also, e.g. in the if statement. 
   For now I chose ugly solution: just ignore this dependencies at all
   Discusss what to do with them
 */

static int stmt_index(node *n)
{
 node *parent=parent_stmt(n);
 if (parent==NULL)
    return NOT_ASSIGNMENT_STATEMENT;
 assert(n!=NULL);
 assert((int)parent->nodevalue<nStmts);
 assert(statements[parent->nodevalue].tree_node == parent);
 return parent->nodevalue;
} /* end stmt_index */


/*NOTE: this function assumes that relation in dd is uncompressed */

static void add_dd_to_stmt_relation(ddnode * dd)
{
 int from_index, to_index;
 StmtDep * elem;
 
 if ((from_index=stmt_index(dd->ddpred))== NOT_ASSIGNMENT_STATEMENT)
    return; 
    
 if ((to_index=stmt_index(dd->ddsucc)) == NOT_ASSIGNMENT_STATEMENT)
    return;

 if (statements[from_index].access==NULL)
   statements[from_index].access=dd->ddpred; 

 if (statements[to_index].access==NULL)
   statements[to_index].access=dd->ddsucc;
    
 elem = &(stmt_deps[from_index][to_index]); 

 if (elem->exists) {
    /* we have a relation between these nodes already */
    elem->dep_rel.uncompress();
    elem->dep_rel=Union(elem->dep_rel,copy(*(dd->dd_relation))); 
    elem->dep_rel.simplify(2,2);
    elem->dep_rel.compress();
   } 
 else {
    elem->dep_rel=*dd->dd_relation; 
    elem->dep_rel.compress();
    elem->exists=1;
    statements[from_index].depth=elem->dep_rel.n_inp();
    statements[to_index].depth=elem->dep_rel.n_out(); 
   } 
} /* end add_dd_to_stmt_relation */
     

static void simplify_all(void)
{
 int i,j;
 for (i=0; i<nStmts; i++)
  for (j=0; j<nStmts; j++)
    if (stmt_deps[i][j].exists)
      {
       stmt_deps[i][j].dep_rel.simplify(1,1);
     }
} /* end simplify_all */
     

static void form_simple_stmt_deps_graph(void)
{
 nametabentry * name;
 node * sd;
 node * n;
 dd_in_iterator dd;
 for (name = NameHead; name!=NULL; name = name->namenext)
   {
    sd = name->namest.symdecl;
    if (sd != NULL && sd->nodelink != NULL && name->namest.symclass!=CLBUILTIN) 
      {
       if (myOmitScalars && name->namest.symdims == 0)
	 continue;
       for (n=sd->nodelink; n!=NULL; n = n->nodelink)
	 {
          for (dd=dd_i_i_for_access(n); !dd_i_i_done(dd); dd_i_i_next(dd))
            {
             dd_flags flags=(petit_args.valueClosure)?ddval:ddmem;
             if ((dd->ddflags & flags) && 
                  dd_i_i_cur_src(dd)!=Entry
                  && dd_i_i_cur_dest(dd)!=ExitNode)
	       {
                 dd_current_relation(dd_i_i_current(dd))->uncompress();
                 add_dd_to_stmt_relation(dd);
                 dd_current_relation(dd_i_i_current(dd))->compress();
               } /* end if */
	   } /* end for dependencies */
	} /* end for access */
    } /* if 'good' name */
 
  } /* end for name */

} /* end form_simple_stmt_dep_graph */	 




static Relation buildBoundsSet(int stmt)
{
 assert(statements[stmt].access!=NULL);
 a_access a=statements[stmt].access;
 Relation it(node_depth(a));
 AccessIteration a1(a, &it, Set_Tuple);

 F_And *f = it.add_and();

 access_in_bounds(f, a1);
 it.finalize();
 it.simplify(1,1); 
 return it;
} /* end buildBoundsSet */


static Relation getBoundsRelation(int stmt1, int stmt2)
{
 assert(statements[stmt1].access!=NULL);
 assert(statements[stmt2].access!=NULL);
 a_access ac1=statements[stmt1].access;
 a_access ac2=statements[stmt2].access;
 Relation it(node_depth(ac1), node_depth(ac2));
 AccessIteration a1(ac1, &it, Input_Tuple);
 AccessIteration a2(ac2, &it, Output_Tuple);

 F_And *f = it.add_and();

// unify_symbolic_constants(f, a1, a2, 0);
       
 access_in_bounds(f, a1);
 access_in_bounds(f, a2);
 it.finalize();
 it.simplify(1,1); 
 return it;
} /* end get BoudnsRelation */


/***
 *  extract_gist_bounds
 ***/
static void extract_gist_bounds(int i,int j)
{
 stmt_deps[i][j].dep_rel= Gist (stmt_deps[i][j].dep_rel,
              Cross_Product(copy(statements[i].bounds), 
			    copy(statements[j].bounds)));
 stmt_deps[i][j].dep_rel.simplify(1,1);
} /* end extract_gist_bounds */	

/***
 * EXTRACT_BOUNDS - build bounds relations for each statement and
 *                  replace every dependence with its gist given boudns
 ***/

static void extract_bounds()
{
 int i,j;
 
 /* build statement bound relations */

 for (i=0; i<nStmts; i++) {
   if (statements[i].access != 0)
     statements[i].bounds=buildBoundsSet(i);    
   else
     statements[i].bounds=Relation::Null();
 }

 /* replace statement dependencies */

 for (i=0; i<nStmts; i++)
   for (j=0; j<nStmts; j++)
       if (stmt_deps[i][j].exists)
	 {
           stmt_deps[i][j].dep_rel.uncompress();  
           extract_gist_bounds(i,j);
           stmt_deps[i][j].dep_rel.compress();
}

} /* end extrace_bounds */


/******
 *  RESTORE_RELATION - form the original relation from
 *     the gist and statement bounds 
 ***/
    
static void restore_relation(int from, int to)
{
 stmt_deps[from][to].dep_rel=
     Intersection( stmt_deps[from][to].dep_rel,
                   Cross_Product( copy(statements[from].bounds),
				  copy(statements[to].bounds)));
} /* end restore_relation */    

/******************************************************************** 
 * CLOSE_STMT_DEPS - calculate a closure through the graph for all the
 *                   dependencies.
 *    calculate:
 *    for k...
 *      for i...
 *        for j...
 *          Rij = Rkj o Rkk* o Rik
 ********************************************************************/
static void close_stmt_deps(void)
{
    int i,j,k;

    if (printBuildInfo)
	{
	    fprintf(debug,"\n\n********* CALCULATING STATEMENT DEPENDENCE CLOSURES ****\n");
	} 
    Relation r;
    Relation r_closure;

    /* compress all the relations */
    for (i=0;i<nStmts; i++) 
	for (j=0;j<nStmts; j++) 
	    if (stmt_deps[i][j].exists)
		stmt_deps[i][j].dep_rel.compress();

/* start loop on k */
    for (k=0;k<nStmts; k++) {
	if (printBuildInfo) 
	    fprintf(debug, "Computing closure via stmt %d\n",k);
	printf("Computing closure via stmt %d\n",k);


	for (i=0;i<nStmts; i++) {
	    if (stmt_deps[i][k].exists) {
		stmt_deps[i][k].dep_rel.uncompress();
		restore_relation(i,k);
	    }  
	    if (i != k  && stmt_deps[k][i].exists) {
		stmt_deps[k][i].dep_rel.uncompress();
		restore_relation(k,i);
	    }
	};
	if (stmt_deps[k][k].exists)
	    {

		/* get  closure  of Rkk */

		stmt_deps[k][k].dep_rel.simplify(1,1);
		if (printBuildInfo) {
		    fprintf(debug, "\nRelation from k to k (%d to %d): [\n",k,k);
		    print_given_bounds(stmt_deps[k][k].dep_rel,getBoundsRelation(k,k));
		}
		if (printClosure)
		    fprintf(debug, "---------------------------------------------------------\n");

		currentStatement=k; 

		r_closure=TransitiveClosure(copy(stmt_deps[k][k].dep_rel),1,
					    copy(statements[k].bounds));
       
		r_closure.simplify(2,2);

		if (printConjunctClosure)
		    fprintf(debug,"\n----> Investigating closure of the whole relation\n");

		if (printClosure) {
		    InvestigateClosure(stmt_deps[k][k].dep_rel, r_closure,statements[k].bounds); 
		}

		stmt_deps[k][k].dep_rel=r_closure; 

		if (printBuildInfo) {
		    fprintf(debug,"\nIts transitive closure:\n"); 
		    print_given_bounds(r_closure,getBoundsRelation(k,k));
		    fprintf(debug,"]\n");
		}         
 
	    }
	else {
	    if (printBuildInfo) 
		fprintf(debug, "Statement %d doesn't currently have any self-dependences\n",k);
	    r_closure=Relation::Null();
	}

	/* get dependencies coming to k */


	if (printBuildInfo) {
	    fprintf(debug,"\nUpdate dependences coming to statment %d\n", k);
	}

	if (stmt_deps[k][k].exists) 
	    {
		for (i=0; i<nStmts; i++)
		    if (i!=k && stmt_deps[i][k].exists)
			{
			    stmt_deps[i][k].dep_rel=Union(stmt_deps[i][k].dep_rel,
							  Composition(copy(r_closure), 
								      copy(stmt_deps[i][k].dep_rel)));
			    stmt_deps[i][k].dep_rel.simplify(1,1);
			    if ( printBuildInfo) {
				fprintf(debug, "Updated dependence %d->%d is:\n",i,k);
				print_given_bounds(stmt_deps[i][k].dep_rel,getBoundsRelation(i,k));
			    }
			} /* end if, for */
	    } /* end if Rkk exists */

	/* update dependencies coming through k */

	if (printBuildInfo) {
	    fprintf(debug, "\nUpdate dependences going through statement %d\n",k);
	}

	for (i=0; i<nStmts; i++)
	    if (i!=k && stmt_deps[i][k].exists) 
		{
		    for (j=0; j<nStmts; j++)
			if (( (i>k&&j!=k) || j>k || i==j)  && stmt_deps[k][j].exists)
			    {  
				if (stmt_deps[i][j].exists) {
				    stmt_deps[i][j].dep_rel.uncompress();
				    restore_relation(i,j);
				}   

				r=Composition(copy(stmt_deps[k][j].dep_rel), 
					      copy(stmt_deps[i][k].dep_rel)); 
				r.simplify(1,1);
				if (printBuildInfo) {
				    fprintf(debug,"\nNew part of the relation from %d to %d is:\n",i,j);
				    print_given_bounds(r,getBoundsRelation(i,j));
				}
				if (stmt_deps[i][j].exists)
				    {
					stmt_deps[i][j].dep_rel= Union(stmt_deps[i][j].dep_rel, r);
					stmt_deps[i][j].dep_rel.simplify(1,1);
					if (printBuildInfo) {
					    fprintf(debug,"\nUpdated relation from %d to %d is:\n", i,j);
					    print_given_bounds(stmt_deps[i][j].dep_rel,getBoundsRelation(i,j));
					}
				    } 
				else
				    {
					stmt_deps[i][j].dep_rel=r;
					stmt_deps[i][j].exists=1;
					if (printBuildInfo) {
					    fprintf(debug, "Relation from %d to %d is stored\n",i,j);
					}
				    }
				extract_gist_bounds(i,j);
				stmt_deps[i][j].dep_rel.compress();
      
			    } /* end for j */
		} /* end for i */


	for (i=0;i<k; i++)  {
	    if (stmt_deps[i][k].exists) {
		if (printBuildInfo)  fprintf(debug, "Erasing dependence %d->%d\n",k,i);
		stmt_deps[i][k].exists = 0;
		stmt_deps[i][k].dep_rel = Null_Relation();
	    };
	    if (stmt_deps[k][i].exists) {
		if (printBuildInfo)  fprintf(debug, "Erasing dependence %d->%d\n",k,i);
		stmt_deps[k][i].exists = 0;
		stmt_deps[k][i].dep_rel = Null_Relation();
	    };
	};
	for (i=k;i<nStmts; i++) {
	    if (stmt_deps[i][k].exists) {
		extract_gist_bounds(i,k);
		stmt_deps[i][k].dep_rel.compress();
	    }    
	    if (i != k && stmt_deps[k][i].exists)  {
		extract_gist_bounds(k,i); 
		stmt_deps[k][i].dep_rel.compress();
	    };
	};
    } /* end for k */ 

    for (k=0;k<nStmts; k++) 
	if (stmt_deps[k][k].exists) stmt_deps[k][k].dep_rel.uncompress();
} /* end close_stmt_deps */ 

            

static void print_stmt_deps_graph(void)
{
 int i,j;
 fprintf(debug,"There are %d statements in this program\n",nStmts);
 fprintf(debug,"DEPENDENCE RELATION LIST:\n"); 
 for (i=0; i<nStmts; i++)
   for (j=0; j<nStmts; j++)
     {
      if (stmt_deps[i][j].exists)
       {  
        print_line_header(i,j);
	stmt_deps[i][j].dep_rel.uncompress();
        stmt_deps[i][j].dep_rel.print_with_subs(debug);
        fprintf(debug,"\n");
	stmt_deps[i][j].dep_rel.compress();
      }
     }
} /* end print_stmt_deps_graph */




/*---------------------------------------------------------------*/

static void build_stmt_deps_graph(void)
{
 find_assignment_stmts();
    
 init_stmt_deps_graph();        
 
  printf("There are %d statements in this program \n",nStmts);

  form_simple_stmt_deps_graph();
  printf("Original statement dependence graph is formed...\n");
  extract_bounds();
  if (printBuildInfo)
    {
     fprintf(debug,"\n*********Original statement dependencies********\n");
     print_stmt_deps_graph();
   }
  close_stmt_deps();
  simplify_all();
 
  if (printBuildInfo)
    {
     fprintf(debug, "\n********Closed statement dependencies******\n");
     print_stmt_deps_graph();
   }
}


/*----------------------------------------------------------------*/


void set_closure_flags(int n)
    {
    closureDebugLevel=1;
    if (n==1)
        closureDebugLevel=3;
    } /* set_closure_flags */




void print_closure_help(void)
    {
    printf("\nOptions for the -T flag:\n");
    printf("\nT0 - check closure\n");
    printf("\nT1 - check closure and debug graph building\n");
    printf("\nT is the same T0\n");
    printf("\nTv - calculate closure of the value-based dependencies\n");
    } /* print_closure_help */





void test_closure(int found)
    {
    if (found)
        build_stmt_deps_graph();
    else
        {
        fprintf(stderr, "Cannot find file %s\n", petit_args.FileName);
        Exit(1);
        }
    } /* test_closure */

} // end namespace omega
