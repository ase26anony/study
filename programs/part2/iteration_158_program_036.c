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
                         HOST_WIDE_INT outer_iters) {
    HOST_WIDE_INT i, j, k;
    
    for (k = 0; k < outer_iters; k++) {
        /* Initialize with offset based on outer iteration */
        HOST_WIDE_INT offset = k * 3;
        
        /* Inner loop with multiple induction variables and complex indexing */
        for (i = 1, j = 0; i < n; i++, j++) {
            /* Multiple data dependencies creating move->def and move->insn relationships */
            HOST_WIDE_INT temp1 = arr2[i + offset] + arr3[j];
            HOST_WIDE_INT temp2 = arr1[i - 1];  /* Recurrence dependency */
            
            /* Conditional operations creating control flow */
            if (temp1 > temp2) {
                /* Complex expression with function call */
                arr1[i] = helper_func(temp1, temp2) + (i % 8);
                
                /* Additional pointer arithmetic access */
                HOST_WIDE_INT *ptr = arr3 + i;
                arr2[i + offset] = *ptr + (j << 2);
            } else {
                /* Different arithmetic path */
                arr1[i] = (temp1 * 3 - temp2) | (i & 0xF);
                arr3[j] = arr2[i + offset] - (i >> 1);
            }
            
            /* Another recurrence with different distance */
            if (i > 2) {
                arr3[i] = arr3[i - 2] + arr1[i - 1];  /* Distance-2 dependency */
            }
        }
        
        /* Cross-iteration dependency for modulo scheduler */
        for (i = n - 1; i >= 1; i--) {
            arr2[i] = arr1[i] + arr2[i + 1];  /* Reverse recurrence */
        }
    }
}

int main() {
    /* Medium-sized arrays to prevent complete unrolling */
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
    
    /* Call computational function with non-constant trip counts */
    compute_loop(arr1, arr2, arr3, 500, 3);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 512; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
        checksum = (checksum << 1) | (checksum >> 31);  /* Rotate */
    }
    
    /* Print checksum to ensure code is live */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
