struct container arr[10];      // Declares an array of 10 'container' structures
struct container *p;           // Declares a pointer to a 'container' structure

for (p = arr; p < &arr[10]; ++p) {  // Loop through the array
    sum += p->value;           // Access the 'value' member of each structure
}
