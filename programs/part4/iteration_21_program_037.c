if (x > 0) {
    // instructions here
    y = x + 1;  // This doesn't modify x, so safe
    x = 5;      // This modifies x, would cause return false
}
