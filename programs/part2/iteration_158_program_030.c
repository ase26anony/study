/* Test case for modulo-scheduling coverage */
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
            /* Multiple induction variables and non-trivial array indexing */
            int idx1 = i + j;
            int idx2 = i - 1;
            int idx3 = i * 2 - j;
            
            /* Conditional operations inside loop */
            if (idx1 % 3 == 0) {
                /* Recurrence: reads from arr1[i-1] */
                int temp = arr1[idx2] + arr2[idx1 % n];
                
                /* Function call with pointer arithmetic */
                arr1[i] = helper(temp, arr3[idx3 % n]) + (j * 2);
                
                /* Additional operation with different indexing */
                arr3[i] = arr2[(i + j) % n] - arr1[idx2];
            } else if (idx1 % 3 == 1) {
                /* Alternative path with different recurrence pattern */
                arr1[i] = arr2[i] * 3 - arr1[idx2];
                arr3[i] = helper(arr1[i], arr2[(i - j + n) % n]);
            } else {
                /* Third path with more complex operations */
                int val1 = arr2[(i * 3) % n];
                int val2 = arr3[(i + j * 2) % n];
                arr1[i] = val1 + val2 + arr1[idx2];
                arr3[i] = helper(val1, val2) * 2;
            }
            
            /* Additional operation outside condition but inside loop */
            arr2[i] = arr2[i] + (i % 5) - j;
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        if (j > 0) {
            arr1[0] = arr1[n-1] + arr2[0];
        }
    }
}

/* Main function with initialization and checksum */
int main(int argc, char **argv) {
    /* Medium-sized arrays as required */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    HOST_WIDE_INT size = 200;  /* Non-constant trip count */
    HOST_WIDE_INT outer = 50;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 2;
        arr2[i] = i + 1;
        arr3[i] = i * 3 - 1;
    }
    
    /* Call computational function */
    compute_loop(arr1, arr2, arr3, size, outer);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 - arr3[i];
    }
    
    /* Print checksum (use simple output to avoid stdio.h) */
    volatile HOST_WIDE_INT *output = (volatile HOST_WIDE_INT *)&checksum;
    
    return (int)(checksum % 1000);
}
