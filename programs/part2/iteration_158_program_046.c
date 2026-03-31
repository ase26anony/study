/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int a, int b) {
    return (a * 3) + (b >> 1);
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < m; j++) {
        int offset = j * 2;
        
        /* Inner loop with tight recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables and pointer arithmetic */
            int *ptr1 = arr1 + i;
            int *ptr2 = arr2 + i + offset;
            int *ptr3 = arr3 + i - 1;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            int base = arr1[i - 1];
            
            /* Conditional operations creating control flow */
            if ((i & 1) == 0) {
                /* Even indices: complex operation with function call */
                *ptr1 = helper(*ptr2, base) + *ptr3 + offset;
            } else {
                /* Odd indices: different arithmetic */
                *ptr1 = (*ptr2 * 2) - base + (*ptr3 >> 3);
            }
            
            /* Additional recurrence with arr3 */
            arr3[i] = arr3[i - 1] + (arr2[i] & 0xFF);
            
            /* Cross-iteration dependency with varying distance */
            if (i > 2) {
                arr2[i] += arr1[i - 2] - arr1[i - 3];
            }
        }
        
        /* Modify arrays for next outer iteration */
        arr1[0] = arr2[offset % n];
        arr3[0] = helper(arr1[n-1], arr2[0]);
    }
}

/* Main function with initialization and checksum */
int main(int argc, char **argv) {
    /* Medium-sized arrays to prevent complete unrolling */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    
    /* Initialize arrays with non-constant patterns */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3;
        arr2[i] = i + 7;
        arr3[i] = (i << 2) | 1;
    }
    
    /* Non-constant trip counts to prevent loop elimination */
    int n = (argc > 1) ? (argc * 50) : 250;
    int m = (argc > 2) ? (argc * 5) : 10;
    
    if (n > 500) n = 500;
    if (m > 20) m = 20;
    
    /* Core computation */
    compute_loop(arr1, arr2, arr3, n, m);
    
    /* Checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += arr1[i];
        checksum += arr2[i] * 2;
        checksum -= arr3[i];
    }
    
    /* Print checksum (prevents optimization) */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
