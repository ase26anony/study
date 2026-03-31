int a = 1, b, c;  // a starts as 1
if (a > 0) {      // true, enter then block
    b = 10;       // b becomes 10
    a = 5;        // a becomes 5 (but we're already in the then block)
}
// b = 10, a = 5
