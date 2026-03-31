/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int a, int b) {
    return (a ^ b) + (a & b) * 2;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int k) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < 3; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n - k; i++) {
            int idx1 = i + j;          /* Multiple induction variables */
            int idx2 = i - 1 + j * 2;  /* Non-trivial indexing */
            int idx3 = i + k;          /* Index dependent on parameter */
            
            /* Recurrence: reads arr1[i-1] written in previous iteration */
            int base = arr1[i - 1] + arr2[idx3];
            
            /* Conditional operations creating control flow */
            if (idx1 % 4 == 0) {
                /* Different arithmetic path */
                arr1[i] = helper(base, arr3[idx2]) * 2;
            } else {
                /* Another arithmetic path with pointer arithmetic */
                int *ptr = &arr3[idx2];
                arr1[i] = base + (*ptr) / 3 + helper(i, k);
            }
            
            /* Additional recurrence with arr2 */
            arr2[i] = arr2[i] + arr1[i] - arr3[idx2 % n];
        }
        
        /* Small update between inner loop iterations */
        arr3[j] = arr1[n - k - 2] + arr2[1];
    }
}

/* Main function with initialization and checksum */
int main(int argc, char **argv) {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    int n = 400;  /* Non-constant trip count */
    int k = argc > 1 ? 5 : 10;  /* Parameter-dependent */
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 2 - 5;
        arr3[i] = i % 37;
    }
    
    /* Core computation */
    compute_loop(arr1, arr2, arr3, n, k);
    
    /* Checksum to prevent optimization */
    HOST_WIDE_INT sum = 0;
    for (i = 0; i < 500; i++) {
        sum += arr1[i] + arr2[i] * 2 - arr3[i];
    }
    
    /* Print checksum (prevents dead code elimination) */
    if (sum < 0) {
        return 1;
    }
    
    return 0;
}
