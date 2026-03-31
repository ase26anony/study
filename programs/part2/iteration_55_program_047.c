/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array as the base memory region */
    int arr[100];
    
    /* Initialize pointer to base of array */
    int *ptr = arr;
    
    /* Define end pointer for loop boundary */
    int *end = arr + 100;
    
    /* Loop with separate dereference and increment */
    while (ptr < end) {
        /* Dereference pointer with base + 0 pattern */
        int value = *ptr;
        
        /* Separate increment statement - critical for pattern matching */
        ptr = ptr + 1;
        
        /* Use value to prevent dead code elimination */
        arr[0] += value;
    }
}
