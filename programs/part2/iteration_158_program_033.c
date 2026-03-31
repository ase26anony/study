/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
static int __attribute__((const)) helper(int a, int b) {
    return (a ^ b) + (a & b) * 2;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < m; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n; i++) {
            int idx1 = i + j;      /* Non-trivial indexing */
            int idx2 = i - j - 1;  /* Another non-trivial index */
            
            /* Conditional operations inside loop */
            if (idx1 % 3 == 0) {
                /* Recurrence: arr1[i] depends on arr1[i-1] */
                int temp = arr1[i-1] + arr2[idx1];
                
                /* Function call creating instruction with latency */
                arr1[i] = helper(temp, arr3[idx2]) + (j * 2);
                
                /* Pointer arithmetic access */
                int *ptr = arr3 + idx2;
                arr2[i] = *ptr + arr1[i] / 2;
            } else if (idx1 % 3 == 1) {
                /* Different arithmetic path */
                arr1[i] = arr2[idx1] * 3 - arr1[i-1];
                
                /* More complex recurrence with helper */
                arr3[i] = helper(arr1[i], arr2[i]) + arr3[i-1];
            } else {
                /* Third path with mixed operations */
                arr1[i] = arr1[i-1] + (arr2[idx1] << 1);
                arr3[i] = arr3[i-1] - arr2[idx2];
                
                /* Nested conditional */
                if (arr1[i] > 1000) {
                    arr2[i] = arr2[i] / 2;
                }
            }
            
            /* Additional operation outside condition to create more moves */
            arr3[i] += (arr1[i] & 0xFF) | (arr2[i] & 0xFF00);
        }
        
        /* Boundary update to maintain dependencies across outer iterations */
        if (j > 0) {
            arr1[0] = arr2[n-1] + arr3[0];
            arr3[0] = helper(arr1[0], arr2[0]);
        }
    }
}

/* Main function with initialization and checksum */
int main(int argc, char **argv) {
    /* Medium-sized arrays to enable modulo-scheduling */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    int i;
    
    /* Initialize arrays with pattern to create dependencies */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 2;
        arr2[i] = i * 3 + 1;
        arr3[i] = i * 5 - 2;
    }
    
    /* Non-constant trip counts to prevent unrolling */
    int n = (argc > 1) ? 400 : 350;
    int m = (argc > 2) ? 5 : 3;
    
    /* Core computation */
    compute_loop(arr1, arr2, arr3, n, m);
    
    /* Checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
        /* Mix in some bit operations */
        checksum = (checksum << 3) | (checksum >> 61);
    }
    
    /* Print checksum (use simple output to avoid stdio.h) */
    volatile HOST_WIDE_INT *output = (volatile HOST_WIDE_INT *)&checksum;
    
    return (int)(checksum & 0x7FFFFFFF);
}
