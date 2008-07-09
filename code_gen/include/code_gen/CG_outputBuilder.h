// $Id: CG_outputBuilder.h,v 1.1.1.1 2004/09/13 21:07:47 mstrout Exp $

//*****************************************************************************
// File: CG_outputBuilder.h
//
// Purpose:
//     definition of Omega code generation interface
//     
// History:
//     04/17/96 - Lei Zhou - created
//
//*****************************************************************************

#ifndef CG_outputBuilder_h
#define CG_outputBuilder_h

#include <code_gen/CG_outputRepr.h>
#include <basic/String.h>

namespace omega {

//*****************************************************************************
// class: CG_outputBuilder
//
// Purpose:
//    abstract base class for Omega codegen
//
//    NOTE: ALL OPERATIONS that may consume any CG_outputRepr
//          type arguments passed as parameters.
//          relying on an operation not to consume its 
//          CG_outputRepr arguments is erroneous.
//     
// History:
//     04/17/96 - Lei Zhou - created
//
//*****************************************************************************
class CG_outputBuilder {
public:
  virtual ~CG_outputBuilder() {};

  //---------------------------------------------------------------------------
  // place holder generation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreatePlaceHolder(int indent, const String &funcName,
					   CG_outputRepr* funcList,bool gen_python) const = 0;

  //---------------------------------------------------------------------------
  // assignment generation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateAssignment(int indent, CG_outputRepr* lhs,
					  CG_outputRepr* rhs) const = 0;

  //---------------------------------------------------------------------------
  // function invocation generation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateInvoke(const String &fname,
				      CG_outputRepr* argList) const = 0;

  //---------------------------------------------------------------------------
  // comment generation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateComment(int indent,
				       const String & commentText) const = 0;

  //---------------------------------------------------------------------------
  // if stmt gen operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateIf(int indent, CG_outputRepr* guardCondition,
				  CG_outputRepr* stmtList) const = 0;

  //---------------------------------------------------------------------------
  // inductive variable generation, to be used in CreateLoop as control
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateInductive(CG_outputRepr* index,
					 CG_outputRepr* lower,
					 CG_outputRepr* upper,
					 CG_outputRepr* step,
					 bool gen_python) const = 0;

  //---------------------------------------------------------------------------
  // loop stmt generation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateLoop(int indent, CG_outputRepr* control,
				    CG_outputRepr* stmtList,bool gen_python) const = 0;

  //---------------------------------------------------------------------------
  // basic parenthesis, interpretation operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateParens(CG_outputRepr* term) const = 0;
 
  //---------------------------------------------------------------------------
  // copy operation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateCopy(CG_outputRepr* orignal) const = 0;

  //---------------------------------------------------------------------------
  // basic operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateInt(int ) const = 0;
  virtual CG_outputRepr* CreateIdent(const String & idStr) const = 0;

  //---------------------------------------------------------------------------
  // binary arithmetic operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreatePlus(CG_outputRepr*, CG_outputRepr*) const = 0;
  virtual CG_outputRepr* CreateMinus(CG_outputRepr*, CG_outputRepr*) const = 0;
  virtual CG_outputRepr* CreateTimes(CG_outputRepr*, CG_outputRepr*) const = 0;
  virtual CG_outputRepr* CreateIntegerDivide(CG_outputRepr*, CG_outputRepr*) const = 0;
  virtual CG_outputRepr* CreateIntegerMod(CG_outputRepr*, CG_outputRepr*) const = 0;

  //---------------------------------------------------------------------------
  // binary logical operation
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateAnd(CG_outputRepr*, CG_outputRepr*) const = 0;

  //---------------------------------------------------------------------------
  // binary relational operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* CreateGE(CG_outputRepr*, CG_outputRepr*) const = 0;
  virtual CG_outputRepr* CreateLE(CG_outputRepr*, CG_outputRepr*) const = 0;
  virtual CG_outputRepr* CreateEQ(CG_outputRepr*, CG_outputRepr*) const = 0;

  //---------------------------------------------------------------------------
  // argument list gen operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* 
  CreateArguList(CG_outputRepr *singleton = CG_REPR_NIL) const = 0;
  virtual CG_outputRepr* 
  ArguListInsertLast(CG_outputRepr* list, CG_outputRepr* node) const = 0;
 
  //---------------------------------------------------------------------------
  // stmt list gen operations
  //---------------------------------------------------------------------------
  virtual CG_outputRepr* 
  CreateStmtList(CG_outputRepr *singleton = CG_REPR_NIL) const = 0;
  virtual CG_outputRepr* 
  StmtListInsertLast(CG_outputRepr* list, CG_outputRepr* node) const = 0;
  virtual CG_outputRepr* 
  StmtListAppend(CG_outputRepr* list1, CG_outputRepr* list2) const = 0;
};

} // end of omega namespace

#endif // CG_outputBuilder_h
