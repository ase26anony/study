/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to generate call RTL */
static int __attribute__((noinline)) helper_mul(int a, int b) {
    return a * b;
}

static float __attribute__((noinline)) helper_fmul(float a, float b) {
    return a * b;
}

/* Function with complex mixed operations */
__attribute__((noinline, optimize("O3")))
int test_mixed_operations(int* arr, int n, float* farr) {
    int int_sum = 0;
    float float_sum = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing mode */
        int val = arr[i];
        
        /* Integer operations with data dependency */
        int prod = helper_mul(val, i);
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : 100;
        
        /* Built-in function for complex RTL */
        int popcnt = __builtin_popcount(val);
        
        /* Floating point operations */
        float fval = farr[i];
        float fprod = helper_fmul(fval, i * 0.1f);
        
        /* Mixed type calculations */
        int_sum += prod + cond_val + popcnt;
        float_sum += fprod;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Branch to create control flow */
        if (i % 3 == 0) {
            int_sum += barrier;
            /* Another memory store */
            arr[i] = int_sum % 256;
        } else if (i % 3 == 1) {
            float_sum += barrier * 0.5f;
        } else {
            /* Use bit manipulation operations */
            int_sum ^= (val << 3);
        }
        
        /* Prevent loop unrolling from simplifying too much */
        barrier = arr[i % n];
    }
    
    return int_sum + (int)float_sum;
}

/* Function with nested loops for outer loop pipelining */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int* matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            /* Complex addressing calculation */
            int idx = i * cols + j;
            
            /* Data-dependent computation */
            int val = matrix[idx];
            
            /* Mix of arithmetic operations */
            int scaled = val * 3;
            int shifted = val >> 2;
            int masked = val & 0xFF;
            
            /* Conditional computation */
            int result = (scaled > shifted) ? scaled : shifted;
            result = (result > masked) ? result : masked;
            
            row_sum += result;
            
            /* Memory store with post-increment */
            matrix[idx] = result % 128;
        }
        
        /* Cross-iteration dependency */
        total += row_sum * i;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

/* Function using SIMD-like operations */
__attribute__((noinline, optimize("O3")))
int test_vector_ops(short* data, int len) {
    int sum = 0;
    
    for (int i = 0; i < len; i += 4) {
        /* Load multiple elements */
        short v0 = data[i];
        short v1 = data[i + 1];
        short v2 = data[i + 2];
        short v3 = data[i + 3];
        
        /* Parallel operations */
        int e0 = v0 * v0;
        int e1 = v1 * v1;
        int e2 = v2 * v2;
        int e3 = v3 * v3;
        
        /* Reduction */
        sum += e0 + e1 + e2 + e3;
        
        /* Conditional store */
        if (sum > 1000000) {
            data[i] = sum & 0xFFFF;
        }
    }
    
    return sum;
}

/* Main test driver */
int main() {
    const int SIZE = 256;
    const int MATRIX_SIZE = 16;
    
    /* Initialize test data */
    int* array = (int*)malloc(SIZE * sizeof(int));
    float* farray = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    short* vector = (short*)malloc(SIZE * sizeof(short));
    
    /* Fill with pseudo-random but deterministic data */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 13 + 7) % 1000;
        farray[i] = (float)(i * 0.1);
        vector[i] = (short)(i % 100);
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = (i * 17 + 11) % 500;
    }
    
    /* Run all test functions */
    int result1 = test_mixed_operations(array, SIZE, farray);
    int result2 = test_nested_loops(matrix, MATRIX_SIZE, MATRIX_SIZE);
    int result3 = test_vector_ops(vector, SIZE);
    
    /* Combine results */
    int final_result = result1 + result2 + result3;
    
    printf("Test results:\n");
    printf("  Mixed ops: %d\n", result1);
    printf("  Nested loops: %d\n", result2);
    printf("  Vector ops: %d\n", result3);
    printf("  Final: %d\n", final_result);
    
    /* Cleanup */
    free(array);
    free(farray);
    free(matrix);
    free(vector);
    
    return (final_result > 0) ? 0 : 1;
}
