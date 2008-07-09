#include <assert.h>
#include <code_gen/mmap-codegen.h>
#include <code_gen/mmap-util.h>
#include <code_gen/mmap-sub.h>
#include <code_gen/mmap-checks.h>
#include <basic/Exit.h>

namespace omega {

// converts an integer to a string
static inline String itos(int i)
{
  char digits[64];
  sprintf(digits, "%d", i);
  return digits;
}

// creates a String of length 1 from a character
static inline String ctos(char c)
{
  static char answer[2]=" ";
  answer[0]=c;
  return answer;
}


/*  parenthesize_vars_hack
    Given a C expression involving some variables that may be macro arguments,
    return an expression with parenthesis where they should have been if we
    want all operations in the macro arguments to be evaluated before the
    operations in the macro text.

    We do this by just putting parens around any [_A-Za-z]+[0-9A-Za-z_]+

    This ensures that when
	#define s7(t1,t2,t3,t4,t5,t6,t7)	r2 = all[(t6)-1][(t4)-(t6)]
    is called with
	s7(2,1,1,t4,1,t4-1,1);
    we get (t4)-(t4-1), not t4-t4-1, as the value of the 2nd subscript.
 */

static String parenthesize_vars_hack(const String &expr)
{
  String answer = "";

  bool in_id = false;
  for (int i=0; i<expr.length(); i++)
    {
      // 4 things could happen:
      // stay in id, stay in non-id, switch non-id to id, switch id to non-id
      // if switching, output ( or ), as necessary
      // then unconditionally output the char
      char next = expr[i];
      if (in_id && !(isalnum(next) || next == '_'))
	{
	  in_id = false;
	  answer += ")";
	}
      else if (!in_id && (isalpha(next) || next == '_'))
	{
	  in_id = true;
	  answer += "(";
	}
      answer += ctos(next);
    }
  if (in_id)
    answer+=")";
  return answer;
}



/*  output_vars_as_subscripts:
    Given a single-conjunct relation R,
    produce a list of subscript expressions for the values of each output var.
    Since many of the variables used in these expressions turn out to be
    macro arguments, we call "parenthesize_vars_hack" on the expression.

    Curretly we have to use "get_sub", as defined in mmap-sub.c, since
    the standard "substitution" class does not understand about mod and
    div constraints.  If at some point that is enhanced then get_sub can
    be simplified and inlined here.

    Note that, since there may be infeasable dataflow/memory-map combinations,
    this function may be called with an unsatisfiable relation.
    In this case, return [/ * can't happen * / exit(1)]
 */
static String output_vars_as_subscripts(Relation &R)
{
  int dnf_len = R.query_DNF()->length();
  if (dnf_len>1)
    {
      cerr << "The current tcodegen implementation requires a single-conjunct Relation,\n";
      cerr << "to express output variables in terms of inputs and symbolic consts.\n";
      cerr << "If possible, replace this Relation with several single-conjunct Relations:\n";
      cerr << R.print_with_subs_to_string();
      Exit(1);
    }

  int output = R.n_out();
  String answer = "";

  if (dnf_len == 0)
    {
      assert(!R.is_satisfiable());
      
      for (int o=1; o<=output; o++) 
	answer = answer + "[/* can't happen */ exit(1)]";
    }
  else
    {
      DNF_Iterator di(R.query_DNF());
      Substitutions subs(R, (*di));
      
      for (int o=1; o<=output; o++) 
	answer = answer + "[" + parenthesize_vars_hack(get_sub(R,o,subs)) + "]";
    }

  return answer;
}



/*****************************************************************************/
// given a MMap and DataFlow, arrayref returns the memory location of the dataFlow   
// MMap:       writeIS -> memory
// DataFlow:   writeIS -> readIS
// returns:    readIS -> writeIS -> memory   Inverse(dataFlow) join MMap
// EX:         a[i-1,t-1]
/*****************************************************************************/
static String arrayref(PartialMMap memory, Relation DataFlow) {
  String answer;
  Relation DFcopy = DataFlow;
  for (int tCount=1; tCount<=DFcopy.n_inp(); tCount++) {
    String tx = "t";
    tx += itos(tCount);
    DFcopy.name_input_var(tCount,tx);
    DFcopy.name_output_var(tCount,tx);
  }

  Relation myrelation = Inverse(DFcopy);
  myrelation = Join(myrelation,memory.mapping);

  answer = memory.var + output_vars_as_subscripts(myrelation);

  return answer;
} // arrayref


/*****************************************************************************/
// returns #define sx(t1..tn)\t
/*****************************************************************************/
static String printDefine(int &statementCount, int input)
{
  int i;
  String s="";
  s += "#define s" + itos(statementCount) + "(";    // #define sx(
  statementCount++;
  i=1;
  while (i<=input) {
    s += "t" + itos(i);      // #define sx(tx
    if (i!=input)
      s += ",";         // #define sx(tx,
    i++;
  }
  s += ")\t";        // #define sx(tx,ty)
  if (input==1)
    s += "\t";
  return s;
} // printDefine



/*****************************************************************************/
// Given a Tuple of stm_info, return all the corresponding #defines for each statement

/*****************************************************************************/
static String printDefStmts(Tuple<stm_info> &In)
{
  print_in_code_gen_style++;
  String s="";
  int input = In[1].IS.n_set(), statementCount=1, readFrom;
  // iterate through each stm_info in Tuple
  for (int count=1; count<=In.length(); count++) { 
    assert(!(In[count].map.partials.length() > 0) ||
	   input == In[count].map.partials[1].mapping.n_inp());

    for (int readCount=1; readCount<=In[count].read.length(); readCount++) {
      for (int pCount=1; pCount<=In[count].read[readCount].partials.length(); pCount++) {
	readFrom = In[count].read[readCount].partials[pCount].from;
	for (int wCount=1; wCount<=In[readFrom].map.partials.length(); wCount++) {
	  s += printDefine(statementCount,input);
	  s += "r" + itos(readCount) + " = " + 
	    arrayref(In[readFrom].map.partials[wCount],In[count].read[readCount].partials[pCount].dataFlow) + "\n";
	}
      } // for pCount
    } // for readCount

    // printing w=something
    s += printDefine(statementCount,input);
    s += In[count].stm + "\n";

    // printing var[t1,t2] = w
    // write param info comes from MMap
    for (int wCount=1; wCount<=In[count].map.partials.length(); wCount++) {
      s += printDefine(statementCount,input);  
      s +=In[count].map.partials[wCount].var;     // #define sx(tx,ty) a
      
      // naming variables t1-tn 
      for (int tCount=1; tCount<=input; tCount++) {
	String tx = "t";
	tx += itos(tCount);
	In[count].map.partials[wCount].mapping.name_input_var(tCount,tx);
      }

      s += output_vars_as_subscripts(In[count].map.partials[wCount].mapping);
      s += " = w\n";
    } // for each write tuple

  } // for each statement in Tuple of stm_info
  print_in_code_gen_style--;
  return s;
} // printDefStmts



// Memory maps need to be single conjuncts -
// this just breaks up multi-conjunct memory maps into several partials.

static Tuple<stm_info> expandMemoryMap(const Tuple<stm_info> &info)
{
  // Tuple<stm_info> &noconst = (Tuple<stm_info> &)info;
  
   Tuple<stm_info> NewInfo;
   MMap oldMMap, newMMap;
   PartialMMap tempPartial;
   stm_info *tempStm;
   PartialRead tempParRead;
   Read oldRead, newRead;
   Relation *made_mapping;

   for(int i=1; i<=info.size(); i++)
     {
      tempStm=new stm_info;
      oldMMap=info[i].map;
      newMMap=*(new MMap);
      {   // extra braces apparently avoid Visual C++ bug
      for(int j=1; j<=oldMMap.partials.size(); j++)
	{
	  if (oldMMap.partials[j].mapping.has_single_conjunct() || !(oldMMap.partials[j].mapping.is_satisfiable()))
	     newMMap.partials.append(oldMMap.partials[j]);
	  else
	     for(DNF_Iterator di(oldMMap.partials[j].mapping.query_DNF()); di; di++)
	       {
		tempPartial.bounds=oldMMap.partials[j].bounds;
		tempPartial.var=oldMMap.partials[j].var;
		made_mapping=new Relation(oldMMap.partials[j].mapping,(*di));
		tempPartial.mapping=(*made_mapping);
		newMMap.partials.append(tempPartial);
	       }
	}
      }

      (*tempStm).read.clear();
      for (int j=1; j<=info[i].read.size(); j++)
	{
	 oldRead=info[i].read[j];
	 newRead=*(new Read);
	 for (int k=1; k<=oldRead.partials.size(); k++)
	   {
	    if (oldRead.partials[k].dataFlow.has_single_conjunct() || !(oldMMap.partials[j].mapping.is_satisfiable()))
	       newRead.partials.append(oldRead.partials[k]);
	    else
	       for(DNF_Iterator di(oldRead.partials[k].dataFlow.query_DNF()); di; di++)
		 {
		  tempParRead.from=oldRead.partials[k].from;
		  made_mapping=new Relation(oldRead.partials[k].dataFlow,(*di));
		  tempParRead.dataFlow=(*made_mapping);
		  newRead.partials.append(tempParRead);
		 }
	   }
	 (*tempStm).read.append(newRead);
	}
      (*tempStm).IS=info[i].IS;
      (*tempStm).stm=info[i].stm;
      (*tempStm).map=newMMap;
      NewInfo.append(*tempStm);
     }

   return NewInfo;
}

static Tuple<stm_info> eliminateFalse(const Tuple<stm_info> &info)
{
   Tuple<stm_info> NewInfo=info;
   int count;

   for (int i=1; i<=NewInfo.size(); i++)
     {
      if (!(NewInfo[i].IS.is_satisfiable()))
	{
	 {
	 for (int j=1; j<=NewInfo[i].map.partials.size(); j++)
	    if (NewInfo[i].map.partials[j].mapping.is_satisfiable())
	      cerr<<"A partial write mapping (info["<<i<<"].map.paartials["<<j<<"].mapping) is satisfiable while the iteration-space is not"<<endl;
	 }
	 {
	 for (int j=1; j<=NewInfo[i].read.size(); j++)
	    for (int k=1; k<=NewInfo[i].read[j].partials.size(); k++)
	       if (NewInfo[i].read[j].partials[k].dataFlow.is_satisfiable())
		  cerr<<"A partial read dataFlow (info["<<i<<"].read["<<j<<"].partials["<<k<<"].dataFlow) is satisfiable while the iteration-space is not"<<endl;
	 }
	 {
	 for (int j=i; j<NewInfo.size(); j++)
	    NewInfo[j]=NewInfo[j+1];
	 }
	 NewInfo.delete_last();
	 for (int j=1; j<=NewInfo.size(); j++)
	    for (int k=1; k<=NewInfo[j].read.size(); k++)
	       for (int m=1; m<=NewInfo[j].read[k].partials.size(); m++)
		 {
		  if (NewInfo[j].read[k].partials[m].from==i)
		     assert(NewInfo[j].read[k].partials[m].dataFlow.is_satisfiable() && "mapped to a FALSE iteration space");
		  else if (NewInfo[j].read[k].partials[m].from>i)
		     NewInfo[j].read[k].partials[m].from--;
		 }
	 i--;
	}
      else
	{
	 bool empty=true;
	 {
	 for (int j=1; j<=NewInfo[i].map.partials.size(); j++)
	   {
	    empty=false;
	    if (!(NewInfo[i].map.partials[j].mapping.is_satisfiable()))
	      {
	       for (int k=j; k<NewInfo[i].map.partials.size(); k++)
		  NewInfo[i].map.partials[k]=NewInfo[i].map.partials[k+1];
	       NewInfo[i].map.partials.delete_last();
	       j--;
	      }
	   }
	 }
	 if (NewInfo[i].map.partials.size()==0 && !empty)
	    cerr<<"No satisfiable partial writes left in info["<<i<<"].map.partials"<<endl;
	 count=0;
	 empty=true;
	 for (int j=1; j<=NewInfo[i].read.size(); j++)
	   {
	    empty=true;
	    for (int k=1; k<=NewInfo[i].read[j].partials.size(); k++)
	       if (!(NewInfo[i].read[j].partials[k].dataFlow.is_satisfiable()))
		 {
		  for (int m=k; m<NewInfo[i].read[j].partials.size(); m++)
		     NewInfo[i].read[j].partials[m]=NewInfo[i].read[j].partials[m+1];
		  NewInfo[i].read[j].partials.delete_last();
		  k--;
		 }
	    count+=NewInfo[i].read[j].partials.size();
	   }
	 if (count==0 && !empty)
	    cerr<<"No satisfiable partial reads left in info["<<i<<"].read"<<endl;
	}
     }

   return NewInfo;
}


static void simplify_everything(Tuple<Relation> &t)
{
  for (int i=1; i<=t.length(); i++)
    t[i].simplify();
}


/*****************************************************************************/


/*****************************************************************************/
String tcodegen(int effort, Tuple<stm_info> &info, const Relation &known1, bool check_input)
{
  String result="";
  RelTuple R;
  SetTuple S;
  Relation identity;
  Relation known = known1;
  Tuple<stm_info> new_info;
  bool value;
  int arity = info[1].IS.n_set();

  if (tcodegen_debug)
     DoDebug("Original tcodegen arguments: ",effort,info,known);     

  if (check_input)
    {
      value=checkDataFlow(info) && true; 
      value=checkMemoryMapping(info) && value; 
      value=checkBounds(info) && value;
      value=checkPartials(info) && value;
      value=checkIS(info) && value;
      value=check_arities(info) && value;
      if (known.n_set() != 0 && known.n_set() != arity)
	{
	  cerr << "Inconsistent arity for \"known\": " << known.n_set() <<endl;
	  value = false;
	}
      if (!value)
	return "";
    }

  new_info = expandMemoryMap(info);
  new_info = eliminateFalse(new_info);

  if (known.n_set() == 0)
    known = Extend_Set(known, arity);

  if (tcodegen_debug)
     DoDebug("tcodegen arguments after expandMemoryMap, eliminateFalse, known.Extend_Set: ",
	     effort,new_info,known);

  if (check_input)
    {
      value=checkDataFlow(new_info) && true; 
      value=checkMemoryMapping(new_info) && value; 
      value=checkBounds(new_info) && value;
      value=checkPartials(new_info) && value;
      value=checkIS(new_info) && value;
      value=check_arities(info) && value;
      if (known.n_set() != arity)
	value = false;

      assert(value && "all tcodegen input checks still work after adjustments");
    }

  result += printDefStmts(new_info);

  // creating SetTuple and RelTuple for MMGenerateCode
  // appending the read statements
  for (int count=1; count<=new_info.size(); count++) {
    for (int readCount=1; readCount<=new_info[count].read.size(); readCount++) {
      for (int pCount=1; pCount<=new_info[count].read[readCount].partials.size(); pCount++) {
	int readFrom = new_info[count].read[readCount].partials[pCount].from;
	for (int wCount=1; wCount<=new_info[readFrom].map.partials.length(); wCount++) {
	  R.append(Identity(new_info[count].read[readCount].partials[pCount].dataFlow.n_inp()));
	  Relation wCopy = new_info[readFrom].map.partials[wCount].mapping;
	  Relation DFcopy =new_info[count].read[readCount].partials[pCount].dataFlow;
	  // domain of write
	  // restrict the domain of dataflow to this
	  // then take the range of it all
	  S.append(Range(Restrict_Domain(DFcopy,Domain(wCopy))));
	}
      }
    }

    // appending for write iteration spaces: first append is for w=exp, second is # of partials
    identity = Identity(arity);
    S.append(new_info[count].IS);
    R.append(identity);

    for (int wCount=1; wCount<=new_info[count].map.partials.length(); wCount++) {
      Relation wCopy = new_info[count].map.partials[wCount].mapping;
      S.append(Domain(wCopy));
      R.append(identity);
    }
  }

  if (tcodegen_debug)
    {
      String debug_info="";
      assert(R.size() == S.size());
      for (int a=1; a<=R.size(); a++)
	{
	  debug_info += debug_mark +
	    "Argument " + itoS(a) + " relation and set are\n";
	  debug_info += debug_mark + R[a].print_with_subs_to_string();
	  debug_info += debug_mark + S[a].print_with_subs_to_string();
	  debug_info += debug_mark + "\n";
	}
      debug_info += debug_mark + "Known is "+ known.print_with_subs_to_string() + "\n";
      
      fprintf(DebugFile, "%s", (const char *) debug_info);
      result += debug_info;
    }

  simplify_everything(R);
  simplify_everything(S);
  known.simplify();
  assert(!check_input || known.n_set() == arity);
  assert(!check_input || check_arities(S, arity));
  assert(!check_input || check_arities(R, arity));
  result += MMGenerateCode(R,S,known,effort);
  return result;
}

} // end namespace omega
