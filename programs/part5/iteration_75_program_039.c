int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Reuse registers by combining operations
        int idx1 = i * 2;
        int idx2 = idx1 + 1;  // Reuse i*2 calculation
        
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Combine multiplications with reuse
        int t1 = val1 * 3;
        int t2 = val2 * 5;
        
        // Immediate use of t1+t2 to reduce live range
        int shift_amount = i & 3;
        sum += (t1 + t2) << shift_amount;
        
        // Reuse t1 and t2 for final computation
        arr[i] = (sum ^ t1) * t2;
    }
    return sum;
}
