/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array with enough elements to avoid trivial optimization */
    int arr[100];
    
    /* Initialize array to prevent undefined behavior */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Pointer that will be incremented separately from dereference */
    int *ptr = arr;
    int *end = arr + 100;
    
    /* Loop with separate dereference and increment */
    while (ptr < end) {
        /* Critical: Dereference first, creating base+0 access pattern */
        int value = *ptr;
        
        /* Use the value to prevent dead code elimination */
        arr[0] += value;
        
        /* Critical: Increment in separate statement after dereference */
        ptr += 1;
    }
    
    /* Additional loop with post-increment pattern */
    char buffer[64];
    char *p = buffer;
    char *limit = buffer + 64;
    
    /* Initialize buffer */
    for (int i = 0; i < 64; i++) {
        buffer[i] = (char)i;
    }
    
    /* Another pattern: for loop with separate increment */
    for (p = buffer; p < limit; ) {
        char c = *p;      /* base + 0 access */
        buffer[0] ^= c;   /* Use value */
        p = p + 1;        /* Separate increment */
    }
    
    /* Float array to test with different data types */
    float farr[50];
    float *fptr = farr;
    float *fend = farr + 50;
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        farr[i] = (float)i;
    }
    
    /* Loop with float pointer */
    while (fptr < fend) {
        float fval = *fptr;  /* base + 0 */
        farr[0] += fval;     /* Use value */
        fptr++;              /* Post-increment operator in separate statement */
    }
}
