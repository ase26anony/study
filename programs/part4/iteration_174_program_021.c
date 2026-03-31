int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    long long expensive = (long long)base[i] * 0x987654321LL; // Keep here
    
    // Precompute expensive + 1 to avoid repeated addition
    long long expensive_plus_1 = expensive + 1;
    
    int arr[10];  // Stack allocation is cheap, but we can optimize access
    
    for (int j = 0; j < 10; ++j) {
      arr[j] = (expensive >> (j * 4)) & 0xF;
      sum += arr[j] * (i + j);
      
      // Convert division to multiplication by reciprocal if possible
      // But expensive_plus_1 changes with i, so can't precompute reciprocal
      double d = (double)sum / expensive_plus_1;
      sum += (int)d;
    }
  }
  return sum;
}
