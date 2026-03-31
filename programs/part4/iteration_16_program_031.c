int *ptr = data;
for (i = 0; i < N; i++) {
    sum += *ptr;
    ptr++;  // Pointer arithmetic
}
