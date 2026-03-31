/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    /* Create a stack array with enough elements to avoid trivial unrolling */
    int arr[128];
    
    /* Initialize array with non-zero values to prevent dead code elimination */
    for (int i = 0; i < 128; i++) {
        arr[i] = i * 3 + 1;
    }
    
    int sum = 0;
    int *ptr = arr;
    int *end = arr + 128;
    
    /* 
     * Critical pattern: Separate dereference and pointer increment
     * This creates a base + 0 memory access followed by distinct increment
     */
    while (ptr < end) {
        /* Memory access with implicit base + 0 offset */
        int value = *ptr;
        
        /* Separate pointer increment statement */
        ptr += 1;
        
        /* Use the value to prevent optimization removal */
        sum += value;
    }
    
    /* Return the sum to ensure the loop has observable effect */
    return sum;
}
