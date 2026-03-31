/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array with enough elements to avoid complete unrolling */
    int arr[128];
    
    /* Initialize array with non-zero values to prevent dead code elimination */
    for (int i = 0; i < 128; i++) {
        arr[i] = i * 3 + 1;
    }
    
    /* Pointer-based loop with separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 128;
    int sum = 0;
    
    /* CRITICAL: Separate dereference and increment operations */
    while (ptr < end) {
        /* Memory access with base + 0 offset pattern */
        int val = *ptr;
        
        /* Separate increment statement - creates distinct RTL instructions */
        ptr = ptr + 1;
        
        /* Use the value to prevent optimization removal */
        sum += val;
    }
    
    /* Prevent dead code elimination */
    volatile int sink = sum;
}
