int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Combine computations to reduce temporaries
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        
        // Compute t1 and t2, use immediately
        int t3 = val1 * 3 + val2 * 5;
        int t4 = t3 << (i & 3);
        sum += t4;
        
        // Reuse val1 and val2 instead of t1 and t2
        int t5 = sum ^ (val1 * 3);
        arr[i] = t5 * (val2 * 5);
    }
    return sum;
}
