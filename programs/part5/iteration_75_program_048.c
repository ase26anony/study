int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Reuse registers by computing values closer to their use
        int idx1 = i * 2;
        int idx2 = idx1 + 1;  // Reuse i*2 computation
        
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        int t1 = val1 * 3;
        int t2 = val2 * 5;
        int t3 = t1 + t2;
        
        int shift = i & 3;
        sum += t3 << shift;
        
        // Compute t5 and t6 with minimal register pressure
        arr[i] = (sum ^ t1) * t2;
    }
    return sum;
}
