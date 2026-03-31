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
            int idx1 = i + j;
            int idx2 = i - 1;
            int idx3 = i + (j % 4);
            
            /* Recurrence: arr1 depends on its previous value */
            int base = arr1[idx2] + arr2[idx1];
            
            /* Conditional operations creating control flow */
            if (idx1 % 3 == 0) {
                /* Complex operation with function call */
                arr1[i] = helper(base, arr3[idx3]) + (j * 2);
            } else if (idx1 % 3 == 1) {
                /* Different operation path */
                arr1[i] = base - arr3[idx3] + (i * 3);
            } else {
                /* Third operation path with pointer arithmetic */
                int *ptr = &arr3[idx3];
                arr1[i] = base + *ptr + (i + j);
            }
            
            /* Additional recurrence with multiple induction variables */
            arr2[i] = arr1[i] + arr2[i-1] + (j % 5);
            
            /* Complex array indexing with modulo */
            arr3[(i * j) % n] = arr1[i] * 2 - arr2[idx2];
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        arr1[0] = arr2[n-1] + j;
    }
}

/* Main function with initialization and checksum */
int main(int argc, char **argv) {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 2 + 1;
        arr2[i] = i * 3 - 2;
        arr3[i] = i % 7 + 5;
    }
    
    /* Non-constant trip counts to prevent unrolling */
    int n = (argc > 1) ? 250 : 300;
    int m = (argc > 2) ? 10 : 15;
    
    /* Core computation */
    compute_loop(arr1, arr2, arr3, n, m);
    
    /* Checksum to prevent optimization */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 - arr3[i];
    }
    
    /* Print checksum to ensure code is live */
    printf("Checksum: %ld\n", (long)checksum);
    
    return 0;
}
