/* Test case for modulo-scheduler coverage of move dependency calculations */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call RTL */
static int __attribute__((const)) helper(int a, int b) {
    return (a * b) >> 1;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int k) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < 3; j++) {
        /* Inner loop with recurrence and complex dependencies */
        for (i = 1; i < n; i++) {
            int idx1 = i + k;
            int idx2 = i - 1;
            int idx3 = i * 2;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            int base = arr1[idx2] + arr2[idx1];
            
            /* Conditional operations creating control flow */
            if (base > 100) {
                /* Complex operation with function call */
                arr1[i] = helper(base, arr3[idx3]) + (j * 7);
            } else {
                /* Different operation path */
                arr1[i] = base - arr3[idx3 % n] + (j * 3);
            }
            
            /* Additional recurrence with pointer arithmetic */
            int *ptr = &arr2[i];
            arr3[i] = *ptr + arr3[idx2] + (i & 0xF);
        }
        
        /* Modify k slightly each outer iteration */
        k = (k + 2) % 5;
    }
}

/* Main function with initialization and checksum */
int main(int argc, char **argv) {
    /* Use non-constant trip count */
    int size = (argc > 1) ? 200 : 250;
    int k_val = (argc > 2) ? 3 : 5;
    
    /* Declare and initialize arrays */
    int array1[500];
    int array2[500];
    int array3[500];
    
    int i;
    for (i = 0; i < 500; i++) {
        array1[i] = i * 2;
        array2[i] = i + 5;
        array3[i] = i * 3;
    }
    
    /* Perform computation */
    compute_loop(array1, array2, array3, size, k_val);
    
    /* Calculate checksum to prevent dead code elimination */
    long checksum = 0;
    for (i = 0; i < size; i++) {
        checksum += array1[i] + array2[i] + array3[i];
    }
    
    /* Print checksum (prevents optimization) */
    volatile long result = checksum;
    
    return 0;
}
