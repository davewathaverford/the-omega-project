/* $Id: search.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <basic/util.h>
#include <basic/Dynamic_Array.h>
#include <uniform/uniform_args.h>
#include <uniform/uniform_misc.h>
#include <uniform/search.h>

namespace omega {

Dynamic_Array1<int> best("Stmts");
search_cost best_cost;
search_cost global_cutoff;

static Dynamic_Array1<int> order("Stmt");

static search_cost total_cost;
static int work;

#if defined (SPEED)

ma *edge_cost;
int candidates[max_stmts];
int valid_candidate[max_stmts][max_candidates];

static mb *extra_cost;
static int _select[max_stmts];
static int connected[max_stmts][max_stmts];
static search_cost cheap[max_stmts][max_stmts];

#else

Dynamic_Array4<search_cost> edge_cost("Stmt", "Stmt", "Nest", "Nest");
Dynamic_Array1<int> candidates("Stmts");
Dynamic_Array2<int> valid_candidate("Stmts", "Candidates");

static Dynamic_Array3<search_cost> extra_cost("Stmt", "Stmt", "Nest");
static Dynamic_Array1<int> _select("Stmt");
static Dynamic_Array2<int> connected("Stmt", "Stmt");
static Dynamic_Array2<search_cost> cheap("Stmt", "Stmt");

#endif



typedef struct _costRecord {
	search_cost knownCosts;
	search_cost lowerBound;
	int selection;
	} costRecord;

static int compare_costs(const void *p, const void *q) 
    {
    search_cost c = 
	   (((costRecord *)p)->lowerBound + ((costRecord *)p)->knownCosts) 
	   - (((costRecord *)q)->lowerBound + ((costRecord *)q)->knownCosts);
    if (c < 0) return -1;
    else if (c > 0) return 1;
    else return 0;
    } /* compare_costs */


#if 0
static void waksort(costRecord *cr,int num_decomps)
    {
    for (int i=0; i<num_decomps; i++)
        for (int j=i+1; j<num_decomps; j++)
	    if (compare_costs(&cr[i], &cr[j]) > 0)
		{
		costRecord tmp = cr[i];
		cr[i] = cr[j];
		cr[j] = tmp;
		}
    } /* waksort */
#endif


static void search(int stmt)
    {
    int next_stmt = -1;
    costRecord cr[maxnest+1];
    costRecord cr_best[maxnest+1];

    work++;

    if (stmt == nr_statements)
	{
        for (int j=0; j<nr_statements; j++)
  	    {
            best[j] = _select[j];
	    assert(valid_candidate[j][best[j]]);
	    }
        best_cost = total_cost;
	return;
	}

    if (uniform_args.simple)
	{
	assert(!uniform_args.manual);
        for (int s=0; s<=candidates[stmt]; s++)
	    {
	    if (! valid_candidate[stmt][s]) continue;
    
	    _select[stmt] = s;
	    if (stmt+1 < nr_statements)
	        search(stmt+1);
            else
	        {
	        search_cost cost = 0;
                for (int i=0; i<nr_statements; i++)
		    {
                    for (int j=0; j<=i; j++)
	                cost += edge_cost[i][j][_select[i]][_select[j]];
		    }

                if (uniform_args.trace_uniform)
		    {
                    for (int i=0; i<nr_statements; i++)
			fprintf(uniform_debug, "s%d=%d, ", i, _select[i]);
                    fprintf(uniform_debug, "cost = %f\n", convert_double(cost));
		    }

	        if (cost < best_cost)
                    {
                    for (int j=0; j<nr_statements; j++)
                        best[j] = _select[j];
                    best_cost = cost;
                    fprintf(uniform_debug, "best_cost = %f\n", convert_double(cost));
		    }
	        }
            }
        return;
	}

    assert(total_cost >= 0);

    search_cost bc2 = -MAX_COST;
    int num_decomps_best = -1;
    int p;
    for (p=0; p<nr_statements; p++)
	{
	if (_select[p] >= 0) continue;
	int i;

	int num_decomps = 0;
        for (i = 0; i<=candidates[p]; i++)
	    {
	    if (! valid_candidate[p][i]) continue;

            _select[p] = i;
	    search_cost tc = total_cost;
   
            for (int j=0; j<nr_statements; j++)
		if (_select[j] >= 0)
	            tc += edge_cost[j][p][_select[j]][i];
    
            search_cost mec = 0;
            for (int j1=0; j1<nr_statements; j1++)
		if (_select[j1] < 0 && j1 != p)
		    {
                    search_cost cheapest = MAX_COST;
		    if (connected[j1][p])
                        {
                        for (int cj1=0; cj1<=candidates[j1]; cj1++)
                            {
	                    if (! valid_candidate[j1][cj1]) continue;

                            search_cost x = extra_cost[stmt][j1][cj1] +
				    edge_cost[p][j1][i][cj1];
        
                            if (x < cheapest)
                                cheapest = x;
                            }
                        }
                    else
			cheapest = cheap[stmt][j1];

                    mec += cheapest;
                    assert(mec >= 0);
	            }

   	    cr[num_decomps].lowerBound = mec; 
   	    cr[num_decomps].knownCosts = tc; 
   	    cr[num_decomps].selection = i; 

	    assert(mec < MAX_COST);
	    assert(tc < MAX_COST);

	    if (tc + mec <= best_cost) 
		num_decomps++;

            _select[p] = -1;
	    }

	if (num_decomps == 0) 
	    return;

	qsort(cr,num_decomps,sizeof(costRecord),compare_costs);

        if (num_decomps == 1 || 
	    cr[0].lowerBound+cr[0].knownCosts >= best_cost || 
	    cr[1].lowerBound + cr[1].knownCosts > bc2)
	    {
	    if (num_decomps > 1)
		bc2 = cr[1].lowerBound + cr[1].knownCosts;
	    next_stmt = p;
	    num_decomps_best = num_decomps;
	    for (int ww=0; ww<num_decomps_best; ww++)
	        cr_best[ww] = cr[ww];

            if (num_decomps == 1 || 
		cr[0].lowerBound+cr[0].knownCosts >= best_cost) 
		break;
	    }
        }

    assert(num_decomps_best > 0);

    p = -MAXINT;

    order[stmt] = next_stmt;

    for (int i = 0; i<num_decomps_best; i++)
	{
        _select[next_stmt] = cr_best[i].selection;

        total_cost = cr_best[i].knownCosts;
        assert(cr_best[i].knownCosts >= 0);
        assert(cr_best[i].lowerBound >= 0);

        search_cost min_extra_cost = 0;
        for (int j1=0; j1<nr_statements; j1++)
	    if (_select[j1] < 0)
		{
                search_cost cheapest = MAX_COST;
                for (int cj1=0; cj1<=candidates[j1]; cj1++)
                    {
		    if (! valid_candidate[j1][cj1]) continue;

                    search_cost x = extra_cost[stmt+1][j1][cj1] =
                        extra_cost[stmt][j1][cj1] +
                        edge_cost[next_stmt][j1][_select[next_stmt]][cj1];
     
                    if (x < cheapest)
                        cheapest = x;
                    }

                cheap[stmt+1][j1] = cheapest;
                min_extra_cost += cheapest;
                }

        assert(cr_best[i].lowerBound == min_extra_cost);
        assert(total_cost >= 0);
        assert(min_extra_cost >= 0);

        if (total_cost + min_extra_cost >= best_cost)
	    {
            _select[next_stmt] = -1;
	    continue;
	    }
    
        assert(total_cost >= 0);
	if (stmt+1 < nr_statements)
	    search(stmt+1);
        else
	    {
            if (uniform_args.trace_uniform)
	        {
                for (i=0; i<nr_statements; i++)
		    fprintf(uniform_debug, "s%d=%d, ", i, _select[i]);
                fprintf(uniform_debug,"cost = %f\n",convert_double(total_cost));
	        }

            for (int j=0; j<nr_statements; j++)
		{
                best[j] = _select[j];
		assert(valid_candidate[j][best[j]]);
		}
            best_cost = total_cost;
	    }
        _select[next_stmt] = -1;
        }
    } /* search */




static void gather_node_costs()  
    {
    int i,j,ci,cj;
	
    for (i=nr_statements-1; i>=0; i--) 
    	for (ci=0; ci<=candidates[i]; ci++) 
	    if (valid_candidate[i][ci])
	        for (j=0; j<nr_statements; j++) 
		    if (i!= j && connected[i][j])  
			{
		        search_cost minCost = MAX_COST;
		        for (cj=0; cj<=candidates[j]; cj++) 
			    if (valid_candidate[j][cj]) 
				{
	                        if (minCost > edge_cost[j][i][cj][ci])
	                            minCost = edge_cost[j][i][cj][ci];
		                }
		        if (j<i) 
			    minCost /= 2;
		        if (minCost)  
			    {
		            for (cj=0; cj<=candidates[j]; cj++) 
				if (valid_candidate[j][cj]) 
				    {
	                            edge_cost[j][i][cj][ci] -= minCost;
	                            edge_cost[i][j][ci][cj] -= minCost;
		                    }
		            edge_cost[i][i][ci][ci] += minCost;
		            }
	                }

    for (i=nr_statements-1; i>=0; i--) 
    	for (ci=0; ci<=candidates[i]; ci++) 
	    if (valid_candidate[i][ci])
	        for (j=0; j<nr_statements; j++) 
		    if (i!= j && connected[i][j])  
			{
		        search_cost minCost = MAX_COST;
		        for (cj=0; cj<=candidates[j]; cj++) 
			    if (valid_candidate[j][cj]) 
				{
	                        if (minCost > edge_cost[j][i][cj][ci])
	                            minCost = edge_cost[j][i][cj][ci];
		                }
		        if (minCost)  
			    {
		            for (cj=0; cj<=candidates[j]; cj++) 
				if (valid_candidate[j][cj]) 
				    {
	                            edge_cost[j][i][cj][ci] -= minCost;
	                            edge_cost[i][j][ci][cj] -= minCost;
		                    }
		            edge_cost[i][i][ci][ci] += minCost;
		            }
	                }
    } /* gather_node_costs */



#if 0
static void print_costs() 
    {
    int i,ci,j,cj;

    for (i=0; i<nr_statements; i++) 
        {
        for (ci=0; ci<=candidates[i]; ci++)
	    {
	    fprintf(uniform_debug, "edge_cost[%d][%d][%d][%d] = %f\n",
                    i,i,ci,ci, convert_double(edge_cost[i][i][ci][ci]));
	    }
        }
    
    for (i=0; i<nr_statements; i++)
        for (j=i+1; j<nr_statements; j++)
	    for (ci=0; ci<=candidates[i]; ci++) 
	        if (valid_candidate[i][ci])
	            for (cj=0; cj<=candidates[j]; cj++)
		        if (valid_candidate[j][cj])
			    if (edge_cost[i][j][ci][cj]) 
				{
	                        fprintf(uniform_debug, 
				       "edge_cost[%d][%d][%d][%d] = %f\n",
			               i,j,ci,cj,
				       convert_double(edge_cost[i][j][ci][cj]));
			        }
    } /* print_costs */
#endif


void process_edge_costs()
    {
    int i,j,ci,cj;

    for (i=0; i<nr_statements; i++)
        for (j=0; j<nr_statements; j++)
            connected[i][j] = 0;

    for (i=0; i<nr_statements; i++)
        for (j=i+1; j<nr_statements; j++)
            for (ci=0; ci<=candidates[i]; ci++)
                {
                if (!valid_candidate[i][ci]) continue;

                for (cj=0; cj<=candidates[j]; cj++)
                    {
                    if (!valid_candidate[j][cj]) continue;

                    if (edge_cost[i][j][ci][cj] != 0)
                        connected[i][j] = connected[j][i] = 1;
                    }
                }

    gather_node_costs();

    for (i=0; i<nr_statements; i++)
	for (j=i+1; j<nr_statements; j++)
	    for (ci=0; ci<=candidates[i]; ci++) if (valid_candidate[i][ci]) 
	        for (cj=0; cj<=candidates[j]; cj++) if (valid_candidate[j][cj])  {
		        assert(edge_cost[i][j][ci][cj] < MAX_COST);
			assert (edge_cost[i][j][ci][cj] >= 0);
			assert (edge_cost[i][j][ci][cj] == edge_cost[j][i][cj][ci]);
			}


    int j1;
    for (j1=0; j1<nr_statements; j1++)
        for (int cj1=0; cj1<=candidates[j1]; cj1++)
	    if (valid_candidate[j1][cj1])
		{
                extra_cost[0][j1][cj1] = edge_cost[j1][j1][cj1][cj1];
		assert(edge_cost[j1][j1][cj1][cj1] < MAX_COST);
		}

    for (j1=0; j1<nr_statements; j1++)
        {
        search_cost cheapest = MAX_COST;
        for (int cj1=0; cj1<=candidates[j1]; cj1++)
            {
	    if (! valid_candidate[j1][cj1]) continue;

            search_cost x = extra_cost[0][j1][cj1];

            if (x < cheapest)
                cheapest = x;
            }
        assert(cheapest < MAX_COST);
        cheap[0][j1] = cheapest;
        }
    } /* process_edge_costs */



void perform_search(int aut)
    {
    global_cutoff = 0;
    best.resize(nr_statements);
    order.resize(nr_statements);

#if defined (SPEED)
    extra_cost = new mb[max_stmts];
#else
    connected.resize(nr_statements, nr_statements);
    extra_cost.resize(nr_statements, nr_statements, 2*global_max_nest+1);
    _select.resize(nr_statements);
    cheap.resize(nr_statements, nr_statements);
#endif

    for (int p = 0; p<nr_statements; p++)
        _select[p] = -1;

    total_cost = 0;
    best_cost = MAX_COST;

#if 0
    print_costs();
#endif

    process_edge_costs();

    int done = 0;
    if (uniform_args.manual)
	{
        for (int i=0; i<nr_statements; i++)
	    {
	    if (aut)
		_select[i] = 0;
	    else
		{
	        printf("stmt %2d: ", i);
	        scanf("%d", &_select[i]);
		}
	    if (_select[i] >= 0)
	        {
		done++;
	        assert(_select[i] <=candidates[i] && 
		       valid_candidate[i][_select[i]]);
                }

            for (int j=0; j<nr_statements; j++)
		if (_select[j] >= 0)
	            total_cost += edge_cost[j][i][_select[j]][_select[i]];
	    }

        for (int j1=0; j1<nr_statements; j1++)
	    if (_select[j1] < 0)
		 {
                 search_cost cheapest = MAX_COST;
                 for (int cj1=0; cj1<=candidates[j1]; cj1++)
	             if (valid_candidate[j1][cj1])
			 {
                         search_cost x = extra_cost[0][j1][cj1];
                         for (int i=0; i<nr_statements; i++)
	                     if (_select[i] >= 0)
				x += edge_cost[i][j1][_select[i]][cj1];

			 extra_cost[done][j1][cj1] = x;

                         if (x < cheapest)
                             cheapest = x;
                         }
                 cheap[done][j1] = cheapest;
		 }
	}

    search(done);

#if 0
    for (int i=0; i<nr_statements; i++)
	fprintf(uniform_debug, "best[%d] = %d\n", i, best[i]);
    fprintf(uniform_debug, "Best Cost %f\n", convert_double(best_cost));
#endif

    assert(best_cost < MAX_COST);

    order.resize(0);

#if defined (SPEED)
    delete extra_cost;
#else
    connected.resize(0, 0);
    extra_cost.resize(0, 0, 0);
    _select.resize(0);
    cheap.resize(0, 0);
#endif
    } /* perform_search */

}
