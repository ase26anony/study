/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a local array to ensure stack-based addressing */
    int arr[100];
    
    /* Initialize array with non-zero values to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    
    /* Pointer that will be incremented separately from dereference */
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* 
     * CRITICAL PATTERN: Separate dereference and increment
     * This creates distinct instructions in RTL:
     * 1. Memory access with base + 0 offset
     * 2. Separate increment instruction
     */
    while (ptr < end) {
        /* Dereference with implicit base + 0 addressing */
        int value = *ptr;
        
        /* Separate increment statement - not combined with dereference */
        ptr += 1;
        
        /* Use the value to prevent optimization removal */
        sum += value;
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int result = sum;
    
    /* Additional test case with post-increment in for loop */
    {
        float farr[50];
        float *fptr = farr;
        float fsum = 0.0f;
        
        /* Initialize */
        for (int i = 0; i < 50; i++) {
            farr[i] = i * 1.5f;
        }
        
        /* Another pattern: for loop with separate increment */
        for (fptr = farr; fptr < farr + 50; ) {
            float fval = *fptr;  /* base + 0 access */
            fptr = fptr + 1;     /* separate increment */
            fsum += fval;
        }
        
        volatile float fresult = fsum;
    }
}
