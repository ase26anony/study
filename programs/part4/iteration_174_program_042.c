int foo(int *base, int n) {
  int sum = 0;
  
  for (int i = 0; i < n; ++i) {
    // Hoist expensive computation out of inner loop
    long long expensive = (long long)base[i] * 0x987654321LL;
    long long expensive_plus_one = expensive + 1;
    
    // Precompute expensive >> (j*4) for j=0..9
    unsigned long long shifted[10];
    for (int j = 0; j < 10; ++j) {
      shifted[j] = expensive >> (j * 4);
    }
    
    // Unroll inner loop to reduce loop overhead
    int j = 0;
    for (; j + 3 < 10; j += 4) {
      // Process 4 iterations at once
      int vals[4];
      vals[0] = (shifted[j] & 0xF);
      vals[1] = (shifted[j+1] & 0xF);
      vals[2] = (shifted[j+2] & 0xF);
      vals[3] = (shifted[j+3] & 0xF);
      
      // Compute contributions
      sum += vals[0] * (i + j);
      sum += vals[1] * (i + j + 1);
      sum += vals[2] * (i + j + 2);
      sum += vals[3] * (i + j + 3);
      
      // Floating point operations - batch them
      double d0 = (double)sum / expensive_plus_one;
      double d1 = (double)(sum + (int)d0) / expensive_plus_one;
      double d2 = (double)(sum + (int)d0 + (int)d1) / expensive_plus_one;
      double d3 = (double)(sum + (int)d0 + (int)d1 + (int)d2) / expensive_plus_one;
      
      sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    }
    
    // Handle remaining iterations
    for (; j < 10; ++j) {
      int val = (shifted[j] & 0xF);
      sum += val * (i + j);
      double d = (double)sum / expensive_plus_one;
      sum += (int)d;
    }
  }
  
  return sum;
}
