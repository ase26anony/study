/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
static int __attribute__((const)) helper(int x, int y) {
    return (x * y) >> 1;
}

/* Another helper with different operation */
static int __attribute__((const)) helper2(int x) {
    return x + (x & 0xFF);
}

/* Core computational function with nested loops */
static void compute_loop(int *arr1, int *arr2, int *arr3, int n, int k) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < 3; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            int idx1 = i + k;
            int idx2 = i - 1;
            int idx3 = i * 2;
            
            /* Conditional operations creating control flow */
            if (i % 3 == 0) {
                /* Path 1: Multiple dependencies with helper calls */
                int temp1 = helper(arr2[idx1], arr3[idx2]);
                int temp2 = helper2(arr1[idx2]);
                arr1[i] = temp1 + temp2 + arr1[idx2];
                
                /* Additional recurrence */
                arr3[i] = arr3[idx2] + (arr2[idx1] >> 2);
            } else if (i % 3 == 1) {
                /* Path 2: Different operations with pointer arithmetic */
                int *ptr1 = &arr2[idx1];
                int *ptr2 = &arr3[idx2];
                arr1[i] = *ptr1 + *ptr2 + helper(arr1[idx2], i);
                arr3[i] = arr2[idx1] - arr3[idx2];
            } else {
                /* Path 3: More complex recurrence chain */
                int val1 = arr2[idx1] + arr1[idx2];
                int val2 = helper2(arr3[idx2]);
                arr1[i] = val1 * 2 - val2;
                arr3[i] = arr1[i] + arr2[idx1 % n];
            }
            
            /* Cross-iteration dependency with multiple induction variables */
            if (i > 2) {
                arr2[i] = arr1[i] + arr1[i-1] + arr1[i-2] + 
                         helper(arr3[i-1], arr3[i-2]);
            }
        }
        
        /* Modify k slightly each outer iteration */
        k = (k + 1) % 5;
    }
}

/* Main function with initialization and checksum */
int main() {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5 + 1;
        arr3[i] = i * 7 + 2;
    }
    
    /* Initial values for recurrence */
    arr1[0] = 1;
    arr2[0] = 2;
    arr3[0] = 3;
    
    /* Call computational function with non-constant trip count */
    compute_loop(arr1, arr2, arr3, 400, 3);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i];
        checksum += arr2[i];
        checksum += arr3[i];
    }
    
    /* Print checksum (prevents optimization) */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
