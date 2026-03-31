struct container arr[10];      // Array of 10 struct container elements
struct container *p;           // Pointer to struct container

// Loop through the array using pointer arithmetic
for (p = arr; p < &arr[10]; ++p) {
    sum += p->value;           // Access the 'value' member of each struct
}
