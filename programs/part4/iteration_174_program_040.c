int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    long long expensive = (long long)base[i] * 0x987654321LL; // Hard to remat
    
    // Precompute expensive + 1 to avoid repeated computation
    long long expensive_plus_1 = expensive + 1;
    
    int arr[10];
    for (int j = 0; j < 10; ++j) {
      arr[j] = (expensive >> (j*4)) & 0xF;
      sum += arr[j] * (i + j);
      
      // Use precomputed expensive_plus_1
      double d = (double)sum / expensive_plus_1;
      sum += (int)d;
    }
  }
  return sum;
}
