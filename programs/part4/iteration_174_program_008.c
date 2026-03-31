int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    long long expensive = (long long)base[i] * 0x987654321LL; // Hard to remat
    
    // This array could be moved outside the inner loop if we don't need all values at once
    int arr[10];
    
    // Precompute i once
    int i_val = i;
    
    for (int j = 0; j < 10; ++j) {
      arr[j] = (expensive >> (j*4)) & 0xF;
      sum += arr[j] * (i_val + j);
      
      // This division is expensive and could be optimized
      double d = (double)sum / (expensive + 1);
      sum += (int)d;
    }
  }
  return sum;
}
