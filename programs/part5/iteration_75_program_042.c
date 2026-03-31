int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Reuse registers by computing values closer to their use
        int idx1 = i * 2;
        int idx2 = idx1 + 1;  // Reuse computation
        
        // Load arr[idx1] and arr[idx2] into registers
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Compute t1 and t2, but don't keep both if possible
        int t1 = val1 * 3;
        int t2 = val2 * 5;
        
        // Compute t3 and use it immediately
        int t3 = t1 + t2;
        int shift = i & 3;
        int t4 = t3 << shift;
        sum += t4;
        
        // Reuse t1 and t2 if registers are scarce
        int t5 = sum ^ t1;
        arr[i] = t5 * t2;  // Compute directly without t6
    }
    return sum;
}
