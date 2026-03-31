int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Combine address calculations
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        
        // Load values early and reuse
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Compute t1 and t2, but try to use them immediately
        int t1_val = val1 * 3;
        int t2_val = val2 * 5;
        
        // Combine t3 and t4 computation
        int shift_amount = i & 3;
        int t4_val = (t1_val + t2_val) << shift_amount;
        sum += t4_val;
        
        // Compute t5 and t6 with minimal intermediates
        arr[i] = (sum ^ t1_val) * t2_val;
    }
    return sum;
}
