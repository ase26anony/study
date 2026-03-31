/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array to ensure base pointer is in memory */
    int arr[100];
    
    /* Initialize array to prevent optimization removal */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Pointer that will be incremented separately from dereference */
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* CRITICAL: Loop with separate dereference and increment */
    /* This creates the pattern: mem_insn.mem_loc = address_of_x */
    /* with reg1_val = 0, followed by find_inc(true) */
    while (ptr < end) {
        /* Dereference pointer (creates base + 0 access) */
        int val = *ptr;
        
        /* Separate increment statement (not combined with dereference) */
        ptr = ptr + 1;
        
        /* Use the value to prevent dead code elimination */
        sum += val;
    }
    
    /* Use result to prevent entire loop from being optimized away */
    __asm__ volatile ("" : : "r"(sum));
}
