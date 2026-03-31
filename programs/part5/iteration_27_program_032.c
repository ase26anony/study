// Using array indexing
for (int i = 0; i < 10; i++) {
    sum += arr[i].value;
}

// Using pointer with index notation (as commented)
for (p = arr; p < &arr[10]; ++p) {
    sum += p[0].value;  // Same as p->value
}
