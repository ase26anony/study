int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Compute address indices once and reuse
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        
        // Load array values early
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Compute t1 and t2
        int t1 = val1 * 3;
        int t2 = val2 * 5;
        
        // Combine computations
        int shift_amount = i & 3;
        int t3 = t1 + t2;
        int t4 = t3 << shift_amount;
        
        // Update sum
        sum += t4;
        
        // Compute final value for arr[i]
        int t5 = sum ^ t1;
        int t6 = t5 * t2;
        arr[i] = t6;
    }
    return sum;
}
