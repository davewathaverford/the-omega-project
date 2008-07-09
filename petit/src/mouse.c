/* $Id: mouse.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

int mouse_in_out;  

#if ! defined BATCH_ONLY_PETIT

#include <basic/bool.h>
#include <stdio.h>
#include <petit/mouse.h>
#include <petit/depend_filter.h>
#include <petit/ops.h>
#include <petit/browse.h>
#include <petit/browsedd.h>
#include <petit/ddcheck.h>
#include <petit/motif.h>
#include <petit/message.h>
#include <petit/add-assert.h>
#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>

namespace omega {


/* The variable mouse_in_out indicates the type of dependences being browsed:*/
/*              1(out), 2(in): These indicate we are browsing dependences    */
/*                             attached to a memory reference                */
/* 3(trans. preventing), 4(loop-carried): These indicate we                  */
/*                             are browsing the link dependences starting at */
/*                             BRDD_link_start                               */
/*                                                                           */


Widget w,w1,w2,w3,w4,w5,w6,w7,w8,w9;
Widget depend_dialog, pane, list_w, rel_text, toggle_box;
int depend_shown = 0;
int show_relation = 1;
ddnode **depend_list;
int middle;

static XmStringTable str_list = NULL;
static int str_length = -1;


ddnode *forward_in_dd( ddnode *from_dd )
    {
    ddnode *dd;

    for (dd = from_dd;
         dd != NULL && !depend_filter(dd);
         dd = dd->ddnextpred);

    return dd;
    } /* forward_in_dd */


ddnode *forward_out_dd( ddnode *from_dd )
    {
    ddnode *dd;

    for (dd = from_dd;
         dd != NULL && !depend_filter(dd);
         dd = dd->ddnextsucc);
    
    return dd;
    } /* forward_out_dd */


ddnode *forward_link_dd( ddnode *from_dd )
    {
    ddnode *dd;

    for (dd = from_dd;
         dd != NULL && !depend_filter(dd);
         dd = dd->ddlink);
    
    return dd;
    } /* forward_link_dd */


ddnode *mouse_in_dd( node *from )
    {
    mouse_in_out = 2;
    return forward_in_dd(from->nodeddin);
    } /* mouse_in_dd */


ddnode *mouse_out_dd( node *from )
    {
    mouse_in_out = 1;
    return forward_out_dd(from->nodeddout);
    } /* mouse_out_dd */


ddnode *mouse_link_dd(void)
{
    mouse_in_out = 3;
    return forward_link_dd(BRDD_link_start);
    } /* mouse_link_dd */


ddnode *mouse_loop_dd(void)
{
    mouse_in_out = 4;
    return forward_link_dd(BRDD_link_start);
    } /* mouse_loop_dd */


ddnode *next_in_dd( ddnode *from_dd )
    {
    if (from_dd != NULL)
        return forward_in_dd(from_dd->ddnextpred);
    else
	return NULL;
    } /* next_in_dd */


ddnode *next_out_dd( ddnode *from_dd )
    {
    if (from_dd != NULL)
        return forward_out_dd(from_dd->ddnextsucc);
    else
	return NULL;
    } /* next_out_dd */


ddnode *next_link_dd( ddnode *from_dd )
    {
    if (from_dd != NULL)
        return forward_link_dd(from_dd->ddlink);
    else
	return NULL;
    } /* next_link_dd */


ddnode *prev_in_dd( ddnode *from_dd )
    {
    ddnode *dd, *valid;

    if (from_dd != Browse_current->nodeddin && from_dd != NULL)
        {
        valid = NULL;
        for (dd = Browse_current->nodeddin;
             dd->ddnextpred != from_dd;
	     dd = dd->ddnextpred)
            if (depend_filter(dd))
                valid = dd;

        if (depend_filter(dd))
            valid = dd;

        return valid;
        }
    else
        return NULL;
    } /* prev_in_dd */



ddnode *prev_out_dd( ddnode *from_dd )
    {
    ddnode *dd, *valid;

    if (from_dd != Browse_current->nodeddout && from_dd != NULL)
        {
	valid = NULL;
	for (dd = Browse_current->nodeddout;
	     dd->ddnextsucc != from_dd;
	     dd = dd->ddnextsucc) 
            if (depend_filter(dd))
		valid = dd;

        if (depend_filter(dd))
	    valid = dd;

        return valid;
	}
    else
	return NULL;
    } /* prev_out_dd */


ddnode *prev_link_dd( ddnode *from_dd )
    {
    ddnode *dd, *valid;

    if (from_dd != BRDD_link_start && from_dd != NULL)
        {
	valid = NULL;
	for (dd = BRDD_link_start;
	     dd->ddlink != from_dd;
	     dd = dd->ddlink) 
            if (depend_filter(dd))
		valid = dd;

        if (depend_filter(dd))
	    valid = dd;

        return valid;
	}
    else
	return NULL;
    } /* prev_link_dd */




ddnode *down_dd( ddnode *from_dd )
    {
    ddnode *dd;

    if (mouse_in_out == 2)
	{
	dd = prev_in_dd(from_dd);

	if (dd != NULL)
	    return dd;
        else
	    {
	    dd = mouse_out_dd(Browse_current);
	    if (dd != NULL)
		{
		mouse_in_out = 1;
		return dd;
		}
            else
		{
		mouse_in_out = 2;
		return NULL;
		}
            }
	}
    else
	return next_out_dd(from_dd);
    } /* down_dd */



ddnode *up_dd( ddnode *from_dd )
    {
    ddnode *dd;

    if (mouse_in_out == 1)
	{
	dd = prev_out_dd(from_dd);

	if (dd != NULL)
	    return dd;
        else
	    {
	    dd = mouse_in_dd(Browse_current);
	    if (dd != NULL)
		{
		mouse_in_out = 2;
		return dd;
		}
            else
		{
		mouse_in_out = 1;
		return NULL;
		}
	    }
	}
    else
	return next_in_dd(from_dd);
    } /* up_dd */




ddnode *mouse_nextdd(void)
{
    switch (mouse_in_out) {
    case 1: 
	if (BRDD_show == 1)
	    return next_out_dd(BRDD_Current);
        else
	    return prev_out_dd(BRDD_Current);
	break;
      case 2:
	if (BRDD_show == 1)
	    return next_in_dd(BRDD_Current);
        else
	    return prev_in_dd(BRDD_Current);
	break;
      case 4:
      default:
	return next_link_dd(BRDD_Current);
	break;
      }
    } /* mouse_nextdd */


ddnode *mouse_nextlinkdd( ddnode *from_dd )
    {
    if (from_dd != NULL)
        return forward_link_dd(from_dd->ddlink);
    else
	return NULL;
    } /* mouse_nextlinkdd */


int mouse_find_loop_dd( node *loop )
    {
    unsigned int loopnest, n;
    dddirection dddir;

    loopnest = loop->nodevalue;
    dddir = 0;
    for( n = 1; n < loopnest; ++n )
	dddirset(dddir,ddind,n);
    dddirset(dddir,ddfwd,loopnest);
    BRDD_link_start = DDcheck_and_return(loop->nodechild->nodechild, loop,
					 dddir, dddir, 0, 0, 0x1100, 1 );
    return ((BRDD_link_start == NULL) ? 0 : 1) ;
    }




void mouse_close_window(void)
{
    if (depend_shown)
	{
        depend_shown = 0;
	for (int i=0; i<str_length; i++)
	    XmStringFree(str_list[i]);
        str_length = -1;
	if (str_list) XtFree((char *)str_list);
	str_list = NULL;
	if (depend_list) 
	    /* delete depend_list; */
	    free((char *) depend_list);
	XtPopdown(depend_dialog);
	}
    } /* mouse_close_window */

 

void list_normal_dd()
{
    int start = -1;
    int old_in_out, top_in_out;
    ddnode *dd, *pdd;

    old_in_out = mouse_in_out;

    pdd = BRDD_Current;
    for (dd = BRDD_Current; dd != NULL; pdd = dd,dd = up_dd(dd));
    top_in_out = mouse_in_out;

    int n = 0;
    dd = pdd;
    mouse_in_out = top_in_out;
    for (; dd != NULL && mouse_in_out == 2; dd = down_dd(dd))
	n++;
    n++;
    for (; dd != NULL && mouse_in_out == 1; dd = down_dd(dd))
	n++;

    for (int i=0; i<str_length; i++)
        XmStringFree(str_list[i]);
    str_length = -1;
    if (str_list) XtFree((char *)str_list);
    str_list = (XmStringTable)XtMalloc((n+10) * sizeof(XmString *));
    if (depend_list) 
	/* delete depend_list; */
        free((char *) depend_list);
     /* depend_list= new (ddnode *)[n+10]; */
     depend_list= (ddnode **) malloc(sizeof(ddnode *) * (n+10));

    str_length = 0;
    dd = pdd;
    mouse_in_out = top_in_out;
    for (; dd != NULL && mouse_in_out == 2; dd = down_dd(dd))
	{
	if (dd == BRDD_Current) start = str_length;
        str_list[str_length] = XmStringCreateLocalized(brdd_print(dd));
	depend_list[str_length] = dd;
	str_length++;
	}

    str_list[str_length] = XmStringCreateLocalized("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	depend_list[str_length] = NULL;
	middle = str_length;
    str_length++;

    for (; dd != NULL && mouse_in_out == 1; dd = down_dd(dd))
	{
	if (dd == BRDD_Current) start = str_length;
        str_list[str_length] = XmStringCreateLocalized(brdd_print(dd));
	depend_list[str_length] = dd;
	str_length++;
	}
  
    mouse_in_out = old_in_out;

    XtVaSetValues(list_w,
                  XmNitems,               str_list,
                  XmNitemCount,           str_length,
		  XmNselectionPolicy,     XmSINGLE_SELECT,
                  NULL);

    if (start >= 0)
	{
        XmListSetPos(list_w, start+1);
        XmListSelectPos(list_w, start+1, 0);
	}
    } /* list_normal_dd */




void list_link_dd(void)
    {
    int start = -1;
    ddnode *dd;

    int n = 0;
    for (dd = forward_link_dd(BRDD_Current);
         dd != NULL; 
         dd = mouse_nextlinkdd(dd))
        n++;

    for (int i=0; i<str_length; i++)
        XmStringFree(str_list[i]);
    str_length = -1;
    if (str_list) XtFree((char *)str_list);
    str_list = (XmStringTable)XtMalloc((n+10) * sizeof(XmString *));
    if (depend_list) 
	/* delete depend_list; */
	free((char *) depend_list);
    /* depend_list = new (ddnode *)[n+10]; */
    depend_list= (ddnode **) malloc(sizeof(ddnode *) * (n+10));

    str_length = 0;
    for (dd = forward_link_dd(BRDD_Current);
         dd != NULL; 
         dd = mouse_nextlinkdd(dd))
        {
	if (dd == BRDD_Current) start = str_length;
        str_list[str_length] = XmStringCreateLocalized(brdd_print(dd));
	depend_list[str_length] = dd;
	str_length++;
        }

    XtVaSetValues(list_w,
                  XmNitems,               str_list,
                  XmNitemCount,           str_length,
		  XmNselectionPolicy,     XmSINGLE_SELECT,
                  NULL);

    if (start >= 0)
	{
        XmListSetPos(list_w, start+1);
        XmListSelectPos(list_w, start+1, 0);
	}
    } /* list_link_dd */


static void display_relation() {
    if (BRDD_Current) {
	if (BRDD_Current->dd_relation != NULL) {
	    BRDD_Current->dd_relation->uncompress();
	    omega::String s = 
		BRDD_Current->dd_relation->print_with_subs_to_string(false);
	    if (BRDD_Current->ddflags & ddzappable)
		{
		s += "\nExists despite the execution of multiple loop iterations iff:\n" + zap_conditions(BRDD_Current).print_with_subs_to_string(false);
		}

	    XmTextSetString(rel_text, (char *) (const char *) s);
	    BRDD_Current->dd_relation->compress();
	} else {
	    XmTextSetString(rel_text, 
			    "Original Dependence relation not available");
	}
    } else
	XmTextSetString(rel_text, "");
}


static void my_select(Widget /*list*/, XtPointer /*client_data*/, void *cbs)
    {
    ddnode *dd;

    dd = depend_list[((XmListCallbackStruct *)cbs)->item_position-1];
    if (dd != NULL)
	{
	BRDD_Current = dd;
	if (mouse_in_out == 1 || mouse_in_out == 2)
	    {
	    if (((XmListCallbackStruct *)cbs)->item_position > middle)
	        {
	        mouse_in_out = 1;
	        assert(BRDD_Current->ddpred == Browse_current);
	        }
            else
	        {
	        mouse_in_out = 2;
	        assert(BRDD_Current->ddsucc == Browse_current);
	        }
	    }
	Message_Add(brdd_print(BRDD_Current));
	brdd_display();
	display_relation();
	}
    } /* my_select */




static void fill_window()
    {
    switch( Browse_current->nodeop )
        {
        CASE_MEMREF:
        case op_entry:
        case op_exit:
	    if (mouse_in_out == 1 || mouse_in_out == 2)
		list_normal_dd();
            else
                list_link_dd();
	    break;
        case op_do:
 	case op_doany:
            list_link_dd();
            break;
        default:
	    Message_Add("this type of statement does not have dependencies");
	    return;
        }
        display_relation();
    } /* fill_window */




static void set_toggles()
    {
    XmToggleButtonSetState(w1, flow_filter, 0);
    XmToggleButtonSetState(w2, output_filter, 0);
    XmToggleButtonSetState(w3, anti_filter, 0);
    XmToggleButtonSetState(w4, reduce_filter, 0);
    XmToggleButtonSetState(w5, memory_filter, 0);
    XmToggleButtonSetState(w6, value_filter, 0);
    XmToggleButtonSetState(w7, noncarried_filter, 0);
    XmToggleButtonSetState(w8, noncyclic_filter, 0);
    XmToggleButtonSetState(w9, scalar_filter, 0);
    } /* set_toggles */



static void recalculate(void);

static void toggled(Widget /*w*/, void *b, void */*toggle_data*/)
    {
    int bit = (int) b;
    switch(bit)
        {
	case 0:
	    flow_filter = !flow_filter;
	    break;
	case 1:
	    output_filter = !output_filter;
	    break;
	case 2:
	    anti_filter = !anti_filter;
	    break;
	case 3:
	    reduce_filter = !reduce_filter;
	    break;
	case 4:
	    memory_filter = !memory_filter;
	    break;
	case 5:
	    value_filter = !value_filter;
	    break;
	case 6:
	    noncarried_filter = !noncarried_filter;
	    break;
	case 7:
	    noncyclic_filter = !noncyclic_filter;
	    break;
	case 8:
	    scalar_filter = !scalar_filter;
	    break;
        }

    recalculate();
    set_toggles();
    fill_window();
    } /* toggled */





static void create_toggles()
    {
    w  = XtVaCreateManagedWidget("Filters:", 
				 xmLabelWidgetClass,       toggle_box, NULL);
    w1 = XtVaCreateManagedWidget("flow",
				xmToggleButtonGadgetClass, toggle_box, NULL);
    w2 = XtVaCreateManagedWidget("output",
				xmToggleButtonGadgetClass, toggle_box, NULL);
    w3 = XtVaCreateManagedWidget("anti",
				xmToggleButtonGadgetClass, toggle_box, NULL);
    w4 = XtVaCreateManagedWidget("reduce",
				xmToggleButtonGadgetClass, toggle_box, NULL);
    w5 = XtVaCreateManagedWidget("memory",
				xmToggleButtonGadgetClass, toggle_box, NULL);
    w6 = XtVaCreateManagedWidget("value",
				xmToggleButtonGadgetClass, toggle_box, NULL);
    w7 = XtVaCreateManagedWidget("~carry",
				xmToggleButtonGadgetClass, toggle_box, NULL);
    w8 = XtVaCreateManagedWidget("~cyclic",
				xmToggleButtonGadgetClass, toggle_box, NULL);
    w9 = XtVaCreateManagedWidget("scalar",
				xmToggleButtonGadgetClass, toggle_box, NULL);

    XtAddCallback(w1, XmNvalueChangedCallback, toggled, (void *) 0);
    XtAddCallback(w2, XmNvalueChangedCallback, toggled, (void *) 1);
    XtAddCallback(w3, XmNvalueChangedCallback, toggled, (void *) 2);
    XtAddCallback(w4, XmNvalueChangedCallback, toggled, (void *) 3);
    XtAddCallback(w5, XmNvalueChangedCallback, toggled, (void *) 4);
    XtAddCallback(w6, XmNvalueChangedCallback, toggled, (void *) 5);
    XtAddCallback(w7, XmNvalueChangedCallback, toggled, (void *) 6);
    XtAddCallback(w8, XmNvalueChangedCallback, toggled, (void *) 7);
    XtAddCallback(w9, XmNvalueChangedCallback, toggled, (void *) 8);
    } /* create_toggles */





void mouse_refresh_window()
    {
    if (!BRDD_mode)
        depend_shown = 0;

    if (!depend_shown)
        return;

    fill_window();
    } /* mouse_refresh_window */




static void popdown(Widget /*list*/, XtPointer /*client_data*/, void */*cbs*/)
    {
    depend_shown = 0;
    } /* popdown */




static void mouse_draw_window()
    {
    XtWidgetGeometry size;
    Atom WM_DELETE_WINDOW;

    depend_shown = 1;

    depend_dialog = XtVaCreatePopupShell("dependenceWindow",
                                         xmDialogShellWidgetClass, top,
                                         XmNdeleteResponse, XmDESTROY,
                                         NULL);

    WM_DELETE_WINDOW = XmInternAtom(XtDisplay(depend_dialog),
				    "WM_DELETE_WINDOW", 0);

    pane = XtVaCreateWidget("pane",xmPanedWindowWidgetClass,depend_dialog,NULL);
  
    XmAddWMProtocolCallback(depend_dialog, WM_DELETE_WINDOW, popdown, pane);

    toggle_box = XtVaCreateWidget("togglebox",
				  xmRowColumnWidgetClass, 	pane,
                                  XmNpacking, 			XmPACK_COLUMN,
                                  XmNnumColumns,		2,
				  XmNorientation,		XmHORIZONTAL,
				  NULL);
    create_toggles();
    /* Set geometry of toggle box so that it stays the same when 
       the window is resized */
    size.request_mode = CWHeight;
    XtQueryGeometry(toggle_box, NULL, &size);
    XtVaSetValues(toggle_box,
		  XmNpaneMaximum,	size.height,
		  XmNpaneMinimum,	size.height,
		  NULL);
    set_toggles();
    XtManageChild(toggle_box);

    list_w = XmCreateScrolledList(pane, "dependList", NULL, 0);
    XtVaSetValues(list_w, XmNvisibleItemCount, 8, NULL);
    XtAddCallback(list_w, XmNsingleSelectionCallback, my_select, NULL);
    XtManageChild(list_w);

    Arg args[12];
    int n_args=0;
    
    XtSetArg(args[n_args], XmNscrollVertical, Motif_True); n_args++;
    XtSetArg(args[n_args], XmNscrollHorizontal, Motif_False); n_args++;
    XtSetArg(args[n_args], XmNeditMode, XmMULTI_LINE_EDIT); n_args++;
    XtSetArg(args[n_args], XmNeditable, Motif_False); n_args++;
    XtSetArg(args[n_args], XmNcursorPositionVisible, Motif_False); n_args++;
    XtSetArg(args[n_args], XmNwordWrap, Motif_True); n_args++;
    XtSetArg(args[n_args], XmNrows, 1); n_args++;

    rel_text = XmCreateScrolledText(pane, "dependRelation", args, n_args);
    XtManageChild(rel_text);

    /* don't call fill_window until both rel_text and list_w are created */
    fill_window();


    XtManageChild(pane);
    XtPopup(depend_dialog, XtGrabNone);
    } /* mouse_draw_window */




void mouse_redo_window()
    {
    if (depend_shown)
        mouse_refresh_window();
    else
	mouse_draw_window();
    } /* mouse_redo_window */



void recalculate(void)
{
    if (mouse_in_out == 1 || mouse_in_out == 2)
	{
        if (BRDD_Current == NULL || !depend_filter(BRDD_Current))
	    {
            if (mouse_in_out == 1) {
	        BRDD_Current = mouse_out_dd(Browse_current);
                if (BRDD_Current == NULL)
                  BRDD_Current=mouse_in_dd(Browse_current);
            }   
            else {
	        BRDD_Current = mouse_in_dd(Browse_current);
                if (BRDD_Current == NULL)
                  BRDD_Current=mouse_out_dd(Browse_current);
	    }
    
	    brdd_display();
	    display_relation();
    
	    if (BRDD_Current != NULL)
                Message_Add(brdd_print(BRDD_Current));
	    }
        }
    else    /* mouse_in_out == 3 or 4 */
	{
        if (BRDD_Current == NULL || !depend_filter(BRDD_Current))
	    {
	    if (mouse_in_out == 3) 
	      {
	      BRDD_Current = mouse_link_dd();
	      if (BRDD_Current != NULL)
		Browse_current = BRDD_Current->ddpred;
  	      }
  	    else
	      BRDD_Current = mouse_loop_dd();
    
	    brdd_display();
	    display_relation();

	    if (BRDD_Current != NULL)
                Message_Add(brdd_print(BRDD_Current));
            }
	}
    } /* recalculate */



int mouse_toggle( int returncode )
    {
    if (depend_shown)
        mouse_close_window();
    else
        {
        if (BRDD_mode)
	  mouse_draw_window();
        else
          Message_Add("you are not currently browsing dependencies");
        }

    return returncode ;  
    } /* mouse_toggle */

} // end omega namespace

#endif
