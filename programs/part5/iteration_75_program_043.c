int func(int n, int *arr) {
    int i, sum = 0;
    int *p = arr;
    
    for (i = 0; i < n; i++) {
        // Reduce address calculations by using pointer arithmetic
        int t1 = *p * 3;
        p++;
        int t2 = *p * 5;
        p++;
        
        int t3 = t1 + t2;
        int shift = i & 3;  // Precompute shift amount
        int t4 = t3 << shift;
        sum += t4;
        
        // Reorder computations to reduce register pressure
        int t6 = (sum ^ t1) * t2;
        arr[i] = t6;
    }
    return sum;
}
