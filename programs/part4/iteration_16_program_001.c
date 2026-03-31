int *ptr = data;
for (i = 0; i < N; i++) {
    sum += *ptr;  // Dereference pointer
    ptr++;        // Increment pointer to next element
}
