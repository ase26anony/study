/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int x, int y) {
    return (x ^ y) + (x & y) * 2;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < m; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            int idx1 = i + j;
            int idx2 = i - 1;
            int idx3 = i * 2 - j;
            
            /* Recurrence: arr1 depends on its previous value */
            int base = arr1[i-1] + arr2[idx1 % n];
            
            /* Conditional operations creating control flow */
            if (idx3 > 0 && idx3 < n) {
                /* Complex operation with function call */
                arr1[i] = helper(base, arr3[idx3]) + (arr2[i] >> 2);
                
                /* Additional recurrence with pointer arithmetic */
                int *ptr = arr3 + idx2;
                arr3[i] = *ptr + arr2[idx1 % n] * 3;
            } else {
                /* Alternative path with different indexing */
                arr1[i] = base - arr3[i % n];
                arr3[i] = arr2[i] * 2 - arr1[i-1];
            }
            
            /* Another operation with cross-iteration dependency */
            arr2[i] = arr1[i] + (arr2[i-1] & 0xFF);
        }
        
        /* Modify loop variables to prevent complete optimization */
        arr1[0] += j;
        arr2[0] ^= arr3[n-1];
    }
}

/* Main function with initialization and checksum */
int main(int argc, char **argv) {
    /* Use non-constant sizes to prevent unrolling */
    int size = (argc > 1) ? 500 : 250;
    int outer = (argc > 2) ? 10 : 5;
    
    /* Declare and initialize arrays */
    int arr1[500], arr2[500], arr3[500];
    int i;
    
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        arr3[i] = i * 7 + 3;
    }
    
    /* Perform computation */
    compute_loop(arr1, arr2, arr3, size, outer);
    
    /* Calculate checksum to prevent dead code elimination */
    long checksum = 0;
    for (i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
        /* Mix in some bit operations */
        checksum ^= (arr1[i] << (i % 16));
    }
    
    /* Print result (prevents optimization) */
    volatile long result = checksum;
    
    return (int)(result % 1000);
}
