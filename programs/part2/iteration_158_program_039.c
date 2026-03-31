/* Test case for modulo-scheduling with complex dependencies */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
__attribute__((const))
static int helper(int x, int y) {
    return (x * 3 + y * 7) & 0xFF;
}

/* Core computational function with nested loops */
static void compute_loop(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < m; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables and non-trivial array indexing */
            int idx1 = i + j;
            int idx2 = i - 1;
            int idx3 = i * 2 - j;
            
            /* Conditional operations inside loop */
            if ((i ^ j) & 1) {
                /* Recurrence: arr1[i] depends on arr1[i-1] */
                int temp = arr1[idx2] + arr2[idx1];
                
                /* Function call creating instruction with latency */
                arr1[i] = helper(temp, arr3[idx3]) + (i % 8);
                
                /* Pointer arithmetic access */
                int *ptr = arr3 + idx3;
                arr2[i] = *ptr + arr1[idx2];
            } else {
                /* Different arithmetic operation in else branch */
                arr1[i] = arr2[idx1] - arr3[idx3] + (j % 4);
                arr3[i] = arr1[idx2] * 2 - arr2[i];
            }
            
            /* Additional recurrence with different distance */
            if (i > 2) {
                arr3[i] += arr1[i-2] + arr2[i-1];
            }
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        if (j > 0) {
            arr1[0] = arr2[n-1] + arr3[0];
        }
    }
}

/* Main function with initialization and checksum */
int main() {
    /* Medium-sized arrays */
    int arr1[512];
    int arr2[512];
    int arr3[512];
    
    int i;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 512; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5 + 1;
        arr3[i] = i * 7 + 2;
    }
    
    /* Call computational function with non-constant trip counts */
    compute_loop(arr1, arr2, arr3, 500, 10);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 512; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
    }
    
    /* Print checksum (use simple output to avoid stdio.h) */
    volatile HOST_WIDE_INT *output = (volatile HOST_WIDE_INT *)&checksum;
    
    return (int)(checksum & 0x7FFFFFFF);
}
