int foo(int *base, int n) {
  int sum = 0;
  
  for (int i = 0; i < n; ++i) {
    // Precompute expensive value once per outer iteration
    long long expensive = (long long)base[i] * 0x987654321LL;
    
    // Precompute i + j values to avoid repeated addition
    int i_plus_j[10];
    for (int j = 0; j < 10; ++j) {
      i_plus_j[j] = i + j;
    }
    
    // Precompute expensive + 1 once
    long long expensive_plus_1 = expensive + 1;
    
    int arr[10];
    for (int j = 0; j < 10; ++j) {
      // Extract nibble without expensive shift each time
      // Since we're shifting by j*4, we can compute incrementally
      int shift_amount = j * 4;
      int nibble = (int)((expensive >> shift_amount) & 0xF);
      arr[j] = nibble;
      
      // Use precomputed i + j
      int term = nibble * i_plus_j[j];
      sum += term;
      
      // Avoid floating-point conversion if possible
      // Since sum/expensive_plus_1 will be small for typical values,
      // we can use integer division
      int d_int = sum / (int)expensive_plus_1;
      sum += d_int;
    }
  }
  return sum;
}
