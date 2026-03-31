int x = 5;
if (x > 0) {
    x = 10;
} else if (x == 0) {
    // this will never execute in original flow, but if x were changed earlier...
}
