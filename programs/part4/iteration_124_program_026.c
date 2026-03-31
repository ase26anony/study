/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Prevent constant propagation and inlining */
static volatile int external_seed = 42;

/* Non-inline helper functions to generate call RTL */
int __attribute__((noinline)) helper_mul(int a, int b) {
    return a * b;
}

float __attribute__((noinline)) helper_fmul(float a, float b) {
    return a * b;
}

/* Complex function with mixed operations and control flow */
int __attribute__((noinline, optimize("O2"))) 
test_selective_scheduling(int* array, int size) {
    int sum_int = 0;
    float sum_float = 0.0f;
    int conditional_sum = 0;
    
    /* Create scheduling barriers with inline assembly */
    asm volatile("" : : : "memory");
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent computation with ILP opportunities */
        int val = array[i] ^ external_seed;
        
        /* Mixed integer operations */
        int mul_result = helper_mul(val, i);
        int shift_result = val << (i & 0x7);
        
        /* Complex conditional with ternary operator */
        int cond_val = (val > 100) ? val * 2 : val / 2;
        
        /* Branch with different computation paths */
        if (i % 3 == 0) {
            /* Path 1: Integer heavy */
            sum_int += mul_result + shift_result;
            conditional_sum ^= cond_val;
            
            /* Builtin function for complex RTL */
            int bitcount = __builtin_popcount(val);
            sum_int += bitcount;
        } else if (i % 3 == 1) {
            /* Path 2: Float heavy */
            float fval = (float)val * 0.5f;
            float fmul = helper_fmul(fval, fval);
            sum_float += fmul;
            
            /* Memory access pattern */
            array[i] = (int)(fmul * 100.0f);
        } else {
            /* Path 3: Mixed operations */
            sum_int += val * i;
            sum_float += sqrtf(fabsf((float)val));
            
            /* Conditional move simulation */
            int max_val = (val > array[(i + 1) % size]) ? val : array[(i + 1) % size];
            conditional_sum += max_val;
        }
        
        /* Scheduling barrier every 8 iterations */
        if ((i & 0x7) == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Final mixing of results */
    int final_result = sum_int + (int)sum_float + conditional_sum;
    
    /* Another scheduling barrier */
    asm volatile("" : : : "memory");
    
    return final_result;
}

/* Second test with different patterns */
int __attribute__((noinline, optimize("O3"), target("arch=core2")))
test_vectorized_ops(short* data, int count) {
    int64_t wide_sum = 0;
    float fp_sum = 0.0f;
    
    /* Unrolled loop for pipelining */
    for (int i = 0; i < count; i += 4) {
        /* 32-bit and 64-bit operations */
        int32_t a = data[i];
        int32_t b = data[i + 1];
        int64_t product = (int64_t)a * (int64_t)b;
        
        /* Floating point with conversion */
        float fa = (float)a;
        float fb = (float)b;
        fp_sum += fa * fb;
        
        /* Complex expression with multiple uses */
        wide_sum += product + (a << 2) - (b >> 1);
        
        /* Conditional store */
        data[i + 2] = (product > 1000) ? (short)a : (short)b;
        
        /* Another builtin */
        int parity = __builtin_parity(a ^ b);
        wide_sum += parity;
    }
    
    /* Mix results with non-linear operation */
    int result = (int)(wide_sum % 1000000) + (int)(fp_sum * 100.0f);
    return result;
}

/* Third test with nested loops */
void __attribute__((noinline))
test_nested_loops(int n, int m, int* matrix) {
    for (int i = 0; i < n; i++) {
        int row_sum = 0;
        float row_avg = 0.0f;
        
        /* Inner loop with complex indexing */
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            int val = matrix[idx] ^ (i * j);
            
            /* Multiple dependent operations */
            int scaled = val * (i + 1);
            row_sum += scaled;
            
            /* Float computation */
            row_avg += (float)val / (j + 1);
            
            /* Conditional update */
            matrix[idx] = (scaled > 1000) ? scaled : val;
        }
        
        /* Store results with barrier */
        matrix[i] = row_sum + (int)(row_avg * 10.0f);
        asm volatile("" : : : "memory");
    }
}

/* Main driver */
int main() {
    const int SIZE = 256;
    const int MATRIX_SIZE = 16;
    
    /* Initialize test data */
    int* array = (int*)malloc(SIZE * sizeof(int));
    short* short_data = (short*)malloc(SIZE * sizeof(short));
    int* matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    
    /* Fill with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 1103515245 + 12345) & 0x7FFF;
        short_data[i] = (short)(array[i] & 0xFFFF);
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = (i * 1664525 + 1013904223) & 0xFFF;
    }
    
    /* Run all tests */
    int result1 = test_selective_scheduling(array, SIZE);
    int result2 = test_vectorized_ops(short_data, SIZE);
    test_nested_loops(MATRIX_SIZE, MATRIX_SIZE, matrix);
    
    /* Compute final verification result */
    int final_sum = result1 + result2;
    for (int i = 0; i < 16; i++) {
        final_sum += matrix[i];
    }
    
    printf("Test results: %d\n", final_sum);
    
    /* Cleanup */
    free(array);
    free(short_data);
    free(matrix);
    
    return 0;
}
