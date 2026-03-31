/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array with enough elements to avoid complete unrolling */
    int arr[100];
    
    /* Initialize array with non-zero values to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* Pointer that will be incremented separately from dereference */
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* Loop with separate dereference and increment operations */
    while (ptr < end) {
        /* Dereference pointer (creates base + 0 memory access) */
        int value = *ptr;
        
        /* Separate increment statement (not combined with dereference) */
        ptr += 1;
        
        /* Use the value to prevent optimization removal */
        sum += value;
    }
    
    /* Prevent dead code elimination of the sum */
    volatile int result = sum;
}
