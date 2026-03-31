int foo(int *base, int n) {
  int sum = 0;
  
  for (int i = 0; i < n; ++i) {
    // Hoist expensive computation out of inner loop
    long long expensive = (long long)base[i] * 0x987654321LL;
    
    // Precompute expensive + 1 once
    long long expensive_plus_one = expensive + 1;
    
    // Unroll inner loop to reduce overhead
    int arr[10];
    int j = 0;
    
    // Process in chunks to improve cache locality
    for (; j < 8; j += 2) {
      // Compute two array elements at once
      int shift1 = j * 4;
      int shift2 = (j + 1) * 4;
      
      arr[j] = (int)((expensive >> shift1) & 0xF);
      arr[j + 1] = (int)((expensive >> shift2) & 0xF);
      
      // Process first element
      int temp_sum = sum + arr[j] * (i + j);
      double d1 = (double)temp_sum / expensive_plus_one;
      sum = temp_sum + (int)d1;
      
      // Process second element
      temp_sum = sum + arr[j + 1] * (i + j + 1);
      double d2 = (double)temp_sum / expensive_plus_one;
      sum = temp_sum + (int)d2;
    }
    
    // Handle remaining elements (if any)
    for (; j < 10; ++j) {
      arr[j] = (int)((expensive >> (j * 4)) & 0xF);
      int temp_sum = sum + arr[j] * (i + j);
      double d = (double)temp_sum / expensive_plus_one;
      sum = temp_sum + (int)d;
    }
  }
  
  return sum;
}
