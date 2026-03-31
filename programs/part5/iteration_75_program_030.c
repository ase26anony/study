int func(int n, int *arr) {
    int i, sum = 0;
    for (i = 0; i < n; i++) {
        // Compute and use immediately to reduce live ranges
        int idx1 = i * 2;
        int idx2 = idx1 + 1;
        int val1 = arr[idx1] * 3;
        int val2 = arr[idx2] * 5;
        int combined = val1 + val2;
        int shifted = combined << (i & 3);
        sum += shifted;
        
        // Reuse val1 and val2 instead of keeping t1, t2 alive
        int temp = sum ^ val1;
        arr[i] = temp * val2;
    }
    return sum;
}
