int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    long long expensive = (long long)base[i] * 0x987654321LL;
    int arr[10];
    
    // Precompute expensive + 1 once
    long long expensive_plus_one = expensive + 1;
    
    for (int j = 0; j < 10; ++j) {
      arr[j] = (expensive >> (j * 4)) & 0xF;
      sum += arr[j] * (i + j);
      
      // Avoid repeated double conversion
      double d = (double)sum / expensive_plus_one;
      sum += (int)d;
    }
  }
  return sum;
}
