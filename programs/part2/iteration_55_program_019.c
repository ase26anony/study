/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array with enough elements to avoid trivial unrolling */
    int arr[128];
    
    /* Initialize array with non-zero values to prevent dead code elimination */
    for (int i = 0; i < 128; i++) {
        arr[i] = i * 3 + 1;
    }
    
    /* Critical pattern: pointer dereference followed by separate increment */
    int sum = 0;
    int *ptr = arr;
    int *end = arr + 128;
    
    /* Loop with separate dereference and increment operations */
    while (ptr < end) {
        /* Dereference pointer (creates base + 0 addressing) */
        int val = *ptr;
        
        /* Use the value to prevent elimination */
        sum += val;
        
        /* SEPARATE pointer increment (not combined with dereference) */
        ptr += 1;
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int sink = sum;
}
