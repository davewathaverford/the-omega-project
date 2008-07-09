#include <omega.h>
#include <omega/Relations.h>
#include <basic/Dynamic_Array.h>
#include <omega/reach.h>

namespace omega {

typedef Dynamic_Array1<Relation> Rel_Array1;
typedef Dynamic_Array2<Relation> Rel_Array2;

// This is from parallelism.c, modified

static void closure_rel(Rel_Array2 &trans, int i, int k, int j)
    {
    Relation tik;

    if (trans[k][k].is_upper_bound_satisfiable())
        {
        Relation tkk = TransitiveClosure(copy(trans[k][k]));
        tkk.simplify(2,4);
        tik = Composition(tkk, copy(trans[i][k]));
        tik.simplify(2,4);
        tik = Union(copy(trans[i][k]), tik);
        tik.simplify(2,4);
        }
    else
        {
        tik = trans[i][k];
        }
    Relation fresh;
    Relation tkj = trans[k][j];
    fresh = Composition(tkj, tik);
    fresh.simplify(2,4);
    trans[i][j] = Union(trans[i][j], fresh);
    trans[i][j].simplify(2,4);

#if 0
    fprintf(debug, "%d -> %d -> %d\n", i, k, j);
    trans[i][j].print_with_subs(debug);
#endif
    } /* closure_rel */



static void close_rels(Rel_Array2 &trans,int n_nodes)
    {
    for (int k=1; k<=n_nodes; k++)
        for (int i=1; i<=n_nodes; i++)
            if (trans[i][k].is_upper_bound_satisfiable())
                for (int j=1; j<=n_nodes; j++)
                    if (trans[k][j].is_upper_bound_satisfiable())
	                   closure_rel(trans, i, k, j);
    } /* close_rels */


void dump_rels(Rel_Array2 &a, reachable_information *reachable_info){
  int i,j;
  int n_nodes = reachable_info->node_names.size();
  for(i = 1; i <= n_nodes; i++)
    for(j = 1; j <= n_nodes; j++){
	fprintf(stderr,"t[%s][%s] = ",
		(const char *) (reachable_info->node_names[i]),
		(const char *) (reachable_info->node_names[j]));
	a[i][j].print_with_subs(stderr);
    }
}


void dump_sets(Rel_Array1 &a, reachable_information *reachable_info){
  int i;
  int n_nodes = reachable_info->node_names.size();
  for(i = 1; i <= n_nodes; i++) {
    fprintf(stderr,"r[%s] = ",
		(const char *) (reachable_info->node_names[i]));
    a[i].print_with_subs(stderr);
  }
}



Rel_Array1 *
Reachable_Nodes(reachable_information *reachable_info){

    Tuple<String> &node_names = reachable_info->node_names;
    Tuple<int> &arity = reachable_info->node_arity;
    Rel_Array2 &transitions = reachable_info->transitions;
    Rel_Array1 &start_nodes = reachable_info->start_nodes;

	int n_nodes = node_names.size(),i,j;

#define DUMP_INITIAL 1
#define DUMP_CLOSED 1

	if(DUMP_INITIAL && relation_debug){
	  fprintf(stderr,"Initially:\n");
	  dump_rels(transitions, reachable_info);
	}

	close_rels(transitions,n_nodes);

	if(DUMP_CLOSED && relation_debug){
	  fprintf(stderr,"Closed:\n");
	  dump_rels(transitions, reachable_info);
	}

	Rel_Array1 *finalp = 
	  new Rel_Array1("node");
	Rel_Array1 &final = *finalp;
        final.resize(n_nodes+1);
	for (i=1; i<=n_nodes; i++)
            final[i] = Relation::False(arity[i]);

	for(i = 1; i <= n_nodes; i++)
	  for(j = 1; j <= n_nodes; j++)
	    if(start_nodes[i].is_upper_bound_satisfiable())
		final[j] = Union(final[j],
			   	 Composition(copy(transitions[i][j]),
				 copy(start_nodes[i])));
	return finalp;	
}

static void
compute_initially_reachable(Rel_Array1 &r,
			    Rel_Array1 &start_nodes,
			    Rel_Array2 &,
			    Rel_Array2 &closed,
 			    Rel_Array1 &end_nodes,
			    int n_nodes, Tuple<int> &arity){


  for(int n = 1; n <= n_nodes; n++) r[n] = Relation::False(arity[n]);

  for(int i = 1; i <= n_nodes; i++)
    for(int j = 1; j <= n_nodes; j++)
      r[i] = Union(r[i],
		   Range(Restrict_Domain(
		     Restrict_Range(copy(closed[j][i]),
				    copy(end_nodes[i])),
		     copy(start_nodes[j]))));
}


static bool
iterate(Rel_Array1 &r, Rel_Array2 &,	Rel_Array2 &closed,
	Rel_Array1 &,
	int n_nodes) {
  bool changed;

  changed = false;
  for(int j = 1; j <= n_nodes; j++) {
    for(int i = 1; i <= n_nodes; i++) {
      /* look for additional steps from interesting states */
      Relation new_rj =  Range(Restrict_Domain(copy(closed[i][j]),
					       copy(r[i])));
      if(!Must_Be_Subset(copy(new_rj),copy(r[j]))) {
	r[j] = Union(r[j],new_rj);
	r[j].simplify(2,2);
	changed = true;
      }
    }
  }
  return changed;
}




Rel_Array1 *
I_Reachable_Nodes(reachable_information * reachable_info){
  bool changed;

  Tuple<String> &node_names = reachable_info->node_names;
  int n_nodes = node_names.size();
  Tuple<int> &arity = reachable_info->node_arity;
  Rel_Array2 &transitions = reachable_info->transitions;
  Rel_Array1 &start_nodes = reachable_info->start_nodes;
  Rel_Array2 closed("node number","node number");
  closed.resize(n_nodes+1,n_nodes+1);  // abuse of dynamic arrays
  Rel_Array1 *rp = new Rel_Array1("node number");
  Rel_Array1 &r = *rp;
  r.resize(n_nodes+1);  // abuse of dynamic arrays

  int i,j;

  Rel_Array1 end_nodes("Hi!");
  end_nodes.resize(n_nodes+1); // for future use
  for(int n = 1; n <= n_nodes; n++) end_nodes[n] = Relation::True(arity[n]);

  for(j = 1; j <= n_nodes; j++) {
    closed[j][j] = TransitiveClosure(copy(transitions[j][j]));
    for(i = 1; i <= n_nodes; i++) if (i != j)
      closed[i][j] = transitions[i][j];
  }

  compute_initially_reachable(r,start_nodes,transitions,closed,end_nodes,
			      n_nodes,arity);

#define DUMP_INITIAL 1
#define DUMP_CLOSED 1

	if(DUMP_INITIAL && relation_debug > 1){
	  fprintf(stderr,"Closed:\n");
	  dump_rels(closed, reachable_info);
	}
	if(DUMP_INITIAL && relation_debug){
	  fprintf(stderr,"start nodes:\n");
	  dump_sets(start_nodes, reachable_info);
	  fprintf(stderr,"Initially reachable:\n");
	  dump_sets(r, reachable_info);
	}

  changed = true;
  int iterations = 0, max_iterations = 1000;
  while(changed && iterations < max_iterations) {
    changed = iterate(r,transitions,closed,start_nodes,n_nodes);
    iterations++;
  }
  if(relation_debug)
    fprintf(stdout,"[Iterations to convergence: %d]\n",iterations);
  return rp;
}
} // end of namespace omeganamespace omega {
