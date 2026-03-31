int foo(int *base, int n) {
  int sum = 0;
  
  for (int i = 0; i < n; ++i) {
    // Hoist expensive computation out of inner loop
    long long expensive = (long long)base[i] * 0x987654321LL;
    long long expensive_plus_one = expensive + 1;
    
    // Precompute i+j values to reduce arithmetic in inner loop
    int i_plus[10];
    for (int j = 0; j < 10; ++j) {
      i_plus[j] = i + j;
    }
    
    // Process in batches to reduce register pressure
    for (int j = 0; j < 10; ++j) {
      // Extract nibble without array storage
      int nibble = (int)((expensive >> (j * 4)) & 0xF);
      int temp = nibble * i_plus[j];
      sum += temp;
      
      // Avoid floating-point conversion if possible
      // Use integer division instead of floating-point
      sum += (int)((sum - temp) / expensive_plus_one);
    }
  }
  
  return sum;
}
