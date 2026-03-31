/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array with enough elements to avoid complete unrolling */
    int arr[128];
    
    /* Initialize array with non-zero values to prevent dead code elimination */
    for (int i = 0; i < 128; i++) {
        arr[i] = i * 3;
    }
    
    /* CRITICAL PATTERN: Loop with separate dereference and pointer increment */
    int *ptr = arr;
    int *end = arr + 128;
    int sum = 0;
    
    while (ptr < end) {
        /* Dereference pointer with base + 0 addressing pattern */
        int value = *ptr;
        
        /* Separate pointer increment - NOT combined with dereference */
        ptr = ptr + 1;
        
        /* Use the value to prevent optimization away */
        sum += value;
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int sink = sum;
}
