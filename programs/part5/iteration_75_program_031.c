int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Compute array indices once
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        
        // Load values and compute in sequence
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Combine computations
        int shift_amount = i & 3;
        int combined = (val1 * 3 + val2 * 5) << shift_amount;
        sum += combined;
        
        // Reuse val1 and val2 for final computation
        arr[i] = (sum ^ (val1 * 3)) * (val2 * 5);
    }
    return sum;
}
