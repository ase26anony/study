/* Test case for modulo-scheduler with complex dependencies */
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
                         HOST_WIDE_INT outer_iters) {
    HOST_WIDE_INT i, j, k;
    
    for (k = 0; k < outer_iters; k++) {
        /* Initialize with offset based on outer iteration */
        arr1[0] = compute_offset(k, n);
        arr2[0] = k * 2;
        arr3[0] = n - k;
        
        /* Inner loop with multiple dependencies and conditions */
        for (i = 1; i < n; i++) {
            HOST_WIDE_INT idx1, idx2, idx3;
            
            /* Complex indexing with multiple induction variables */
            idx1 = i + k;
            idx2 = i - 1;
            idx3 = compute_offset(i, k);
            
            /* Recurrence: arr1 depends on its previous value */
            HOST_WIDE_INT base = arr1[idx2] + arr2[idx1 % n];
            
            /* Conditional operations creating control flow */
            if ((i ^ k) & 1) {
                /* Path A: More complex computation */
                arr1[i] = base * 3 - arr3[idx3 % n];
                arr2[i] = (arr2[idx2] << 2) | (arr1[i] & 0xF);
            } else {
                /* Path B: Different computation */
                arr1[i] = base / 2 + arr3[i % n];
                arr2[i] = (arr2[idx2] >> 1) ^ arr1[i];
            }
            
            /* Additional recurrence with pointer arithmetic */
            HOST_WIDE_INT *ptr = arr3 + i;
            *ptr = *(ptr - 1) + arr1[i] - arr2[i];
            
            /* Cross-iteration dependency with variable distance */
            if (i > 2) {
                arr3[i] += arr1[i - 2] * arr2[i - 1];
            }
        }
        
        /* Final processing step */
        for (i = n - 1; i > 0; i--) {
            arr1[i] = arr1[i] ^ arr1[i - 1];
        }
    }
}

int main() {
    /* Medium-sized arrays to ensure loop body is substantial */
    HOST_WIDE_INT arr1[512];
    HOST_WIDE_INT arr2[512];
    HOST_WIDE_INT arr3[512];
    
    HOST_WIDE_INT i;
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < 512; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        arr3[i] = i * 7 + 3;
    }
    
    /* Call computational function with non-constant trip counts */
    compute_loop(arr1, arr2, arr3, 500, 3);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] ^ arr2[i];
        checksum += arr3[i] * (i + 1);
        checksum = (checksum << 3) | (checksum >> 61); /* Rotate */
    }
    
    /* Print checksum to ensure code is live */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
