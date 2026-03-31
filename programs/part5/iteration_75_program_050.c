int func_optimized(int n, int *arr) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        // Compute addresses once
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        
        // Load and compute in registers
        int val1 = ptr[idx1];
        int val2 = ptr[idx2];
        
        int t1 = val1 * 3;    // Keep in register
        int t2 = val2 * 5;    // Keep in register
        
        int t3 = t1 + t2;
        int shift = i & 3;
        int t4 = t3 << shift;
        sum += t4;
        
        int t5 = sum ^ t1;
        int t6 = t5 * t2;
        ptr[i] = t6;
    }
    return sum;
}
