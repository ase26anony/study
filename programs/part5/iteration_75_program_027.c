int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Calculate indices once
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        
        // Load values and compute immediately
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Compute t1 and use it immediately
        int t1_val = val1 * 3;
        int t2_val = val2 * 5;
        
        // Chain computations to reduce simultaneous live values
        int shift_amount = i & 3;
        int combined = (t1_val + t2_val) << shift_amount;
        sum += combined;
        
        // Compute final value with minimal intermediates
        arr[i] = (sum ^ t1_val) * t2_val;
    }
    return sum;
}
