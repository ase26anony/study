/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    /* Create a stack array to ensure base pointer is on stack */
    int arr[100];
    
    /* Initialize array with non-zero values to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    int sum = 0;
    int *ptr = arr;          /* Base pointer */
    int *end = arr + 100;    /* End pointer */
    
    /* 
     * CRITICAL PATTERN: Separate dereference and increment
     * This creates distinct RTL instructions for:
     * 1. Memory access with base + 0 offset
     * 2. Pointer increment instruction
     */
    while (ptr < end) {
        /* Dereference with base + 0 addressing */
        int value = *ptr;
        
        /* Separate increment statement - NOT combined with dereference */
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
    
    for (int i = 0; i < 50; i++) {
        arr[i] = i + 1;
    }
    
    int total = 0;
    int *p = arr;
    int *limit = arr + 50;
    
    /* Another pattern: for loop with separate increment */
    for (; p < limit; ) {
        /* Access memory at p + 0 */
        int elem = *p;
        
        /* Increment in separate statement */
        p += 1;
        
        total += elem;
    }
    
    return total;
}

/* Main function to compile both test patterns */
int main(void) {
    int result1 = test_func();
    int result2 = test_func2();
    return result1 + result2;
}
