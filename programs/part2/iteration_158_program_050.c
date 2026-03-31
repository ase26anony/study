/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int x, int y) {
    return (x ^ y) + (x & y);
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < m; j++) {
        /* Inner loop with recurrence and complex dependencies */
        for (i = 1; i < n; i++) {
            int idx1 = i + j;      /* Multiple induction variables */
            int idx2 = i - j;
            
            /* Conditional operations creating control flow */
            if (idx1 % 3 == 0) {
                /* Recurrence: reads from previous iteration */
                int temp = arr1[i-1] + arr2[idx1 % n];
                
                /* Function call creating RTL with latency */
                arr1[i] = helper(temp, arr3[i]) + (idx2 > 0 ? arr3[idx2 % n] : 0);
                
                /* Pointer arithmetic access */
                int *ptr = arr2 + (i % 16);
                arr3[i] = *ptr + (i & 7);
            } else {
                /* Alternative path with different recurrence */
                arr1[i] = arr2[i] + arr1[i-2] - arr3[(i+j) % n];
                
                /* More complex indexing */
                arr3[i] = arr2[(i * 3) % n] ^ arr1[(i / 2) % n];
            }
            
            /* Additional operation with cross-iteration dependency */
            if (i > 2) {
                arr2[i] = arr1[i-1] + arr1[i-3] + helper(i, j);
            }
        }
        
        /* Inter-iteration dependency between outer loop iterations */
        if (j > 0) {
            arr1[0] = arr2[n-1] + arr3[0];
        }
    }
}

/* Main function with initialization and checksum */
int main() {
    /* Medium-sized arrays as required */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    
    /* Initialize arrays with non-uniform values */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        arr3[i] = i * 7 + 3;
    }
    
    /* Make initial values non-zero for recurrence */
    arr1[0] = 42;
    arr1[1] = 17;
    
    /* Call computational function with non-constant trip counts */
    compute_loop(arr1, arr2, arr3, 250, 4);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
    }
    
    /* Print checksum (use simple output to avoid stdio.h) */
    volatile HOST_WIDE_INT *output = (volatile HOST_WIDE_INT *)&checksum;
    
    return 0;
}
