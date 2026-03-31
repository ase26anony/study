/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array as the base data structure */
    int arr[100];
    
    /* Initialize array with non-zero values to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    
    /* Pointer that will be incremented separately from dereference */
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* CRITICAL: Loop with separate dereference and increment operations */
    /* This creates the pattern: mem_insn.mem_loc = address_of_x with reg1_val = 0 */
    while (ptr < end) {
        /* Dereference pointer (creates base + 0 access pattern) */
        int value = *ptr;
        
        /* Separate increment statement (not combined with dereference) */
        ptr += 1;
        
        /* Use the value to prevent optimization removal */
        sum += value;
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int result = sum;
    (void)result;
}
