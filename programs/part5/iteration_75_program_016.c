int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Reuse registers by combining operations
        int idx1 = i * 2;
        int idx2 = idx1 + 1;  // Reuse i*2 calculation
        
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Compute t1 and t2, but reuse registers
        val1 *= 3;  // t1
        val2 *= 5;  // t2
        
        int t3 = val1 + val2;
        int shift = i & 3;
        int t4 = t3 << shift;
        sum += t4;
        
        // Reuse val1 and val2 (t1 and t2) which are already in registers
        int t5 = sum ^ val1;
        int t6 = t5 * val2;
        arr[i] = t6;
    }
    return sum;
}
