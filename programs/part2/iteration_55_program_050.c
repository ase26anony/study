/* { dg-do compile } */
/* { dg-options "-O1" } */

int test_func(void) {
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Critical loop pattern: separate dereference and increment */
    int *ptr = arr;
    int *end = arr + 100;
    
    while (ptr < end) {
        /* Dereference with base + 0 addressing pattern */
        int val = *ptr;
        
        /* Separate increment statement - NOT combined with dereference */
        ptr += 1;
        
        /* Use the value to prevent dead code elimination */
        sum += val;
    }
    
    return sum;
}

/* Alternative version with for loop for redundancy */
int test_func2(void) {
    float farr[50];
    float total = 0.0f;
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        farr[i] = i * 0.5f;
    }
    
    /* Another loop with the required pattern */
    float *fptr = farr;
    float *fend = farr + 50;
    
    for (; fptr < fend; ) {
        /* Access memory at base + 0 offset */
        float element = *fptr;
        
        /* Separate increment */
        fptr = fptr + 1;
        
        total += element;
    }
    
    return (int)total;
}
