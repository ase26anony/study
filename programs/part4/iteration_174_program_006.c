int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    long long expensive = (long long)base[i] * 0x987654321LL; // Hard to remat
    
    // This could be precomputed once per outer iteration
    long long expensive_plus_one = expensive + 1;
    
    for (int j = 0; j < 10; ++j) {
      int arr_val = (expensive >> (j*4)) & 0xF;
      sum += arr_val * (i + j);
      
      // Use precomputed expensive_plus_one
      double d = (double)sum / expensive_plus_one;
      sum += (int)d;
    }
  }
  return sum;
}
