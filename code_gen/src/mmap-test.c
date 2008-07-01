// RelTuple and SetTuple are Tuple's of Relations
// RelTuple is Transformation: [i,j] -> [i,j]
// SetTuple is IS: [i,j]: 1 <= i,j <= n  // corresponds to # of statements in codegen
// Relation is normal relation: [i,j], 0=0
// int is effort that will default as 0

#include <code_gen/mmap-codegen.h>


#error "This code is no longer being maintained.  MMap is tested via oc in test-parser"

int main() {

  // Transformation or Data Flow
  Relation dataFlow(2,2); // [i,j] -> [i,j]
  dataFlow.name_input_var(1, "i");
  dataFlow.name_input_var(2, "j");
  Variable_ID i = dataFlow.input_var(1);
  Variable_ID j = dataFlow.input_var(2);

  dataFlow.name_output_var(1, "I");
  dataFlow.name_output_var(2, "J");
  Variable_ID I = dataFlow.output_var(1);
  Variable_ID J = dataFlow.output_var(2);

  F_And *root = dataFlow.add_and();
  EQ_Handle ieq = root->add_EQ();
  ieq.update_coef(i,1);
  ieq.update_coef(I,-1);
  EQ_Handle jeq = root->add_EQ();
  jeq.update_coef(j,1);
  jeq.update_coef(J,-1);

  //  dataFlow.prefix_print();
  cout << "Data Flow:\t";
  dataFlow.print_with_subs(stdout);
  RelTuple R;
  R.append(dataFlow);

  Relation IS(2);
  IS.name_set_var(1, "i");
  IS.name_set_var(2, "j");
  
  Free_Var_Decl n("n");
  Variable_ID IS_n = IS.get_local(&n);

  Variable_ID IS_i = IS.set_var(1);
  Variable_ID IS_j = IS.set_var(2);

  F_And *IS_root = IS.add_and();
  GEQ_Handle imin = IS_root->add_GEQ();
  imin.update_coef(IS_i, 1);
  imin.update_const(-1);
  GEQ_Handle imax = IS_root->add_GEQ();
  imax.update_coef(IS_n,1);
  imax.update_coef(IS_i, -1);

  GEQ_Handle jmin = IS_root->add_GEQ();
  jmin.update_coef(IS_j, 1);
  jmin.update_const(-1);
  GEQ_Handle jmax = IS_root->add_GEQ();
  jmax.update_coef(IS_n,1);
  jmax.update_coef(IS_j, -1);

  cout << "IS:\t\t";
  IS.print_with_subs(stdout);
  SetTuple S;
  S.append(IS);

  Relation norm(2);
  norm.name_set_var(1, "i");
  norm.name_set_var(2, "j");
  i = norm.set_var(1);
  j = norm.set_var(2);

  F_And *norm_root = norm.add_and();
  EQ_Handle equal = norm_root->add_EQ();
  equal.update_const(0);

  cout << "Relation:\t";
  norm.print_with_subs(stdout);

  // creating MMaps
  MMap mmtest;		// { [t,i] -> [i,j] , "a" }
  //defining mmtest
  Relation map(2,2);  // making it output 2 vars

  map.name_input_var(1, "i");
  map.name_input_var(2, "j");
  Variable_ID t1 = map.input_var(1);
  Variable_ID i1 = map.input_var(2);

  map.name_output_var(1, "i");
  map.name_output_var(2, "j");  // output var 2
  Variable_ID x1 = map.output_var(1);
  Variable_ID y1 = map.output_var(2);  // output var 2

  F_And *map_root = map.add_and();
  EQ_Handle map_xeq = map_root->add_EQ();  // x=i  === x-i = 0
  map_xeq.update_coef(x1,1);
  map_xeq.update_coef(t1,-1);

  EQ_Handle map_yeq = map_root->add_EQ();  // y=t  === y-t = 0
  map_yeq.update_coef(y1,1);
  map_yeq.update_coef(i1,-1);

  mmtest.mapping = map;
  mmtest.var = "a";
  cout << "Memory Map:\t";
  map.print_with_subs(stdout);

  MMap mmtest2;
  Relation map2(2,1);  // making it output 2 vars
  map2.name_input_var(1, "i");
  map2.name_input_var(2, "j");
  t1 = map2.input_var(1);
  i1 = map2.input_var(2);

  map2.name_output_var(1, "i");
  x1 = map2.output_var(1);
  
  F_And *map2_root = map2.add_and();
  EQ_Handle map2_xeq = map2_root->add_EQ();  // x=i+j  === x-i-j = 0
  map2_xeq.update_coef(x1,1);
  map2_xeq.update_coef(t1,-1);
  map2_xeq.update_coef(i1,-1);

  mmtest2.mapping = map2;
  mmtest2.var = "b";
  cout << "Memory Map 2:\t";
  map2.print_with_subs(stdout);

#if 0  // NO LONGER IMPLEMENTED
  Tuple<MMap> tests;
  tests.append(mmtest);
  tests.append(mmtest2);

  cout << list(tests,"0");
#endif

  cout << MMGenerateCode(R,S,norm,0);
  //  CG_funkyStringBuilder x;
  //  MMGenerateCode(&x, R, S, norm, 0)->Dump();

  /* Example of output function:  for i=2 to N
                                    a[i] = a[i-1] + 2
     IS : 2 <= i <= N
     MMap : "a", [i]->[i]
     Reads : 0, [i]->[i+1]
     Stmt : "w = r1 + 2"
     */
  
  Relation IS2(1);
  IS2.name_set_var(1, "i");
  
  IS_n = IS2.get_local(&n);
  IS_i = IS2.set_var(1);

  F_And *IS2_root = IS2.add_and();
  imin = IS2_root->add_GEQ();
  imin.update_coef(IS_i, 1);
  imin.update_const(-2);
  imax = IS2_root->add_GEQ();
  imax.update_coef(IS_n,1);
  imax.update_coef(IS_i, -1);

  cout << "IS:\t\t";
  IS2.print_with_subs(stdout);

  MMap mmtest3;
  Relation map3(1,1);  // making it output 2 vars

  map3.name_input_var(1, "i");
  i1 = map3.input_var(1);

  map3.name_output_var(1, "i");
  x1 = map3.output_var(1);

  F_And *map3_root = map3.add_and();
  EQ_Handle map3_xeq = map3_root->add_EQ();  // x=i  === x-i = 0
  map3_xeq.update_coef(x1,1);
  map3_xeq.update_coef(t1,-1);

  mmtest3.mapping = map3;
  mmtest3.var = "a";
  cout << "Memory Map 3:\t";
  map3.print_with_subs(stdout);

  PartialRead readTest;
  Relation df(1,1);
  df.name_input_var(1, "i");
  i1 = df.input_var(1);
  x1 = df.output_var(1);
  F_And *df_root = df.add_and();
  EQ_Handle df_eq = df_root->add_EQ();
  df_eq.update_coef(x1,1);
  df_eq.update_coef(i1,-1);
  df_eq.update_const(-1);

  readTest.from = 1;
  readTest.dataFlow = df;
  Read oneReadTest;
  oneReadTest.partials.append(readTest);

  Tuple<Read> readTuple;
  readTuple.append(oneReadTest);
  cout << "Data Flow:\t";
  df.print_with_subs(stdout);

  String stmtTest = "w = r1 + 2";
  
  stm_info testing;
  testing.IS = IS2;
  testing.map = mmtest3;
  testing.read = readTuple;
  testing.stm = stmtTest;
  Tuple<stm_info> example;
  example.append(testing);
  cout << output(example);
} // Main






