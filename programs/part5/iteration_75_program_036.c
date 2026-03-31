int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Reuse registers by combining operations
        int idx1 = i * 2;
        int idx2 = idx1 + 1;  // Reuse i*2 calculation
        
        // Load values into registers once
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Combine operations where possible
        int t1 = val1 * 3;
        int t2 = val2 * 5;
        int t3 = t1 + t2;
        int shift = i & 3;
        int t4 = t3 << shift;
        sum += t4;
        
        // Reuse already loaded values
        int t5 = sum ^ t1;
        int t6 = t5 * t2;
        arr[i] = t6;
    }
    return sum;
}
