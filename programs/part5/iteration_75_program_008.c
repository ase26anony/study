int func(int n, int *arr) {
    int i, sum = 0;
    // Precompute mask to avoid recomputing i & 3 each time
    int mask = 3;
    
    for (i = 0; i < n; i++) {
        // Compute array indices once
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        
        // Load array values
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Compute t1 and t2 with strength reduction
        int t1 = val1 * 3;
        int t2 = val2 * 5;
        
        // Compute t3 and t4
        int t3 = t1 + t2;
        int shift = i & mask;  // Reuse mask
        int t4 = t3 << shift;
        
        // Update sum
        sum += t4;
        
        // Compute t5 and t6
        int t5 = sum ^ t1;
        int t6 = t5 * t2;
        
        // Store result
        arr[i] = t6;
    }
    return sum;
}
