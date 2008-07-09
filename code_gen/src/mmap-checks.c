#include <code_gen/mmap-checks.h>
#include <code_gen/mmap-util.h>
#include <stdio.h>
#include <iostream>
#include <omega.h>

namespace omega {

/*****************************************************************************/
// checks the reads and writes to make sure they cover all of the iteration space
// they are calculating it in
// 
/*****************************************************************************/
bool checkIS(Tuple<stm_info> info)
{
  bool value;

  value=true;
  for (int count=1; count<=info.size(); count++) {
    for (int readCount=1; readCount<=info[count].read.size(); readCount++) {
      Relation dFcopy = info[count].read[readCount].partials[1].dataFlow;
      Relation IS = info[count].IS;
      Relation IS2 = IS;
      Relation readTest = Range(dFcopy);
      Relation readTest2 = readTest;
      for (int pCount=1; pCount<=info[count].read[readCount].partials.size(); pCount++) {	
	Relation copy1 = info[count].read[readCount].partials[pCount].dataFlow;
	Relation Rcopy1 = Range(copy1);
	if (pCount!=1) {
	  dFcopy = info[count].read[readCount].partials[pCount].dataFlow;
	  readTest = Union(readTest,Range(dFcopy));
	  readTest2 = readTest;
	}
	// makes sure partials don't intersect
	for (int pNext=pCount+1; pNext<=info[count].read[readCount].partials.size(); pNext++) {
	  Relation copy2 = info[count].read[readCount].partials[pNext].dataFlow;
	  Relation Rcopy2 = Range(copy2);
	  Relation anotherCopy = Rcopy1;
	  if (tcodegen_debug)
	    {
	      fprintf(DebugFile, "%s", debug_mark_cp);
	      anotherCopy.print_with_subs(DebugFile);
	      fprintf(DebugFile, "%s", debug_mark_cp);
	      Rcopy2.print_with_subs(DebugFile);
	    }
	  if ((Intersection(anotherCopy,Rcopy2)).is_satisfiable()) {
	    cerr << "Statement "<<count<<" Read "<<readCount<< "'s partials "<<pCount<<" and "<<pNext<<" intersect"<<endl;
	    value=false;
	  }
	}
      }
      // checks that reads cover the whole iteration space
      if ( ((Difference(readTest,IS)).is_satisfiable()) || 
	   ((Difference(IS2,readTest2)).is_satisfiable()) ) {
	cerr << "Statement "<<count<<" Read "<<readCount<<"'s relations does not describe IS: ";
	info[count].IS.print_with_subs(stderr); 
	value=false;
      }
      
    } 
    // within the statment loop
    if (info[count].map.partials.length() == 0)
      {
	if (copy(info[count].IS).is_satisfiable())
	  {
	    cerr << "statement "<<count<<"'s has no writes, but IS: ";
	    info[count].IS.print_with_subs(stderr);
	    value = false;
	  }
	continue;
      }

    Relation mapCopy = info[count].map.partials[1].mapping;
    Relation writeTest = Domain(mapCopy);
    Relation writeTest2 = writeTest;
    Relation IS = info[count].IS;
    Relation IS2 = IS;
    for (int wCount=1; wCount<=info[count].map.partials.length(); wCount++) {
      Relation copy1 = info[count].map.partials[wCount].mapping;
      Relation Dcopy1 = Domain(copy1);
      if (wCount!=1) {
	mapCopy = info[count].map.partials[wCount].mapping;
	writeTest = Union(writeTest,Domain(mapCopy));
	writeTest2 = writeTest;
      }
      // makes sure the partial writes don't intersect
      for (int wNext=wCount+1; wNext<=info[count].map.partials.length(); wNext++) {
	Relation copy2 = info[count].map.partials[wNext].mapping;
	Relation Dcopy2 = Domain(copy2);
	Relation another = Dcopy1;
	if ((Intersection(another,Dcopy2)).is_satisfiable()) {
	  cerr << "Statement "<<count<<"'s write partials "<<wCount<<" and "<<wNext<<" intersect"<<endl;
	  value=false;
	}
      }
    }
    // makes sure writes cover the entire iteration space
    if ( ((Difference(writeTest,IS)).is_satisfiable()) || 
	 ((Difference(IS2,writeTest2)).is_satisfiable()) ) {
      cerr << "statement "<<count<<"'s write relation does not describe IS: ";
      info[count].IS.print_with_subs(stderr); 
      value=false;
    }
  } // statment loop 
  return value; 
} // checkIS


/*****************************************************************************/
// checks to make sure the partials don't intersect
/*****************************************************************************/
bool checkPartials(Tuple<stm_info> info)
{
  bool value;

  value=true;
  for (int count=1; count<=info.size(); count++) {
    for (int readCount=1; readCount<=info[count].read.size(); readCount++) {
      // for each partial, compare it with the rest and make sure they don't intersect
      for (int pCount=1; pCount<info[count].read[readCount].partials.size(); pCount++) {
	Relation copy1 = info[count].read[readCount].partials[pCount].dataFlow;
	Relation Rcopy1 = Range(copy1);
	for (int pNext=pCount+1; pNext<=info[count].read[readCount].partials.size(); pNext++) {
	  Relation copy2 = info[count].read[readCount].partials[pNext].dataFlow;
	  Relation Rcopy2 = Range(copy2);
	  Relation anotherCopy = Rcopy1;
	  if (tcodegen_debug)
	    {
	      fprintf(DebugFile, "%s", debug_mark_cp);
	      anotherCopy.print_with_subs(DebugFile);
	      fprintf(DebugFile, "%s", debug_mark_cp);
	      Rcopy2.print_with_subs(DebugFile);
	    }
	  if ((Intersection(anotherCopy,Rcopy2)).is_satisfiable()) {
	    cerr << "Statement "<<count<<" Read "<<readCount<< "'s partials "<<pCount<<" and "<<pNext<<" intersect"<<endl;
	    value=false;
	  }
	}
      }
    }
    for (int wCount=1; wCount<=info[count].map.partials.length(); wCount++) {
      Relation copy1 = info[count].map.partials[wCount].mapping;
      Relation Dcopy1 = Domain(copy1);
      for (int wNext=wCount+1; wNext<=info[count].map.partials.length(); wNext++) {
	Relation copy2 = info[count].map.partials[wNext].mapping;
	Relation Dcopy2 = Domain(copy2);
	Relation another = Dcopy1;
	if ((Intersection(another,Dcopy2)).is_satisfiable()) {
	  cerr << "Statement "<<count<<"'s write partials "<<wCount<<" and "<<wNext<<" intersect"<<endl;
	  value=false;
	}
      }
    }
  } // statement loop
  return value;
} // checkPartials


static Relation LexicographicallyForward(int size)
{
  Relation LexFwd(size, size);
  F_Or *topOr=LexFwd.add_or();
  for(int bli=1; bli<=size; bli++)
    {
      F_And *addPiece=(*topOr).add_and();
      for(int blj=1; blj<bli; blj++)
	{
	  EQ_Handle aneq=(*addPiece).add_EQ();
	  aneq.update_coef(LexFwd.input_var(blj),-1);
	  aneq.update_coef(LexFwd.output_var(blj),1);
	}
      GEQ_Handle anineq=(*addPiece).add_GEQ();
      anineq.update_coef(LexFwd.input_var(bli),-1);
      anineq.update_coef(LexFwd.output_var(bli),1);
      anineq.update_const(-1);
    }
  LexFwd.simplify();
  return LexFwd;
}  

bool checkDataFlow(Tuple<stm_info> &info)
{
  Relation LexFwd = LexicographicallyForward(info[1].IS.n_set());
  bool value = true;

  for(int i=1; i<=info.size(); i++)
    for(int j=1; j<=info[i].read.size(); j++)
      for(int k=1; k<=info[i].read[j].partials.size(); k++)
	if (Difference(copy(info[i].read[j].partials[k].dataFlow),
		       copy(LexFwd)).is_satisfiable())
	  {
	    cerr<<"DataFlow goes back in time in statement "<<i<<", in read "<<j<<", in partial read "<<k<<endl;
	    cerr<<"DataFlow relation: "<<info[i].read[j].partials[k].dataFlow;
	    value=false;
	  }
  return value;
}


bool checkMemoryMapping(Tuple<stm_info> info)
{
   int lexsize;
   PartialMMap PMM;
   Tuple<PartialMMap> ArrayMapping;
   bool found, value;
   Tuple<PartialMMap> AfterSameMem;
   PartialMMap tempartialMMap;
   Relation dF;
   bool debugthis=false;

   value=true;
   assert(info[1].IS.is_set());
   lexsize=info[1].IS.n_set();
   {   // extra braces apparently avoid Visual C++ bug
   for(int i=2; i<=info.size(); i++)
      assert(info[i].IS.is_set() && info[i].IS.n_set()==lexsize);
   }

   Relation LexFwd = LexicographicallyForward(lexsize);

   if (debugthis)
      cerr<<"Lexfwd: "<<LexFwd;

   {
   for(int i=1; i<=info.size(); i++)
      for(int j=1; j<=info[i].map.partials.size(); j++)
	{
	 PMM=info[i].map.partials[j];
	 found=false;
	 for(int k=1; k<=ArrayMapping.size() && !found; k++)
	    if (PMM.var==ArrayMapping[k].var)
	      {
	       ArrayMapping[k].mapping=Union(ArrayMapping[k].mapping, copy(PMM.mapping));
	       found=true;
	      }
	 if(!found)
	    ArrayMapping.append(PMM);
	}
   }
 
   if (debugthis)
     {
      for(int i=1; i<=ArrayMapping.size(); i++)
	{ 
	  cerr<<"i= "<<i<<"  ArrayMapping[i].var= "<<ArrayMapping[i].var<<"  ArrayMapping[i].mapping= "<<ArrayMapping[i].mapping;
	}
     }

   {
   for(int i=1; i<=ArrayMapping.size(); i++)
     {
      tempartialMMap.mapping=Join(copy(ArrayMapping[i].mapping),Inverse(copy(ArrayMapping[i].mapping)));
      tempartialMMap.mapping=Intersection(copy(tempartialMMap.mapping),copy(LexFwd));
      tempartialMMap.mapping=Join(copy(tempartialMMap.mapping),copy(LexFwd));
      tempartialMMap.var=ArrayMapping[i].var;
      AfterSameMem.append(tempartialMMap);
     }
   }

   for(int i=1; i<=info.size(); i++)
      for(int j=1; j<=info[i].read.size(); j++)
	 for(int k=1; k<=info[i].read[j].partials.size(); k++)
	   {
	    dF=info[i].read[j].partials[k].dataFlow;
	    for(int m=1; m<=AfterSameMem.size(); m++)
	       if(Intersection(copy(dF),copy(AfterSameMem[m].mapping)).is_satisfiable())
		 {
		   Relation sample_bad = Sample_Solution(Intersection(copy(dF),copy(AfterSameMem[m].mapping)));
		   Relation sb_dom     = Domain(copy(sample_bad));
		   
		   cerr<<"for dataFlow: "<<dF;
		   cerr<<"  in statement "<<i<<", in read "<<j<<", in partial read "<<k<<endl;
		   cerr<<"  the memory map is invalid for the Array \""<<ArrayMapping[m].var<<"\"."<<endl;
		   Relation shut_up_about_non_constant_refs = copy(sample_bad);
		   cerr<<"  intersection(dataflow, AfterSameMem.mapping) includes " << shut_up_about_non_constant_refs << endl;

		   for(int i=1; i<=ArrayMapping.size(); i++)
		     {
		       Relation addr   = Join(copy(sb_dom), copy(ArrayMapping[i].mapping));
		       Relation writes = Join(addr, Inverse(copy(ArrayMapping[i].mapping)));
		       if (writes.is_satisfiable())
			 {
			   cout << "  that address is written, via memory map " << i << ", in iterations " << writes << endl;
			 }
		     }

		  value=false;
		 }
	   }
   return value;
}

bool checkBounds(Tuple<stm_info> info)
{
   int i,j,k;
   Relation bounds, mapping, IS;
   bool value;

   value=true;
   for(i=1; i<=info.size(); i++)
      for(j=1; j<=info[i].map.partials.size(); j++)  //checks array bounds within memory map
        {
         bounds=info[i].map.partials[j].bounds;
	 if (!bounds.is_obvious_tautology())
	   {
	    mapping=info[i].map.partials[j].mapping;
	    if (mapping.n_out() != bounds.n_set())
	      {
	       cerr <<"Array mapping has wrong number of subscripts for:"<<endl;
	       cerr <<"bounds: "<<bounds<<"mapping: "<<mapping;
	       value=false;
	      }
	    else if (!Must_Be_Subset(Range(copy(mapping)),copy(bounds)))
	      {
	       cerr<<"Array mapping not within memory map bounds for:"<<endl<<"bounds: "<<bounds<<"mapping: "<<mapping;
	       value=false;
	      }
	   }
        }
   for(i=1; i<=info.size(); i++)    //checks data flow relation within iteration space
      for(j=1; j<=info[i].read.size(); j++)
         for(k=1; k<=info[i].read[j].partials.size(); k++)
	   {
	    if(!Must_Be_Subset(Domain(copy(info[i].read[j].partials[k].dataFlow)),copy(info[info[i].read[j].partials[k].from].IS)))
	      {
		cerr<<"Data flow source not within iteration space for " <<
		  "statement " << i << ", read " << j << ", partial " << k <<
		  " (source stmt " << info[i].read[j].partials[k].from << ")\n";
		cerr << "dataFlow:  " << info[i].read[j].partials[k].dataFlow;
		cerr << "source IS: " << info[info[i].read[j].partials[k].from].IS;
		value=false;
	      }
	   }
   return value;
}


bool check_arities(Tuple<Relation> &sets, int arity)
{
  for (int i=1; i<=sets.size(); i++)
    if (sets[i].is_set())
      {
	if (sets[i].n_set() != arity)
	  return false;
      }
    else
      {
	if (sets[i].n_inp() != arity || sets[i].n_out() != arity)
	  return false;
      }
      
  return true;
}


bool check_arities(Tuple<stm_info> &info)
{
  int arity = info[1].IS.n_set();
  bool value = true;

  for(int i=1; i<=info.size(); i++)
    {
      if (info[i].IS.n_set() != arity)
	{
	  cerr << "Inconsistent I.S. dimensions for statements 1 (" <<
	    info[1].IS.n_set() <<
	    ") and " << i << " (" << info[i].IS.n_set() << ")" << endl;
	  value = false;
	}
      
      for(int m=1; m<=info[i].map.partials.size(); m++)
	{
	  if (info[i].map.partials[m].mapping.n_inp() != arity)
	    {
	      cerr << "Inconsistent I.S. dimensions (" <<
		info[i].map.partials[m].mapping.n_inp() << 
		") used in memory mapping " <<
		m << " of statement " << i << endl;
	      value = false;
	    }
	}
      
      
      for(int j=1; j<=info[i].read.size(); j++)
	for(int k=1; k<=info[i].read[j].partials.size(); k++)
	  {
	    if (info[i].read[j].partials[k].dataFlow.n_inp() != arity)
	      {
		cerr << "Inconsistent I.S. dimensions (" <<
		  info[i].read[j].partials[k].dataFlow.n_inp() <<
		  ") used in source of dataflow " << k <<
		  " of read " << j << " of statement " << i << endl;
		value=false;
	      }
	    if (info[i].read[j].partials[k].dataFlow.n_out() != arity)
	      {
		cerr << "Inconsistent I.S. dimensions (" << 
		  info[i].read[j].partials[k].dataFlow.n_out() <<
		  " ) used in sink of dataflow " << k <<
		  " of read " << j << " of statement " << i << endl;
		value=false;
	      }
	  }
    }
  return value;
}

} // end namespace omega
