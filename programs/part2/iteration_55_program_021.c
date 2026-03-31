/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Loop with post-increment pointer access - CRITICAL PATTERN */
    int *ptr = arr;
    int *end = arr + 100;
    
    while (ptr < end) {
        /* Dereference pointer with base + 0 addressing */
        int value = *ptr;
        
        /* Separate increment statement - creates distinct RTL instructions */
        ptr = ptr + 1;
        
        /* Use the value to prevent dead code elimination */
        sum += value;
    }
    
    return sum;
}
