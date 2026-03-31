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
    
    /* CRITICAL: Dereference and increment as separate statements */
    while (ptr < end) {
        /* Memory access with base + 0 pattern */
        int val = *ptr;      /* This should generate mem_insn with reg1_val = 0 */
        
        /* Separate increment statement - not combined with dereference */
        ptr = ptr + 1;       /* Or ptr++ as a separate statement */
        
        /* Use the value to prevent dead code elimination */
        sum += val;
    }
    
    /* Prevent the entire function from being optimized away */
    volatile int dummy = sum;
}

/* Alternative version with for loop for clarity */
int main(void) {
    /* Stack array ensures base pointer is known */
    float data[64];
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        data[i] = i * 1.5f;
    }
    
    float total = 0.0f;
    float *p = data;
    float *limit = data + 64;
    
    /* Loop with explicit separation of access and increment */
    for (; p < limit; ) {
        /* Base + 0 memory access */
        float element = *p;
        
        /* Increment in loop update would be combined, so do it here */
        p = p + 1;  /* Constant stride of 1 */
        
        /* Use value */
        total += element;
    }
    
    /* Return value to ensure function isn't eliminated */
    return (int)total;
}
