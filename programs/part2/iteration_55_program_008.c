/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    /* Stack array to ensure base pointer is on stack */
    int arr[100];
    
    /* Initialize array with sequential values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    int sum = 0;
    int *ptr = arr;            /* Base pointer */
    int *end = arr + 100;      /* End pointer */
    
    /* CRITICAL PATTERN: 
     * 1. Dereference pointer with offset 0 (*ptr)
     * 2. Increment pointer in separate statement (ptr++)
     * This should generate base+0 addressing followed by increment
     */
    while (ptr < end) {
        /* Access memory at base + 0 offset */
        int value = *ptr;
        
        /* Separate increment statement - NOT combined with dereference */
        ptr = ptr + 1;
        
        /* Use value to prevent dead code elimination */
        sum += value;
    }
    
    /* Return result to ensure loop isn't optimized away */
    return sum;
}
