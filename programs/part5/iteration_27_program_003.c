struct container arr[10];      // Declare an array of 10 struct container elements
struct container *p;           // Declare a pointer to struct container

// Loop through the array using pointer arithmetic
for (p = arr; p < &arr[10]; ++p) {
    sum += p->value;           // Access the 'value' member of each struct
    // sum += p[0].value;      // Alternative syntax (equivalent)
}
