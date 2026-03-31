/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array to ensure base pointer is known at compile time */
    int arr[100];
    
    /* Initialize array to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Pointer that will be incremented separately from dereference */
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* 
     * CRITICAL PATTERN: Separate dereference and increment
     * This creates: mem_insn.mem_loc = address_of_x with reg1_val = 0
     * followed by a distinct increment instruction
     */
    while (ptr < end) {
        /* Dereference with base + 0 addressing */
        int val = *ptr;
        
        /* Separate increment statement - NOT combined with dereference */
        ptr = ptr + 1;
        
        /* Use the value to prevent optimization */
        sum += val;
    }
    
    /* Use result to prevent dead code elimination */
    volatile int result = sum;
}

/* Alternative version with for loop for clearer pattern */
int main(void) {
    /* Stack array with known size */
    float data[64];
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        data[i] = i * 1.5f;
    }
    
    float *p = data;
    float total = 0.0f;
    
    /*
     * Loop where pointer increment is separate from dereference
     * This should generate RTL with:
     * 1. Memory access at (reg_base + 0)
     * 2. Separate increment of reg_base
     */
    for (int i = 0; i < 64; i++) {
        /* Access memory at current pointer location */
        float element = *p;
        
        /* Increment pointer in separate statement */
        p = p + 1;
        
        /* Process value */
        total += element;
    }
    
    /* Prevent optimization */
    volatile float output = total;
    return 0;
}
