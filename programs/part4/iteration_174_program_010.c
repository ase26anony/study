int foo(int *base, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        // Hoist expensive calculation out of inner loop
        long long expensive = (long long)base[i] * 0x987654321LL;
        long long expensive_plus_one = expensive + 1;
        
        for (int j = 0; j < 10; ++j) {
            // Eliminate array, compute directly
            int arr_val = (expensive >> (j * 4)) & 0xF;
            sum += arr_val * (i + j);
            
            // Avoid floating-point if possible
            // Since expensive_plus_one is constant in inner loop,
            // we could use integer division if precision allows
            sum += (int)((double)sum / expensive_plus_one);
        }
    }
    return sum;
}
