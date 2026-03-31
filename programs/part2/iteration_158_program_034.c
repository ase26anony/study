/* Test case for modulo-scheduling coverage */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int a, int b) {
    return (a * b) >> 1;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int k) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < 3; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n - k; i++) {
            /* Multiple induction variables and non-trivial array indexing */
            int idx1 = i + k;
            int idx2 = i - 1;
            
            /* Conditional operations inside loop */
            if (arr2[idx1] > 0) {
                /* Recurrence: arr1[i] depends on arr1[i-1] */
                int temp = arr1[idx2] + arr3[i];
                
                /* Function call with pointer arithmetic */
                arr1[i] = helper(temp, arr2[idx1]) + (j * 2);
                
                /* Additional complex operation */
                arr3[i] = arr3[idx2] + (arr1[i] >> 3);
            } else {
                /* Alternative path with different operations */
                arr1[i] = arr1[idx2] - arr2[idx1];
                
                /* Pointer arithmetic access */
                int *ptr = &arr3[i];
                *ptr = helper(arr1[i], i) - (j * 3);
                
                /* Another recurrence */
                arr2[i] = arr2[idx2] + (*ptr & 0xFF);
            }
            
            /* Cross-iteration dependency with multiple arrays */
            if (i % 2 == 0) {
                arr3[i] = arr1[idx2] + arr2[idx1] + arr3[i];
            } else {
                arr2[i] = arr1[i] - arr3[idx2] + (arr2[idx1] >> 2);
            }
        }
        
        /* Modify k slightly for next outer iteration */
        k = (k + 1) & 3;
    }
}

/* Main function */
int main() {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 2;
        arr2[i] = i % 100;
        arr3[i] = 500 - i;
    }
    
    /* Add some variation to prevent complete optimization */
    arr1[0] = 1;
    arr2[0] = 2;
    arr3[0] = 3;
    
    /* Call computational function with non-constant trip count */
    compute_loop(arr1, arr2, arr3, 400, 5);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum to ensure code is live */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
