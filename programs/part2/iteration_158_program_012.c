/* Test case for modulo-scheduling with complex dependencies */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
static HOST_WIDE_INT __attribute__((const)) 
helper_func(HOST_WIDE_INT a, HOST_WIDE_INT b) {
    return (a ^ b) + (a & b) * 2;
}

/* Core computational function with nested loops */
static void compute_loop(HOST_WIDE_INT *arr1, 
                         HOST_WIDE_INT *arr2, 
                         HOST_WIDE_INT *arr3,
                         HOST_WIDE_INT n,
                         HOST_WIDE_INT outer_iter) {
    HOST_WIDE_INT i, j;
    
    /* Outer loop - provides context */
    for (j = 0; j < outer_iter; j++) {
        /* Initialize boundary condition for recurrence */
        if (j == 0) {
            arr1[0] = arr2[0] + arr3[0];
        }
        
        /* Inner loop with complex dependencies - target for modulo-scheduling */
        for (i = 1; i < n; i++) {
            HOST_WIDE_INT idx1, idx2, idx3;
            HOST_WIDE_INT temp1, temp2;
            
            /* Multiple induction variables and non-trivial indexing */
            idx1 = i + j;          /* Forward-looking index */
            idx2 = i - 1;          /* Backward-looking index (creates recurrence) */
            idx3 = i * 2 - j;      /* Complex index expression */
            
            /* Conditional operations creating control flow */
            if ((i ^ j) & 1) {
                /* Path 1: Complex recurrence with function call */
                temp1 = helper_func(arr2[idx1 % n], arr3[i]);
                temp2 = arr1[idx2] * 3 - temp1;
                
                /* Pointer arithmetic access */
                *(arr1 + i) = temp2 + *(arr2 + (idx3 % n));
            } else {
                /* Path 2: Different recurrence pattern */
                temp1 = arr2[i] + arr3[idx2];
                temp2 = helper_func(temp1, arr1[idx2]);
                
                /* Another recurrence: arr1[i] depends on arr1[i-1] */
                *(arr1 + i) = temp2 + (*(arr1 + idx2) >> 1);
            }
            
            /* Additional operation with cross-iteration dependency */
            arr3[i] = arr3[idx2] + (arr1[i] & 0xFF);
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        if (j > 0) {
            arr2[0] = arr1[n-1] - arr2[0];
        }
    }
}

/* Main function */
int main() {
    /* Medium-sized arrays to work with */
    HOST_WIDE_INT arr1[500];
    HOST_WIDE_INT arr2[500];
    HOST_WIDE_INT arr3[500];
    HOST_WIDE_INT i, n = 500;
    HOST_WIDE_INT checksum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < n; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5 + 1;
        arr3[i] = i * 7 + 2;
    }
    
    /* Perform computation with nested loops */
    compute_loop(arr1, arr2, arr3, n, 10);
    
    /* Calculate checksum to prevent dead code elimination */
    for (i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
        /* Mix in some bit operations */
        checksum ^= (checksum << 13) | (checksum >> 19);
    }
    
    /* Print checksum to ensure code is live */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
