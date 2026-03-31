/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
__attribute__((const))
static int helper(int a, int b) {
    return (a * b) >> 1;
}

/* Core computational function with nested loops */
static void compute_loop(int *arr1, int *arr2, int *arr3, int n, int k) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < 3; j++) {
        /* Inner loop with recurrence and complex dependencies */
        for (i = 1; i < n; i++) {
            int idx1 = i + k;
            int idx2 = i - 1;
            int idx3 = i * 2;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            int base = arr1[idx2] + arr2[idx1];
            
            /* Conditional operations creating control flow */
            if (base > 100) {
                /* Complex operation with function call */
                arr1[i] = helper(base, arr3[idx3]) + (arr2[i] >> 2);
            } else {
                /* Different operation path */
                arr1[i] = (base * 3) - arr3[i] + (arr2[idx1] & 0xFF);
            }
            
            /* Additional recurrence with pointer arithmetic */
            int *ptr = arr3 + i;
            *ptr = *(ptr - 1) + arr2[i] * 2;
            
            /* Cross-iteration dependency through arr3 */
            arr2[i] = arr3[idx2] + (i % 8);
        }
        
        /* Modify k slightly each outer iteration */
        k = (k + 1) & 0x7;
    }
}

int main() {
    /* Medium-sized arrays to prevent complete unrolling */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    /* Initialize arrays with non-uniform data */
    for (int i = 0; i < 500; i++) {
        arr1[i] = i * 3;
        arr2[i] = 500 - i;
        arr3[i] = (i * i) & 0xFF;
    }
    
    /* Perform computation with modulo-schedulable inner loop */
    compute_loop(arr1, arr2, arr3, 250, 5);
    
    /* Checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (int i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
    }
    
    /* Print checksum to ensure code execution */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
