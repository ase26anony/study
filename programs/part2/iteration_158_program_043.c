/* Test case for modulo-scheduling coverage */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
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
        /* Initialize with some values based on outer iteration */
        arr1[0] = k * 7;
        arr2[0] = k * 11;
        arr3[0] = k * 13;
        
        /* Inner loop with complex data dependencies */
        for (i = 1; i < n; i++) {
            HOST_WIDE_INT idx1 = i + k;
            HOST_WIDE_INT idx2 = i - 1;
            HOST_WIDE_INT idx3 = i * 2 - k;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            HOST_WIDE_INT base = arr1[idx2] + arr2[idx1 % n];
            
            /* Conditional operations creating control flow */
            if ((i ^ k) & 1) {
                /* Complex indexing with pointer arithmetic */
                HOST_WIDE_INT *ptr1 = arr1 + (idx3 % n);
                HOST_WIDE_INT *ptr2 = arr3 + (i % n);
                
                /* Multiple operations with function call */
                HOST_WIDE_INT val1 = helper_func(*ptr1, base);
                HOST_WIDE_INT val2 = helper_func(*ptr2, i);
                
                arr1[i] = val1 + val2 + (arr3[idx2] >> 1);
                arr2[i] = (arr2[idx2] * 3) - val1;
            } else {
                /* Different recurrence pattern */
                HOST_WIDE_INT temp = arr3[idx2] + arr2[i];
                
                /* More complex indexing */
                HOST_WIDE_INT idx4 = (i * 3 + k) % n;
                HOST_WIDE_INT idx5 = (i * 5 - k) % n;
                
                arr1[i] = temp + arr1[idx4] - arr2[idx5];
                arr2[i] = helper_func(temp, arr1[idx2]);
            }
            
            /* Additional recurrence for arr3 */
            arr3[i] = arr3[idx2] + (arr1[i] ^ arr2[i]) + i;
        }
        
        /* Cross-iteration dependency for next outer iteration */
        if (k < outer_iters - 1) {
            arr1[n-1] = arr2[0] + arr3[n-1];
        }
    }
}

/* Main function */
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
    
    /* Call computational function with non-constant trip counts */
    compute_loop(arr1, arr2, arr3, 250, 10);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] ^ arr2[i];
        checksum += arr3[i];
        checksum = (checksum << 3) | (checksum >> (sizeof(HOST_WIDE_INT)*8 - 3));
    }
    
    /* Print checksum (simplified for portability) */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
