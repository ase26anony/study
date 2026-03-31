/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array with known size */
    int arr[100];
    
    /* Initialize array with some values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Pointer-based loop with separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* CRITICAL: Dereference and increment in separate statements */
    while (ptr < end) {
        /* Memory access with base + 0 pattern */
        int value = *ptr;  /* This should generate mem_loc = address_of_x, reg1_val = 0 */
        
        /* Separate pointer increment - not combined with dereference */
        ptr = ptr + 1;     /* This creates the increment instruction for find_inc() to match */
        
        /* Use the value to prevent dead code elimination */
        sum += value;
    }
    
    /* Additional loop with alternative syntax */
    float farr[50];
    float *fptr = farr;
    float *fend = farr + 50;
    float fsum = 0.0f;
    
    /* Another pattern: for loop with increment in body */
    for (fptr = farr; fptr < fend; ) {
        float fval = *fptr;    /* base + 0 access */
        fptr++;                /* separate increment statement */
        fsum += fval;
    }
    
    /* Prevent optimization of unused results */
    volatile int dummy = sum;
    volatile float fdummy = fsum;
}
