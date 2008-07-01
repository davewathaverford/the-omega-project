// $Id: CG_stringRepr.h,v 1.1.1.1 2000/06/29 19:23:29 dwonnaco Exp $

//*****************************************************************************
// File: CG_stringRepr.h
//
// Purpose:
//     definition of internal representation of String code generation
//     from Omega
//     
// History:
//     04/17/96 - Lei Zhou - created
//
//*****************************************************************************

#ifndef CG_stringRepr_h
#define CG_stringRepr_h

#include <stdio.h>
#include <basic/String.h>
#include <code_gen/CG_outputRepr.h>

//*****************************************************************************
// class: CG_stringRepr
//
// Purpose:
//    class of omega String codegen representation.
//     
// History:
//     04/17/96 - Lei Zhou - created
//
//*****************************************************************************
class CG_stringRepr : public CG_outputRepr {
public:
  CG_stringRepr();
  CG_stringRepr(const String& _s);
  virtual ~CG_stringRepr();

  //---------------------------------------------------------------------------
  // basic operation
  //---------------------------------------------------------------------------
  String GetString() const;

  //---------------------------------------------------------------------------
  // Dump operations
  //---------------------------------------------------------------------------
  virtual void Dump() const;
  virtual void DumpToFile(FILE *fp = stderr) const;
private:
  String s;
};



#endif // CG_stringRepr_h
