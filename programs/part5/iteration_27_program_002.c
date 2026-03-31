struct container arr[10];        // Declares an array of 10 struct container elements
struct container *p;             // Declares a pointer to struct container

for (p = arr; p < &arr[10]; ++p) {
    sum += p->value;            // Accesses the 'value' member of each struct
}
