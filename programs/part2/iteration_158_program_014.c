/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
static HOST_WIDE_INT __attribute__((const)) 
compute_offset(HOST_WIDE_INT val, int shift) {
    return (val << shift) | (val >> (32 - shift));
}

/* Core computational function with nested loops */
static void compute_loop(HOST_WIDE_INT *arr1, 
                         HOST_WIDE_INT *arr2, 
                         HOST_WIDE_INT *arr3,
                         HOST_WIDE_INT *arr4,
                         int n, int outer_iter) {
    int i, j;
    
    /* Outer loop - provides context */
    for (j = 0; j < outer_iter; j++) {
        /* Initialize boundary values for recurrence */
        if (j == 0) {
            arr1[0] = arr2[0] + arr3[0];
            arr4[0] = arr2[0] * 2;
        }
        
        /* Inner loop with complex dependencies - target for modulo scheduling */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables and complex indexing */
            HOST_WIDE_INT idx1 = i + (j % 3);
            HOST_WIDE_INT idx2 = i - (j % 2);
            HOST_WIDE_INT idx3 = i * 2 - 1;
            
            /* Recurrence dependency: arr1[i] depends on arr1[i-1] */
            HOST_WIDE_INT base = arr1[i-1];
            
            /* Conditional operations creating control flow */
            if ((i ^ j) & 1) {
                /* Path 1: Complex arithmetic with function call */
                HOST_WIDE_INT temp = compute_offset(arr2[idx1 % n], 3);
                arr1[i] = temp + base + arr3[idx2 % n];
                
                /* Pointer arithmetic access */
                HOST_WIDE_INT *ptr = arr4 + (idx3 % n);
                *ptr = arr1[i] * 3 - arr2[i];
            } else {
                /* Path 2: Different arithmetic pattern */
                HOST_WIDE_INT temp = compute_offset(arr3[idx1 % n], 2);
                arr1[i] = base * 2 - temp + arr2[idx2 % n];
                
                /* Another recurrence: arr4 depends on previous arr4 */
                arr4[i] = arr4[i-1] + arr1[i] / 2;
            }
            
            /* Additional operation with cross-iteration dependency */
            HOST_WIDE_INT idx4 = (i + j) % n;
            arr3[idx4] = arr1[i] + (arr2[idx4] << 2);
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        if (j > 0) {
            arr2[0] = arr1[n-1] - arr4[n-1];
        }
    }
}

/* Main function with array initialization and checksum */
int main(int argc, char **argv) {
    /* Use non-constant sizes to prevent complete unrolling */
    int size = (argc > 1) ? 500 : 250;
    int outer_iters = (argc > 2) ? 10 : 5;
    
    /* Medium-sized arrays with different initial patterns */
    HOST_WIDE_INT arr1[500];
    HOST_WIDE_INT arr2[500];
    HOST_WIDE_INT arr3[500];
    HOST_WIDE_INT arr4[500];
    
    /* Initialize arrays with non-uniform patterns */
    for (int i = 0; i < 500; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        arr3[i] = i * 7 + 3;
        arr4[i] = i * 11 - 5;
    }
    
    /* Perform computation with nested loops */
    compute_loop(arr1, arr2, arr3, arr4, size, outer_iters);
    
    /* Calculate checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + arr4[i];
        /* Mix operations to create complex dependency */
        checksum = (checksum << 3) | (checksum >> (sizeof(HOST_WIDE_INT)*8 - 3));
    }
    
    /* Print checksum to ensure code is live */
    volatile HOST_WIDE_INT result = checksum;
    
    return (int)(result % 256);
}
