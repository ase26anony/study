/* Test case for modulo-scheduling coverage */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
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
            /* Multiple induction variables and non-trivial array indexing */
            int idx1 = i + j;
            int idx2 = i - 1;
            int idx3 = i * 2;
            
            /* Conditional operations inside loop */
            if (idx1 % 3 == 0) {
                /* Recurrence: arr1[i] depends on arr1[i-1] */
                int temp = arr1[idx2] + arr2[idx1];
                
                /* Function call with pointer arithmetic */
                arr1[i] = helper(temp, arr3[idx3 % n]) + (j * 2);
                
                /* Additional operation with complex indexing */
                arr3[i] = arr2[idx1] - arr1[idx2];
            } else {
                /* Alternative path with different recurrence pattern */
                int temp = arr1[idx2] * 2 - arr2[idx1];
                
                /* Different function call pattern */
                arr1[i] = helper(temp, arr3[(idx3 + 1) % n]) / (j + 1);
                
                /* More complex indexing */
                arr3[i] = arr2[(idx1 + 2) % n] + arr1[idx2];
            }
            
            /* Additional recurrence with multiple dependencies */
            if (i > 2) {
                arr2[i] = arr1[i] + arr1[i-1] + arr1[i-2] + 
                         arr3[(i + j) % n];
            }
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        if (j > 0) {
            arr1[0] = arr1[n-1] + arr2[0];
        }
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
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 2 + 5;
        arr3[i] = i * 4 + 3;
    }
    
    /* Non-constant trip counts to prevent unrolling */
    int n = (argc > 1) ? 250 : 300;
    int m = (argc > 2) ? 10 : 15;
    
    /* Core computation */
    compute_loop(arr1, arr2, arr3, n, m);
    
    /* Calculate checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum to ensure code is live */
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
