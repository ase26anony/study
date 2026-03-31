/* Test case for modulo-scheduler coverage of debug printing block */
/* Compile with: -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms */

typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
static HOST_WIDE_INT __attribute__((const)) 
compute_offset(HOST_WIDE_INT x, HOST_WIDE_INT y) {
    return (x ^ y) + (x & y) * 2;
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
        /* Initialize boundary values */
        if (j == 0) {
            arr1[0] = arr2[0] + arr3[0];
            arr1[1] = arr2[1] * arr3[1];
        }
        
        /* Inner loop - target for modulo scheduling */
        /* Complex recurrence with multiple dependencies */
        for (i = 2; i < n - 2; i++) {
            HOST_WIDE_INT idx1, idx2, idx3;
            HOST_WIDE_INT temp1, temp2;
            
            /* Multiple induction variables and non-trivial indexing */
            idx1 = i + j;
            idx2 = i - j;
            idx3 = compute_offset(i, j);  /* Function call creates RTL with latency */
            
            /* Conditional operations inside loop */
            if ((i ^ j) & 1) {
                /* Path 1: Complex recurrence with pointer arithmetic */
                temp1 = arr2[idx1 % n] + arr3[idx2 % n];
                temp2 = arr1[i - 1] * 3;  /* Recurrence dependency */
                
                /* Multiple array operations with dependencies */
                arr1[i] = temp1 + temp2 + (arr1[i - 2] >> 1);  /* Another recurrence */
                arr3[i] = arr1[i] - arr2[(i + 3) % n];
            } else {
                /* Path 2: Different arithmetic operations */
                temp1 = arr2[(i + 2) % n] - arr3[(i - 2) % n];
                temp2 = arr1[i - 1] / 2;  /* Recurrence dependency */
                
                /* More complex indexing with pointer arithmetic */
                arr1[i] = temp1 * temp2 + (arr1[i - 2] & 0xFF);
                arr3[i] = arr2[i] ^ arr3[(i + 1) % n];
            }
            
            /* Additional operation with mixed indexing */
            arr2[i] = arr1[i] + arr2[(idx3 + i) % n] - arr3[(i * 2) % n];
        }
        
        /* Post-inner loop processing */
        arr1[n - 2] = arr1[n - 3] + arr2[n - 2];
        arr1[n - 1] = arr1[n - 2] * arr3[n - 1];
    }
}

int main() {
    /* Medium-sized arrays to work with */
    HOST_WIDE_INT arr1[512];
    HOST_WIDE_INT arr2[512];
    HOST_WIDE_INT arr3[512];
    HOST_WIDE_INT i, sum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 512; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    /* Add some variation to prevent complete optimization */
    arr1[0] = 7;
    arr2[0] = 11;
    arr3[0] = 13;
    
    /* Call computational function with nested loops */
    /* Non-constant trip counts prevent unrolling */
    compute_loop(arr1, arr2, arr3, 500, 3);
    
    /* Compute checksum to ensure computation isn't optimized away */
    for (i = 0; i < 500; i++) {
        sum += arr1[i] + arr2[i] - arr3[i];
        sum = (sum << 3) | (sum >> (sizeof(HOST_WIDE_INT)*8 - 3));  /* Rotate */
    }
    
    /* Print result to prevent dead code elimination */
    volatile HOST_WIDE_INT result = sum;
    
    return 0;
}
