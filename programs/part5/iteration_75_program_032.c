int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Compute array indices once
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        
        // Load and compute t1, use immediately
        int val1 = arr[idx1];
        int t1 = val1 * 3;
        
        // Load and compute t2, use immediately
        int val2 = arr[idx2];
        int t2 = val2 * 5;
        
        // Combine and shift
        int t3 = t1 + t2;
        int shift = i & 3;
        int t4 = t3 << shift;
        sum += t4;
        
        // Recompute t1 if needed, or use saved values
        arr[i] = (sum ^ t1) * t2;
    }
    return sum;
}
