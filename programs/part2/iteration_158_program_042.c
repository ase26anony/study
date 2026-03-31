/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static HOST_WIDE_INT __attribute__((const)) 
compute_offset(HOST_WIDE_INT x, HOST_WIDE_INT y) {
    return (x ^ y) + (x & y) * 2;
}

/* Core computational function with nested loops */
void compute_loop(HOST_WIDE_INT *arr1, HOST_WIDE_INT *arr2, 
                  HOST_WIDE_INT *arr3, HOST_WIDE_INT n, HOST_WIDE_INT k) {
    HOST_WIDE_INT i, j;
    
    /* Outer loop - provides context */
    for (j = 0; j < 3; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            HOST_WIDE_INT idx1, idx2, idx3;
            
            /* Multiple induction variables and non-trivial indexing */
            idx1 = i + k;
            idx2 = i - 1;
            idx3 = compute_offset(i, k);  /* Function call creates RTL */
            
            /* Conditional operations creating control flow */
            if (idx1 % 3 == 0) {
                /* Recurrence: reads from previous iteration */
                arr1[i] = arr2[idx1] + arr1[idx2] + arr3[idx3];
                
                /* Additional operation with pointer arithmetic */
                *(arr3 + i) = *(arr2 + idx1) - *(arr1 + idx2);
            } else if (idx1 % 3 == 1) {
                /* Different recurrence pattern */
                arr1[i] = arr2[idx1] * 2 - arr1[idx2];
                arr3[i] = arr1[idx2] + compute_offset(arr2[idx1], i);
            } else {
                /* Yet another pattern with complex expression */
                arr1[i] = (arr2[idx1] >> 2) | (arr1[idx2] << 2);
                arr3[i] = compute_offset(arr1[i], arr3[idx3]);
            }
            
            /* Additional operation outside condition */
            arr2[i] = arr1[i] + arr3[i] - k;
        }
        
        /* Modify k for next outer iteration */
        k = compute_offset(k, j);
    }
}

/* Main function with array initialization and checksum */
int main(int argc, char **argv) {
    HOST_WIDE_INT arr1[500];
    HOST_WIDE_INT arr2[500];
    HOST_WIDE_INT arr3[500];
    HOST_WIDE_INT i, sum = 0;
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        arr3[i] = i * 7 + 3;
    }
    
    /* Non-constant trip count from argument or default */
    HOST_WIDE_INT n = (argc > 1) ? 250 : 200;
    HOST_WIDE_INT k = (argc > 2) ? 5 : 3;
    
    /* Call computational function */
    compute_loop(arr1, arr2, arr3, n, k);
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < 500; i++) {
        sum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum (prevents optimization) */
    volatile HOST_WIDE_INT result = sum;
    
    return 0;
}
