/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
static int __attribute__((const)) helper(int x, int y) {
    return (x ^ y) + (x & y) * 2;
}

/* Core computational function with nested loops */
static void compute_loop(int *arr1, int *arr2, int *arr3, 
                         int *arr4, int n, int m, int k) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < m; j++) {
        int offset = j * k;
        
        /* Inner loop with tight recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables */
            int idx1 = i + offset;
            int idx2 = i - 1;
            int idx3 = i + k;
            
            /* Recurrence: arr1 depends on its previous value */
            int base = arr1[idx2];
            
            /* Conditional operations inside loop */
            if (idx1 % 3 == 0) {
                /* Complex operation path 1 */
                int temp = helper(arr2[idx3], arr3[idx2]);
                arr1[idx1] = temp + base * 2;
                arr4[idx1] = arr2[idx3] - arr3[idx2];
            } else if (idx1 % 3 == 1) {
                /* Complex operation path 2 */
                int temp = helper(arr3[idx2], arr2[idx1]);
                arr1[idx1] = temp + base / 2;
                arr4[idx1] = arr3[idx2] * arr2[idx1];
            } else {
                /* Complex operation path 3 */
                int temp = helper(base, arr2[idx1]);
                arr1[idx1] = temp + arr3[idx3];
                arr4[idx1] = base ^ arr3[idx3];
            }
            
            /* Additional recurrence with pointer arithmetic */
            int *ptr = &arr2[idx1];
            *ptr = *ptr + arr1[idx2] + (i & 7);
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        if (j > 0) {
            arr3[offset] = arr1[offset] + arr2[offset - k];
        }
    }
}

/* Main function with initialization and checksum */
int main(int argc, char **argv) {
    /* Use parameters to prevent constant folding */
    int size = (argc > 1) ? 200 : 250;
    int outer = (argc > 2) ? 10 : 15;
    int stride = (argc > 3) ? 3 : 5;
    
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    int arr4[500];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < 500; i++) {
        arr1[i] = i * 2 + 1;
        arr2[i] = i * 3 - 2;
        arr3[i] = i * 5 + 3;
        arr4[i] = i * 7 - 4;
    }
    
    /* Perform computation with nested loops */
    compute_loop(arr1, arr2, arr3, arr4, size, outer, stride);
    
    /* Calculate checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (int i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 3 - arr3[i] + arr4[i] / 2;
        /* Mix in some bit operations */
        checksum ^= (arr1[i] << (i & 15));
    }
    
    /* Print checksum to ensure code is live */
    volatile HOST_WIDE_INT result = checksum;
    
    return (result > 0) ? 0 : 1;
}
