/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create RTL with latency */
static int __attribute__((const)) helper(int a, int b) {
    return (a * b) >> 1;
}

/* Core computational function with nested loops */
static void compute_loop(int *arr1, int *arr2, int *arr3, int n, int k) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < 3; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            int idx1 = i + k;
            int idx2 = i - 1;
            
            /* Conditional operations creating control flow */
            if (i % 2 == 0) {
                /* Even iterations: use helper function */
                int temp = helper(arr2[idx1], arr3[i]);
                
                /* Recurrence with multiple induction variables */
                arr1[i] = temp + arr1[idx2] + (j * 2);
                
                /* Additional operation with pointer arithmetic */
                *(arr3 + i) = arr1[i] - *(arr2 + idx1);
            } else {
                /* Odd iterations: different arithmetic */
                int temp = arr2[idx1] * 3;
                
                /* Different recurrence pattern */
                arr1[i] = temp + arr1[idx2] - (j * 3);
                
                /* Pointer arithmetic with offset */
                *(arr3 + i) = arr1[i] + *(arr2 + idx1 + 1);
            }
            
            /* Additional operation outside condition */
            arr2[i] = arr1[i] + i;
        }
        
        /* Modify k slightly each outer iteration */
        k = (k + 1) % 5;
    }
}

/* Main function */
int main() {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    HOST_WIDE_INT checksum = 0;
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 2;
        arr2[i] = i + 1;
        arr3[i] = i * 3;
    }
    
    /* Set initial recurrence seed */
    arr1[0] = 1;
    
    /* Call computational function with non-constant trip count */
    compute_loop(arr1, arr2, arr3, 400, 2);
    
    /* Calculate checksum to prevent optimization */
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum (use system call to avoid stdio) */
    __builtin_printf("Checksum: %ld\n", checksum);
    
    return 0;
}
