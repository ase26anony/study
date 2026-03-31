/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array with sufficient size to avoid trivial optimization */
    int arr[128];
    
    /* Initialize array to prevent undefined behavior */
    for (int i = 0; i < 128; i++) {
        arr[i] = i;
    }
    
    /* Pointer traversal with separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 128;
    int sum = 0;
    
    /* CRITICAL: Loop with separate dereference and increment operations */
    while (ptr < end) {
        /* Dereference pointer (creates base + 0 memory access) */
        int value = *ptr;
        
        /* Separate increment statement (not combined with dereference) */
        ptr += 1;
        
        /* Use the value to prevent dead code elimination */
        sum += value;
    }
    
    /* Additional loop with different type to increase pattern matching chances */
    char buffer[64];
    char *cptr = buffer;
    char *cend = buffer + 64;
    
    /* Initialize buffer */
    for (int i = 0; i < 64; i++) {
        buffer[i] = (char)i;
    }
    
    /* Another loop with the same pattern */
    while (cptr < cend) {
        char c = *cptr;  /* Dereference */
        cptr = cptr + 1; /* Separate increment */
        sum += c;        /* Use value */
    }
    
    /* Prevent the entire function from being optimized away */
    volatile int dummy = sum;
}
