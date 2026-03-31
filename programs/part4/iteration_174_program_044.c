int foo(int *base, int n) {
  int sum = 0;
  
  for (int i = 0; i < n; ++i) {
    // Hoist expensive computation out of inner loop
    long long expensive = (long long)base[i] * 0x987654321LL;
    long long expensive_plus_one = expensive + 1;
    
    // Precompute i+j values to reduce register pressure
    int i_plus_j[10];
    for (int j = 0; j < 10; ++j) {
      i_plus_j[j] = i + j;
    }
    
    // Process in smaller chunks to reduce register pressure
    for (int j = 0; j < 10; ++j) {
      // Extract nibble without array indexing overhead
      int nibble = (int)((expensive >> (j * 4)) & 0xF);
      
      // Compute contribution
      int contribution = nibble * i_plus_j[j];
      sum += contribution;
      
      // Convert to double once
      double d = (double)sum / expensive_plus_one;
      sum += (int)d;
    }
  }
  
  return sum;
}
