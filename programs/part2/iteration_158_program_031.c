/* Test case for modulo-scheduling with complex dependencies */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
static int __attribute__((const)) helper(int a, int b) {
    return (a * b) >> 1;
}

/* Core computational function with nested loops */
static void compute_loop(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < m; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            int idx1 = i + j;
            int idx2 = i - 1;
            int idx3 = i * 2 - j;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            int base = arr1[idx2];
            
            /* Conditional operations inside loop */
            if (idx1 % 3 == 0) {
                /* Use helper function call */
                int temp = helper(arr2[idx1], arr3[idx3 % n]);
                arr1[i] = base + temp + (idx1 >> 2);
            } else if (idx1 % 3 == 1) {
                /* Different arithmetic operation */
                arr1[i] = base - arr2[idx1] + (arr3[idx3 % n] * 2);
            } else {
                /* Yet another operation with pointer arithmetic */
                int *ptr = &arr2[idx1];
                arr1[i] = base + *ptr + (idx3 & 0xFF);
            }
            
            /* Additional recurrence with multiple induction variables */
            if (i > 2) {
                int idx4 = i - 2 + j;
                arr3[i] = arr1[i] + arr2[idx4 % n] - arr3[i-1];
            }
        }
        
        /* Modify arrays between inner loop iterations */
        for (i = 0; i < n; i++) {
            arr2[i] = arr1[i] + j;
        }
    }
}

/* Main function */
int main() {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    
    /* Initialize arrays */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 2;
        arr2[i] = i * 3;
        arr3[i] = i * 5;
    }
    
    /* Set up initial recurrence value */
    arr1[0] = 1;
    arr3[0] = 10;
    
    /* Call computational function with non-constant trip counts */
    compute_loop(arr1, arr2, arr3, 250, 100);
    
    /* Compute checksum to prevent optimization */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum (use simple output to avoid stdio.h) */
    volatile HOST_WIDE_INT *output = (volatile HOST_WIDE_INT *)&checksum;
    
    return (int)(checksum % 256);
}
