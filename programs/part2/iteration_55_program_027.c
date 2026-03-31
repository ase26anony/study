/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    /* Create a stack array to ensure base pointer is in memory */
    int arr[100];
    
    /* Initialize array to prevent optimization removal */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    int sum = 0;
    int *ptr = arr;
    int *end = arr + 100;
    
    /* CRITICAL: Loop with separate dereference and increment */
    while (ptr < end) {
        /* Dereference pointer (creates base + 0 access pattern) */
        int val = *ptr;
        
        /* Separate increment statement - NOT combined with dereference */
        ptr += 1;
        
        /* Use the value to prevent dead code elimination */
        sum += val;
    }
    
    /* Return result to ensure function has observable effect */
    return sum;
}

/* Alternative version with for loop for additional coverage */
int test_func2(void) {
    int arr[50];
    
    for (int i = 0; i < 50; i++) {
        arr[i] = i * 2;
    }
    
    int sum = 0;
    int *ptr = arr;
    
    /* For loop with separate increment in body */
    for (int i = 0; i < 50; i++) {
        /* Dereference with zero offset */
        int val = *ptr;
        
        /* Increment in separate statement */
        ptr = ptr + 1;
        
        sum += val;
    }
    
    return sum;
}

/* Main wrapper for standalone compilation */
int main(void) {
    int result1 = test_func();
    int result2 = test_func2();
    return result1 + result2 - 3675; /* Expected sum: 4950 + 2450 = 7400 */
}
