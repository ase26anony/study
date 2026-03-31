int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Combine calculations where possible
        int idx1 = i * 2;
        int idx2 = idx1 + 1;  // Reuse i*2 calculation
        
        int val1 = arr[idx1] * 3;
        int val2 = arr[idx2] * 5;
        
        // Combine t3 and t4 calculations
        int shifted = (val1 + val2) << (i & 3);
        sum += shifted;
        
        // Store result directly without t5/t6
        arr[i] = (sum ^ val1) * val2;
    }
    return sum;
}
