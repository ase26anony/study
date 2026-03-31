int func(int n, int *arr) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + n * 2;
    
    for (int i = 0; ptr < end; i++) {
        // Load two consecutive elements at once
        int val1 = *ptr++;
        int val2 = *ptr++;
        
        // Compute t1 and t2 with strength reduction
        int t1 = val1 + (val1 << 1);  // val1 * 3
        int t2 = (val2 << 2) + val2;  // val2 * 5
        
        // Combine computations
        int t3 = t1 + t2;
        int shift = i & 3;
        int t4 = t3 << shift;
        sum += t4;
        
        // Reuse t1 and t2 instead of reloading
        int t5 = sum ^ t1;
        int t6 = t5 * t2;
        arr[i] = t6;
    }
    return sum;
}
