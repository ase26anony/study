int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Calculate indices once
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        
        // Load values and compute in place
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Reuse registers for intermediate results
        val1 *= 3;      // t1
        val2 *= 5;      // t2
        
        int temp = val1 + val2;  // t3
        temp <<= (i & 3);        // t4
        sum += temp;
        
        // Reuse temp register
        temp = sum ^ val1;       // t5
        temp *= val2;            // t6
        arr[i] = temp;
    }
    return sum;
}
