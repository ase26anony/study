/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int x, int y) {
    return (x ^ y) + (x & y) * 2;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < m; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables and non-trivial indexing */
            int idx1 = i + j;
            int idx2 = i - 1;
            int idx3 = i * 2 - j;
            
            /* Conditional operations creating control flow */
            if (idx1 % 3 == 0) {
                /* Recurrence: arr1 depends on its previous value */
                int temp = arr1[idx2] + arr2[idx1];
                
                /* Function call creating RTL with latency */
                arr1[i] = helper(temp, arr3[idx3]) + (j * 2);
                
                /* Additional operation with pointer arithmetic */
                *(arr3 + i) = *(arr2 + idx1) ^ *(arr1 + idx2);
            } else {
                /* Alternative path with different recurrence */
                arr1[i] = arr1[idx2] * 2 - arr2[idx1];
                
                /* More complex indexing with modulo */
                arr3[i] = arr2[(idx1 * 7) % n] + arr3[idx2];
                
                /* Another function call */
                arr2[i] = helper(arr1[i], arr3[i]) >> 1;
            }
            
            /* Cross-iteration dependency with multiple arrays */
            if (i > 2) {
                arr3[i] += arr1[i-2] - arr2[i-1];
            }
        }
        
        /* Inter-iteration dependency between outer loop iterations */
        if (j > 0) {
            arr2[0] += arr1[n-1] * 3;
        }
    }
}

/* Main function with initialization and checksum */
int main(int argc, char **argv) {
    /* Medium-sized arrays as required */
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
    
    /* Non-constant trip counts to prevent unrolling */
    int n = (argc > 1) ? 250 : 300;
    int m = (argc > 2) ? 10 : 15;
    
    /* Core computation */
    compute_loop(arr1, arr2, arr3, n, m);
    
    /* Checksum to prevent dead code elimination */
    long sum = 0;
    for (i = 0; i < 500; i++) {
        sum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
    }
    
    /* Print checksum (simplified output) */
    if (sum < 0) {
        return 1; /* Should never happen, but prevents optimization */
    }
    
    return 0;
}
