/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* CRITICAL LOOP PATTERN: Separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 100;
    
    while (ptr < end) {
        /* Dereference with base + 0 addressing pattern */
        int val = *ptr;
        
        /* Separate increment statement - NOT combined with dereference */
        ptr += 1;
        
        /* Use the value to prevent dead code elimination */
        sum += val;
    }
    
    return sum;
}
