int foo(int *base, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    long long expensive = (long long)base[i] * 0x987654321LL; // Already outside inner loop
    int arr[10];
    for (int j = 0; j < 10; ++j) {
      // ...
    }
  }
  return sum;
}
