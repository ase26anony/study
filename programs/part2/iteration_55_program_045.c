/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    /* Create a stack array to ensure base address is known */
    int arr[100];
    int sum = 0;
    
    /* Pointer initialization - separate from loop condition */
    int *ptr = arr;
    const int *end = arr + 100;
    
    /* Loop with separate dereference and increment */
    while (ptr < end) {
        /* Critical: Dereference with base+0 pattern */
        int val = *ptr;
        
        /* Critical: Separate increment statement */
        ptr = ptr + 1;
        
        /* Use the value to prevent dead code elimination */
        sum += val;
    }
    
    /* Return result to ensure function has observable effect */
    return sum;
}

/* Alternative implementation with for loop */
int test_func2(void) {
    int arr[50];
    int *p = arr;
    int total = 0;
    
    /* For loop with increment in body, not in header */
    for (int i = 0; i < 50; ) {
        /* Access memory at p + 0 */
        int element = *p;
        
        /* Increment pointer separately */
        p++;
        
        /* Use value */
        total ^= element;  /* XOR to prevent aggressive optimization */
        i++;
    }
    
    return total;
}

/* Test with different data type */
float test_func3(void) {
    float data[64];
    float *iter = data;
    float accumulator = 0.0f;
    
    /* Simple while loop with explicit bounds */
    int count = 0;
    while (count < 64) {
        /* Load from current pointer position */
        float sample = *iter;
        
        /* Move pointer forward by one element */
        iter = iter + 1;
        
        accumulator += sample;
        count++;
    }
    
    return accumulator;
}
