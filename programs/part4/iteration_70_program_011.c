// Handle the first element separately
a[0] = b[0] * c[0];  // No a[-1] to depend on

for (int i = 1; i < N; i++) {
    a[i] = b[i] * c[i] + a[i-1];
}
