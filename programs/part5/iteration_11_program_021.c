if (x > 0) {    // test_expr is 'x'
    // then_bb starts here
    y = 10;     // doesn't modify x - OK
    x = 5;      // modifies x - would cause return false
    // then_last_head might be here
    z = 20;
}
