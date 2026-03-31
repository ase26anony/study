int foo(int *base, int n) {
  int sum = 0;
  
  for (int i = 0; i < n; ++i) {
    // Precompute expensive value once per outer iteration
    long long expensive = (long long)base[i] * 0x987654321LL;
    long long expensive_plus_one = expensive + 1;
    
    for (int j = 0; j < 10; ++j) {
      // Eliminate the array, compute directly
      int arr_val = (expensive >> (j * 4)) & 0xF;
      sum += arr_val * (i + j);
      
      // Avoid floating-point if possible
      // If we must keep the same logic, at least reduce conversions
      sum += (int)((double)sum / expensive_plus_one);
    }
  }
  
  return sum;
}
