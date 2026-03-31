/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array to ensure base pointer is in memory */
    int arr[100];
    
    /* Initialize array to prevent optimization removal */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Pointer traversal with separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* CRITICAL: Separate dereference and increment statements */
    while (ptr < end) {
        /* Dereference with base + 0 addressing pattern */
        int val = *ptr;
        
        /* Separate increment statement - not combined with dereference */
        ptr = ptr + 1;
        
        /* Use the value to prevent dead code elimination */
        sum += val;
    }
    
    /* Prevent optimization of entire function */
    volatile int sink = sum;
}
