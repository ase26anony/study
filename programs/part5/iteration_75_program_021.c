int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Reuse registers for common calculations
        int idx1 = i * 2;
        int idx2 = idx1 + 1;  // Reuse i*2 calculation
        
        // Load array values into registers
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Compute with immediate reuse
        int t1 = val1 * 3;
        int t2 = val2 * 5;
        int t3 = t1 + t2;
        
        // Compute shift amount once
        int shift_amt = i & 3;
        int t4 = t3 << shift_amt;
        
        sum += t4;
        
        // Reuse existing register values
        int t5 = sum ^ t1;  // sum already in register
        int t6 = t5 * t2;   // t2 still in register
        
        arr[i] = t6;
    }
    return sum;
}
