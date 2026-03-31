int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Calculate indices once
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        
        // Load values and compute immediately
        int val1 = arr[idx1];
        int t1 = val1 * 3;
        
        int val2 = arr[idx2];
        int t2 = val2 * 5;
        
        // Compute t3 and use it immediately
        int t3 = t1 + t2;
        int t4 = t3 << (i & 3);
        sum += t4;
        
        // Recompute t1 if needed, or use saved val1
        int t5 = sum ^ t1;
        int t6 = t5 * t2;
        arr[i] = t6;
    }
    return sum;
}
