int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    long long expensive = (long long)base[i] * 0x987654321LL; // Keep this
    int arr[10];  // Stack allocation is cheap, but we can optimize access
    
    // Precompute i*10 to avoid repeated multiplication
    int i_times_10 = i * 10;
    
    for (int j = 0; j < 10; ++j) {
      arr[j] = (expensive >> (j*4)) & 0xF;
      sum += arr[j] * (i + j);
      
      // Avoid division if possible - check if we can use integer math
      // Division by (expensive + 1) is expensive, but we need to keep precision
      double d = (double)sum / (expensive + 1);
      sum += (int)d;
    }
  }
  return sum;
}
