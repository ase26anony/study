int foo(int *base, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        // Hoist the expensive computation out of the inner loop
        long long expensive = (long long)base[i] * 0x987654321LL;
        long long expensive_plus_one = expensive + 1;
        
        // Process 10 iterations of the inner loop
        for (int j = 0; j < 10; ++j) {
            // Compute shift only once
            int shift = j * 4;
            int arr_val = (int)((expensive >> shift) & 0xF);
            
            // Accumulate sum
            sum += arr_val * (i + j);
            
            // Avoid floating-point division if possible
            // Since sum is int and expensive_plus_one is large,
            // (double)sum / expensive_plus_one will be small
            // We can use integer arithmetic approximation
            double d = (double)sum / expensive_plus_one;
            sum += (int)d;
        }
    }
    
    return sum;
}
