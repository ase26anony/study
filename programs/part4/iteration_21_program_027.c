if (x > 0) {
    // then_bb starts here
    y = x + 1;  // OK - doesn't modify x
    x = 5;      // Would cause this check to fail
    z = x * 2;
}
