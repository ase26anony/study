/* Test case for modulo-scheduler coverage of lines 596-606 in modulo-sched.cc
   This creates complex dependencies requiring modulo-scheduling analysis. */

typedef long HOST_WIDE_INT;

/* Pure helper function to create RTL with latency */
static int __attribute__((const)) helper_func(int a, int b) {
    return (a * b) >> 1;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int k) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < 3; j++) {
        /* Inner loop with recurrence and complex dependencies */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables and non-trivial indexing */
            int idx1 = i + k;
            int idx2 = i - 1;
            int idx3 = i * 2 - k;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            int base = arr1[idx2] + arr2[idx1];
            
            /* Conditional operations creating control flow */
            if (base > 0) {
                /* Complex operation with function call */
                arr1[i] = helper_func(base, arr3[idx3]) + (i % 4);
                
                /* Pointer arithmetic access */
                int *ptr = arr2 + i;
                arr3[i] = *ptr + arr1[idx2];
            } else {
                /* Alternative path with different operations */
                arr1[i] = arr2[idx1] - arr3[idx3];
                arr3[i] = helper_func(arr1[idx2], i);
            }
            
            /* Additional recurrence with offset */
            if (i > 2) {
                arr2[i] = arr1[i] + arr1[i-2] + arr2[i-1];
            }
        }
        
        /* Modify k for next outer iteration */
        k = helper_func(k, j + 1);
    }
}

int main() {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < 500; i++) {
        arr1[i] = i * 2;
        arr2[i] = i + 1;
        arr3[i] = 500 - i;
    }
    
    /* Set initial recurrence seed */
    arr1[0] = 1;
    
    /* Call computational function with non-constant trip count */
    compute_loop(arr1, arr2, arr3, 450, 5);
    
    /* Compute checksum to prevent optimization */
    HOST_WIDE_INT checksum = 0;
    for (int i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum (prevents dead code elimination) */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
