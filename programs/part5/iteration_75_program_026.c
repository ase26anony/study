int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Reuse registers by combining operations
        int idx1 = i * 2;
        int idx2 = idx1 + 1;  // Reuse idx1 calculation
        
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Combine multiplications with shifts where possible
        int t1 = val1 * 3;
        int t2 = val2 * 5;
        
        // Immediate use of t1 and t2 to shorten live ranges
        int t3 = t1 + t2;
        int shift = i & 3;
        int t4 = t3 << shift;
        sum += t4;
        
        // Reuse t1 and t2 before they go out of scope
        int t5 = sum ^ t1;
        arr[i] = t5 * t2;  // Direct assignment, no t6 needed
    }
    return sum;
}
