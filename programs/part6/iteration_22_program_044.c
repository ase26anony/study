/* File 1: Integer processing with complex control flow */
#include <stdlib.h>
#include <stdio.h>

/* Complex integer computation with data dependencies */
int process_integers(int n, int* restrict arr1, int* restrict arr2, int* restrict result) {
    int sum = 0;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Outer loop with multiple exit conditions */
    for (int i = 0; i < n; i++) {
        int temp = arr1[i];
        
        /* Complex conditional with arithmetic */
        if (temp > 0) {
            /* Multiply-add chain with dependencies */
            temp = temp * 3 + 7;
            temp = (temp << 2) | (temp >> 30); /* Rotate */
            temp = temp * arr2[i] - i;
            
            /* Nested loop with variable bound */
            for (int j = 0; j < (temp & 0xF); j++) {
                temp += (j * arr1[(i + j) % n]) >> 1;
            }
        } else {
            /* Different computation path */
            temp = (temp * 5) - (arr2[i] << 1);
            temp = temp ^ (temp >> 16);
            
            /* Small unrolled computation */
            temp += (temp & 1) ? arr1[(i + 1) % n] : arr2[(i + 2) % n];
            temp += (temp & 2) ? arr1[(i + 3) % n] : arr2[(i + 4) % n];
        }
        
        /* Memory store with address computation */
        result[i] = temp;
        sum += temp;
        
        /* Artificial scheduling barrier */
        barrier = i;
    }
    
    /* Post-processing loop */
    int final = 0;
    for (int i = 0; i < (n & ~3); i += 4) {
        /* SIMD-like computation */
        int t0 = result[i] * result[i + 1];
        int t1 = result[i + 2] * result[i + 3];
        final += (t0 - t1) * (i >> 2);
    }
    
    return sum + final + barrier;
}

/* Another function with different pattern */
int matrix_chain_mult(int size, int* matrix) {
    int prod = 1;
    
    /* Triple nested loop - creates scheduling pressure */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int acc = 0;
            for (int k = 0; k < size; k++) {
                /* Complex address calculation */
                int idx1 = (i * size + k) % (size * size);
                int idx2 = (k * size + j) % (size * size);
                acc += matrix[idx1] * matrix[idx2];
                
                /* Conditional in innermost loop */
                if ((acc & 0xFF) == 0) {
                    acc ^= matrix[(i + j + k) % (size * size)];
                }
            }
            prod *= (acc & 0x7F) + 1;
        }
    }
    
    return prod;
}
