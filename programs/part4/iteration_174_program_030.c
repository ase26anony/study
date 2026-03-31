int foo_optimized(int *base, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        // Hoist expensive computation out of inner loop
        long long expensive = (long long)base[i] * 0x987654321LL;
        long long expensive_plus_one = expensive + 1;
        
        // Precompute shift patterns if possible
        // Since expensive changes per outer iteration, we can't precompute all shifts,
        // but we can optimize the inner loop
        
        int arr[10];  // Could potentially use registers instead
        
        for (int j = 0; j < 10; ++j) {
            // Compute shift once
            int shift_amount = j * 4;
            arr[j] = (int)((expensive >> shift_amount) & 0xF);
            
            // Compute i+j once
            int i_plus_j = i + j;
            sum += arr[j] * i_plus_j;
            
            // Avoid floating point division if possible
            // Since we're just adding (int)d, we can use integer arithmetic
            // (int)((double)sum / (expensive + 1)) ≈ sum / (expensive_plus_one)
            // But careful with precision - depends on requirements
            
            // If exact floating point semantics aren't critical:
            // sum += sum / (int)expensive_plus_one;
            
            // Otherwise, keep but compute expensive_plus_one once
            double d = (double)sum / expensive_plus_one;
            sum += (int)d;
        }
    }
    return sum;
}
