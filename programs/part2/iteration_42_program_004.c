static int __attribute__((noinline))
process_comparisons(int *restrict a, int *restrict b, int *restrict c, int *restrict d,
                    int *restrict out1, int *restrict out2, int n) {
    int sum = 0;
    
    // Add alignment hints for better SIMD performance
    #pragma omp simd aligned(a, b, c, d, out1, out2: 32) reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        // Branchless version of first comparison - often better for SIMD
        int diff = a[i] - b[i];
        int mask = -(a[i] > b[i]);  // All 1s if true, all 0s if false
        out1[i] = diff & mask;      // Equivalent to: diff if true, 0 if false
        
        // Original ternary - compilers usually handle this well
        out2[i] = (c[i] >= d[i]) ? (c[i] & 0xFF) : (d[i] & 0xFF);
        
        sum += out1[i] + out2[i];
    }
    return sum;
}
