/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array as the base data structure */
    int arr[100];
    
    /* Initialize the array with some values to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Pointer that will be incremented separately from dereference */
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* CRITICAL LOOP PATTERN:
     * 1. Dereference pointer (*ptr) - creates base + 0 memory access
     * 2. Use the value in a separate operation
     * 3. Increment pointer in a separate statement (ptr++)
     * This separation creates distinct RTL instructions that find_inc() needs to match
     */
    while (ptr < end) {
        /* Separate dereference operation - should generate mem_loc = address_of_x with reg1_val = 0 */
        int value = *ptr;
        
        /* Use the value to prevent optimization removal */
        sum += value;
        
        /* Separate pointer increment - creates distinct increment instruction */
        ptr++;
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int result = sum;
}

/* Alternative implementation with for loop for additional coverage */
int main(void) {
    /* Another array with different type */
    char buffer[256];
    
    /* Initialize */
    for (int i = 0; i < 256; i++) {
        buffer[i] = (char)(i % 128);
    }
    
    char *p = buffer;
    char *limit = buffer + 256;
    int count = 0;
    
    /* For loop version with same pattern */
    for (; p < limit; ) {
        /* Dereference first */
        char c = *p;
        
        /* Use the value */
        if (c > 64) count++;
        
        /* Increment separately */
        p += 1;  /* Using p += 1 instead of p++ for variety */
    }
    
    volatile int final_count = count;
    return 0;
}
