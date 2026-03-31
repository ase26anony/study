// Using array indexing
for (int i = 0; i < 10; i++) {
    sum += arr[i].value;
}

// Using pointer with count
struct container *end = arr + 10;
for (p = arr; p < end; p++) {
    sum += p->value;
}
