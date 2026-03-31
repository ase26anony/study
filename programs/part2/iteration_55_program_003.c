/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void)
{
    /* Create a stack array with enough elements to avoid complete unrolling */
    int arr[100];
    
    /* Initialize array to prevent optimization removal */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Pointer that will be incremented separately from dereference */
    int *ptr = arr;
    int *end = arr + 100;
    
    /* CRITICAL: Loop with separate dereference and increment */
    /* This creates the pattern: mem_insn.mem_loc = address_of_x with reg1_val = 0 */
    while (ptr < end) {
        /* Dereference pointer (creates base + 0 access) */
        int value = *ptr;
        
        /* Separate increment statement (not combined with dereference) */
        ptr += 1;
        
        /* Use value to prevent dead code elimination */
        arr[0] += value;
    }
    
    /* Additional loop with post-increment pattern */
    ptr = arr;
    for (int i = 0; i < 100; i++) {
        /* Another base + 0 access pattern */
        int val = *ptr;
        
        /* Explicit increment in separate statement */
        ptr = ptr + 1;
        
        /* Prevent optimization */
        if (val > 50) {
            arr[1] = val;
        }
    }
}
