/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    /* Create a stack array with enough elements to avoid complete unrolling */
    int arr[128];
    int sum = 0;
    
    /* Initialize array to prevent optimization of dead stores */
    for (int i = 0; i < 128; i++) {
        arr[i] = i;
    }
    
    /* CRITICAL PATTERN: Loop with separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 128;
    
    while (ptr < end) {
        /* Dereference pointer with base + 0 addressing pattern */
        int val = *ptr;
        
        /* Separate increment statement - NOT combined with dereference */
        ptr = ptr + 1;
        
        /* Use the value to prevent dead code elimination */
        sum += val;
    }
    
    return sum;
}

/* Alternative version with for loop for clarity */
int test_func2(void) {
    int arr[64];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        arr[i] = i * 2;
    }
    
    /* Another valid pattern: for loop with separate increment */
    int *p = arr;
    for (int i = 0; i < 64; i++) {
        /* Access memory at p + 0 */
        int element = *p;
        
        /* Increment in separate statement */
        p += 1;
        
        sum ^= element;  /* Different operation to avoid pattern merging */
    }
    
    return sum;
}
