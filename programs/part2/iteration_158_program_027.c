/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create RTL with latency */
static int __attribute__((const)) helper(int a, int b) {
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
                /* Complex expression with function call */
                arr1[i] = helper(base, arr3[idx3 % n]) + (arr2[i] >> 2);
            } else {
                /* Different arithmetic path */
                arr1[i] = (base * 3) - arr3[i % n] + (k & 0xF);
            }
            
            /* Additional recurrence with pointer arithmetic */
            int *ptr = arr3 + i;
            arr2[i] = *ptr + arr2[idx2] + (i & 7);
            
            /* Cross-iteration dependency through arr3 */
            arr3[i] = arr1[idx2] + arr3[idx1 % n] - (j * 5);
        }
        
        /* Modify k slightly each outer iteration */
        k = (k + j) & 0x1F;
    }
}

/* Main function with initialization and checksum */
int main() {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 500; i++) {
        arr1[i] = i * 2;
        arr2[i] = i + 5;
        arr3[i] = 100 - (i % 7);
    }
    
    /* Set initial recurrence value */
    arr1[0] = 42;
    
    /* Call computational function with non-constant trip count */
    int n = 400;  /* Non-constant prevents unrolling */
    int k = 7;    /* Offset for indexing */
    compute_loop(arr1, arr2, arr3, n, k);
    
    /* Compute checksum to prevent optimization */
    HOST_WIDE_INT checksum = 0;
    for (int i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 3 - arr3[i];
    }
    
    /* Print checksum (prevents dead code elimination) */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
