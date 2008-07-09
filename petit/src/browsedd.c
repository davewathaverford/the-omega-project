/* $Id: browsedd.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <basic/bool.h>
#include <basic/Map.h>
#include <basic/assert.h>
#include <omega/Relations.h>
#include <petit/Zima.h>
#include <petit/tree.h>
#include <petit/print.h>
#include <petit/missing.h>
#include <petit/mouse.h>
#include <petit/depend_filter.h>
#include <petit/motif.h>
#include <petit/message.h>
#include <petit/browse.h>
#include <petit/browsedd.h>
#include <petit/dddriver.h>
#include <petit/debug.h>
#include <petit/ops.h>
#include <petit/Exit.h>
#include <petit/definitions.h>
#include <petit/omega2flags.h>
#include <petit/lang-interf.h>
#include <petit/add-assert.h>
#include <petit/arrayExpand.h>
#include <petit/ddcheck.h>
#include <petit/petit_args.h>
#include <petit/parse_and_analyse.h>

namespace omega {

int BRDD_mode;
int BRDD_show;	/* 2 to show prev, 1 to show next */
ddnode *BRDD_Current, *BRDD_link_start, *BRDD_Tagged;
node *prev_currentdd;
int GLOBAL_SCREEN;
void *GLOBAL_BROWSEDD;

void brdd_add_dir(char *line, unsigned int l, dddirection dddir, int dddiff[],
		       unsigned int nest, signed char *loop_dirs)
{
unsigned int n;
char ch;
dddirection thisdd;

    dddir &= dddirBits;

    if( nest == 0 ){
	line[l] = 0;
	return;
    }
    ch = '(';
    for(n=1; n<=nest; ++n) {
        Assert(loop_dirs[n]==1 || loop_dirs[n]==-1 || loop_dirs[n]==0,
	       "brdd_add_dir: bad loop direction");
	line[l++] = ch;
	ch = ',';
	thisdd = ddextract1(dddir,n);
	if((unsigned)dddiff[n] != ddunknown) {
	    sprintf( &(line[l]), "%d", dddiff[n] );
	    l = strlen( line );
	    if(dddiff[n]!=0 && loop_dirs[n]<0) {
	      line[l++] = '#';
	    }
	} else if(thisdd == ddfwd+ddind+ddbck) {
	    line[l++] = '*';
	} else {
	    if( ddtest1(thisdd,ddind)) line[l++] = '0';
	    if( ddtest1(thisdd,ddfwd)) line[l++] = '+';
	    if( ddtest1(thisdd,ddbck)) line[l++] = '-';
	}
    }
    line[l++] = ')';

    line[l] = 0;
}/* brdd_add_dir */


#define DD_OUTPUT_LINESIZE 256

char * brdd_print(ddnode *ddn)
{

    static char line[DD_OUTPUT_LINESIZE];
    char depend_diff[PETITBUFSIZ];
    char from[PETITBUFSIZ], to[PETITBUFSIZ];
    where old_where;

    old_where = print_where;
    brdd_add_dir(depend_diff, 0, ddn->dddir, ddn->dddiff,
		 ddn->ddnest, ddn->loop_dir);
    print_to_buf( ddn->ddpred, 0 , 0);
    strcpy( from, (const char *)printBuf );
    if (strlen(from) > 15) sprintf(from, "%-12.12s...", from);
    print_to_buf( ddn->ddsucc, 0 , 0);
    strcpy( to, (const char *)printBuf );

    if (strlen(to) > 15) sprintf(to, "%-12.12s...", to);
    sprintf(line, "%-6s %3d: %-15.15s --> %3d: %-15.15s %-15.18s",
	    ddn->ddtype==-1 ? "?" : get_ddtype_name(ddn->ddtype),
	    ddn->ddpred->nodesequence, from, 
	    ddn->ddsucc->nodesequence, to, depend_diff);

    char *i_type = "*";
    if (ddn->dd_relation)
	{
	Relation *ddr = ddn->dd_relation;
	assert( (!(ddn->ddflags & ddIncompFlags)) == ddr->is_exact() );
	if (!ddr->is_exact())
	    {
	    switch (ddr->unknown_uses())
		{
	    case and_u | no_u: // exact v inexact
		i_type = "i";
		break;
	    case and_u:        // all inexact conjuncts
		i_type = "I";
		break;
	    case or_u | no_u:  // exact v unknown
		i_type = "j";
		break;
	    case or_u:         // just "unknown"
		i_type = "J";
		break;
	    default:
		assert(0 && "Bad inexactness information");
		}
	    }
	}

    append_dd_flags(line, ddn->ddflags, i_type);

    assert(strlen(line) < DD_OUTPUT_LINESIZE -1);

    print_where = old_where;
    return line;
} /* brdd_print */


#if ! defined BATCH_ONLY_PETIT

/* Returns the current dd link */
/* wak@cs.umd.edu 2/16/92 */
node *brdd_current(void)
{
    if ( BRDD_mode && BRDD_Current != NULL )
        return mouse_in_out == 2 ? BRDD_Current->ddpred : BRDD_Current->ddsucc;
    else
        return NULL;
} /* brdd_current */

void brdd_display(void)
/* move to 'n' as with br_move.
   then also highlight the dd link */
{
    motif_lowlight(prev_current);
    motif_lowlight(prev_currentdd);

    motif_highlight(br_current());
    motif_highlight(brdd_current());
    prev_current = br_current();
    prev_currentdd = brdd_current();
}/* brdd_display */

node *Browsedd_Most_Recent;
ddnode *Browsedd_Recent_Succ;

int brdd_refresh( int always )
{

    if( always )
	print_petit( Entry, 1 );

    brdd_display();
    mouse_refresh_window();

    Browsedd_Most_Recent = Browse_current;
    Browsedd_Recent_Succ = BRDD_Current;
    return 0;
}/* brdd_refresh */


void brdd_move( node* source, ddnode* dest )
{
    if( source != Browse_current ) Browse_back = Browse_current;
    Browse_current = source;
    BRDD_Current = dest;

    if( BRDD_Current == NULL ){
	switch ( BRDD_show ){
  	  case 1:
  	  case 3:
	    if( Browse_current->nodeddout == NULL ){
	        Message_Add( "No DD successors." );
	    }else{
	        Message_Add( "No more DD successors." );
	      }
	    break;
	  case 2:
	    if( Browse_current->nodeddin == NULL ){
	        Message_Add( "No DD predecessors." );
	    }else{
	        Message_Add( "No more DD predecessors." );
	    }
	    break;
	  case 4:
	    /* should probably use mouse_loop_dd_start instead */
	    if (BRDD_link_start == NULL){
	      Message_Add( "No loop-carried dependences." );
	    }else{
	      Message_Add( "No more loop-carried dependences." );
	    }
	    break;
	  }

	return;
    }

    Message_Add( brdd_print(BRDD_Current) );
}/* brdd_move */


int brdd_goto( int dummy )
{
    dummy = dummy; /* dead code */
    if( BRDD_Current != NULL )
      switch (mouse_in_out) {
        case 1:
 	  brdd_move(BRDD_Current->ddsucc, mouse_out_dd(BRDD_Current->ddsucc));
	  break;
	case 2:
	  brdd_move(BRDD_Current->ddpred, mouse_in_dd(BRDD_Current->ddpred));
	  break;
	case 4:
	  mouse_in_out = 1;  BRDD_show = 1;
	  brdd_move(BRDD_Current->ddpred, mouse_out_dd(BRDD_Current->ddpred));
	  break;
	}
    return 0;
}/* brdd_goto */

void brdd_start( node *n )
{
    switch ( BRDD_show ){
      case 1:
	brdd_move( n, mouse_out_dd(n) );
	break;
      case 2:
	brdd_move( n, mouse_in_dd(n) );
	break;
      case 4:
	brdd_move( n, mouse_loop_dd() );
	break;
    }
}/* brdd_start */

int brdd_next( int dummy )
{
    dummy = dummy; /* dead code */
    if( BRDD_Current != NULL ){
        brdd_move( Browse_current, mouse_nextdd() );
    }
    return 0;
}/* brdd_next */


node *brdd_nextvar( node *start ) {
    node *n;
    int more = 1;
    for(n=br_go_to_another(start); more; n = more ? br_go_to_another(n) : n) {
	switch( n->nodeop ){
	CASE_MEMREF:
	case op_entry:
	case op_exit:
	    more = 0;
	    break;
	default:
	    break;
	}
    }
    return n;
}/* brdd_nextvar */


node *brdd_nextloop( node *start ) {
    node *n;
    int more = 1;
    for(n=br_go_to_another(start); more; n = more ? br_go_to_another(n) : n) {
      switch( n->nodeop ){
	case op_do:
	case op_doany:
	  more = 0;
	  break;
        default:
	  break;
	}
    }
    mouse_find_loop_dd(n);
    return n;
}/* brdd_nextloop */


int brdd_findvar( int dummy )
{
    dummy = dummy; /* dead code */
    BRDD_show = 1;
    brdd_start( brdd_nextvar( Browse_current ) );
    return 0;
}/* brdd_findvar */


int brdd_findloop( int dummy )
{
    dummy = dummy; /* dead code */
    BRDD_show = 4;
    brdd_start( brdd_nextloop( Browse_current ) );
    return 0;
}/* brdd_findvar */


int brdd_var_cycle(void)
{
node *n;
ddnode *dd;

  if( mouse_nextdd() != NULL ){
    brdd_move( Browse_current, mouse_nextdd());
    return 0;
  }
  
  for(n=brdd_nextvar(Browse_current); n!=Browse_current; n=brdd_nextvar(n)) {
    if (BRDD_show == 1) {
      dd = mouse_out_dd(n);
      if (dd != NULL){
	brdd_move( n, dd );
	return 0;
      }
    } else {
      dd = mouse_in_dd(n);
      if (dd != NULL){
	brdd_move( n, dd );
	return 0;
      }
    }
  }

  if (BRDD_show == 1) {
    dd = mouse_out_dd(n);
    if (dd != NULL){
      brdd_move( n, dd );
      return 0;
    }
  } else {
    dd = mouse_in_dd(n);
    if (dd != NULL){
      brdd_move( n, dd );
      return 0;
    }
  }
  return 0;
}/* brdd_var_cycle */


int brdd_loop_cycle(void) {
  node *n;
  ddnode *dd;
  
/* see if there is another dd attached to this loop */  
  if( mouse_nextdd() != NULL ){
    brdd_move( Browse_current, mouse_nextdd());
    return 0;
  }
/* if not, see if there is a loop following this one */
  for(n=brdd_nextloop(Browse_current); n!=Browse_current; n=brdd_nextloop(n)) {
      dd = mouse_loop_dd();
      if (dd != NULL){
	brdd_move( n, dd );
	return 0;
      }
    }
/* Otherwise, no next loop -- use this one */

/*    mouse_find_loop_dd(n); also not necessary, since nextloop was just run*/
    dd = mouse_loop_dd(); /* possibly remove if guard */
/*    if (dd != NULL){*/
      brdd_move( n, dd );
      return 0;
/*    }
  return 0;*/
}/* brdd_loop_cycle */


int brdd_cycle(int dummy)
{
  dummy = dummy; /* dead code */

  if (mouse_in_out == 4)
    return brdd_loop_cycle();
  else
    return brdd_var_cycle();
} /* brdd_cycle */


int brdd_list( int w )
{
    BRDD_show = w;
    brdd_start( Browse_current );
    return 0;
}/* brdd_list */

#endif

bool is_level_carried (ddnode *dd) {
for (int i=1; i<=(int)dd->ddnest; i++)
  if (ddextract1(dd->dddir,i) != ddind)
    return true;
return false;
}


void write_graph_header(FILE *f, char *inputfile) {
    fprintf(f,"graph: { title:\"Petit dependence graph for %s\""
	    " layoutalgorithm: maxdepth \n",
	    inputfile);
    fprintf(f,"xmax: 900 ymax:900\n");
    fprintf(f,"classname %d:\"flow\""
	    " classname %d:\"anti\"\n"
	    " classname %d:\"output\""
	    " classname %d:\"reduction\"\n"
	    " display_edge_labels: yes\n",
	    GRAPH_FLOW_CLASS, GRAPH_ANTI_CLASS, GRAPH_OUTPUT_CLASS,
	    GRAPH_REDUCE_CLASS);
}

void write_graph_footer(FILE *f) {
    fprintf(f,"\n}\n");
}

char *graph_edge_attributes(ddnode *ddn, char *buffer) {
/* could also add line style and thickness here */
/*  linestyle: \"%s\"  thickness: \"%s\"",*/

    switch(ddn->ddtype) {
    case ddflow:
	sprintf(buffer, "color: %s class: %d",
		GRAPH_FLOW_COLOR, GRAPH_FLOW_CLASS);
	break;
    case ddanti:
	sprintf(buffer, "color: %s class: %d",
		GRAPH_ANTI_COLOR, GRAPH_ANTI_CLASS);
	break;
    case ddoutput:
	sprintf(buffer, "color: %s class: %d",
		GRAPH_OUTPUT_COLOR, GRAPH_OUTPUT_CLASS);
	break;
    case ddreduce:
	sprintf(buffer, "color: %s class: %d",
		GRAPH_REDUCE_COLOR, GRAPH_REDUCE_CLASS);
	break;
    }
    return buffer;
}

void write_graph_edge(FILE *f, ddnode *ddn,
		      const Map<node*,omega::String> &nodemap) {
    char diffbuf[PETITBUFSIZ],attrbuf[PETITBUFSIZ];

    diffbuf[0] = '\0';
    brdd_add_dir(diffbuf, 0, ddn->dddir, ddn->dddiff,
		 ddn->ddnest, ddn->loop_dir);
    fprintf(f,"edge: { sourcename: \"%s\" "
	    "targetname: \"%s\" %s ",
	    (const char *) nodemap(ddn->ddpred),
	    (const char *) nodemap(ddn->ddsucc),
	    graph_edge_attributes(ddn,attrbuf));
    if (strlen(diffbuf) != 0)
	fprintf(f,"label: \"%s\"",
		diffbuf);
    fprintf(f,"}\n");
} /* write_graph_edge */


void write_graph_nodes(FILE *printout,
		       bool graph_by_statement,
		       Map<node*,omega::String> &nodemap) {
    Tuple<omega::String> seen_labels;
//    Map<node *,omega::String> nodemap(omega::String("nonexistent node"));
    char label[PETITBUFSIZ];
    int vorder=1,horder=1,lastseq=Entry->nodesequence-1;
    

    for(node *n = Entry; ;) {
	switch( n->nodeop ){
	  CASE_MEMREF:
	case op_entry:
	case op_exit:
	    {
		if(graph_by_statement) {
		    sprintf(label,"%d",n->nodesequence);
		    omega::String slabel(label);
		    if (seen_labels.index(slabel) == 0) {
			fprintf(printout,"node: { title: \"%s\" "
				"label: \"%s\""
				" vertical_order: %d"
				" horizontal_order: %d }\n",
				(const char *) slabel, (const char *) slabel,
				++vorder,1);
			seen_labels.append(slabel);
		    }
		    nodemap[n] = slabel;
		} else {
		    print_to_buf( n, 0, 0);
		    sprintf(label,"%d: %s",n->nodesequence,(const char *)printBuf);
		    omega::String slabel(label);
		    while (seen_labels.index(slabel) != 0)
			slabel = slabel + "'";
		    if ((int)n->nodesequence == lastseq) {
			horder++;
		    } else {
			vorder++;
			horder=1;
		    }
		    fprintf(printout,"node: { title: \"%s\" "
			    "label: \"%s\" vertical_order: %d"
				" horizontal_order: %d }\n",
			    (const char *) slabel, (const char *) slabel,
			    vorder,horder);
		    seen_labels.append(slabel);
		    lastseq=n->nodesequence;
		    nodemap[n] = slabel;
		}
	    }
	    break;
	default:
	    break;
	}
	n = br_go_to_another( n );
	if (n == Entry) break;
    }
}

/* This could be done as a default argument to the other version,
   except that to pass it to motif it needs to take just one arg. 
   This is the standard form called for writing dependences not as a 
   graph.  The "unknown" arg isn't used in this situation.
*/
void write_deps(char *filename) {
    write_deps(filename , "unknown", false, false);
}

void write_deps(char *filename , char *inputfile, 
		bool write_as_graph, bool graph_by_statement)
{
    node *n;
    ddnode *dd;
    int scren = GLOBAL_SCREEN;

    if( filename[0] != 0 )
	{
	printout = fopen( filename, "w" );
	if (!printout)
	    {
	    Message_Add("Can't open file");
	    return;
	    }
        } 
    else 
	printout = stdout;

    Assert(printout!=NULL, "brdd_write: bad dependence file name");
    Map<node*,omega::String> nodemap("nonexistent node");
    if (write_as_graph) 
	{  /* print graph header and nodes */
	write_graph_header(printout, inputfile);
	write_graph_nodes(printout,graph_by_statement,nodemap);
        }	    

    for(n = Entry;  n!=NULL && n != ExitNode;  n = br_go_to_another( n )) {
	switch( n->nodeop ){
	  CASE_MEMREF:
	case op_entry:
	    /* print out dependence relations here */
	    for( dd = n->nodeddout; dd != NULL; dd = dd->ddnextsucc )
		{
#if defined Live_RT
		    if (depend_filter(dd))
#else
                      if ((!scren || depend_filter(dd)) &&
			!(petit_args.printNoDeadDeps &&
			  (ddisDead(dd) || dd->ddtype!=ddflow ||
			   dd->ddsucc==ExitNode)))
#endif
		        {
			symtabentry *var = get_nodevalue_sym(get_nodevalue_node((dd->ddpred==Entry)?dd->ddsucc:dd->ddpred));
                        if (scren ||
                            ((!(petit_args.depPrintFilter & ddmem) ||
			      dd_current_is_mem(dd)) &&
			     (!(petit_args.depPrintFilter & ddval) ||
			      dd_current_is_val(dd)) &&
			     (!(petit_args.depPrintFilter & ddIsFlow) ||
			      dd_current_is_flow(dd)) &&
			     (!(petit_args.depPrintFilter & ddIsAnti) ||
			      dd_current_is_anti(dd)) &&
			     (!(petit_args.depPrintFilter & ddIsOutput) ||
			      dd_current_is_output(dd)) &&
			     (!(petit_args.depPrintFilter & ddloopCarried) ||
                              is_level_carried(dd)) &&
			     (!(petit_args.onlyPrintRealArrayDeps) ||
			      (var->symtype == symreal && var->symdims > 0))))
			    {
				if(! write_as_graph) {
				  fprintf(printout, "%s\n", brdd_print( dd ));
				  if (petit_args.print_relations) {
				      (*dd->dd_relation).uncompress();

				      Relation r = *dd->dd_relation;
				      (*dd->dd_relation).compress();
				      r.print_with_subs(printout);
				      if (dd->ddsucc!=ExitNode && dd->ddpred != Entry) {
			    
					a_access access1 = dd->ddpred;
					a_access access2 = dd->ddsucc;
					int d1 = access_depth(access1);
					int d2 = access_depth(access2);
					int common = min(d1,d2);
					Relation d = Project_Sym(Deltas(copy(r),common));
					assert(d.n_set() == common);
				

					Relation it(d1,d2);
					AccessIteration a(access1, &it, Input_Tuple);
					AccessIteration b(access2, &it, Output_Tuple);
					
					F_And *f = it.add_and();
					
					access_in_bounds(f, a);
					access_in_bounds(f, b);
					it.finalize();

					it = Intersection(it,
							  DeltasToRelation(copy(d),r.n_inp(),r.n_out()));
					//fprintf(printout,"Deltas as relation: ");
					//it.print_with_subs(printout);
					

					r = Difference(it,r);
					if (r.is_upper_bound_satisfiable()) {
					  //fprintf(printout,"false edges: ");
					  //r.print_with_subs(printout);

					  r = Project_Sym(Deltas(r,common));
					  //fprintf(printout,"their distances: ");
					  //r.print_with_subs(printout);

					  r = Difference(copy(d),r);

					  fprintf(printout,"may dd: ");  d.print_with_subs(printout);
					  if (r.is_upper_bound_satisfiable())  {
						fprintf(printout,"must dd: "); 
						r.print_with_subs(printout);
						}
					  }
					else {
					    fprintf(printout,"exact dd: ");  
					    d.print_with_subs(printout);
					    }
				      }
				  }
				} else {
				    write_graph_edge(printout,dd,nodemap);
				}
                            }
			}
		    }
	        break;
	    default:
		break;
	    }
        }
	if(write_as_graph) write_graph_footer(printout);
	if (printout != stdout) fclose( printout );
} /* write_deps */

int do_graph(int) {
    char *fname = new char[L_tmpnam];
    tmpnam(fname);
    brdd_graph(fname, NULL, true, true);
    delete fname;
    return 0;
}


int brdd_graph(char *filename, char *inputfile, bool graph_by_statement, bool popup_vcg)
{
    write_deps(filename, inputfile, true, graph_by_statement);
    if (popup_vcg) {
	omega::String call_vcg = omega::String("xvcg ") + filename;
	system(call_vcg);
    }
    return 0;
}  /* brdd_graph */

int brdd_write(int scren) 
{
    GLOBAL_SCREEN = scren;
    if(scren) {
#if ! defined BATCH_ONLY_PETIT
      motif_getfile(" DD File: ", write_deps, "*" );
#else
      assert(0 && "scren should not be set in batch-only mode");
#endif
    } else {
      write_deps( (char *)petit_args.DepWrFile );
    }

    return 0;
}/* brdd_write */


char *brdd_tally(void)
{
static char s[PETITBUFSIZ];
node *n;
ddnode *dd;
unsigned int tally, filtered;

    tally = 0;
    filtered = 0;
    for(n = Entry; n != ExitNode; n = br_go_to_another( n )) {
        switch( n->nodeop ){
	    /* print out dependence relations here */
	CASE_MEMREF:
	case op_entry:
	    for( dd = n->nodeddout; dd != NULL; dd = dd->ddnextsucc )
		if (depend_filter(dd))
		    tally++;
                else
		    filtered++;
            break;
	default:
	    break;
        }
    }
    sprintf(s,"%d dependences, %d others filtered", tally, filtered);
    return s;
}/* brdd_tally */


#if ! defined BATCH_ONLY_PETIT

int brdd_zap( int )
{
    if (!BRDD_Current)
	Message_Add( "No current DD to zap." );
    else if (ddisDead(BRDD_Current))
	Message_Add( "That dependence is already dead");
    else 
	if (try_to_eliminate(BRDD_Current)) 
	    {
	    re_analyse(1);
	    brdd_refresh(1);
	    brdd_start(Browse_current);
	    }
/*       BRDD_Current->dddir |= try_to_eliminate(BRDD_Current); */

    return 0;
}/* brdd_zap */


/*
 * Zap storage dependence by array expansion
 */
int brdd_expand( int ) {
  if (!BRDD_Current)
    Message_Add( "No current DD to zap." );
  else if (ddisDead(BRDD_Current))
    Message_Add( "That dependence is already dead");
  else if (BRDD_Current->ddtype!=ddanti && BRDD_Current->ddtype!=ddoutput)
    Message_Add( "Only storage dependencies can be zapped by array expansion");
  else 
    if (ZapExpand(BRDD_Current)) {
      re_analyse(1);
      // brdd_refresh(1);
      brdd_start(Browse_current);
    } else {
      Message_Add( "Can not expand here");
    }
  return 0;
}/* brdd_expand */

int unhighlights( int )
{
    motif_lowlight(brdd_current());
    BRDD_mode = 0;
    return 1;
} /* unhighlights */


void *build_brdd_menu()
    {
    /* browse menu */
    static struct Menu_struc M1[] = {
	{"Cycl",0,brdd_cycle,   0,1},
	{"Loop",0,brdd_findloop,0,1},
	{"Wrte",0,brdd_write,   0,1},
	{"Expd",0,brdd_expand,  0,1},
	{"Zap", 0,brdd_zap,     0,1},
	{"Togg",0,mouse_toggle, 0,0},
	{"Quit",0,Quit,         0,0},
	{"Xcap",0,unhighlights, 0,0},
	{0,     0,0,            0,0}};

    GLOBAL_BROWSEDD = Build_Menu( M1, brdd_refresh, 0 );
    return GLOBAL_BROWSEDD;
    } /* build_brdd_menu */


int brdd_menu( int )
{
    if(Browse_current->nodeop == op_do || Browse_current->nodeop == op_doany )
      {
      mouse_find_loop_dd(Browse_current);
      BRDD_show = 4;
      BRDD_mode = 1;
      }
    else
      {
      BRDD_show = 1;
      if (mouse_out_dd(Browse_current) == NULL)
	  {
          BRDD_show = 2;
	  if (mouse_in_dd(Browse_current) != NULL)
              BRDD_mode = 2;
          else
              BRDD_show = 1;
	  }
      else
          BRDD_mode = 1;
      }

    brdd_start( Browse_current );
    brdd_refresh(0);

    return 1;
}/* brdd_menu */
    

int brdd_menu_epilog( int )
{
    BRDD_mode = 0; 
    mouse_close_window();

    return 0;
}/* brdd_menu_epilog */

#endif

} // end omega namespace
