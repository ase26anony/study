/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array with known size */
    int arr[100];
    
    /* Initialize array to prevent optimization removal */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Pointer traversal with separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* CRITICAL: Loop with separate dereference and increment */
    while (ptr < end) {
        /* Dereference pointer (creates base + 0 access pattern) */
        sum += *ptr;
        
        /* Separate increment statement (not combined with dereference) */
        ptr += 1;
    }
    
    /* Use result to prevent dead code elimination */
    volatile int result = sum;
}
