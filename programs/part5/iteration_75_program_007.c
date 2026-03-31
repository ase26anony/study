int func(int n, int *arr) {
    int sum = 0;
    
    // Unroll loop to reduce address calculation overhead
    for (int i = 0; i < n; i += 2) {
        // Load two array elements at once to reuse address calculation
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        int idx3 = (i + 1) * 2;
        int idx4 = idx3 + 1;
        
        // Process two iterations together
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        int val3 = arr[idx3];
        int val4 = arr[idx4];
        
        // First iteration
        int t1 = val1 * 3;
        int t2 = val2 * 5;
        int t3 = t1 + t2;
        int shift1 = i & 3;
        int t4 = t3 << shift1;
        sum += t4;
        int t5 = sum ^ t1;
        int t6 = t5 * t2;
        arr[i] = t6;
        
        // Second iteration (if within bounds)
        if (i + 1 < n) {
            int t7 = val3 * 3;
            int t8 = val4 * 5;
            int t9 = t7 + t8;
            int shift2 = (i + 1) & 3;
            int t10 = t9 << shift2;
            sum += t10;
            int t11 = sum ^ t7;
            int t12 = t11 * t8;
            arr[i + 1] = t12;
        }
    }
    
    return sum;
}
