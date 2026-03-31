int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    long long expensive = (long long)base[i] * 0x987654321LL; // Hard to remat
    int arr[10];
    // Hoist expensive calculation if possible
    for (int j = 0; j < 10; ++j) {
      arr[j] = (expensive >> (j*4)) & 0xF;
      sum += arr[j] * (i + j);
      // Introduce more live values
      double d = (double)sum / (expensive + 1);
      sum += (int)d;
    }
  }
  return sum;
}
