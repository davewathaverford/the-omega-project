/* motif.h,v 1.1.1.2 1992/07/10 02:40:57 davew Exp */
#ifndef Already_Included_Motif
#define Already_Included_Motif

#if ! defined BATCH_ONLY_PETIT

#if defined(_Omega_String_h) && defined(String)
#include <basic/leave_String.h>
#endif

#include <X11/Intrinsic.h>
#undef True
#undef False
#define Motif_True 1
#define Motif_False 0
#include <petit/tree.h>

namespace omega {


typedef void (*refresher)( int );
typedef int (*Handler)( int dummy );

typedef struct Menu_struc{
	char *menu_name;
	void* (*menu_builder)( void );
	Handler menu_prolog;
	Handler menu_epilog;
	int menu_parameter;
	}Menu_struc;

typedef struct mm
    {
    struct mm *parent;
    Handler refresh;
    int num;
    Widget menu_box;
    Menu_struc root[20];
    struct mm *child[20];
    } menu;

extern menu *GLOBAL_MENU;
extern Widget top, form, text, message;
extern XtAppContext app;



menu *Build_Menu(Menu_struc MM[], Handler MMF, unsigned int def);

void motif_init(int argc, char *argv[]);

void motif_fini();

void motif_loop(menu *m);

void motif_menu(menu *m);

void motif_message(char *msg);

void motif_display(char *buffer);

void motif_lowlight(node *n);

void motif_highlight(node *n);

void motif_getword(char *prompt, void (*action_routine)(char *));

void motif_getfile(char *prompt, void (*action_routine)(char *), char *pattern);

void motif_list(char *Message_Buffer, unsigned int Message_Buffer_Size,
		unsigned int Message_Width);

void set_mouse_buttons();

}

#endif


#endif

