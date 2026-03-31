// To increase pressure further:
void even_higher_pressure() {
    // Add more interdependent variables
    int v1 = a + b, v2 = v1 * c, v3 = v2 ^ d;
    
    // Add more complex data dependencies
    for (...) {
        // Loop-carried dependencies
        a = b + c;
        b = c + a;  // Creates dependency chain
        c = a + b;
    }
    
    // Add more floating point in different precision
    double da = fa * 2.0;
    long double lda = da / 3.0L;
}
