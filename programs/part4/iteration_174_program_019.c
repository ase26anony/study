int foo_optimized(int *base, int n) {
    int sum = 0;
    int arr[10];  // Move outside loops
    
    for (int i = 0; i < n; ++i) {
        // Compute expensive value once per outer iteration
        long long expensive = (long long)base[i] * 0x987654321LL;
        long long expensive_plus_1 = expensive + 1;
        
        for (int j = 0; j < 10; ++j) {
            arr[j] = (int)((expensive >> (j * 4)) & 0xF);
            int term = arr[j] * (i + j);
            sum += term;
            
            // Avoid floating-point division if possible
            // Use integer approximation or precompute reciprocal
            if (expensive_plus_1 != 0) {
                // Integer division approximation instead of floating point
                sum += (int)((double)sum / expensive_plus_1);
            }
        }
    }
    return sum;
}
