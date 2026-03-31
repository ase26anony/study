/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int a, int b) {
    return (a ^ b) + (a & b);
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int k) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < 3; j++) {
        /* Inner loop with recurrence and complex dependencies */
        for (i = 1; i < n; i++) {
            /* Multiple induction variables and complex indexing */
            int idx1 = i + k;
            int idx2 = i - 1;
            int idx3 = i + j;
            
            /* Conditional operations creating control flow */
            if (i % 2 == 0) {
                /* Recurrence: arr1[i] depends on arr1[i-1] */
                int temp = arr1[idx2] + arr2[idx1];
                /* Function call with pointer arithmetic */
                arr1[i] = helper(temp, arr3[idx3 % n]);
                
                /* Additional operation with different indexing */
                arr3[i] = arr2[idx1] - arr1[idx2];
            } else {
                /* Alternative path with different recurrence */
                int temp = arr1[idx2] * 2 - arr2[idx1];
                /* Function call with different arguments */
                arr1[i] = helper(temp, arr3[(i + k) % n]);
                
                /* More complex indexing */
                arr3[i] = arr1[idx2] + arr2[(i * 2) % n];
            }
            
            /* Additional recurrence with stride */
            if (i > 2) {
                arr2[i] = arr1[i] + arr1[i-2] + arr3[i-1];
            }
        }
        
        /* Modify k slightly each outer iteration */
        k = (k + j) % 5;
    }
}

/* Main function */
int main() {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    int i;
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        arr3[i] = i * 7 + 3;
    }
    
    /* Call computational function with non-constant trip count */
    compute_loop(arr1, arr2, arr3, 400, 7);
    
    /* Compute checksum to prevent optimization */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
    }
    
    /* Print checksum (use simple output) */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
