int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    long long expensive = (long long)base[i] * 0x987654321LL;
    long long expensive_plus_1 = expensive + 1;  // Precompute
    int arr[10];
    
    for (int j = 0; j < 10; ++j) {
      arr[j] = (expensive >> (j * 4)) & 0xF;
      sum += arr[j] * (i + j);
      
      // Avoid double conversion if possible
      // Since expensive_plus_1 is constant in inner loop,
      // we could precompute 1.0 / expensive_plus_1
      double inv_expensive = 1.0 / (double)expensive_plus_1;
      sum += (int)((double)sum * inv_expensive);
    }
  }
  return sum;
}
