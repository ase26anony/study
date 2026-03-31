int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    // Hoist expensive computation outside inner loop
    long long expensive = (long long)base[i] * 0x987654321LL;
    
    // Precompute i + j pattern
    int i_plus_j[10];
    for (int j = 0; j < 10; ++j) {
      i_plus_j[j] = i + j;
    }
    
    for (int j = 0; j < 10; ++j) {
      int arr_val = (expensive >> (j * 4)) & 0xF;
      sum += arr_val * i_plus_j[j];
      
      // Avoid expensive double division if possible
      // Since expensive is constant per outer iteration, compute reciprocal once
      double inv_expensive_plus_1 = 1.0 / (expensive + 1);
      sum += (int)((double)sum * inv_expensive_plus_1);
    }
  }
  return sum;
}
