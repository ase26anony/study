/* Test case for modulo-scheduling coverage */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int x, int y) {
    return (x * y) >> 1;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int k) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < 3; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables and non-trivial array indexing */
            int idx1 = i + k;
            int idx2 = i - 1;
            int idx3 = i * 2;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            int base = arr1[idx2] + arr2[idx1];
            
            /* Conditional operations inside loop */
            if (base > 100) {
                /* Complex operation with function call */
                arr1[i] = helper(base, arr3[idx3 % n]) + (i << 2);
                
                /* Pointer arithmetic access */
                int *ptr = arr2 + i;
                arr3[i] = *ptr + arr1[idx2];
            } else {
                /* Different arithmetic path */
                arr1[i] = base - arr3[idx3 % n];
                arr3[i] = arr2[i] * 3;
            }
            
            /* Additional recurrence with multiple dependencies */
            if (i > 2) {
                arr2[i] = arr1[i] + arr1[i-2] + arr2[i-1];
            }
        }
        
        /* Modify k to change indexing pattern */
        k = (k + 7) % 13;
    }
}

/* Main function */
int main() {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < 500; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
        arr3[i] = i * 7;
    }
    
    /* Non-constant trip count from parameter */
    int n = 400;
    int k = 5;
    
    /* Call computational function */
    compute_loop(arr1, arr2, arr3, n, k);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (int i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum (prevents optimization) */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
