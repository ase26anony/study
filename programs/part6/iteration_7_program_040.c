int temp = a;
if (temp > 0) {
    b = 10;
    a = 5;  // Still modifies original, but at least condition uses temp
}
