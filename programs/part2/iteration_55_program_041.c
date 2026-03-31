/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    /* Create a stack array to ensure base pointer is on stack */
    int arr[100];
    
    /* Initialize array with values to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    int sum = 0;
    int *ptr = arr;
    int *end = arr + 100;
    
    /* 
     * Critical pattern: Separate dereference and increment
     * This creates: MEM[base: ptr, offset: 0] followed by ptr = ptr + 1
     */
    while (ptr < end) {
        /* Dereference with base + 0 addressing */
        int val = *ptr;
        
        /* Separate increment statement - NOT combined with dereference */
        ptr = ptr + 1;
        
        /* Use the value to prevent optimization */
        sum += val;
    }
    
    /* Return result to ensure function has observable effect */
    return sum;
}

/* Alternative version with for loop for clearer pattern */
int test_func2(void) {
    int arr[50];
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        arr[i] = i * 2;
    }
    
    int total = 0;
    int *p = arr;
    
    /* 
     * For loop with separate increment in body
     * Note: increment is in body, not in for() header
     */
    for (int i = 0; i < 50; i++) {
        /* Base + 0 memory access */
        int element = *p;
        
        /* Separate pointer increment */
        p += 1;
        
        total += element;
    }
    
    return total;
}

/* Simple char array version */
int test_func3(void) {
    char buffer[256];
    
    /* Initialize */
    for (int i = 0; i < 256; i++) {
        buffer[i] = (char)(i % 128);
    }
    
    int count = 0;
    char *current = buffer;
    char *limit = buffer + 256;
    
    while (current < limit) {
        /* Access with zero offset */
        char c = *current;
        
        /* Increment separately */
        current = current + 1;
        
        if (c > 64) count++;
    }
    
    return count;
}
