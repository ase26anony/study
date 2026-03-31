/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int a, int b) {
    return (a * b) & 0xFF;
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
            
            /* Conditional operations inside loop */
            if (i % 2 == 0) {
                /* Recurrence: reads from previous iteration */
                arr1[i] = helper(arr2[idx1], arr3[idx2]) + arr1[idx2];
                
                /* Additional operation with pointer arithmetic */
                int *ptr = arr2 + i;
                arr3[i] = *ptr + (arr1[i] >> 2);
            } else {
                /* Different arithmetic for odd iterations */
                arr1[i] = arr2[i] * 2 - arr3[idx2];
                arr3[i] = helper(arr1[i], arr2[i]) | 0x1;
            }
            
            /* Another recurrence with different distance */
            if (i > 2) {
                arr2[i] = arr1[i] + arr1[i-2] + arr1[i-3];
            }
        }
        
        /* Modify k slightly each outer iteration */
        k = (k + j) & 0x3;
    }
}

/* Main function with array initialization and checksum */
int main(int argc, char **argv) {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 + 2;
        arr3[i] = i * 7 + 3;
    }
    
    /* Non-constant trip count from argument or default */
    int n = 200;
    if (argc > 1) {
        /* Use argument to prevent constant propagation */
        n = (argv[0][0] % 100) + 100;
    }
    
    /* Offset for complex indexing */
    int k = 5;
    
    /* Core computation */
    compute_loop(arr1, arr2, arr3, n, k);
    
    /* Checksum to prevent dead code elimination */
    long sum = 0;
    for (i = 0; i < 500; i++) {
        sum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum (use system call to avoid stdio includes) */
    /* Simple output for verification */
    if (sum != 0) {
        return 0;  /* Success */
    }
    
    return 1;
}
