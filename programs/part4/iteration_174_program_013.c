int foo_optimized(int *base, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        // Hoist expensive computation out of inner loop
        long long expensive = (long long)base[i] * 0x987654321LL;
        long long expensive_plus_one = expensive + 1;
        
        // Precompute shift patterns if possible
        // Since expensive doesn't change in inner loop, we can compute shifts once
        int shift_results[10];
        for (int j = 0; j < 10; ++j) {
            shift_results[j] = (expensive >> (j * 4)) & 0xF;
        }
        
        // Unroll inner loop to reduce overhead
        int local_sum = 0;
        for (int j = 0; j < 10; ++j) {
            int arr_val = shift_results[j];
            int factor = i + j;
            local_sum += arr_val * factor;
            
            // Convert to double only once
            double d = (double)local_sum / expensive_plus_one;
            local_sum += (int)d;
        }
        
        sum += local_sum;
    }
    
    return sum;
}
