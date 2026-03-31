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
            
            /* Conditional operations creating different move dependencies */
            if (i % 2 == 0) {
                /* Path 1: Multiple induction variable usage */
                int temp = helper(arr2[idx1], arr3[i]);
                arr1[i] = temp + arr1[idx2] + (j * 2);
            } else {
                /* Path 2: Different arithmetic pattern */
                int temp = helper(arr3[i], arr2[idx2]);
                arr1[i] = temp - arr1[idx2] + (j * 3);
            }
            
            /* Additional recurrence with pointer arithmetic */
            int *ptr = &arr2[i];
            arr3[i] = *ptr + arr3[i-1];
        }
        
        /* Modify k to change indexing pattern */
        k = (k + 1) % 5;
    }
}

/* Alternate version with different dependency pattern */
static void compute_loop2(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    
    for (j = m; j > 0; j--) {
        /* Loop with multiple inter-iteration dependencies */
        for (i = 2; i < n - 1; i++) {
            int idx_a = i + j;
            int idx_b = i - j;
            
            /* Complex conditional with nested expressions */
            if ((i ^ j) & 1) {
                int val1 = helper(arr1[i-1], arr2[idx_a]);
                int val2 = helper(arr3[i+1], arr1[idx_b]);
                arr2[i] = val1 + val2 + arr2[i-2];
                
                /* Pointer chain dependency */
                int *p1 = arr3 + i;
                int *p2 = p1 - 1;
                arr3[i] = *p1 + *p2;
            } else {
                int val1 = helper(arr2[i], arr3[idx_b]);
                int val2 = helper(arr1[idx_a], arr2[i-1]);
                arr1[i] = val1 - val2 + arr1[i-2];
            }
        }
    }
}

int main() {
    /* Medium-sized arrays to enable modulo-scheduling */
    int arr1[512];
    int arr2[512];
    int arr3[512];
    
    int i;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 512; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    /* Non-constant trip count from function parameter */
    int n = 500;
    int k = 3;
    
    /* First computation */
    compute_loop(arr1, arr2, arr3, n, k);
    
    /* Second computation with different parameters */
    compute_loop2(arr1, arr2, arr3, n, 4);
    
    /* Checksum to prevent dead code elimination */
    long sum = 0;
    for (i = 0; i < 512; i++) {
        sum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum (use system call to avoid stdio dependency) */
    __builtin_printf("Checksum: %ld\n", sum);
    
    return 0;
}
