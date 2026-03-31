int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    // Compute once per outer iteration
    long long expensive = (long long)base[i] * 0x987654321LL;
    long long expensive_plus_one = expensive + 1;  // Precompute
    
    int arr[10];
    for (int j = 0; j < 10; ++j) {
      arr[j] = (expensive >> (j * 4)) & 0xF;
      sum += arr[j] * (i + j);
      
      // Avoid repeated division by same value
      double d = (double)sum / expensive_plus_one;
      sum += (int)d;
    }
  }
  return sum;
}
