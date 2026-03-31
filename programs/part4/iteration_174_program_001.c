int foo_optimized(int *base, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        // Hoist expensive computation out of inner loop
        long long expensive = (long long)base[i] * 0x987654321LL;
        long long expensive_plus_one = expensive + 1;
        
        // Precompute shift patterns if possible
        // Since expensive depends on base[i], we can't precompute all shifts,
        // but we can optimize the inner loop
        
        for (int j = 0; j < 10; ++j) {
            // Direct computation without array
            int nibble = (int)((expensive >> (j * 4)) & 0xF);
            int term = nibble * (i + j);
            sum += term;
            
            // Optimize division: use integer arithmetic if precision allows
            // Or precompute reciprocal if expensive_plus_one is constant across j
            // Since expensive doesn't change in inner loop, we can compute reciprocal once
            double reciprocal = 1.0 / (double)expensive_plus_one;
            double d = (double)sum * reciprocal;
            sum += (int)d;
        }
    }
    return sum;
}
