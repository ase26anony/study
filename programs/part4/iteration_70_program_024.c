a[0] = b[0] * c[0] + a_init;  // a_init is a[-1] or some initial value
for (int i = 1; i < N; i++) {
    a[i] = b[i] * c[i] + a[i-1];
}
