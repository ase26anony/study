/* Test case for modulo-scheduling coverage */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int x, int y) {
    return (x * y) >> 1;
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
            if (i % 2 == 0) {
                /* Recurrence: reads from arr1[i-1] written in previous iteration */
                int temp = arr2[idx1] + arr1[idx2];
                
                /* Function call with pointer arithmetic */
                arr1[i] = helper(temp, arr3[idx3]) + (i * 2);
                
                /* Additional operation with different indexing */
                arr3[i] = arr2[i] - arr1[idx2];
            } else {
                /* Alternative path with different recurrence pattern */
                int temp = arr2[i] * arr3[idx2];
                
                /* More complex indexing with modulo */
                int idx4 = (i + k) % n;
                arr1[i] = helper(temp, arr1[idx4]) + (i / 2);
                
                /* Pointer arithmetic access */
                arr3[i] = *(arr2 + idx1) + *(arr1 + idx2);
            }
            
            /* Additional recurrence to create more dependencies */
            arr2[i] = arr1[i] + arr2[i-1];
        }
        
        /* Modify k to change indexing pattern */
        k = (k + j) % 5;
    }
}

/* Main function */
int main() {
    /* Medium-sized arrays */
    int arr1[500];
    int arr2[500];
    int arr3[500];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < 500; i++) {
        arr1[i] = i % 100;
        arr2[i] = (i * 2) % 100;
        arr3[i] = (i * 3) % 100;
    }
    
    /* Initialize boundary values for recurrences */
    arr1[0] = 1;
    arr2[0] = 2;
    
    /* Call computational function with non-constant trip count */
    int n = 400;  /* Non-constant prevents complete unrolling */
    int k = 7;    /* Offset for indexing */
    compute_loop(arr1, arr2, arr3, n, k);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (int i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    /* Print checksum (use simple output to avoid stdio.h) */
    volatile HOST_WIDE_INT *output = (volatile HOST_WIDE_INT *)&checksum;
    
    return 0;
}
