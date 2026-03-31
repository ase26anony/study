/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    /* Create a stack array to ensure base pointer is in memory */
    int arr[100];
    
    /* Initialize array with values to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    int sum = 0;
    int *ptr = arr;
    int *end = arr + 100;
    
    /* 
     * CRITICAL: Separate dereference and increment operations
     * This creates the pattern: mem_insn.mem_loc = address_of_x with reg1_val = 0
     * followed by a distinct increment instruction
     */
    while (ptr < end) {
        /* Dereference pointer with base + 0 addressing */
        int value = *ptr;
        
        /* Separate increment statement - not combined with dereference */
        ptr = ptr + 1;
        
        /* Use the value to prevent optimization */
        sum += value;
    }
    
    /* Return result to ensure function has observable effect */
    return sum;
}

/* Alternative implementation with for loop for additional coverage */
int test_func2(void) {
    int arr[50];
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        arr[i] = i * 2;
    }
    
    int total = 0;
    int *p = arr;
    
    /* 
     * For loop with separate increment in body
     * This should also generate the base + 0 pattern
     */
    for (int i = 0; i < 50; i++) {
        /* Access memory with zero offset */
        int val = *p;
        
        /* Increment in separate statement */
        p += 1;
        
        total += val;
    }
    
    return total;
}
