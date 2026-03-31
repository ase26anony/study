/* Test case for modulo-scheduler debug output in modulo-sched.cc lines 596-606 */
/* Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms */

typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
static HOST_WIDE_INT __attribute__((const)) 
compute_offset(HOST_WIDE_INT x, HOST_WIDE_INT y) {
    return (x * 3 + y * 7) & 0xFF;
}

/* Core computational function with nested loops */
static void compute_loop(HOST_WIDE_INT *arr1, 
                         HOST_WIDE_INT *arr2, 
                         HOST_WIDE_INT *arr3,
                         HOST_WIDE_INT n,
                         HOST_WIDE_INT outer_iter) {
    HOST_WIDE_INT i, j;
    
    /* Outer loop */
    for (j = 0; j < outer_iter; j++) {
        HOST_WIDE_INT base = j * 10;
        
        /* Inner loop with recurrence, multiple induction variables, and complex indexing */
        for (i = 1; i < n; i++) {
            HOST_WIDE_INT idx1 = i;
            HOST_WIDE_INT idx2 = i + base;
            HOST_WIDE_INT idx3 = i - 1;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            HOST_WIDE_INT recur = arr1[idx3];
            
            /* Complex array indexing with pointer arithmetic */
            HOST_WIDE_INT val1 = *(arr2 + idx1);
            HOST_WIDE_INT val2 = *(arr3 + (idx2 % n));
            
            /* Conditional operations inside loop */
            HOST_WIDE_INT temp;
            if ((i & 3) == 0) {
                /* Different operation for multiples of 4 */
                temp = val1 - val2 + recur;
            } else if ((i & 1) == 0) {
                /* Operation for even indices */
                temp = val1 * 2 - val2;
            } else {
                /* Default operation for odd indices */
                temp = val1 + val2;
            }
            
            /* Function call to create additional dependencies */
            HOST_WIDE_INT offset = compute_offset(temp, i);
            
            /* Final assignment with recurrence update */
            arr1[idx1] = temp + offset + (recur >> 2);
            
            /* Additional store with different indexing pattern */
            arr2[(i * 7) % n] = arr3[(i * 3) % n] + arr1[idx3];
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        arr3[0] = arr1[n-1] + base;
    }
}

int main() {
    /* Medium-sized arrays */
    HOST_WIDE_INT arr1[500];
    HOST_WIDE_INT arr2[500];
    HOST_WIDE_INT arr3[500];
    
    HOST_WIDE_INT i;
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 + 2;
        arr3[i] = i * 7 + 3;
    }
    
    /* Break simple patterns */
    arr1[100] = 999;
    arr2[200] = -555;
    arr3[300] = 777;
    
    /* Call computational function with non-constant trip counts */
    compute_loop(arr1, arr2, arr3, 250, 5);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
        /* Add some non-linear operation */
        checksum ^= (arr1[i] << 3);
    }
    
    /* Print checksum to ensure code is live */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
