if (x > 0) {
    // then block
    y = x + 1;  // OK - doesn't modify x
    x = 5;      // Would cause this check to fail!
}
