int func(int n, int *arr) {
    int i, sum = 0;
    // Precompute constants outside loop
    const int mul1 = 3;
    const int mul2 = 5;
    
    for (i = 0; i < n; i++) {
        // Combine address calculations to reduce temporaries
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        
        // Load values early and reuse
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Combine multiplications
        int t1 = val1 * mul1;
        int t2 = val2 * mul2;
        
        // Reduce live ranges by computing t4 directly
        int shift_amount = i & 3;
        int t3 = t1 + t2;
        int t4 = t3 << shift_amount;
        sum += t4;
        
        // Compute t6 with minimal temporaries
        int t5 = sum ^ t1;
        arr[i] = t5 * t2;
    }
    return sum;
}
