for (int i = 0; i < N; i++) {
    a[i] = b[i] * c[i] + a[i-1]; // Recurrence: a[i] depends on a[i-1]
}
