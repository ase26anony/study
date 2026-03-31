struct container arr[10];      // Declares an array of 10 'container' structures
struct container *p;           // Declares a pointer to a 'container' structure

// Loop through the array using pointer arithmetic
for (p = arr; p < &arr[10]; ++p) {
    sum += p->value;           // Access the 'value' member of each structure
}
