/* Test case for modulo-scheduling with complex dependencies */
typedef long HOST_WIDE_INT;

/* Pure helper function to create RTL with latency */
static int __attribute__((const)) helper(int a, int b) {
    return (a * b) >> 1;
}

/* Core computational function with nested loops */
static void compute_loop(int *arr1, int *arr2, int *arr3, 
                         int *arr4, int size, int offset) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < 10; j++) {
        /* Inner loop with modulo-scheduling characteristics */
        for (i = 1; i < size - 1; i++) {
            int idx1 = i + offset;
            int idx2 = i - 1;
            int idx3 = i + j;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            int base = arr1[idx2] + arr2[idx1];
            
            /* Conditional operations creating control flow */
            if (base > 100) {
                /* Complex operation with helper call */
                arr1[i] = helper(base, arr3[idx3]) + arr4[i];
                
                /* Additional recurrence with different distance */
                arr3[i] = arr2[idx1] * 2 - arr3[idx2];
            } else {
                /* Alternative path with pointer arithmetic */
                int *ptr1 = &arr2[idx1];
                int *ptr2 = &arr3[idx3];
                arr1[i] = (*ptr1) + (*ptr2) / 2;
                
                /* Another recurrence */
                arr4[i] = arr4[idx2] + (*ptr1);
            }
            
            /* Cross-iteration dependency with multiple induction variables */
            arr2[i] = arr1[idx2] + arr3[i] + (i % 8);
        }
        
        /* Modify offset for next outer iteration */
        offset = (offset + j) % 7;
    }
}

/* Main function */
int main() {
    /* Declare and initialize arrays */
    int arr1[512];
    int arr2[512];
    int arr3[512];
    int arr4[512];
    
    int i;
    for (i = 0; i < 512; i++) {
        arr1[i] = i * 2;
        arr2[i] = i + 5;
        arr3[i] = i % 20;
        arr4[i] = 100 - i;
    }
    
    /* Perform computation with non-constant trip count */
    int size = 500;
    int offset = 3;
    compute_loop(arr1, arr2, arr3, arr4, size, offset);
    
    /* Checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 512; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + arr4[i];
    }
    
    /* Print checksum (use simple output) */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
