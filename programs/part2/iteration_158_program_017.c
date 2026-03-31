/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create RTL with latency */
static int __attribute__((const)) helper(int x, int y) {
    return (x * y) >> 1;
}

/* Core computational function with nested loops */
static void compute_loop(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < m; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            int idx1 = i + j;
            int idx2 = i - 1;
            int idx3 = i + (j % 4);
            
            /* Recurrence: arr1 depends on its previous value */
            int base = arr1[idx2] + arr2[idx1];
            
            /* Conditional operations creating control flow */
            if (base > 100) {
                /* Complex operation with function call */
                arr1[i] = helper(base, arr3[idx3]) + (arr2[i] >> 2);
            } else {
                /* Different operation path */
                arr1[i] = (base * 3) - arr3[idx3] + (arr2[i] << 1);
            }
            
            /* Additional recurrence with pointer arithmetic */
            int *ptr = &arr3[i];
            *ptr = *(ptr - 1) + arr1[i] + (j * 7);
        }
        
        /* Cross-iteration dependency with varying distance */
        arr2[j % n] = arr1[n - 1] + arr3[j % 8];
    }
}

/* Secondary loop with different pattern to increase scheduling complexity */
static void compute_loop2(int *arr1, int *arr2, int *arr3, int n) {
    int i;
    /* Loop with multiple induction-like variables */
    for (i = 2; i < n - 2; i++) {
        int k = i * 2;
        int m = i / 2;
        
        /* Multiple interleaved recurrences */
        int val1 = arr1[i - 2] + arr2[k];
        int val2 = arr3[i + 1] - arr1[m];
        
        /* Nested conditional with function calls */
        if (val1 > val2) {
            arr1[i] = helper(val1, i) + arr2[i + 1];
            arr3[i] = arr1[i - 1] * 3;
        } else if (val1 < val2) {
            arr1[i] = helper(val2, i) - arr2[i - 1];
            arr3[i] = arr1[i + 1] / 2;
        } else {
            arr1[i] = val1 + val2 + helper(i, 7);
            arr3[i] = arr2[i] ^ arr1[i];
        }
        
        /* Pointer arithmetic with offset */
        int *p1 = arr1 + i;
        int *p2 = arr2 + (i % 16);
        *p2 = *p1 + *(p1 - 1);
    }
}

int main() {
    /* Medium-sized arrays to enable modulo-scheduling */
    int arr1[512];
    int arr2[512];
    int arr3[512];
    
    int i;
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < 512; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        arr3[i] = i * 7 + 3;
    }
    
    /* Non-constant trip counts to prevent unrolling */
    int outer_iter = 8;
    int inner_size = 256;
    
    /* Call computational functions multiple times */
    compute_loop(arr1, arr2, arr3, inner_size, outer_iter);
    compute_loop2(arr1, arr2, arr3, inner_size);
    compute_loop(arr2, arr3, arr1, inner_size / 2, outer_iter * 2);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 512; i++) {
        checksum += arr1[i] + arr2[i] * 2 - arr3[i];
    }
    
    /* Print checksum to ensure code execution */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
