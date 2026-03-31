/* Test case for modulo-scheduling coverage */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call in loop */
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
            /* Multiple induction variables */
            int idx1 = i + j;
            int idx2 = i - 1;
            int idx3 = i + k;
            
            /* Conditional operations inside loop */
            if (idx1 % 2 == 0) {
                /* Recurrence: arr1 depends on its previous value */
                int temp = arr1[idx2] + arr2[idx3];
                
                /* Function call with pointer arithmetic */
                arr1[idx1] = helper(temp, arr3[i]) + (arr2[idx3] >> 2);
                
                /* Additional computation with different indexing */
                arr3[i] = arr2[idx3] - arr1[idx2];
            } else {
                /* Alternative path with different operations */
                int temp = arr1[idx2] * 2 - arr2[idx3];
                
                /* More complex indexing */
                arr1[idx1] = helper(temp, arr3[i - j]) | (arr2[idx3] << 1);
                
                /* Cross-iteration dependency */
                arr3[i] = arr3[i - 1] + arr2[idx3];
            }
            
            /* Additional recurrence with stride */
            if (i > 2) {
                arr2[i] = arr1[idx1] + arr1[i - 2] + arr3[i - 1];
            }
        }
        
        /* Modify k to vary inner loop trip count */
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
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < 500; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        arr3[i] = i * 7 + 3;
    }
    
    /* Call computational function with parameters */
    compute_loop(arr1, arr2, arr3, 400, 3);
    
    /* Compute checksum to prevent optimization */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 500; i++) {
        checksum += arr1[i] + arr2[i] * 2 + arr3[i] * 3;
    }
    
    /* Print checksum (simplified output) */
    if (checksum != 0) {
        /* Simple output mechanism */
        volatile int *output = (volatile int *)0x100000;
        *output = (int)(checksum & 0xFFFFFFFF);
    }
    
    return 0;
}
