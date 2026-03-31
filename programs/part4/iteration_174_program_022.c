int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    long long expensive = (long long)base[i] * 0x987654321LL;
    int arr[10];
    // Rest of inner loop...
  }
  return sum;
}
