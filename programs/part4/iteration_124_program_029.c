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

/* Complex function with mixed operations to generate diverse RTL */
int __attribute__((noinline, optimize("O3"))) 
test_selective_scheduler(int* arr, int n, float* farr) {
    int int_sum = 0;
    float float_sum = 0.0f;
    int cond_result = 0;
    
    /* Create scheduling barriers and complex control flow */
    asm volatile ("" : : : "memory");
    
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with ILP opportunities */
        int val = arr[i];
        
        /* Integer operations with builtins */
        int popcnt = __builtin_popcount(val);
        int rev = __builtin_bswap32(val);
        
        /* Mixed integer operations */
        int prod = helper_mul(val, i);
        int_sum += prod + popcnt - rev;
        
        /* Floating point operations */
        float fval = farr[i];
        float fprod = helper_fmul(fval, i * 0.5f);
        float_sum += fprod;
        
        /* Conditional move/ternary operation */
        cond_result = (val > 1000) ? val : cond_result;
        
        /* Memory access pattern */
        arr[i] = int_sum & 0xFF;
        
        /* Complex conditional with multiple basic blocks */
        if (i % 3 == 0) {
            /* More operations in this branch */
            int_sum += __builtin_ctz(val | 1);
            asm volatile ("" : : : "memory");
        } else if (i % 3 == 1) {
            float_sum *= 0.99f;
        } else {
            /* Use bit manipulation operations */
            int_sum ^= (val << 3) | (val >> 29);
        }
        
        /* Another scheduling barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Final computation mixing types */
    return int_sum + (int)float_sum + cond_result;
}

/* Second test function with different patterns */
int __attribute__((noinline, optimize("O3"), target("arch=haswell")))
test_vectorized_ops(short* sarr, int n) {
    int sum = 0;
    long long lsum = 0;
    
    /* Mixed 32-bit and 64-bit operations */
    for (int i = 0; i < n; i++) {
        /* Sign extension operations */
        int extended = (int)sarr[i];
        
        /* 64-bit operations */
        lsum += (long long)extended * i;
        
        /* Bitfield operations */
        int masked = extended & 0x0F0F0F0F;
        sum += masked;
        
        /* Conditional with arithmetic */
        sum += (extended > 32767) ? extended : -extended;
        
        /* Complex expression with multiple uses */
        sum = (sum * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Mix 32-bit and 64-bit results */
    return sum + (int)(lsum & 0xFFFFFFFF);
}

/* Third test with nested loops */
float __attribute__((noinline, optimize("O3")))
test_nested_loops(float* matrix, int size) {
    float total = 0.0f;
    
    /* Outer loop with pipelining opportunities */
    for (int i = 0; i < size; i++) {
        float row_sum = 0.0f;
        
        /* Inner loop with reduction */
        for (int j = 0; j < size; j++) {
            float val = matrix[i * size + j];
            
            /* Complex floating point expression */
            row_sum += val * val * 0.5f - val * 0.25f;
            
            /* Conditional floating point operation */
            if (val > 0.0f) {
                row_sum += val * 0.1f;
            } else {
                row_sum -= val * 0.1f;
            }
        }
        
        total += row_sum;
        
        /* Memory barrier between outer loop iterations */
        asm volatile ("" : : : "memory");
    }
    
    return total;
}

/* Main driver that uses volatile to prevent optimization */
int main() {
    const int SIZE = 256;
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    short* short_array = (short*)malloc(SIZE * sizeof(short));
    float* matrix = (float*)malloc(SIZE * SIZE * sizeof(float));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        float_array[i] = (float)(i % 100) * 0.7f;
        short_array[i] = (short)(i * 13);
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = (float)((i * 17) % 100 - 50) * 0.02f;
    }
    
    /* Call test functions with volatile results */
    volatile int result1 = test_selective_scheduler(int_array, SIZE, float_array);
    volatile int result2 = test_vectorized_ops(short_array, SIZE);
    volatile float result3 = test_nested_loops(matrix, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %.2f\n", result1, result2, result3);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(short_array);
    free(matrix);
    
    return 0;
}
