struct container arr[10];        // Array of 10 struct container elements
struct container *p;             // Pointer to struct container

// Loop initialization: p points to first element of arr
for (p = arr; p < &arr[10]; ++p) {
    sum += p->value;            // Access current element's value member
}
