// Alternative 1: Using array indexing
for (int i = 0; i < 10; i++) {
    sum += arr[i].value;
}

// Alternative 2: Using pointer arithmetic differently
struct container *end = arr + 10;
for (p = arr; p < end; p++) {
    sum += p->value;
}

// Alternative 3: Using pointer to end
for (p = arr; p != &arr[10]; p++) {
    sum += p->value;
}
