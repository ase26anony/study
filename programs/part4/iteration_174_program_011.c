int foo_optimized(int *base, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        // Hoist expensive computation out of inner loop
        long long expensive = (long long)base[i] * 0x987654321LL;
        long long expensive_plus_one = expensive + 1;
        
        // Precompute shift patterns if possible
        // Since expensive depends on base[i], we can't precompute all shifts,
        // but we can optimize the inner loop
        
        // Use register variables and minimize memory access
        int local_sum = sum;  // Work on local copy
        
        for (int j = 0; j < 10; ++j) {
            // Extract nibble directly without array
            int nibble = (int)((expensive >> (j * 4)) & 0xF);
            
            // Accumulate in local variable
            local_sum += nibble * (i + j);
            
            // Avoid floating point if possible
            // Since we're just adding integer part of division,
            // we can use integer division
            int div_result = (int)((double)local_sum / expensive_plus_one);
            local_sum += div_result;
        }
        
        sum = local_sum;  // Update global sum once
    }
    
    return sum;
}
