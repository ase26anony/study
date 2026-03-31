struct container arr[10];      // Declare an array of 10 'container' structs
struct container *p;           // Declare a pointer to a 'container' struct

// Loop through the array using pointer arithmetic
for (p = arr; p < &arr[10]; ++p) {
    sum += p->value;           // Access the 'value' member of each struct
}
