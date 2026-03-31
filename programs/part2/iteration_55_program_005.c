/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Loop with separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 100;
    
    while (ptr < end) {
        /* Critical: Dereference first, then increment separately */
        int val = *ptr;      /* Creates base + 0 memory access */
        ptr += 1;            /* Separate increment statement */
        sum += val;
    }
    
    return sum;
}
