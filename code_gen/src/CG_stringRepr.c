// $Id: CG_stringRepr.c,v 1.1.1.1 2004/09/13 21:07:47 mstrout Exp $

//*****************************************************************************
// File: CG_stringRepr.C
//
// Purpose:
//     implementation of internal representation of String code generation
//     from Omega
//     
// History:
//     04/17/96 - Lei Zhou - created
//
//*****************************************************************************

#include <code_gen/CG_stringRepr.h>
#include <stdio.h>
#include <iostream>

namespace omega {

CG_stringRepr::CG_stringRepr()
{
}


CG_stringRepr::~CG_stringRepr()
{
   
}

CG_stringRepr::CG_stringRepr(const String& _s) : s(_s)
{
}


//-----------------------------------------------------------------------------
// basic operation
//-----------------------------------------------------------------------------
String CG_stringRepr::GetString() const
{ 
  return s;
}




//-----------------------------------------------------------------------------
// Dump operations
//-----------------------------------------------------------------------------
void CG_stringRepr::Dump() const
{
    printf("%s\n",(const char *)s);
}

void CG_stringRepr::DumpToFile(FILE *fp) const
{
  fprintf(fp,"%s", (const char *)s);
}

} // end namespace omega
