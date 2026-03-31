/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    /* Create a stack array with enough elements to avoid complete unrolling */
    int arr[128];
    
    /* Initialize array with non-zero values to prevent dead code elimination */
    for (int i = 0; i < 128; i++) {
        arr[i] = i * 3 + 1;
    }
    
    int sum = 0;
    
    /* CRITICAL PATTERN: Separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 128;
    
    while (ptr < end) {
        /* Dereference with base + 0 addressing pattern */
        int value = *ptr;
        
        /* Separate increment statement - creates distinct RTL instructions */
        ptr = ptr + 1;
        
        /* Use the value to prevent optimization removal */
        sum += value;
    }
    
    /* Return result to ensure computation isn't optimized away */
    return sum;
}

/* Alternative version with for loop for additional coverage */
int test_func2(void) {
    float farr[64];
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        farr[i] = i * 1.5f;
    }
    
    float total = 0.0f;
    float *fptr = farr;
    
    /* Another pattern: for loop with separate increment */
    for (int i = 0; i < 64; i++) {
        float val = *fptr;  /* base + 0 access */
        fptr = fptr + 1;    /* separate increment */
        total += val;
    }
    
    return (int)total;
}
