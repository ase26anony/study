/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array as the base memory region */
    int arr[100];
    
    /* Initialize the array with some values to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Pointer that will be incremented separately from dereference */
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* 
     * CRITICAL PATTERN: Loop with separate dereference and pointer increment
     * This creates the base+0 addressing pattern followed by distinct increment
     */
    while (ptr < end) {
        /* Dereference with base+0 addressing (ptr hasn't been incremented yet) */
        int value = *ptr;
        
        /* Separate pointer increment statement - not combined with dereference */
        ptr = ptr + 1;
        
        /* Use the value to prevent optimization */
        sum += value;
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int sink = sum;
}

/* Alternative implementation with for loop for clarity */
int main(void) {
    /* Stack array with volatile to prevent aggressive optimization */
    volatile int data[50];
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        data[i] = i;
    }
    
    int *p = (int*)data;
    int total = 0;
    
    /*
     * Another pattern: for loop with increment in body
     * This ensures the increment is separate from the memory access
     */
    for (int i = 0; i < 50; ) {
        /* Memory access at p + 0 */
        int element = *p;
        
        /* Separate increment */
        p = p + 1;
        i = i + 1;
        
        total += element;
    }
    
    /* Prevent optimization */
    volatile int result = total;
    return result > 0 ? 0 : 1;
}
