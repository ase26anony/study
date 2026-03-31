int a, b, c;
if (a > 0) {
    b = 10;
    // Do something with original a
    a = 5;  // Only if this modification is intentional
} else {
    b = 20;
}
// Now a might be 5 or unchanged, depending on the path
