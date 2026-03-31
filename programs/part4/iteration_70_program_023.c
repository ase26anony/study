a[0] = b[0] * c[0];  // Base case
for (int i = 1; i < N; i++) {
    a[i] = b[i] * c[i] + a[i-1];
}
