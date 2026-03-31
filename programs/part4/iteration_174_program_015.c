int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    // Hoist expensive computation out of inner loop
    long long expensive = (long long)base[i] * 0x987654321LL;
    long long expensive_plus_one = expensive + 1;
    
    // Process 10 iterations without array
    for (int j = 0; j < 10; ++j) {
      int nibble = (int)((expensive >> (j * 4)) & 0xF);
      int term = nibble * (i + j);
      sum += term;
      
      // Avoid floating-point if possible
      // Since sum/expensive_plus_one will be small for typical values,
      // we can use integer division
      sum += (int)((double)sum / expensive_plus_one);
    }
  }
  return sum;
}
