// Using array indexing
for (int i = 0; i < 10; i++) {
    sum += arr[i].value;
}

// Using pointer arithmetic with different syntax
for (p = arr; p != &arr[10]; p++) {
    sum += p->value;
}

// Using the array size directly
for (p = arr; p < arr + 10; ++p) {
    sum += p->value;
}
