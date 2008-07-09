// $Id: CG_stringBuilder.h,v 1.1.1.1 2004/09/13 21:07:47 mstrout Exp $

//*****************************************************************************
// File: CG_stringBuilder.h
//
// Purpose:
//     definition of interface of String code generation
//     from Omega
//     
// History:
//     04/17/96 - Lei Zhou - created
//
//*****************************************************************************

#ifndef CG_stringBuilder_h
#define CG_stringBuilder_h

// class String;                      // minimal external declaration
// class CG_outputBuilder;                // minimal external declaration
// class CG_outputRepr;            // minimal external declaration

#include <basic/String.h>
#include <code_gen/CG_outputBuilder.h>
#include <code_gen/CG_outputRepr.h>

namespace omega {

//*****************************************************************************
// class: CG_stringBuilder
//
// Purpose:
//    generate Omega String as output.
//     
// History:
//     04/17/96 - Lei Zhou - created
//
//*****************************************************************************
class CG_stringBuilder : public CG_outputBuilder { 
public:
  CG_stringBuilder();
  ~CG_stringBuilder();

  //---------------------------------------------------------------------------
  // place holder generation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreatePlaceHolder(int indent, const String &funcName,
					   CG_outputRepr* funcList,bool gen_python) const;

  //---------------------------------------------------------------------------
  // assignment generation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateAssignment(int indent, CG_outputRepr* lhs,
					  CG_outputRepr* rhs) const;

  //---------------------------------------------------------------------------
  // function invocation generation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateInvoke(const String &fname,
				      CG_outputRepr* argList) const;

  //---------------------------------------------------------------------------
  // comment generation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateComment(int indent, const String &commentText) const;

  //---------------------------------------------------------------------------
  // if stmt gen operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateIf(int indent, CG_outputRepr* guardCondition,
				  CG_outputRepr* stmtList) const;

  //---------------------------------------------------------------------------
  // inductive variable generation, to be used in CreateLoop as control
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateInductive(CG_outputRepr* index,
					 CG_outputRepr* lower,
					 CG_outputRepr* upper,
					 CG_outputRepr* step,
					 bool gen_python) const;

  //---------------------------------------------------------------------------
  // loop stmt generation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateLoop(int indent, CG_outputRepr* control,
				    CG_outputRepr* stmtList,bool gen_python) const;

  //---------------------------------------------------------------------------
  // basic parenthesis, interpretation operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateParens(CG_outputRepr* term) const;
 
  //---------------------------------------------------------------------------
  // copy operation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateCopy(CG_outputRepr* orignal) const;

  //---------------------------------------------------------------------------
  // basic operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateInt(int ) const;
  virtual CG_outputRepr* CreateIdent(const String & idStr) const;

  //---------------------------------------------------------------------------
  // binary arithmetic operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreatePlus(CG_outputRepr*, CG_outputRepr*) const;
  virtual CG_outputRepr* CreateMinus(CG_outputRepr*, CG_outputRepr*) const;
  virtual CG_outputRepr* CreateTimes(CG_outputRepr*, CG_outputRepr*) const;
  virtual CG_outputRepr* CreateIntegerDivide(CG_outputRepr*, CG_outputRepr*) const;
  virtual CG_outputRepr* CreateIntegerMod(CG_outputRepr*, CG_outputRepr*) const;

  //---------------------------------------------------------------------------
  // binary logical operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateAnd(CG_outputRepr*, CG_outputRepr*) const;

  //---------------------------------------------------------------------------
  // binary relational operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateGE(CG_outputRepr*, CG_outputRepr*) const;
  virtual CG_outputRepr* CreateLE(CG_outputRepr*, CG_outputRepr*) const;
  virtual CG_outputRepr* CreateEQ(CG_outputRepr*, CG_outputRepr*) const;
    
  //---------------------------------------------------------------------------
  // argument list gen operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* 
  CreateArguList(CG_outputRepr *singleton = CG_REPR_NIL) const;
  virtual CG_outputRepr* 
  ArguListInsertLast(CG_outputRepr* list, CG_outputRepr* node) const;
 
  //---------------------------------------------------------------------------
  // stmt list gen operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* 
  CreateStmtList(CG_outputRepr *singleton = CG_REPR_NIL) const;
  virtual CG_outputRepr* 
  StmtListInsertLast(CG_outputRepr* list, CG_outputRepr* node) const;
  virtual CG_outputRepr* 
  StmtListAppend(CG_outputRepr* list1, CG_outputRepr* list2) const;
};

} // end of omega namespace

#endif // CG_stringBuilder_h
