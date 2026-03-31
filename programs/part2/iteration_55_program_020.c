/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    int arr[100];
    int sum = 0;
    
    /* Initialize array with values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* CRITICAL PATTERN: Loop with separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 100;
    
    while (ptr < end) {
        /* Dereference pointer (creates base + 0 access) */
        int val = *ptr;
        
        /* Separate increment statement - NOT combined with dereference */
        ptr = ptr + 1;
        
        /* Use the value to prevent dead code elimination */
        sum += val;
    }
    
    return sum;
}

/* Alternative version with for loop */
int test_func2(void) {
    int arr[100];
    int sum = 0;
    
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Another valid pattern: for loop with separate increment */
    int *ptr = arr;
    int *end = arr + 100;
    
    for (; ptr < end; ) {
        /* Access memory at current pointer */
        int current = *ptr;
        
        /* Increment in separate statement */
        ptr++;
        
        /* Use value */
        sum ^= current;  /* XOR to create non-trivial operation */
    }
    
    return sum;
}

/* Simple char array version */
char test_func3(void) {
    char buffer[256];
    char result = 0;
    
    /* Initialize */
    for (int i = 0; i < 256; i++) {
        buffer[i] = (char)(i % 128);
    }
    
    char *p = buffer;
    char *limit = buffer + 256;
    
    while (p < limit) {
        /* Access at offset 0 */
        char c = *p;
        
        /* Separate increment */
        p = p + 1;
        
        result |= c;  /* Combine values */
    }
    
    return result;
}
