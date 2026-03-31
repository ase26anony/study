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
    
    /* CRITICAL: Loop with dereference and increment as separate statements */
    while (ptr < end) {
        /* Dereference pointer (creates base + 0 access pattern) */
        int val = *ptr;
        
        /* Separate increment statement (not combined with dereference) */
        ptr += 1;
        
        /* Use value to prevent dead code elimination */
        sum += val;
    }
    
    /* Additional loop with different type to increase pattern matching chances */
    char buffer[50];
    char *cptr = buffer;
    char *cend = buffer + 50;
    
    /* Initialize buffer */
    for (int i = 0; i < 50; i++) {
        buffer[i] = i;
    }
    
    /* Another loop with the required pattern */
    while (cptr < cend) {
        char cval = *cptr;  /* base + 0 access */
        cptr = cptr + 1;    /* separate increment */
        sum += cval;        /* prevent optimization */
    }
    
    /* Volatile to ensure computation isn't optimized away */
    volatile int result = sum;
}
