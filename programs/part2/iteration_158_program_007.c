/* Test case for modulo-scheduler coverage of debug printing block */
/* Compile with: -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms */

typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) 
compute_offset(int x, int y) {
    return (x * 3 + y * 7) & 0xF;
}

/* Core computational function with nested loops */
static void 
compute_loop(int *arr1, int *arr2, int *arr3, int *arr4, 
             HOST_WIDE_INT n, HOST_WIDE_INT outer_iter) {
    HOST_WIDE_INT i, j;
    
    /* Outer loop - provides context */
    for (j = 0; j < outer_iter; j++) {
        int base_offset = compute_offset(j, outer_iter);
        
        /* Inner loop - target for modulo scheduling */
        /* Multiple induction variables and complex indexing */
        for (i = 1; i < n; i++) {
            int idx1 = i + base_offset;
            int idx2 = i - 1;
            int idx3 = i + (j & 3);
            
            /* Recurrence: arr1 depends on its previous value */
            int temp = arr1[idx2] + arr2[idx1];
            
            /* Conditional operations inside loop */
            if (temp > 100) {
                /* Complex operation with pointer arithmetic */
                arr3[i] = temp * 2 - arr4[idx3];
                arr1[i] = arr3[i] + compute_offset(i, temp);
            } else {
                /* Different arithmetic path */
                arr3[i] = temp / 2 + arr4[idx3];
                arr1[i] = arr3[i] - compute_offset(temp, i);
            }
            
            /* Additional recurrence with multiple dependencies */
            arr2[i] = arr1[i] + arr2[idx2] * 3;
            
            /* Another conditional with different indexing */
            if ((i & 1) == 0) {
                arr4[i] = arr3[idx2] + arr1[idx3];
            } else {
                arr4[i] = arr3[idx3] - arr1[idx2];
            }
        }
        
        /* Cross-iteration dependency for outer loop */
        arr1[0] = arr1[n-1] + arr2[0];
    }
}

/* Alternate version with different access pattern */
static void
compute_loop_variant(int *arr1, int *arr2, int *arr3,
                     HOST_WIDE_INT n, int stride) {
    HOST_WIDE_INT i;
    
    /* Loop with stride to create non-unit distances */
    for (i = stride; i < n; i += stride) {
        int prev = i - stride;
        int next = i + stride < n ? i + stride : i;
        
        /* Multiple interleaved recurrences */
        arr1[i] = arr2[prev] + arr3[next];
        arr2[i] = arr1[prev] * 2 - arr3[i];
        
        /* Conditional with function call */
        int offset = compute_offset(i, stride);
        if (arr1[i] > arr2[i]) {
            arr3[i] = arr1[prev] + arr2[next] + offset;
        } else {
            arr3[i] = arr1[next] - arr2[prev] - offset;
        }
        
        /* Additional dependency chain */
        arr1[i] += arr3[prev] >> 2;
    }
}

int main() {
    /* Medium-sized arrays to work with */
    int array1[512];
    int array2[512];
    int array3[512];
    int array4[512];
    
    HOST_WIDE_INT i;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 512; i++) {
        array1[i] = i * 3;
        array2[i] = i * 5 + 1;
        array3[i] = i * 7 + 2;
        array4[i] = i * 11 + 3;
    }
    
    /* Call computational function with non-constant trip counts */
    HOST_WIDE_INT n = 256;  /* Non-constant prevents unrolling */
    HOST_WIDE_INT outer = 8;
    
    compute_loop(array1, array2, array3, array4, n, outer);
    
    /* Second call with different parameters */
    compute_loop_variant(array1, array2, array3, n, 3);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 512; i++) {
        checksum += array1[i] + array2[i] * 2 + array3[i] * 3 + array4[i];
        /* Mix in some bit operations */
        checksum ^= (array1[i] << 3);
        checksum ^= (array2[i] >> 2);
    }
    
    /* Print checksum to ensure code is live */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
