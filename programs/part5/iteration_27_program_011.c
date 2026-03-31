// Using array indexing
for (int i = 0; i < 10; i++) {
    sum += arr[i].value;
}

// Using pointer arithmetic explicitly
for (p = arr; p != &arr[10]; p++) {
    sum += (*p).value;  // equivalent to p->value
}
