/* Test case for modulo-scheduling coverage of lines 596-606 in modulo-sched.cc
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -fdump-rtl-all -dA
 */

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
                         HOST_WIDE_INT outer_iters) {
    HOST_WIDE_INT i, j, k;
    
    for (k = 0; k < outer_iters; k++) {
        /* Initialize with offset based on outer iteration */
        HOST_WIDE_INT offset = k * 3;
        
        /* Tight inner loop with data dependencies */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables */
            HOST_WIDE_INT idx1 = i;
            HOST_WIDE_INT idx2 = i + offset;
            HOST_WIDE_INT idx3 = i - 1;
            
            /* Complex array indexing with pointer arithmetic */
            HOST_WIDE_INT *ptr1 = arr1 + idx1;
            HOST_WIDE_INT *ptr2 = arr2 + idx2;
            HOST_WIDE_INT *ptr3 = arr3 + idx3;
            
            /* Recurrence: read from previous iteration's result */
            HOST_WIDE_INT prev_val = *(ptr1 - 1);  /* arr1[i-1] */
            
            /* Conditional operations inside loop */
            if (i % 3 == 0) {
                /* Different arithmetic for divisible by 3 */
                HOST_WIDE_INT temp = helper_func(*ptr2, *ptr3);
                *ptr1 = prev_val + temp * 2;
            } else if (i % 3 == 1) {
                /* Another operation pattern */
                HOST_WIDE_INT temp = helper_func(*ptr3, *ptr2);
                *ptr1 = prev_val - temp + offset;
            } else {
                /* Third operation pattern */
                HOST_WIDE_INT temp = helper_func(prev_val, *ptr2);
                *ptr1 = temp * 3 - *ptr3;
            }
            
            /* Additional recurrence with second array */
            if (i > 2) {
                HOST_WIDE_INT *ptr2_prev = arr2 + idx3 - 1;  /* arr2[i-2] */
                *ptr2 = *ptr2_prev + *ptr1 / 4;
            }
        }
        
        /* Cross-array dependency */
        for (i = n - 2; i > 0; i--) {
            arr3[i] = arr1[i + 1] + arr2[i - 1];
        }
    }
}

int main() {
    /* Medium-sized arrays to ensure loop execution */
    HOST_WIDE_INT arr1[512];
    HOST_WIDE_INT arr2[512];
    HOST_WIDE_INT arr3[512];
    
    HOST_WIDE_INT i;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 512; i++) {
        arr1[i] = i * 2;
        arr2[i] = i * 3 + 1;
        arr3[i] = i * 5 - 2;
    }
    
    /* Perform computation with non-constant trip counts */
    compute_loop(arr1, arr2, arr3, 500, 3);
    
    /* Checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 512; i++) {
        checksum += arr1[i] + arr2[i] * 2 - arr3[i];
        /* Mix in some conditional to prevent vectorization */
        if (checksum % 7 == 0) {
            checksum ^= arr1[i];
        }
    }
    
    /* Print checksum to ensure code is live */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
