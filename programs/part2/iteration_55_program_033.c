/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array to ensure base pointer is in memory */
    int arr[100];
    
    /* Initialize array with some values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Pointer that will be incremented separately from dereference */
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* 
     * CRITICAL PATTERN: Separate dereference and increment
     * This creates: mem_insn.mem_loc = address_of_x with reg1_val = 0
     * followed by a distinct increment instruction
     */
    while (ptr < end) {
        /* Dereference with base + 0 addressing */
        int val = *ptr;
        
        /* Separate increment statement (not *ptr++) */
        ptr = ptr + 1;
        
        /* Use the value to prevent dead code elimination */
        sum += val;
    }
    
    /* Use sum to prevent entire loop from being optimized away */
    volatile int sink = sum;
}
