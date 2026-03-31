// This pointer version:
for (int i = 0; i < n; i++) {
    *p = 0;
    p++;
}

// Is equivalent to:
for (int i = 0; i < n; i++) {
    arr[i] = 0;
}

// Or this pointer arithmetic version:
for (int i = 0; i < n; i++) {
    *(arr + i) = 0;
}
