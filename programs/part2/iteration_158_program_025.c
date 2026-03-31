/* Test case for modulo-scheduler coverage of debug printing block */
/* Compile with: -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms */

typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static HOST_WIDE_INT __attribute__((const)) 
compute_offset(HOST_WIDE_INT base, HOST_WIDE_INT delta) {
    return base + (delta * 3) / 2;
}

/* Core computational function with nested loops */
static void compute_loop(HOST_WIDE_INT *arr1, HOST_WIDE_INT *arr2, 
                         HOST_WIDE_INT *arr3, HOST_WIDE_INT n) {
    HOST_WIDE_INT i, j;
    HOST_WIDE_INT outer_limit = n > 100 ? 100 : n;
    
    /* Outer loop */
    for (j = 0; j < outer_limit; j++) {
        HOST_WIDE_INT offset = compute_offset(j, 2);
        
        /* Inner loop with modulo-scheduling characteristics */
        /* Multiple induction variables and complex indexing */
        HOST_WIDE_INT k1 = 1;
        HOST_WIDE_INT k2 = offset;
        
        /* Initialize first element for recurrence */
        if (j == 0) {
            arr1[0] = arr2[0] + arr3[0];
        }
        
        /* Tight inner loop with data dependencies between iterations */
        for (i = 1; i < n; i++) {
            /* Recurrence: reads from arr1[i-1] written in previous iteration */
            HOST_WIDE_INT recur = arr1[i-1];
            
            /* Complex array indexing with multiple induction variables */
            HOST_WIDE_INT idx1 = i + k1;
            HOST_WIDE_INT idx2 = i + k2;
            
            /* Conditional operations creating control flow */
            if ((i + j) % 3 == 0) {
                /* Different arithmetic path 1 */
                HOST_WIDE_INT val1 = arr2[idx1 % n];
                HOST_WIDE_INT val2 = arr3[(i * 2) % n];
                arr1[i] = compute_offset(recur, val1) + val2;
                
                /* Additional operation with pointer arithmetic */
                HOST_WIDE_INT *ptr = &arr2[(idx2 + 1) % n];
                arr3[i] = *ptr + (val1 >> 2);
            } else if ((i + j) % 5 == 0) {
                /* Different arithmetic path 2 */
                HOST_WIDE_INT val1 = arr2[(idx2 - 1) % n];
                HOST_WIDE_INT val2 = arr3[idx1 % n];
                arr1[i] = recur - compute_offset(val1, val2);
                
                /* More pointer arithmetic */
                HOST_WIDE_INT *ptr = &arr3[(i * 3) % n];
                arr2[i] = *ptr - (val2 << 1);
            } else {
                /* Default arithmetic path with recurrence */
                HOST_WIDE_INT val1 = arr2[i % n];
                HOST_WIDE_INT val2 = arr3[(n - i - 1) % n];
                arr1[i] = recur + val1 * 2 - val2;
                
                /* Cross-iteration dependency with arr3 */
                arr3[i] = arr3[i-1] + val1;
            }
            
            /* Update induction variables with non-trivial patterns */
            k1 = (k1 + i) % 7;
            k2 = (k2 + (i % 3)) % 5;
        }
        
        /* Rotate arrays for next outer iteration */
        HOST_WIDE_INT temp = arr1[0];
        for (i = 0; i < n - 1; i++) {
            arr1[i] = arr1[i+1];
        }
        arr1[n-1] = temp;
    }
}

int main() {
    /* Medium-sized arrays to enable modulo-scheduling */
    HOST_WIDE_INT arr1[500];
    HOST_WIDE_INT arr2[500];
    HOST_WIDE_INT arr3[500];
    
    HOST_WIDE_INT i;
    HOST_WIDE_INT n = 250;  /* Non-constant trip count */
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 2;
        arr2[i] = i * 3 + 1;
        arr3[i] = i * 5 - 2;
    }
    
    /* Perform computation with nested loops */
    compute_loop(arr1, arr2, arr3, n);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 - arr3[i];
        checksum = (checksum >> 1) | (checksum << 63);  /* Simple rotate */
    }
    
    /* Print checksum to ensure code is live */
    volatile HOST_WIDE_INT result __attribute__((unused)) = checksum;
    
    return 0;
}
