// $Id: CG_stringBuilder.c,v 1.1.1.1 2004/09/13 21:07:47 mstrout Exp $

//*****************************************************************************
// File: CG_stringBuilder.C
//
// Purpose:
//     implementation of interface of String code generation
//     from Omega
//     
// History:
//     04/17/96 - Lei Zhou - created
//
//*****************************************************************************

#include <assert.h>

#include <basic/String.h>
#include <code_gen/CG_stringBuilder.h>
#include <code_gen/CG_stringRepr.h>

using namespace omega;

//*****************************************************************************
// static function declarations
//*****************************************************************************
static String GetIndentSpaces(int indent);
static String GetString(CG_outputRepr* repr);


//*****************************************************************************
// Class implementation
//*****************************************************************************
CG_stringBuilder::CG_stringBuilder()
{
}


CG_stringBuilder::~CG_stringBuilder()
{
}

//-----------------------------------------------------------------------------
// place holder generation
//-----------------------------------------------------------------------------
CG_outputRepr* 
CG_stringBuilder::CreatePlaceHolder(int indent, const String &funcName,
				    CG_outputRepr* funcList,bool gen_python) const
{
  String listStr = "";

  if ( funcList != CG_REPR_NIL ) {
    listStr = GetString(funcList);
  }

  String fStr(funcName);

  String indentStr = GetIndentSpaces(indent);

  if(gen_python)
    return new CG_stringRepr(indentStr + fStr + "(" + listStr + ")\n");
  else
    return new CG_stringRepr(indentStr + fStr + "(" + listStr + ");\n");
}

//-----------------------------------------------------------------------------
// assignment generation
//-----------------------------------------------------------------------------
CG_outputRepr* CG_stringBuilder::CreateAssignment(int indent, 
						  CG_outputRepr* lhs,
						  CG_outputRepr* rhs) const
{
  if ( lhs == CG_REPR_NIL || rhs == CG_REPR_NIL ) {
    assert(0 && "Missing lhs or rhs");
    return CG_REPR_NIL;
  }

  String lhsStr = GetString(lhs);
  String rhsStr = GetString(rhs);

  String indentStr = GetIndentSpaces(indent);

  return new CG_stringRepr(indentStr + lhsStr + "=" + rhsStr + ";\n");
}

//-----------------------------------------------------------------------------
// function invocation generation
//-----------------------------------------------------------------------------
CG_outputRepr* CG_stringBuilder::CreateInvoke(const String &fname,
					      CG_outputRepr* list) const
{
  String listStr = "";

  if ( list != CG_REPR_NIL ) {
    listStr = GetString(list);
  }

  return new CG_stringRepr(fname + "(" + listStr + ")");
}


//-----------------------------------------------------------------------------
// comment generation
//-----------------------------------------------------------------------------
CG_outputRepr* 
CG_stringBuilder::CreateComment(int indent, const String & commentText) const
{
  if ( commentText == String("") ) {
    return CG_REPR_NIL;
  }

  String indentStr = GetIndentSpaces(indent);

  return new CG_stringRepr(indentStr + "// " + commentText + "\n");
}


//-----------------------------------------------------------------------------
// if stmt gen operations
//-----------------------------------------------------------------------------
CG_outputRepr* CG_stringBuilder::CreateIf(int indent, CG_outputRepr* guardList,
					  CG_outputRepr* stmtList) const
{
  if ( stmtList == CG_REPR_NIL ) {
    delete guardList;
    return CG_REPR_NIL;
  }
  else if ( guardList == CG_REPR_NIL ) {
    return stmtList;
  }

  String guardListStr = GetString(guardList);
  String stmtListStr = GetString(stmtList);

  String indentStr = GetIndentSpaces(indent);
  
  String s = indentStr + "if " + guardListStr + " {\n" 
                               + stmtListStr
           + indentStr + "}\n";

  return new CG_stringRepr(s);
}


//-----------------------------------------------------------------------------
// inductive variable generation, to be used in CreateLoop as control
//-----------------------------------------------------------------------------
CG_outputRepr* CG_stringBuilder::CreateInductive(CG_outputRepr* index,
						 CG_outputRepr* lower,
						 CG_outputRepr* upper,
						 CG_outputRepr* step,
						 bool gen_python) const
{
  if ( index == CG_REPR_NIL || lower == CG_REPR_NIL || upper == CG_REPR_NIL ) {
    assert(0 && "Something wrong in CreateInductive");
    return CG_REPR_NIL;
  }

  String indexStr = GetString(index);
  String lowerStr = GetString(lower);
  String upperStr = GetString(upper);

  String doStr;
  if(gen_python)
  {
    doStr = "for " + indexStr + " in range("+
                    lowerStr + "," + upperStr + "+1";
    if ( step != CG_REPR_NIL )
    {
      String stepStr = GetString(step);
      doStr+=","+stepStr;
    }
    doStr+="):";
  }
  else
  {
     doStr = "for(" + indexStr + " = " + lowerStr + "; " 
                          + indexStr + " <= " + upperStr + "; " 
                          + indexStr;

    if ( step != CG_REPR_NIL ) {
     String stepStr = GetString(step);
     doStr += " += " + stepStr;
     }
    else {
     doStr += "++";
    }

    doStr += ")";
  }

  return new CG_stringRepr(doStr);
}


//-----------------------------------------------------------------------------
// loop stmt generation
//-----------------------------------------------------------------------------
CG_outputRepr* CG_stringBuilder::CreateLoop(int indent, CG_outputRepr* control,
					    CG_outputRepr* stmtList,bool gen_python) const
{
  if ( stmtList == CG_REPR_NIL ) {
    delete control;
    return CG_REPR_NIL;
  }
  else if ( control == CG_REPR_NIL ) {
    assert(0 && "No inductive for this loop");
    return stmtList;
  }

  String ctrlStr = GetString(control);
  String stmtStr = GetString(stmtList);

  String indentStr = GetIndentSpaces(indent);

  String s;
  if(gen_python)
  {
     s = indentStr + ctrlStr + "\n" 
                       + stmtStr;
  }
  else
  {
     s = indentStr + ctrlStr + " {\n" 
                       + stmtStr 
           + indentStr + "}\n";
  }

  return new CG_stringRepr(s);
}


//-----------------------------------------------------------------------------
// basic parenthesis operations
//-----------------------------------------------------------------------------
CG_outputRepr* CG_stringBuilder::CreateParens(CG_outputRepr* list) const
{
  if ( list == CG_REPR_NIL ) {
    return CG_REPR_NIL;
  }

  String listStr = GetString(list);

  return new CG_stringRepr("(" + listStr + ")");
}


//-----------------------------------------------------------------------------
// copy operation
//-----------------------------------------------------------------------------
CG_outputRepr*
CG_stringBuilder::CreateCopy(CG_outputRepr* original) const
{
  if ( original == CG_REPR_NIL ) {
    return CG_REPR_NIL;
  }

  // we can not use static function GetString here, since we 
  // don't want to destroy the original.
  String originalStr = ((CG_stringRepr *) original)->GetString();


  return new CG_stringRepr(originalStr);
}

//-----------------------------------------------------------------------------
// basic int, identifier gen operations
//-----------------------------------------------------------------------------
CG_outputRepr* CG_stringBuilder::CreateInt(int _i) const
{
  String _s = itoS(_i);
  return new CG_stringRepr(_s);
}


CG_outputRepr* CG_stringBuilder::CreateIdent(const String &_s) const
{
  if ( &_s == NULL || _s == String("") ) {
    return CG_REPR_NIL;
  }

  return new CG_stringRepr(_s);
}


//-----------------------------------------------------------------------------
// binary arithmetic operations
//-----------------------------------------------------------------------------
CG_outputRepr* 
CG_stringBuilder::CreatePlus(CG_outputRepr* lop, CG_outputRepr* rop) const
{
  if ( rop == CG_REPR_NIL ) {
    return lop;
  }
  else if ( lop == CG_REPR_NIL ) {
    return rop;
  }

  String lopStr = GetString(lop);
  String ropStr = GetString(rop);

  return new CG_stringRepr(lopStr + "+" + ropStr);
}


CG_outputRepr* 
CG_stringBuilder::CreateMinus(CG_outputRepr* lop, CG_outputRepr* rop) const
{
  if ( rop == CG_REPR_NIL ) {
    return lop;
  }
  else if ( lop == CG_REPR_NIL ) {
    String ropStr = GetString(rop);
    return new CG_stringRepr("-" + ropStr);
  }

  String lopStr = GetString(lop);
  String ropStr = GetString(rop);

  return new CG_stringRepr(lopStr + "-" + ropStr);
}


CG_outputRepr* 
CG_stringBuilder::CreateTimes(CG_outputRepr* lop, CG_outputRepr* rop) const
{
  if ( rop == CG_REPR_NIL || lop == CG_REPR_NIL ) {
    return CG_REPR_NIL;
  }

  // we need to figure out whether parenthesis is needed.
  // lop has no parens around, but has +, - operations inside
  //  (6+5)*10 / 4, currently no parens
  String lopStr = GetString(lop);
  String ropStr = GetString(rop);

  return new CG_stringRepr(lopStr + "*" + ropStr);
}


CG_outputRepr* 
CG_stringBuilder::CreateIntegerDivide(CG_outputRepr* lop, CG_outputRepr* rop) const
{
  if ( rop == CG_REPR_NIL ) {
    assert(0 && "Should never happen");
    return CG_REPR_NIL;
  }
  else if ( lop == CG_REPR_NIL ) {
    delete rop;
    return CG_REPR_NIL;
  }

  //  (6+5)*10 / 4
  String lopStr = GetString(lop);
  String ropStr = GetString(rop);

  return new CG_stringRepr("intDiv(" + lopStr + "," + ropStr + ")");
}


CG_outputRepr* 
CG_stringBuilder::CreateIntegerMod(CG_outputRepr* lop, CG_outputRepr* rop) const
{
  if ( rop == CG_REPR_NIL || lop == CG_REPR_NIL ) {
    return CG_REPR_NIL;
  }

  String lopStr = GetString(lop);
  String ropStr = GetString(rop);

  return new CG_stringRepr("intMod(" + lopStr + "," + ropStr + ")");
}


//-----------------------------------------------------------------------------
// binary logical operations
//-----------------------------------------------------------------------------
CG_outputRepr* 
CG_stringBuilder::CreateAnd(CG_outputRepr* lop, CG_outputRepr* rop) const
{
  if ( rop == CG_REPR_NIL || lop == CG_REPR_NIL ) {
    return CG_REPR_NIL;
  }

  String lopStr = GetString(lop);
  String ropStr = GetString(rop);

  return new CG_stringRepr(lopStr + " && " + ropStr);
}


//-----------------------------------------------------------------------------
// binary relational operations
//-----------------------------------------------------------------------------
CG_outputRepr* 
CG_stringBuilder::CreateGE(CG_outputRepr* lop, CG_outputRepr* rop) const
{
  if ( rop == CG_REPR_NIL || lop == CG_REPR_NIL ) {
    return CG_REPR_NIL;
  }

  String lopStr = GetString(lop);
  String ropStr = GetString(rop);

  return new CG_stringRepr(lopStr + " >= " + ropStr);
}


CG_outputRepr* 
CG_stringBuilder::CreateLE(CG_outputRepr* lop, CG_outputRepr* rop) const
{
  if ( rop == CG_REPR_NIL || lop == CG_REPR_NIL ) {
    return CG_REPR_NIL;
  }

  String lopStr = GetString(lop);
  String ropStr = GetString(rop);

  return new CG_stringRepr(lopStr + " <= " + ropStr);
}


CG_outputRepr* 
CG_stringBuilder::CreateEQ(CG_outputRepr* lop, CG_outputRepr* rop) const
{
  if ( rop == CG_REPR_NIL || lop == CG_REPR_NIL ) {
    return CG_REPR_NIL;
  }

  String lopStr = GetString(lop);
  String ropStr = GetString(rop);

  return new CG_stringRepr(lopStr + " == " + ropStr);
}

//-----------------------------------------------------------------------------
// argument list gen operations
//-----------------------------------------------------------------------------
CG_outputRepr* CG_stringBuilder::CreateArguList(CG_outputRepr *singleton) const
{
  if ( singleton == CG_REPR_NIL ) {
    return new CG_stringRepr("");
  }

  String listStr = GetString(singleton);

  return new CG_stringRepr(listStr);
}


CG_outputRepr* 
CG_stringBuilder::ArguListInsertLast(CG_outputRepr* list, 
				     CG_outputRepr* node) const
{
  if ( list == CG_REPR_NIL ) {
    assert(0 && "list has not been created yet, can not be inserted a node");
    return CG_REPR_NIL;
  }
  else if ( node == CG_REPR_NIL ) {
    return list;
  }

  String listStr = GetString(list);
  String nodeStr = GetString(node);


  String s;
  if ( listStr != String("") ) {
    s = listStr + ",";
  }
  s += nodeStr;

  return new CG_stringRepr(s);
}


//-----------------------------------------------------------------------------
// stmt list gen operations
//-----------------------------------------------------------------------------
CG_outputRepr* CG_stringBuilder::CreateStmtList(CG_outputRepr *singleton) const
{
  return CreateArguList(singleton);
}


CG_outputRepr* CG_stringBuilder::StmtListInsertLast(CG_outputRepr* list, 
						    CG_outputRepr* node) const
{
  if ( list == CG_REPR_NIL ) {
    assert(0 && "list is empty, can not be inserted a node");
    return CG_REPR_NIL;
  }
  else if ( node == CG_REPR_NIL ) {
    return list;
  }

  String listStr = GetString(list);
  String nodeStr = GetString(node);

  return new CG_stringRepr(listStr + nodeStr);
}


CG_outputRepr* CG_stringBuilder::StmtListAppend(CG_outputRepr* list1, 
						CG_outputRepr* list2) const
{
  if ( list2 == CG_REPR_NIL ) {
    return list1;
  }
  else if ( list1 == CG_REPR_NIL ) {
    return list2;
  }

  String list1Str = GetString(list1);
  String list2Str = GetString(list2);

  return new CG_stringRepr(list1Str + list2Str);
}


//*****************************************************************************
// static function implementations
//*****************************************************************************
static String GetIndentSpaces(int indent)
{
  String indentStr;
  for (int i=1; i<indent; i++) {
    indentStr += "  ";
  }
  return indentStr;
}

static String GetString(CG_outputRepr* repr)
{
  String result = ((CG_stringRepr *) repr)->GetString();
  delete repr;
  return result;
}
