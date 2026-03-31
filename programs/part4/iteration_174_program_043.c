int foo(int *base, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        // Hoist expensive computation out of inner loop
        long long expensive = (long long)base[i] * 0x987654321LL;
        long long expensive_plus_one = expensive + 1;
        
        for (int j = 0; j < 10; ++j) {
            // Compute arr[j] value directly without array
            int arr_val = (int)((expensive >> (j * 4)) & 0xF);
            
            // Update sum
            sum += arr_val * (i + j);
            
            // Avoid floating-point division if possible
            // Since we're converting back to int, we can use integer division
            // But careful: (int)((double)sum / x) != sum / x for negative values
            // Since sum is positive in this context, we can use integer division
            sum += sum / (int)expensive_plus_one;
        }
    }
    
    return sum;
}
