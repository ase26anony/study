/* File 1: Integer processing with complex dependencies */
#include <stdlib.h>

/* Complex integer computation with data dependencies */
int process_integers(int n, int* restrict arr1, int* restrict arr2) {
    int sum = 0;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Outer loop creates scheduling pressure */
    for (int i = 0; i < n; i++) {
        int temp = arr1[i];
        
        /* Inner loop with mixed operations */
        for (int j = 0; j < 8; j++) {
            /* Create data dependencies and varied operations */
            temp = (temp * 3 + j) >> 1;
            temp = temp ^ (temp << 3);
            temp = temp + arr2[j];
            
            /* Conditional creates branch scheduling needs */
            if (temp & 1) {
                temp = temp * 7 - 5;
            } else {
                temp = temp / 3 + 2;
            }
        }
        
        /* Memory access with dependency */
        arr1[i] = temp;
        sum += temp;
        
        /* Create anti-dependency */
        barrier = i;
    }
    
    return sum;
}

/* Another function with different pattern */
int matrix_multiply_partial(int size, int* restrict A, int* restrict B, int* restrict C) {
    int result = 0;
    
    /* Nested loops create scheduling complexity */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int sum = 0;
            for (int k = 0; k < size; k++) {
                /* Complex addressing calculations */
                int idx_a = i * size + k;
                int idx_b = k * size + j;
                sum += A[idx_a] * B[idx_b];
                
                /* Conditional inside innermost loop */
                if (sum > 1000) {
                    sum = sum >> 2;
                }
            }
            C[i * size + j] = sum;
            result ^= sum; /* Non-linear accumulation */
        }
    }
    
    return result;
}
