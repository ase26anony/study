void process_array(int *arr, int n) {
    int *p = arr;           // p points to the first element of arr
    for (int i = 0; i < n; i++) {
        *p = 0;             // Set current element to 0
        p++;                // Move pointer to next element
    }
}
