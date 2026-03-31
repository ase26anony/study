/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int a, int b) {
    return (a * 3) ^ (b * 7);
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < m; j++) {
        /* Inner loop with modulo-scheduling characteristics */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables and complex indexing */
            int idx1 = i + j;
            int idx2 = i - 1;
            int idx3 = i * 2 - j;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            int base = arr1[idx2];
            
            /* Conditional operations creating control flow */
            if (idx1 % 3 == 0) {
                /* Different arithmetic path */
                arr1[i] = helper(arr2[idx1], arr3[idx3]) + base * 2;
            } else {
                /* Another arithmetic path with pointer arithmetic */
                int *ptr1 = &arr2[idx1];
                int *ptr2 = &arr3[idx3];
                arr1[i] = (*ptr1) + (*ptr2) + base;
            }
            
            /* Additional recurrence with arr3 */
            if (i > 2) {
                arr3[i] = arr3[i-2] + arr2[i] * 3;
            }
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        arr2[j] = arr1[n-1] + j;
    }
}

/* Main function with initialization and checksum */
int main(int argc, char **argv) {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 2 + 1;
        arr2[i] = i * 3 - 2;
        arr3[i] = i * 5 + 7;
    }
    
    /* Non-constant loop bounds to prevent unrolling */
    int n = (argc > 1) ? 250 : 300;
    int m = (argc > 2) ? 10 : 15;
    
    /* Core computation */
    compute_loop(arr1, arr2, arr3, n, m);
    
    /* Checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
    }
    
    /* Print checksum (use simple output) */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
