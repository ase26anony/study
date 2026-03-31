/* Test case for modulo-scheduler coverage of debug printing block */
/* Compile with: -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms */

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
        
        /* Inner loop - target for modulo scheduling */
        /* Multiple induction variables and complex indexing */
        HOST_WIDE_INT k = j * 2;
        HOST_WIDE_INT m = j;
        
        for (i = 1; i < n; i++) {
            /* Recurrence: reads from arr1[i-1] written in previous iteration */
            HOST_WIDE_INT base = arr1[i - 1];
            
            /* Complex array indexing with multiple terms */
            HOST_WIDE_INT idx1 = i + k;
            HOST_WIDE_INT idx2 = i - m;
            
            /* Ensure indices are within bounds */
            if (idx1 >= n) idx1 = n - 1;
            if (idx2 < 0) idx2 = 0;
            
            /* Conditional operations inside loop */
            HOST_WIDE_INT temp;
            if ((i ^ j) & 1) {
                /* Path 1: Use helper function and pointer arithmetic */
                temp = helper_func(arr2[idx1], arr3[idx2]);
                
                /* More complex computation with multiple dependencies */
                HOST_WIDE_INT *ptr1 = &arr2[idx1];
                HOST_WIDE_INT *ptr2 = &arr3[idx2];
                temp += *ptr1 - *ptr2;
            } else {
                /* Path 2: Different computation */
                temp = arr2[i] * 3 - arr3[i] / 2;
                
                /* Additional recurrence-like dependency */
                if (i > 2) {
                    temp += arr1[i - 2] & 0xFF;
                }
            }
            
            /* Final assignment with recurrence dependency */
            arr1[i] = base + temp;
            
            /* Update secondary array with cross-iteration dependency */
            if (i > 0) {
                HOST_WIDE_INT idx3 = (i + j) % n;
                arr3[idx3] = arr1[i] + (arr2[i] << 1);
            }
            
            /* Update induction variables within loop */
            k = (k + 1) % 8;
            m = (m + 3) % 5;
        }
        
        /* Cross-iteration update for next outer iteration */
        if (j > 0) {
            arr2[0] = arr1[n - 1] ^ arr3[0];
        }
    }
}

int main() {
    /* Medium-sized arrays to work with */
    HOST_WIDE_INT arr1[500];
    HOST_WIDE_INT arr2[500];
    HOST_WIDE_INT arr3[500];
    
    HOST_WIDE_INT i;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5 + 1;
        arr3[i] = i * 7 + 2;
    }
    
    /* Perform computation with non-constant trip counts */
    compute_loop(arr1, arr2, arr3, 250, 10);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 - arr3[i];
        checksum = (checksum << 3) | (checksum >> (sizeof(HOST_WIDE_INT)*8 - 3));
    }
    
    /* Print checksum to ensure code is live */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
