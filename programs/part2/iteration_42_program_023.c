static int __attribute__((noinline))
process_comparisons_optimized(int *restrict a, int *restrict b, int *restrict c, 
                              int *restrict d, int *restrict out1, int *restrict out2, 
                              int n) {
    int sum = 0;
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        // Branchless version of a[i] > b[i]
        int diff = a[i] - b[i];
        int mask = -(a[i] > b[i]);  // All 1s if true, all 0s if false
        out1[i] = diff & mask;      // Equivalent to: diff if true, 0 if false
        
        // Branchless version of c[i] >= d[i]
        int use_c = -(c[i] >= d[i]);  // Mask for c[i]
        int use_d = ~use_c;           // Mask for d[i] (inverse)
        out2[i] = (c[i] & use_c & 0xFF) | (d[i] & use_d & 0xFF);
        
        sum += out1[i] + out2[i];
    }
    
    return sum;
}
