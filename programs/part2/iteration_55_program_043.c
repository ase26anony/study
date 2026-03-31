/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    /* Create a stack array with enough elements to avoid trivial unrolling */
    int arr[128];
    
    /* Initialize array with non-zero values to prevent dead code elimination */
    for (int i = 0; i < 128; i++) {
        arr[i] = i + 1;
    }
    
    int sum = 0;
    int *ptr = arr;
    int *end = arr + 128;
    
    /* CRITICAL: Loop with separate dereference and pointer increment */
    while (ptr < end) {
        /* Dereference pointer with base + 0 addressing pattern */
        int value = *ptr;
        
        /* Separate pointer increment - NOT combined with dereference */
        ptr = ptr + 1;
        
        /* Use the value to prevent optimization removal */
        sum += value;
    }
    
    /* Return result to ensure computation isn't optimized away */
    return sum;
}

/* Alternative implementation with for loop for additional coverage */
int test_func2(void) {
    int arr[64];
    
    for (int i = 0; i < 64; i++) {
        arr[i] = i * 2;
    }
    
    int total = 0;
    int *p = arr;
    
    /* For loop with explicit increment in body */
    for (int i = 0; i < 64; i++) {
        int element = *p;  /* Should generate mem_loc = address_of_x, reg1_val = 0 */
        p = p + 1;         /* Separate increment instruction for find_inc to match */
        total ^= element;  /* Non-linear operation to prevent over-optimization */
    }
    
    return total;
}
