int foo(int *base, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        // Hoist expensive calculation out of inner loop
        long long expensive = (long long)base[i] * 0x987654321LL;
        
        // Precompute expensive + 1 once
        long long expensive_plus_one = expensive + 1;
        
        // Unroll inner loop to reduce loop overhead
        int arr[10];
        int j = 0;
        
        // Process 2 iterations at a time to enable better pipelining
        for (; j < 8; j += 2) {
            // Calculate shift amounts once
            int shift1 = j * 4;
            int shift2 = (j + 1) * 4;
            
            // Extract nibbles
            arr[j] = (int)((expensive >> shift1) & 0xF);
            arr[j + 1] = (int)((expensive >> shift2) & 0xF);
            
            // Calculate i + j values
            int i_plus_j1 = i + j;
            int i_plus_j2 = i + j + 1;
            
            // Update sum with first iteration
            sum += arr[j] * i_plus_j1;
            double d1 = (double)sum / expensive_plus_one;
            sum += (int)d1;
            
            // Update sum with second iteration
            sum += arr[j + 1] * i_plus_j2;
            double d2 = (double)sum / expensive_plus_one;
            sum += (int)d2;
        }
        
        // Handle remaining iterations (if any)
        for (; j < 10; ++j) {
            arr[j] = (int)((expensive >> (j * 4)) & 0xF);
            sum += arr[j] * (i + j);
            double d = (double)sum / expensive_plus_one;
            sum += (int)d;
        }
    }
    
    return sum;
}
