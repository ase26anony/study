int foo(int *base, int n) {
  int sum = 0;
  
  for (int i = 0; i < n; ++i) {
    // Hoist expensive computation out of inner loop
    long long expensive = (long long)base[i] * 0x987654321LL;
    long long expensive_plus_one = expensive + 1;
    
    // Precompute i-dependent values
    int i_plus_j_start = i;
    
    for (int j = 0; j < 10; ++j) {
      // Compute nibble extraction directly without array
      int nibble = (int)((expensive >> (j * 4)) & 0xF);
      
      // Compute i+j once
      int i_plus_j = i + j;
      
      // Update sum with nibble contribution
      sum += nibble * i_plus_j;
      
      // Avoid floating-point division if possible
      // Since d = sum / (expensive + 1), and we cast to int,
      // we can use integer division
      int d = sum / (int)expensive_plus_one;
      sum += d;
    }
  }
  
  return sum;
}
