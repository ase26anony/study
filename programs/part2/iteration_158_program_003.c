/* Test case for modulo-scheduler coverage of debug printing block */
/* Compile with: -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms */

typedef long HOST_WIDE_INT;

/* Pure helper function to create function call in loop */
static HOST_WIDE_INT __attribute__((const)) 
compute_offset(HOST_WIDE_INT x, HOST_WIDE_INT y) {
    return (x * 3 + y * 7) & 0xFF;
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
        HOST_WIDE_INT base = compute_offset(k, n);
        arr1[0] = arr2[0] + base;
        arr3[0] = arr2[0] - base;
        
        /* Tight inner loop with complex dependencies */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables */
            HOST_WIDE_INT idx1 = i + (k & 3);      /* Non-trivial indexing */
            HOST_WIDE_INT idx2 = i - (k % 2);
            HOST_WIDE_INT idx3 = compute_offset(i, k);  /* Function call */
            
            /* Recurrence: reads from previous iteration */
            HOST_WIDE_INT prev = arr1[i - 1];
            
            /* Conditional operations creating control flow */
            if (idx1 < n && idx2 > 0) {
                /* Complex data dependency chain */
                HOST_WIDE_INT temp = arr2[idx1] + prev;
                
                /* Pointer arithmetic access */
                HOST_WIDE_INT *ptr = arr3 + idx2;
                HOST_WIDE_INT val = *ptr + temp;
                
                /* Multiple writes with dependencies */
                arr1[i] = val + compute_offset(temp, val);
                arr3[i] = arr2[i] - (prev >> 2);
            } else {
                /* Alternative computation path */
                HOST_WIDE_INT alt = arr2[i] * 3;
                arr1[i] = alt + (prev << 1);
                arr3[i] = alt - prev;
                
                /* Additional recurrence */
                if (i > 2) {
                    arr1[i] += arr3[i - 2];
                }
            }
            
            /* Cross-iteration dependency with variable distance */
            if (k > 0 && i > k) {
                arr2[i] += arr1[i - k];
            }
        }
        
        /* Feedback for next outer iteration */
        if (k + 1 < outer_iters) {
            for (i = 0; i < n; i++) {
                arr2[i] = arr1[i] + arr3[i];
            }
        }
    }
}

int main() {
    /* Medium-sized arrays to enable modulo-scheduling */
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
    
    /* Non-constant trip counts to prevent unrolling */
    HOST_WIDE_INT inner_size = 256;  /* Could be function parameter */
    HOST_WIDE_INT outer_iters = 8;
    
    /* Core computation */
    compute_loop(arr1, arr2, arr3, inner_size, outer_iters);
    
    /* Checksum to prevent dead code elimination */
    HOST_WIDE_INT sum = 0;
    for (i = 0; i < 512; i++) {
        sum += arr1[i] + arr2[i] + arr3[i];
        /* Mix operations to prevent simple optimizations */
        sum = (sum << 3) | (sum >> (sizeof(HOST_WIDE_INT)*8 - 3));
    }
    
    /* Print result (prevents complete optimization) */
    volatile HOST_WIDE_INT result = sum;
    
    return 0;
}
