/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    /* Create a stack array with enough elements to avoid complete unrolling */
    int arr[100];
    
    /* Initialize array with non-zero values to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2 + 1;
    }
    
    int sum = 0;
    
    /* CRITICAL PATTERN: Separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 100;
    
    while (ptr < end) {
        /* Dereference with base + 0 addressing */
        int value = *ptr;
        
        /* Separate increment statement - creates distinct RTL instructions */
        ptr = ptr + 1;
        
        /* Use the value to prevent optimization */
        sum += value;
    }
    
    /* Return result to prevent dead code elimination */
    return sum;
}

/* Alternative implementation with for loop for clarity */
int test_func2(void) {
    int arr[50];
    
    for (int i = 0; i < 50; i++) {
        arr[i] = i * 3;
    }
    
    int sum = 0;
    int *ptr = arr;
    
    /* Another valid pattern: for loop with separate increment */
    for (int i = 0; i < 50; i++) {
        /* Dereference */
        int val = *ptr;
        
        /* Separate increment */
        ptr++;
        
        /* Use value */
        sum += val;
    }
    
    return sum;
}

/* Main wrapper if needed */
int main(void) {
    return test_func() + test_func2();
}
