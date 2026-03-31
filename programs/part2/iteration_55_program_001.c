/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void)
{
    /* Create a stack array as the base memory region */
    int arr[100];
    
    /* Initialize with some values to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    int sum = 0;
    
    /* CRITICAL: Pointer increment in separate statement after dereference */
    int *ptr = arr;
    int *end = arr + 100;
    
    while (ptr < end) {
        /* Base + 0 memory access pattern */
        int val = *ptr;      /* This should generate mem_loc = address_of_x, reg1_val = 0 */
        
        /* Separate increment statement - not combined with dereference */
        ptr = ptr + 1;       /* This creates the increment for find_inc() to match */
        
        /* Use the value to prevent optimization */
        sum += val;
    }
    
    /* Additional loop with different syntax but same pattern */
    float farr[50];
    float *fptr = farr;
    float fsum = 0.0f;
    
    for (int i = 0; i < 50; ) {
        float fval = *fptr;   /* Another base + 0 access */
        fptr++;               /* Post-increment in separate statement */
        fsum += fval;
        i++;
    }
    
    /* Return computed value to ensure code isn't eliminated */
    return sum + (int)fsum;
}

/* Alternative main function for standalone compilation */
int main(void)
{
    return test_func();
}
