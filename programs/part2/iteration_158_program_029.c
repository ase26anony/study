/* Test case for modulo-scheduling coverage */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
static int __attribute__((const)) helper(int x, int y) {
    return (x ^ y) + (x & y) * 2;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int k) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < 3; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            int idx1 = i + k;
            int idx2 = i - 1;
            int idx3 = i * 2 - k;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            int base = arr1[idx2] + arr2[idx1];
            
            /* Conditional operations creating control flow */
            if (base > 0) {
                /* Complex operation with function call */
                arr1[i] = helper(base, arr3[idx3]) + (i % 4);
                
                /* Additional recurrence with pointer arithmetic */
                int *ptr = &arr2[i];
                arr3[i] = *ptr + arr3[idx2] * 2;
            } else {
                /* Alternative path with different indexing */
                arr1[i] = arr2[idx1] - arr3[idx3];
                arr3[i] = arr1[idx2] >> 1;
            }
            
            /* Cross-iteration dependency with multiple induction variables */
            arr2[i] = arr1[i] + arr2[i-1] + (j * i);
        }
        
        /* Modify k to change indexing pattern */
        k = (k + j) % 5;
    }
}

/* Main function with initialization and checksum */
int main(int argc, char **argv) {
    /* Use non-constant trip count to prevent unrolling */
    int size = (argc > 1) ? 500 : 250;
    
    /* Declare and initialize arrays */
    int arr1[500], arr2[500], arr3[500];
    int i;
    
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 2;
        arr2[i] = i + 1;
        arr3[i] = i * 3;
    }
    
    /* Call computational function with nested loops */
    compute_loop(arr1, arr2, arr3, size, 3);
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 - arr3[i];
    }
    
    /* Print checksum (use simple output) */
    volatile long result = checksum;
    
    return 0;
}
