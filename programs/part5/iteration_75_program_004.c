int func(int n, int *arr) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + n * 2;
    
    for (int i = 0; ptr < end; i++) {
        // Load two consecutive elements at once
        int val1 = *ptr++;
        int val2 = *ptr++;
        
        // Compute with fewer intermediate registers
        int t1 = val1 * 3;
        int t2 = val2 * 5;
        int t3 = t1 + t2;
        
        // Shift amount can be computed once
        int shift = i & 3;
        int t4 = t3 << shift;
        
        sum += t4;
        
        // Reuse t1 and t2 instead of loading from memory again
        int t5 = sum ^ t1;
        int t6 = t5 * t2;
        
        // Store back to arr[i] (note: overwrites original array)
        arr[i] = t6;
    }
    return sum;
}
