int func(int n, int *arr) {
    int sum = 0;
    
    // Try to keep these in registers:
    // - sum (accumulator)
    // - i (loop counter)
    // - arr (base pointer)
    // - t1, t2 (most frequently used)
    
    for (int i = 0; i < n; i++) {
        // Calculate array indices once
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        
        // Load array values
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Compute t1 and t2
        int t1 = val1 * 3;
        int t2 = val2 * 5;
        
        // Combine computations
        int t3 = t1 + t2;
        int shift = i & 3;  // Keep shift amount
        int t4 = t3 << shift;
        sum += t4;
        
        // Final computation
        int t5 = sum ^ t1;
        int t6 = t5 * t2;
        arr[i] = t6;
    }
    return sum;
}
