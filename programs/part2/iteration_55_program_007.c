/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    /* Create a stack array as the base data structure */
    int arr[100];
    
    /* Initialize array to prevent optimization removal */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Pointer that will be incremented separately from dereference */
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* 
     * CRITICAL PATTERN: Loop with separate dereference and increment
     * This creates: mem_insn.mem_loc = address_of_x with reg1_val = 0
     * followed by a distinct increment instruction for find_inc(true) to match
     */
    while (ptr < end) {
        /* Dereference pointer with implicit base + 0 offset */
        int value = *ptr;
        
        /* Separate increment statement - NOT combined with dereference */
        ptr = ptr + 1;
        
        /* Use value to prevent dead code elimination */
        sum += value;
    }
    
    /* Use sum to prevent entire loop from being optimized away */
    if (sum < 0) {
        __builtin_unreachable();
    }
}

/* Alternative version with for loop for clarity */
int main(void) {
    /* Another stack array */
    float data[50];
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        data[i] = i * 1.5f;
    }
    
    float *p = data;
    float total = 0.0f;
    
    /* 
     * For loop variant with explicit separate increment
     * The pointer increment is in the loop body, not the for-header
     */
    for (int count = 0; count < 50; ) {
        /* Access memory at p + 0 */
        float element = *p;
        
        /* Separate increment */
        p = p + 1;
        count = count + 1;
        
        total += element;
    }
    
    /* Prevent optimization */
    if (total < 0.0f) {
        return 1;
    }
    
    return 0;
}
