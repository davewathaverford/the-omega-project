/* $Id: depcycles.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

/*
 * Code to find cycles in dependence garph.
 * vadik@cs.umd.edu, June 93.
 */

#include <basic/bool.h>
#include <petit/Exit.h>
#include <petit/ivr.h>
#include <petit/tree.h>
#include <petit/omega2flags.h>
#include <petit/debug.h>
#include <petit/petit_args.h>

namespace omega {

#define maxDepSccNo 500

pEXPR dep_scc[maxDepSccNo];
int dep_scc_no;

static void DCinitStmts(pSTMT);
static void DCcomputeEdges(pSTMT);
static void DCcallTarjan(pSTMT);
static node *DCgoDown(node *, void **);
static node *DCgoRight(node *, node *, void **);
static node *RefToStmt(node *);
static void DCaddToScc(node *);
static void DCprocessScc(node *);
static void DCsetDepFlag(node *);

bool print_now_or_never;
bool valueBasedCycles;

void FindFlowCycles(bool print, bool valueBased) {
  vertex_no = 0;
  ps_i = 0;
  sc_region = 1;

  if(petit_args.ivr_debug) {
	fprintf(debug,"============= DepCycles [\n");
	if (valueBased) fprintf(debug,"Finding flow cycles\n");
	else fprintf(debug,"Finding all cycles\n");
	if (print) fprintf(debug,"Print now or never\n");
	}
  print_now_or_never = print;
  valueBasedCycles = valueBased;

  traverse_stmts(Entry, &DCinitStmts);
  traverse_stmts(Entry, &DCcomputeEdges);

  GoDownFun  = &DCgoDown; 
  GoRightFun = &DCgoRight;
  AddToScc   = &DCaddToScc; 
  ProcessScc = &DCprocessScc;
  FindSccTxt = "DepCycle";
  dep_scc_no = 0;

  traverse_stmts(Entry, &DCcallTarjan);

  /* tag dependences which go from SCC to the same SSC as cyclic */
  traverse_stmts(Entry, &DCsetDepFlag);
  if(petit_args.ivr_debug) {
	  fprintf(debug,"]============= DepCycles\n");
	}
}


static
void DCinitStmts(pSTMT p) {
  if(is_assign(p)) {
    scc_number(p) = 0;
    scc_region(p) = 0;
    p->edges.clear();
    reset(scc_onstack(p));
  }
}
static
void DCcomputeEdges(pSTMT p) {
  if(is_assign(p)) {
  ddnode *dd;
  for(dd=p->nodechild->nodenext->nodeddout; dd!=NULL; dd=dd->ddnextsucc) {
    if (dd->ddtype != ddreduce &&
	(!valueBasedCycles || (!ddisDead(dd)  && dd->ddtype==ddflow)))
	{
	node *s = RefToStmt(dd->ddsucc);
	if (is_assign(s) && s != p) {
	    p->edges.insert(s);
	    if(petit_args.ivr_debug) {
		fprintf(debug,"adding edge from %p: %s",
			p, access_as_string(p));
		fprintf(debug,"              to %p: %s",
			s, access_as_string(s));
		fprintf(debug," %s\n", brdd_print(dd));
		}
	    };
	};
    };
  if (!valueBasedCycles) 
   for(dd=p->nodechild->nodenext->nodeddin; dd!=NULL; dd=dd->ddnextpred) {
    if (dd->ddtype != ddreduce &&
	(!valueBasedCycles || (!ddisDead(dd)  && dd->ddtype==ddflow)))
	{
	node *s = RefToStmt(dd->ddpred);
	if (is_assign(s) && s != p) {
	    s->edges.insert(p);
	    if(petit_args.ivr_debug) {
		fprintf(debug,"adding edge (%p->%p):\n  R %s\n",
			s,p,brdd_print(dd));
		
		}
	    };
	};
    }
  }
}

static
void DCcallTarjan(pSTMT p) {
  if(is_assign(p)) {
    if(scc_number(p)==0)
      FindSCC(p);
  }
}

static 
node *DCgoDown(node *p, void ** /*w*/) { // w unused
  Assert(is_assign(p), "DCgoDown: not assignment statement");
  if (p->edges.empty()) return 0;
  Any_Iterator<node*> edgeIterator = p->edges.any_iterator();
  assert(edgeIterator);
  return edgeIterator.curr();
  }
  

static // w unused
node *DCgoRight(node *p, node *p0, void ** /*w*/) {
  Assert(is_assign(p), "DCgoRight: not assignment statement");
  Assert(is_assign(p0), "DCgoRight: not assignment statement");
  Any_Iterator<node*> edgeIterator = p0->edges.any_iterator();
  assert(edgeIterator);
  while (edgeIterator.curr() != p) {
      assert(edgeIterator);
      edgeIterator++;
  }
  assert(edgeIterator);
  edgeIterator++;
  if (!edgeIterator.live()) return 0;
  return edgeIterator.curr();
}

static
node *RefToStmt(node *r) {
  if(r==NULL) {
    return(NULL);
  } else {
    if(r==ExitNode) return(r);
    if(r==Entry) return(r);
    Assert(is_mem_ref(r), "RefToStmt: not a reference");
    node * p = r;
    do {
      p = p->nodeparent;
    } while(!is_assign(p) && is_expr(p));
    if (0 && petit_args.ivr_debug && expr_lineno(p) != expr_lineno(r)) {
	fprintf(debug,"Unusual expr -> stmt translation\n");
        node *q = r;
        do {
	  fprintf(debug,"%d: %s\n",expr_lineno(q),access_as_string(q));
          q = q->nodeparent;
        } while(!is_assign(q) && is_expr(q));
	fprintf(debug,"%d: %s\n",expr_lineno(q),access_as_string(q));
        assert(q == p);
        };
    return(p);
  }
}

static
void DCaddToScc(node *p) {
  if(petit_args.ivr_debug) {
    fprintf(debug, "+++DepCycle: stmt %p: %s \n", p, access_as_string(p));
  }
  Assert(dep_scc_no<maxDepSccNo, "DCaddToScc: too many nodes");
  dep_scc[dep_scc_no++] = p;
}

static
void DCprocessScc(node * /*p*/) { // p unused
  if (petit_args.ivr_debug || print_now_or_never && petit_args.print_SCC) {
    int i,j;
    if (petit_args.ivr_debug || dep_scc_no > 1)  {
      for(i=0; i<dep_scc_no; i++) 
        for(j=0; j<dep_scc_no; j++) 
          if ( dep_scc[i]->nodesequence <  dep_scc[j]->nodesequence) {
		int t;
		t = dep_scc[i]->nodesequence;
		dep_scc[i]->nodesequence = dep_scc[j]->nodesequence;
		dep_scc[j]->nodesequence = t;
		}
		
		
      fprintf(debug, "SCC: \n");
      for(i=0; i<dep_scc_no; i++) {
        fprintf(debug, "   (%p) %s", dep_scc[i], access_as_string(dep_scc[i]));
      }
      fprintf(debug, "\n");
    }
  }
  dep_scc_no = 0;
}

static
void DCsetDepFlag(node *p) {
  if(is_assign(p)) {
    ddnode *dd;
    Assert(is_assign(p), "DCgoDown: not assignment statement");
  for(dd=p->nodechild->nodenext->nodeddout; dd!=NULL; dd=dd->ddnextsucc) {
    if (dd->ddtype != ddreduce &&
	(!valueBasedCycles || (!ddisDead(dd)  && dd->ddtype==ddflow)))
	{
	node *s = RefToStmt(dd->ddsucc);
	if (is_assign(s) && scc_region(s) == scc_region(p))  {
	  dd->ddflags |= valueBasedCycles?ddisInFlowCycle:ddisInCycle;
	  if(petit_args.ivr_debug) 
	   fprintf(debug,"marking edge %s\n",brdd_print(dd));
	  }
	};
    };
  if (!valueBasedCycles) 
   for(dd=p->nodechild->nodenext->nodeddin; dd!=NULL; dd=dd->ddnextpred) {
    if (dd->ddtype != ddreduce)
	{
	node *s = RefToStmt(dd->ddpred);
	if (is_assign(s) && scc_region(s) == scc_region(p)) 
	  dd->ddflags |= ddisInCycle;
	};
      }
    }
  }

static
void ResetDepFlag(node *p)
{
    if(is_assign(p)) {
	ddnode *dd;
	Assert(is_assign(p), "not assignment statement");
	for(dd=p->nodechild->nodenext->nodeddout; dd; dd=dd->ddnextsucc) {
	    if (valueBasedCycles) 
		dd->ddflags &= ~ddisInFlowCycle;
	    else dd->ddflags &= ~ddisInCycle;
	}
    }
}

void reset_flow_cycle_bits(bool valueBased)
{
    valueBasedCycles = valueBased;
    traverse_stmts(Entry, &ResetDepFlag);
}


static
void DDinvert(node *p) {
  if(is_mem_ref(p)) {
    ddnode *dd;
    for(dd=p->nodeddout; dd; dd=dd->ddnextsucc) {
      int i;
      for(i=1; i<=(int)dd->ddnest; i++) {
	if(dd->loop_dir[i]<0 && (unsigned)dd->dddiff[i]!=ddunknown) {
	  dd->dddiff[i] = -dd->dddiff[i];
	}
      }
    }
  }
}

void InvertDDdistances() {
    traverse_exprs(Entry, &DDinvert);
}

} // end of namespace omega
