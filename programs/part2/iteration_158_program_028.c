/* Test for modulo-scheduling coverage of debug printing block */
/* Compile with: -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms */

typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
static HOST_WIDE_INT __attribute__((const)) 
helper_func(HOST_WIDE_INT a, HOST_WIDE_INT b) {
    return (a ^ b) + (a & b) * 2;
}

/* Another helper with different operation */
static HOST_WIDE_INT __attribute__((const))
helper_func2(HOST_WIDE_INT x) {
    return (x << 3) | (x >> 5);
}

/* Core computational function with nested loops */
static void compute_loop(HOST_WIDE_INT *arr1, 
                         HOST_WIDE_INT *arr2, 
                         HOST_WIDE_INT *arr3,
                         HOST_WIDE_INT n,
                         HOST_WIDE_INT outer_iters) {
    HOST_WIDE_INT i, j, k;
    
    /* Outer loop - provides context */
    for (k = 0; k < outer_iters; k++) {
        /* Initialize boundary values */
        if (k == 0) {
            arr1[0] = arr2[0] + arr3[0];
            arr3[0] = helper_func(arr1[0], arr2[0]);
        }
        
        /* Inner loop - target for modulo scheduling */
        /* Complex indexing with multiple induction variables */
        for (i = 1, j = n - 1; i < n && j > 0; i++, j--) {
            HOST_WIDE_INT idx1 = i;
            HOST_WIDE_INT idx2 = j;
            HOST_WIDE_INT idx3 = i + k;  /* Non-trivial indexing */
            HOST_WIDE_INT idx4 = j - k;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            HOST_WIDE_INT base = arr1[idx1 - 1] + arr2[idx2];
            
            /* Conditional operations creating control flow */
            if ((idx1 + idx2) % 3 == 0) {
                /* Path 1: Complex arithmetic with function call */
                arr1[idx1] = helper_func(base, arr3[idx3 % n]);
                arr3[idx2] = helper_func2(arr1[idx1]);
            } else if ((idx1 + idx2) % 3 == 1) {
                /* Path 2: Different operations */
                arr1[idx1] = base * 2 - arr3[idx4 % n];
                arr3[idx2] = arr1[idx1] ^ arr2[idx1];
            } else {
                /* Path 3: Yet another set of operations */
                arr1[idx1] = base + (arr2[idx1] << 2);
                arr3[idx2] = arr1[idx1] | arr3[idx2];
            }
            
            /* Additional recurrence with pointer arithmetic */
            HOST_WIDE_INT *ptr1 = &arr1[idx1];
            HOST_WIDE_INT *ptr2 = &arr2[idx2];
            *ptr1 += helper_func(*ptr1, *ptr2);
        }
        
        /* Cross-iteration dependency for outer loop */
        arr2[0] = arr1[n-1] + k;
    }
}

/* Main function */
int main() {
    /* Medium-sized arrays to work with */
    HOST_WIDE_INT arr1[512];
    HOST_WIDE_INT arr2[512];
    HOST_WIDE_INT arr3[512];
    
    HOST_WIDE_INT i;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 512; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        arr3[i] = i * 7 + 3;
    }
    
    /* Perform computation with non-constant trip counts */
    compute_loop(arr1, arr2, arr3, 500, 10);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 512; i++) {
        checksum += arr1[i] ^ arr2[i];
        checksum += arr3[i];
        checksum = helper_func(checksum, i);
    }
    
    /* Print checksum (use simple output to avoid stdio.h) */
    volatile HOST_WIDE_INT *output = (volatile HOST_WIDE_INT *)&checksum;
    
    return 0;
}
