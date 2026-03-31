int foo(int *base, int n) {
  int sum = 0;
  
  for (int i = 0; i < n; ++i) {
    // Hoist the expensive computation out of the inner loop
    long long expensive = (long long)base[i] * 0x987654321LL;
    long long expensive_plus_one = expensive + 1;
    
    // Precompute shift patterns if possible
    // The expensive >> (j*4) operation could be optimized by incremental shifting
    
    int arr[10];  // Could potentially be eliminated or register allocated
    
    for (int j = 0; j < 10; ++j) {
      // Compute shift incrementally if the compiler doesn't optimize it
      int shift_amount = j * 4;
      arr[j] = (int)((expensive >> shift_amount) & 0xF);
      
      int temp = arr[j] * (i + j);
      sum += temp;
      
      // Avoid floating-point if possible - but this depends on the required precision
      // If we can approximate or use integer math:
      double d = (double)sum / expensive_plus_one;
      sum += (int)d;
    }
  }
  return sum;
}
