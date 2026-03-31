/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create RTL with latency */
static int __attribute__((const)) helper(int x, int y) {
    return (x * y) >> 1;
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
                arr1[i] = helper(base, arr3[idx3 % n]) + (i & 0xF);
                
                /* Additional recurrence with pointer arithmetic */
                int *ptr = arr2 + i;
                arr3[i] = *ptr + arr3[idx2];
            } else {
                /* Different operation path */
                arr1[i] = base - arr3[i % n];
                arr3[i] = arr2[idx1] >> 2;
            }
            
            /* Multiple induction variable updates */
            arr2[i] = arr1[i] + (k * i);
        }
        
        /* Modify k slightly each outer iteration */
        k = (k + j) & 0x7;
    }
}

/* Main function */
int main() {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    /* Initialize arrays */
    for (int i = 0; i < 500; i++) {
        arr1[i] = i * 2;
        arr2[i] = i + 5;
        arr3[i] = i * 3;
    }
    
    /* Set initial recurrence value */
    arr1[0] = 42;
    arr3[0] = 17;
    
    /* Call computational function with non-constant trip count */
    int n = 250;  /* Non-constant inner loop bound */
    int k = 10;   /* Offset for indexing */
    compute_loop(arr1, arr2, arr3, n, k);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (int i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
    }
    
    /* Print checksum (prevents optimization) */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
