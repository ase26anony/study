/* Test case for modulo-scheduling coverage */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
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
            /* Multiple induction variables and non-trivial array indexing */
            int idx1 = i + k;
            int idx2 = i - 1;
            int idx3 = i + j;
            
            /* Conditional operations inside loop */
            if (idx1 % 3 == 0) {
                /* Recurrence: reads from arr1[i-1] */
                int temp = arr2[idx1] + arr1[idx2];
                /* Function call with pointer arithmetic */
                arr1[i] = helper(temp, arr3[idx3]) + (j * 2);
            } else if (idx1 % 3 == 1) {
                /* Different arithmetic operation */
                int temp = arr2[idx1] - arr1[idx2];
                /* More complex expression */
                arr1[i] = helper(temp, arr3[idx3] >> 1) * (j + 1);
            } else {
                /* Third variant with bitwise operations */
                int temp = arr2[idx1] ^ arr1[idx2];
                arr1[i] = helper(temp | arr3[idx3], i) & 0xFF;
            }
            
            /* Additional recurrence with arr3 */
            if (i > 2) {
                arr3[i] = arr3[i-1] + arr3[i-2] + arr1[i-1];
            }
        }
        
        /* Modify k to create varying patterns */
        k = (k + j) % 5;
    }
}

int main() {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5 + 1;
        arr3[i] = i * 7 + 2;
    }
    
    /* Non-constant trip count */
    int n = 400;
    int k = 7;
    
    /* Core computation */
    compute_loop(arr1, arr2, arr3, n, k);
    
    /* Checksum to prevent optimization */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
    }
    
    /* Print checksum (simulate printf) */
    volatile HOST_WIDE_INT *output = (volatile HOST_WIDE_INT *)&checksum;
    
    return 0;
}
