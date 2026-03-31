/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array with enough elements to avoid trivial loop unrolling */
    int arr[128];
    
    /* Initialize array to prevent optimization of dead stores */
    for (int i = 0; i < 128; i++) {
        arr[i] = i;
    }
    
    /* Core pattern: pointer dereference followed by separate increment */
    int *ptr = arr;
    int *end = arr + 128;
    int sum = 0;
    
    /* Loop with separate dereference and increment operations */
    while (ptr < end) {
        /* Dereference pointer (creates base + 0 memory access) */
        int val = *ptr;
        
        /* Separate increment statement (not combined with dereference) */
        ptr += 1;
        
        /* Use the value to prevent dead code elimination */
        sum += val;
    }
    
    /* Use the result to prevent entire loop elimination */
    arr[0] = sum;
}
