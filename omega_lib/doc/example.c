#include <presburger.h>

THIS NEEDS TO BE A LOT SIMPLER - I'M CUTTING IT DOWN

// R := { [i,j] -> [i', j'] :
//        1 <= i, i' <= n && 1 <= j <= L(i) && 1 <= j' <= m &&
//        j = j' && i < i' }
//
// S := { [x,y] : 1 <= x <= n && y <= x + 5 && x is divisible by 17 &&
//        there exists z such that y <= z <= x &&
//        ( z is divisible by 8 || z+5x is divisible by 12 ) }

main()
    {
    Relation R(2,2), S(2);
    S.name_set_var(1, "x");
    S.name_set_var(2, "y");

    assert(!R.is_set());
    assert( S.is_set());

    Free_Var_Decl n("n");
    Free_Var_Decl m("m");
    Free_Var_Decl l("L", 1);

    Variable_ID local_n = R.get_local(&n);
    Variable_ID local_m = R.get_local(&m);
    Variable_ID l_in    = R.get_local(&l, Input_Tuple);
    Variable_ID l_out   = R.get_local(&l, Output_Tuple);
    
    Variable_ID in1 = R.input_var(1);
    Variable_ID in2 = R.input_var(2);
    Variable_ID out1 = R.output_var(1);
    Variable_ID out2 = R.output_var(2);

    Variable_ID x = S.set_var(1);
    Variable_ID y = S.set_var(2);

    F_And *S_root = S.add_and();

    GEQ_Handle xmin = S_root->add_GEQ();  // x-1 >= 0
    xmin.update_coef(x, 1);
    xmin.update_const(-1);
    GEQ_Handle xmax = S_root->add_GEQ();  // n-x >= 0
    xmax.update_coef(x, -1);
    xmax.update_coef(S.get_local(&n), 1);
    GEQ_Handle ymax = S_root->add_GEQ();  // x+5-y >= 0
    ymax.update_coef(x, 1);
    ymax.update_coef(y, -1);
    ymax.update_const(5);

    // x is divisible by 17
    S_root->add_stride(17).update_coef(x,1);  

    F_Exists *e = S_root->add_exists();

    Variable_ID z = e->declare("z");      // exists z
    F_And *z_stuff = e->add_and();

    GEQ_Handle zmin = z_stuff->add_GEQ(); // z-y >= 0
    zmin.update_coef(z,1);
    zmin.update_coef(y,-1);
    GEQ_Handle zmax = z_stuff->add_GEQ(); // x-z >= 0
    zmax.update_coef(x,1);
    zmax.update_coef(z,-1);

    F_Or *o = z_stuff->add_or();
    Stride_Handle z8  = o->add_and()->add_stride(8);
    z8.update_coef(z,1);         // z divisible by 8

    Stride_Handle z12 = o->add_and()->add_stride(12);
    z12.update_coef(z,1);
    z12.update_coef(x,5);    // z+5x divisible by 12

    S.finalize();
    S.prefix_print();
    S.is_upper_bound_satisfiable();
    S.prefix_print();
    }

