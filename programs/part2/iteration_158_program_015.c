/* Test case for modulo-scheduling coverage */
typedef long HOST_WIDE_INT;

/* Pure helper function to create RTL with latency */
static int __attribute__((const)) helper(int a, int b) {
    return (a * b) >> 1;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int m) {
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
            
            /* Conditional operations */
            if (base > 100) {
                /* Complex operation with function call */
                arr1[i] = helper(base, arr3[idx3]) + (i * 2);
                
                /* Additional pointer arithmetic */
                int *ptr = arr2 + i;
                arr3[i] = *ptr + arr1[i - 1];
            } else {
                /* Different arithmetic path */
                arr1[i] = base - arr3[idx3];
                
                /* More pointer arithmetic */
                int *ptr = arr3 + i;
                arr2[i] = *ptr - arr1[i - 1];
            }
            
            /* Additional recurrence with multiple induction variables */
            arr2[i + 1] = arr1[i] + arr2[i] + (j * 3);
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        arr3[0] = arr1[n - 1] + j;
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
        arr1[i] = i % 100;
        arr2[i] = (i * 2) % 150;
        arr3[i] = (i * 3) % 200;
    }
    
    /* Non-constant trip counts to prevent unrolling */
    int n = 250;
    int m = 100;
    
    /* Core computation */
    compute_loop(arr1, arr2, arr3, n, m);
    
    /* Checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (int i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum (use simple output) */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
