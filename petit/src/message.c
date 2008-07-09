/* $Id: message.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#if ! defined BATCH_ONLY_PETIT


/* Handle message window */

#include <string.h>
#include <petit/message.h>
#include <petit/print.h>
#include <petit/debug.h>
#include <petit/Exit.h>
#include <petit/motif.h>

namespace omega {

unsigned int Message_Max = 50;
unsigned int Message_Width = 80;
unsigned int Message_Buffer_Size;

char*	Message_Buffer;
unsigned int	Message_Pos;

int In_Uniform;

void Message_Startup(void)
{
    Message_Pos = 0;
    Message_Buffer_Size = Message_Max * Message_Width;
    Message_Buffer = (char*)calloc(1, Message_Buffer_Size);
    if (!Message_Buffer) out_of_memory();

    Message_Add("Petit version 1.00 3/96 [based on Wolfe's Tiny]" );
}/* Message_Startup */



void Message_Add( char* str )
{
unsigned int i;

    for (i=0; i<strlen(str); i++)
	Message_Buffer[Message_Pos+i] = str[i];
    for (i=strlen(str); i<Message_Width-1; i++)
	Message_Buffer[Message_Pos+i] = ' ';
    Message_Buffer[Message_Pos+Message_Width-1] = '\n';
    Message_Pos = ( Message_Pos + Message_Width ) % Message_Buffer_Size;

    motif_message(str);
}/* Message_Add */


int Message_Dump( int /*always*/ )
{
    motif_list(Message_Buffer, Message_Buffer_Size, Message_Width);
    return 0;
}/* Message_Dump */



void *build_Message_Menu()
    {
    static struct Menu_struc M1[] = {
	{"Xcape",0,0,0,1},
	{"Quit", 0,Quit,0,0},
	{0,      0,0,   0,0 }};

    return Build_Menu( M1, Message_Dump, 0 );
    } /* build_Message_Menu */

} // end omega namespace

#endif
