int foo(int *base, int n) {
  int sum = 0;
  
  for (int i = 0; i < n; ++i) {
    // Compute expensive once per outer iteration
    long long expensive = (long long)base[i] * 0x987654321LL;
    long long expensive_plus_one = expensive + 1;
    
    for (int j = 0; j < 10; ++j) {
      // Compute arr[j] directly without array storage
      int arr_val = (expensive >> (j * 4)) & 0xF;
      sum += arr_val * (i + j);
      
      // Avoid repeated floating-point conversions
      // Use integer division if precision allows, or compute once
      double d = (double)sum / expensive_plus_one;
      sum += (int)d;
    }
  }
  
  return sum;
}
