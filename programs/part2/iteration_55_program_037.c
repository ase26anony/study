/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array with enough elements to avoid complete unrolling */
    int arr[128];
    int *ptr = arr;
    int *end = arr + 128;
    int sum = 0;
    
    /* Loop with separate dereference and increment */
    while (ptr < end) {
        /* Dereference pointer (creates base + 0 access pattern) */
        sum += *ptr;
        
        /* Separate increment statement - critical for find_inc() */
        ptr += 1;
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int sink = sum;
}
