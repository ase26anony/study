// Using array indexing
for (int i = 0; i < 10; i++) {
    sum += arr[i].value;
}

// Using pointer arithmetic with different notation
for (p = arr; p < arr + 10; p++) {
    sum += (*p).value;  // Also equivalent
}
