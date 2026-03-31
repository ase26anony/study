/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    int arr[100];
    int sum = 0;
    
    /* Initialize array with values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* Loop with separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 100;
    
    while (ptr < end) {
        /* Dereference pointer with base + 0 addressing */
        int val = *ptr;
        
        /* Separate increment statement - critical for pattern matching */
        ptr = ptr + 1;
        
        /* Use the value to prevent dead code elimination */
        sum += val;
    }
    
    return sum;
}
