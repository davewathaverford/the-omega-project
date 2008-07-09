/* $Id: motif.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#if ! defined BATCH_ONLY_PETIT

#include <Xm/Xm.h>
#include <X11/keysymdef.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/FileSB.h>
#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/MainW.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <ctype.h>
#undef True
#undef False
#include <petit/ops.h>
#include <petit/language.h>
#include <petit/print.h>
#include <petit/browse.h>
#include <petit/browsedd.h>
#include <petit/message.h>
#include <petit/motif.h>
#include <petit/mouse.h>
#include <petit/omega.xbm>

namespace omega {
#define XK_LATIN1       1


Widget top, form, text, message;
XtAppContext app;
menu *GLOBAL_MENU, *GLOBAL_MENU2;


static int Motif_Started = 0;



void motif_fini()
  {
  if (Motif_Started)
      {
      XtUnmanageChild(text);
      }
  }



static void Press1(Widget /*w*/, XEvent */*event*/, String */*params*/,
		   Cardinal */*num_params*/)
  {
  node *n;
  XmTextPosition pos = XmTextGetInsertionPosition(text);

  if (mouse_in_out == 3)
      {
      Message_Add("you are currently browsing transformation preventing dependencies");
      return;
      }

  if (Entry == 0)
      {
      Message_Add("There is no program to browse");
      return;
      }

  for (n = Entry;
       n->noderight != NULL && (int)n->noderight->nodeposition <= (long)pos;
       n = n->noderight);

  Browse_current = n;
  Browse_back = n;

  if (Browse_mode)
      {
      if (BRDD_mode) 
	  brdd_menu(0);
      else
          br_refresh(0);
      }
  else
      {
      motif_menu((menu *)GLOBAL_BROWSE);

      NORMAL_ENTRY = 0;
      br_browse(0);
      }
  }




static void Press2(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {
  node *n;
  XmTextPosition pos = XmTextGetInsertionPosition(text);

  if (mouse_in_out == 3)
      {
      Message_Add("you are currently browsing transformation preventing dependencies");
      return;
      }


  if (Entry == 0)
      {
      Message_Add("There is no program to browse");
      return;
      }


  for (n = Entry;
       n->noderight != NULL && (int)n->noderight->nodeposition <= (long)pos;
       n = n->noderight);
    
  Browse_current = n;
  Browse_back = n;

  if (! Browse_mode)
      {
      motif_menu((menu *)GLOBAL_BROWSE);

      NORMAL_ENTRY = 0;
      br_browse(0);
      }

  if (! BRDD_mode)
      motif_menu((menu *)GLOBAL_BROWSEDD);

  brdd_menu(0);
  mouse_redo_window();
  } /* Press2 */


static void Press3(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {
  mouse_toggle(0);
  } /* Press3 */



typedef struct
    {
    menu *m;
    int num;
    } mydata;

void buttonCB(Widget w,XtPointer client_data,XmPushButtonCallbackStruct *cbs);

void menu_car(char c)
  {
  int num;
  menu *m;
  mydata *data;

  m = GLOBAL_MENU2;

  for(num=0; m->root[num].menu_name!=0; num++) 
      if (toupper(c) == m->root[num].menu_name[0])
	  {
          data = XtNew(mydata);
          data->m = m;
          data->num = num;
	  buttonCB(form, data, NULL);
	  return;
	  }

  printf("\007");
  }


static void Car_a(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('a');}
static void Car_b(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('b');}
static void Car_c(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('c');}
static void Car_d(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('d');}
static void Car_e(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('e');}
static void Car_f(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('f');}
static void Car_g(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('g');}
static void Car_h(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('h');}
static void Car_i(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('i');}
static void Car_j(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('j');}
static void Car_k(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('k');}
static void Car_l(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('l');}
static void Car_m(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('m');}
static void Car_n(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('n');}
static void Car_o(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('o');}
static void Car_p(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('p');}
static void Car_q(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('q');}
static void Car_r(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('r');}
static void Car_s(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('s');}
static void Car_t(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('t');}
static void Car_u(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('u');}
static void Car_v(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('v');}
static void Car_w(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('w');}
static void Car_x(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('x');}
static void Car_y(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('y');}
static void Car_z(Widget /*w*/, XEvent */*event*/, String */*params*/, 
		   Cardinal */*num_params*/)
  {menu_car('z');}


void set_mouse_buttons()
  {
  static char newTranslations1[] = "#override\n\
      <Btn1Down>:	move-destination() press1()\n\
      <Btn2Down>:       move-destination() press2()\n\
      <Btn3Down>:       press3()";
  XtTranslations myTranslations1;

  myTranslations1 = XtParseTranslationTable(newTranslations1);
  XtOverrideTranslations(text, myTranslations1);
  } /* set_mouse_buttons */


static void text_window()
  {
  Arg args[20];
  static XtActionsRec new_actions[] = { {"press1", Press1}, 
					{"press2", Press2},
					{"press3", Press3},
					{"car_a", Car_a},
					{"car_b", Car_b},
					{"car_c", Car_c},
					{"car_d", Car_d},
					{"car_e", Car_e},
					{"car_f", Car_f},
					{"car_g", Car_g},
					{"car_h", Car_h},
					{"car_i", Car_i},
					{"car_j", Car_j},
					{"car_k", Car_k},
					{"car_l", Car_l},
					{"car_m", Car_m},
					{"car_n", Car_n},
					{"car_o", Car_o},
					{"car_p", Car_p},
					{"car_q", Car_q},
					{"car_r", Car_r},
					{"car_s", Car_s},
					{"car_t", Car_t}, 
					{"car_u", Car_u}, 
					{"car_v", Car_v}, 
					{"car_w", Car_w}, 
					{"car_x", Car_x}, 
					{"car_y", Car_y}, 
					{"car_z", Car_z}};

  static char newTranslations2[] = "#override\n\
      <Key>a:		car_a()\n\
      <Key>b:		car_b()\n\
      <Key>c:		car_c()\n\
      <Key>d:		car_d()\n\
      <Key>e:		car_e()\n\
      <Key>f:		car_f()\n\
      <Key>g:		car_g()\n\
      <Key>h:		car_h()\n\
      <Key>i:		car_i()\n\
      <Key>j:		car_j()\n\
      <Key>k:		car_k()\n\
      <Key>l:		car_l()\n\
      <Key>m:		car_m()\n\
      <Key>n:		car_n()\n\
      <Key>o:		car_o()\n\
      <Key>p:		car_p()\n\
      <Key>q:		car_q()\n\
      <Key>r:		car_r()\n\
      <Key>s:		car_s()\n\
      <Key>t:		car_t()\n\
      <Key>u:		car_u()\n\
      <Key>v:		car_v()\n\
      <Key>w:		car_w()\n\
      <Key>x:		car_x()\n\
      <Key>y:		car_y()\n\
      <Key>z:		car_z()";
  XtTranslations myTranslations2;

  XtSetArg(args[0], XmNrows, 30);
  XtSetArg(args[1], XmNcolumns, 80);
  XtSetArg(args[2], XmNeditMode, XmMULTI_LINE_EDIT);
  XtSetArg(args[3], XmNeditable, Motif_False);
  XtSetArg(args[4], XmNcursorPositionVisible, Motif_False);

  text = XmCreateScrolledText(form, "text", args, 5);
  XtManageChild(text);

  XtVaSetValues(XtParent(text),
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		NULL);

  XtAppAddActions(app,new_actions, XtNumber(new_actions));

  set_mouse_buttons();

  myTranslations2 = XtParseTranslationTable(newTranslations2);
  XtOverrideTranslations(text, myTranslations2);
  XtOverrideTranslations(top, myTranslations2);
  }


static void message_window()
  {
  message = XtVaCreateManagedWidget("message", xmTextWidgetClass, form, 
				    XmNeditable, Motif_False,  
                                    XmNcursorPositionVisible, Motif_False,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    NULL);
  }


void configure(Widget shell, XtPointer /*client_data*/, XEvent *event, Boolean */*b*/)
  {
  XConfigureEvent *cevent = (XConfigureEvent *) event;
  if (cevent->type != ConfigureNotify)
      return;
  XtVaSetValues(shell,
		XmNminWidth, cevent->width,
		NULL);
  XtRemoveEventHandler(shell, StructureNotifyMask, Motif_False, configure, NULL);
  }


void motif_init(int argc, char *argv[])
  {
  Pixmap pixmap;

  Motif_Started = 1;

  argc = 1;
  top=XtVaAppInitialize(&app,"Petit",NULL,0,&argc,argv,NULL,NULL);

  XtVaSetValues(top, XmNtitle, "Petit", NULL);
  XtVaSetValues(top, XmNiconName, "Petit", NULL);

  pixmap = XCreatePixmapFromBitmapData(XtDisplay(top),
				       RootWindowOfScreen(XtScreen(top)),
				       omega_bits, omega_width, omega_height,
				       1, 0, 1);

  XtVaSetValues(top,
		XmNiconPixmap,	pixmap,
		XmNiconic,	1,
		NULL);

  XtAddEventHandler(top, StructureNotifyMask, Motif_False, configure, NULL);

  form = XtVaCreateWidget("form",xmFormWidgetClass,top,NULL);

  text_window();
  message_window();
  }


menu *Build_Menu(Menu_struc MM[], Handler MMF, unsigned int /*def*/)
    {
    int num;
    menu *m;

    m = new menu;
    m->parent = NULL;
    m->refresh = MMF;
    for(num=0; MM[num].menu_name!=0; num++) 
	{
        m->root[num] = MM[num];
	if (MM[num].menu_builder != NULL)
	    {
	    m->child[num] = (menu *)(MM[num].menu_builder)();
	    m->child[num]->parent = m;
	    m->child[num]->num = num;
	    }
        }
    m->root[num] = MM[num];
    return m;
    } /* Build_Menu */




static void backtrace(menu *m, int x)
  {
  int val;

  if (m->parent != NULL)
      {
      GLOBAL_MENU2 = m->parent;

      if (m->parent->root[m->num].menu_epilog != NULL)
          val = (m->parent->root[m->num].menu_epilog)(x);
      else
          val = x;
    
      if (val > 0)
          {
          XtUnmanageChild(m->parent->menu_box);
          backtrace(m->parent, val);
          }
      }
  }



static Widget build_menu_row()
  {
  Widget menu_box;

  menu_box = XtVaCreateWidget("menu_box",xmRowColumnWidgetClass,form,
			      XmNpacking, XmPACK_TIGHT,
			      XmNorientation, XmHORIZONTAL,
			      XmNleftAttachment, XmATTACH_FORM,
			      XmNrightAttachment, XmATTACH_FORM,
			      XmNbottomAttachment, XmATTACH_WIDGET,
			      XmNbottomWidget, message,
			      NULL);

  static int first = 1;
  if(first) {
    first = 0;
    XtVaSetValues(XtParent(text),
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, menu_box,
		  NULL);
  }

  return menu_box;
  }

void motif_menu(menu *m)
  {
  int num;
  mydata *data;
  Widget menu_box, button;

  GLOBAL_MENU2 = m;

  menu_box = build_menu_row();
  m->menu_box = menu_box;

  for(num=0; m->root[num].menu_name!=0; num++) 
      {
      if (m->root[num].menu_name[0] != '\0')
          {
          button=XtVaCreateManagedWidget(m->root[num].menu_name, 
					 xmPushButtonWidgetClass, 
					 menu_box, NULL);
          data = XtNew(mydata);
          data->m = m;
          data->num = num;

          XtAddCallback(button,XmNactivateCallback,(XtCallbackProc)buttonCB, 
			data);
          }
      }
  XtManageChild(menu_box);
  XtManageChild(form);
  XtRealizeWidget(top);

  (m->refresh)(1);
  }


void buttonCB(Widget /*w*/, XtPointer client_data, XmPushButtonCallbackStruct */*cbs*/)
  {
  menu *m;
  int num;
  int val;
  mydata *data;

  motif_message("");

  data = (mydata *)client_data;
  m = data->m;
  GLOBAL_MENU = m;
  num = data->num;

  if (m->root[num].menu_builder != NULL)
      {
      val = 1;
      if (m->root[num].menu_prolog != NULL)
          val = (m->root[num].menu_prolog)(m->root[num].menu_parameter);
      if (val)
	  {
	  motif_menu(m->child[num]);
	  if (m->child[num]->refresh != NULL)
	      (m->child[num]->refresh)(0);
	  }
      }
  else
      {
      if (m->root[num].menu_prolog != NULL)
	  {
          val = (m->root[num].menu_prolog)(m->root[num].menu_parameter);
	  if (val != 0)
	      {
              XtUnmanageChild(m->menu_box);
	      backtrace(m, val);
	      }
          else
	      {
	      if (m->refresh != NULL)
		  (m->refresh)(1);
	      }
	  }
      else
	  {
	  val = m->root[num].menu_parameter;
          XtUnmanageChild(m->menu_box);
	  backtrace(m, val);
	  }
      }
  }



void motif_loop(menu *m)
  {
  motif_menu(m);

  XtAppMainLoop(app);
  }



void motif_message(char *msg)
  {
  XmTextSetString(message, msg);
  }



void motif_display(char *buffer)
  {
  XmTextSetString(text, buffer);
  }



static int node_length(node *n)
    {
    char *op_string="";

    if (is_mem_ref(n))
	op_string = ((symtabentry*)((node*)n->nodevalue)->nodevalue)->symname;
    else
        switch(language) 
            {
            case petitlang :
                op_string = get_op_petit_string( n->nodeop );
                break;
            case fortran_alliant :
                op_string = get_op_ftn_string( n->nodeop );
                break;
	    default:
                break;
            }

    return omega::max((int)1, (int)strlen(op_string));
    } /* node_length */



void motif_highlight( node *n)
  {
  int posn, lngth;

  if (n == NULL) return;
  posn = n->nodeposition;
  lngth = node_length(n);
  XmTextSetHighlight(text, posn, posn+lngth, XmHIGHLIGHT_SELECTED);
  XmTextShowPosition(text, posn+lngth);
  }



void motif_lowlight(node *n)
  {
  int posn, lngth;

  if (n == NULL) return;
  posn = n->nodeposition;
  lngth = node_length(n);
  XmTextSetHighlight(text, posn, posn+lngth, XmHIGHLIGHT_NORMAL);
  }



void translateCB(Widget w, XtPointer client_data,
		 XmSelectionBoxCallbackStruct *cbs)
  {
  char *string;
  char empty[1];
  XmString xstring = cbs->value;

  XmStringGetLtoR(xstring, XmSTRING_DEFAULT_CHARSET, &string);
  XtUnmanageChild(w);

  if (string == NULL)
      {
      empty[0] = '\0';
      ((void (*)(char *)) client_data)(empty);
      }
  else
      {
      ((void (*)(char *)) client_data)(string);
      }
  }


void repressedCB(Widget /*w*/, XtPointer /*client_data*/,
		 XmSelectionBoxCallbackStruct */*cbs*/)
  {
  system("/fs/savoir2/ejr/bin/myplay /fs/elan/ejr/junk/sound/represd.au");
  }


void closeCB(Widget w, XtPointer /*client_data*/,
		 XmSelectionBoxCallbackStruct */*cbs*/)
  {
  XtUnmanageChild(w);
  }


void motif_getword(char *prompt, void (*action_routine)(char *))
  {
  Arg args[10];
  Widget dialog;
  XmString t = XmStringCreateSimple(prompt);

  XtSetArg(args[0], XmNselectionLabelString, t);
  XtSetArg(args[1], XmNautoUnmanage, Motif_False);
  dialog = XmCreatePromptDialog(top, "filesb",args,2);
  XmStringFree(t);

  XtVaSetValues(dialog,
		XmNdialogStyle,	XmDIALOG_FULL_APPLICATION_MODAL,
		NULL);

 XtAddCallback(dialog,XmNokCallback,(XtCallbackProc)translateCB,action_routine);
  XtAddCallback(dialog, XmNcancelCallback, (XtCallbackProc)closeCB, NULL);
  XtAddCallback(dialog, XmNhelpCallback, (XtCallbackProc)repressedCB, NULL);
  XtManageChild(dialog);
  }


void motif_getfile(char *prompt, void (*action_routine)(char *), char *pattern)
  {
  Arg args[10];
  Widget dialog, message_line;
  XmString t = XmStringCreateSimple(prompt);
  XmString t2 = XmStringCreateSimple(pattern);

  XtSetArg(args[0], XmNselectionLabelString, t);
  XtSetArg(args[1], XmNautoUnmanage, Motif_False);
  XtSetArg(args[2], XmNpattern, t2);
  dialog = XmCreateFileSelectionDialog(top, "file", args, 3);
  XmStringFree(t);

  XtVaSetValues(dialog,
		XmNdialogStyle,	XmDIALOG_FULL_APPLICATION_MODAL,
		NULL);

  message_line = XmFileSelectionBoxGetChild(dialog, XmDIALOG_TEXT);
  
 XtAddCallback(dialog,XmNokCallback,(XtCallbackProc)translateCB,action_routine);
  XtAddCallback(dialog, XmNcancelCallback, (XtCallbackProc)closeCB, NULL);
  XtAddCallback(dialog, XmNhelpCallback, (XtCallbackProc)repressedCB, NULL);
  XtManageChild(dialog);
  }


void motif_list(char *Message_Buffer, unsigned int /*Message_Buffer_Size*/,
		unsigned int /*Message_Width*/)
  {
  motif_display(Message_Buffer);
  }

} // end omega namespace

#endif
