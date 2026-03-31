/* Test case for modulo-scheduling coverage */
typedef long HOST_WIDE_INT;

/* Pure helper function to create function call dependencies */
static int __attribute__((const)) helper(int x, int y) {
    return (x ^ y) + (x & y) * 2;
}

/* Core computational function with nested loops */
void compute_loop(int *arr1, int *arr2, int *arr3, int n, int k) {
    int i, j;
    
    /* Outer loop */
    for (j = 0; j < 3; j++) {
        /* Inner loop with recurrence and complex indexing */
        for (i = 1; i < n - k; i++) {
            int idx1 = i + k;
            int idx2 = i - 1;
            int idx3 = i + j;
            
            /* Recurrence: arr1[i] depends on arr1[i-1] */
            int base = arr1[idx2] + arr2[idx1];
            
            /* Conditional operations creating control flow */
            if (idx3 % 4 == 0) {
                /* Different operation path */
                arr1[i] = helper(base, arr3[idx1]) >> 1;
            } else if (idx3 % 4 == 1) {
                /* Another operation path */
                arr1[i] = helper(base, arr3[idx2]) * 2;
            } else if (idx3 % 4 == 2) {
                /* Yet another path with pointer arithmetic */
                int *ptr1 = &arr2[idx1];
                int *ptr2 = &arr3[idx2];
                arr1[i] = *ptr1 + (*ptr2 << 1);
            } else {
                /* Default path with multiple dependencies */
                arr1[i] = base + arr3[i] + (arr2[idx2] & 0xFF);
            }
            
            /* Additional computation with cross-iteration dependency */
            arr3[i] = arr1[i] + arr2[i] + (i % 8);
        }
        
        /* Small constant trip count loop to create more scheduling opportunities */
        for (i = n - k; i < n; i++) {
            arr2[i] = arr1[i - 1] + arr3[i];
        }
    }
}

/* Another computational function with different pattern */
void compute_loop2(int *a, int *b, int *c, int m, int stride) {
    int i, j;
    
    for (j = 0; j < 2; j++) {
        /* Loop with multiple induction variables */
        for (i = stride; i < m; i++) {
            int idx_a = i;
            int idx_b = i + stride;
            int idx_c = i - stride;
            
            /* Complex recurrence chain */
            int temp1 = a[idx_c] + b[idx_b];
            int temp2 = c[i] - a[idx_a];
            
            /* Conditional with function call */
            if ((i ^ j) & 1) {
                a[i] = helper(temp1, temp2) | 0x1;
            } else {
                a[i] = (temp1 * temp2) & 0xFF;
            }
            
            /* Multiple writes with dependencies */
            b[i] = a[i] + c[idx_c];
            c[i] = b[idx_b] - a[idx_a];
        }
    }
}

int main() {
    /* Medium-sized arrays to work with */
    int arr1[512];
    int arr2[512];
    int arr3[512];
    int arr4[256];
    int arr5[256];
    int arr6[256];
    
    int i;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 512; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5 + 1;
        arr3[i] = i * 7 + 2;
    }
    
    for (i = 0; i < 256; i++) {
        arr4[i] = i * 11;
        arr5[i] = i * 13 + 3;
        arr6[i] = i * 17 + 5;
    }
    
    /* Non-constant trip counts to prevent unrolling */
    int n1 = 500;
    int n2 = 240;
    int k1 = 7;
    int stride1 = 3;
    
    /* Call computational functions multiple times */
    compute_loop(arr1, arr2, arr3, n1, k1);
    compute_loop2(arr4, arr5, arr6, n2, stride1);
    
    /* Additional call with different parameters */
    compute_loop(arr3, arr1, arr2, n1 - 50, k1 + 2);
    
    /* Compute checksum to prevent dead code elimination */
    HOST_WIDE_INT checksum = 0;
    for (i = 0; i < 512; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    for (i = 0; i < 256; i++) {
        checksum += arr4[i] + arr5[i] + arr6[i];
    }
    
    /* Print checksum to ensure code is live */
    volatile HOST_WIDE_INT result = checksum;
    
    return 0;
}
