/* Test case for modulo-scheduling coverage */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int x, int y) {
    return (x * y) >> 1;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < m; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables and non-trivial array indexing */
            int idx1 = i + j;
            int idx2 = i - 1;
            int idx3 = i * 2 - j;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            int base = arr1[idx2];
            
            /* Conditional operations inside loop */
            if (idx1 % 3 == 0) {
                /* Complex operation with function call */
                arr1[i] = helper(arr2[idx1], arr3[idx3]) + base + (j * 2);
            } else if (idx1 % 3 == 1) {
                /* Different arithmetic operation */
                arr1[i] = (arr2[idx1] ^ arr3[idx3]) - base + (j / 2);
            } else {
                /* Pointer arithmetic access */
                int *ptr1 = arr2 + idx1;
                int *ptr2 = arr3 + idx3;
                arr1[i] = (*ptr1) * (*ptr2) + base - j;
            }
            
            /* Additional recurrence with arr3 */
            if (i > 2) {
                arr3[i] = arr3[i-1] + arr3[i-2] + arr1[i-1];
            }
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        if (j > 0) {
            arr2[0] = arr1[n-1] + arr2[0];
        }
    }
}

/* Main function */
int main() {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        arr3[i] = i * 7 + 3;
    }
    
    /* Call computational function with nested loops */
    /* Non-constant trip counts to prevent unrolling */
    int outer_iter = 10;
    int inner_iter = 100;
    
    compute_loop(arr1, arr2, arr3, inner_iter, outer_iter);
    
    /* Compute checksum to prevent optimization */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum (use simple output) */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
