int x = 5;
if (x++ > 0) {   // test_expr modifies x
    x = 10;      // modifies x again
}
