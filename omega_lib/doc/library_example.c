/*
  IF THESE EXAMPLES CHANGE, CHANGE construction.tex CORRESPONDINGLY

   S1 := { [t] : 1 <= t <= n }

   S2 := { [x] : (0 <= x <= 100 and
                  exists y : (2n <= y <= x and y is odd))
                 or x = 17 }

    R := { [i,j] -> [i',j'] : 1 <= i <= i' <= n and not (F(i) = F(i'))
                              and 1 <= j, j' <= m }
 */

//BEGIN PART 1
#include <omega.h>

main()
    {
    Relation S1(1), S2(1), R(2,2);
    S1.name_set_var(1, "t");
    S2.name_set_var(1, "x");

    assert(!R.is_set());
    assert(S1.is_set());
    assert(S2.is_set());

    Free_Var_Decl n("n");
    Free_Var_Decl m("m");
    Free_Var_Decl f("F", 1);

    Variable_ID S1s_n = S1.get_local(&n);
    Variable_ID S2s_n = S2.get_local(&n);

    Variable_ID Rs_n = R.get_local(&n);
    Variable_ID Rs_m = R.get_local(&m);
    Variable_ID Rs_f_in  = R.get_local(&f, Input_Tuple);
    Variable_ID Rs_f_out = R.get_local(&f, Output_Tuple);
    
    Variable_ID i = R.input_var(1);
    Variable_ID j = R.input_var(2);
    Variable_ID i2 = R.output_var(1);
    Variable_ID j2 = R.output_var(2);

    Variable_ID t = S1.set_var(1);
    Variable_ID x = S2.set_var(1);
//END PART 1
//BEGIN PART 2
    F_And *S1_root = S1.add_and();

    GEQ_Handle tmin = S1_root->add_GEQ();   // t-1 >= 0
    tmin.update_coef(t, 10);
    tmin.update_coef(t, -9);       // t now has coef. 1
    tmin.update_const(-1);
    GEQ_Handle tmax = S1_root->add_GEQ();   // n-t >= 0
    tmax.update_coef(S1s_n,1);
    tmax.update_coef(t, -1);


    F_Or  *S2_root = S2.add_or();
    F_And *part1 = S2_root->add_and();

    GEQ_Handle xmin = part1->add_GEQ();
    xmin.update_coef(x,1);
    GEQ_Handle xmax = part1->add_GEQ();
    xmax.update_coef(x,-1);
    xmax.update_const(100);

    F_Exists *exists_y = part1->add_exists();
    Variable_ID y = exists_y->declare("y");

    F_And *y_stuff = exists_y->add_and();
    GEQ_Handle ymin = y_stuff->add_GEQ();
    ymin.update_coef(y,1);
    ymin.update_coef(S2s_n,-2);
    GEQ_Handle ymax = y_stuff->add_GEQ();
    ymax.update_coef(x,1);
    ymax.update_coef(y,-1);
    Stride_Handle y_even = y_stuff->add_stride(2);
    y_even.update_coef(y,1);
    y_even.update_const(1);

    F_And *part2 = S2_root->add_and();

    EQ_Handle xvalue = part2->add_EQ();
    xvalue.update_coef(x,1);
    xvalue.update_const(-17);
//END PART 2
//BEGIN PART 3
    F_And *R_root = R.add_and();

    GEQ_Handle imin = R_root->add_GEQ();
    imin.update_coef(i,1);
    imin.update_const(-1);
    GEQ_Handle imax = R_root->add_GEQ();
    imax.update_coef(i2,1);
    imax.update_coef(i,-1);
    GEQ_Handle i2max = R_root->add_GEQ();
    i2max.update_coef(Rs_n,1);
    i2max.update_coef(i2,-1);

    EQ_Handle f_eq = R_root->add_not()->add_and()->add_EQ();
    f_eq.update_coef(Rs_f_in,-1);
    f_eq.update_coef(Rs_f_out,1);  // F(In) - F(Out) = 0

    GEQ_Handle jmin = R_root->add_GEQ();
    jmin.update_coef(j,1);
    jmin.update_const(-1);
    GEQ_Handle jmax = R_root->add_GEQ();
    jmax.update_coef(Rs_m,1);
    jmax.update_coef(j,-1);
    
    GEQ_Handle j2min = R_root->add_GEQ();
    j2min.update_coef(j2,1);
    j2min.update_const(-1);
    GEQ_Handle j2max = R_root->add_GEQ();
    j2max.update_coef(Rs_m,1);
    j2max.update_coef(j2,-1);
//END PART 3
//BEGIN PART 4
    S1.print_with_subs(stdout);
    assert(S1.is_upper_bound_satisfiable());
    assert(!S1.is_tautology());
    S1.print_with_subs(stdout);  // same as above print
    printf("\n");

    S2.print();
    assert(S2.is_upper_bound_satisfiable());
    assert(!S2.is_tautology());
    S2.print();                 // different from above
    printf("\n");

    assert(R.is_upper_bound_satisfiable());
    assert(!R.is_tautology());
    R.print_with_subs(stdout);

    int lb, ub;
    bool coupled;
    R.query_difference(i2, i, lb, ub, coupled);
    assert(lb == 1);        // i < i2: i2 - i1 > 0
    assert(ub == posInfinity);

    for(DNF_Iterator di(R.query_DNF()); di; di++)
        {
        printf("In next conjunct,\n");
        for(EQ_Iterator ei = (*di)->EQs(); ei; ei++)
            {
            printf("  In next equality constraint,\n");
            for(Constr_Vars_Iter cvi(*ei); cvi; cvi++)
                printf("    Variable \%s has coefficient \%d\n",
                       (*cvi).var->char_name(), 
                       (*cvi).coef);
            }
        for(GEQ_Iterator gi = (*di)->GEQs(); gi; gi++)
            {
            printf("  In next inequality constraint,\n");
            for(Constr_Vars_Iter cvi(*gi); cvi; cvi++)
                printf("    Variable \%s has coefficient \%d\n",
                       (*cvi).var->char_name(), 
                       (*cvi).coef);
            }
        printf("\n");
        }
//END PART 4
//BEGIN PART 5
    Relation S1_or_S2 = Union(copy(S1), copy(S2));

    // NOTE! THE FOLLOWING KILLS S1 AND S2
    Relation S1_and_S2 = Intersection(S1, S2);

    S1_or_S2.is_upper_bound_satisfiable();
    S1_and_S2.is_upper_bound_satisfiable();

    S1_or_S2.print();
    printf("\n");

    S1_and_S2.print();    
    printf("\n");

    Relation R_R = Composition(copy(R), R);
    R_R.query_difference(i2, i, lb, ub, coupled);
    assert(lb == 2);
    assert(ub == posInfinity);
    }
//END PART 5
