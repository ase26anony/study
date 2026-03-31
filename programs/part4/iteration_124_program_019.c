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

/* Complex loop with mixed operations - Test 1 */
__attribute__((noinline, optimize("O3")))
int test_complex_loop(int* arr, int n) {
    int sum_int = 0;
    float sum_float = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing computation */
        int val = arr[i];
        
        /* Mixed integer operations */
        int temp1 = val * i;
        int temp2 = (val > 100) ? val : 100;  /* Conditional move */
        sum_int += temp1 + temp2;
        
        /* Floating point operations */
        float fval = (float)val;
        sum_float += fval * i;
        
        /* Built-in function */
        sum_int += __builtin_popcount(val);
        
        /* Inline assembly as scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 3 == 0) {
            sum_int += helper_mul(val, i);
        } else if (i % 3 == 1) {
            sum_float += helper_fmul(fval, i);
        } else {
            /* Complex expression with multiple operations */
            sum_int += ((val << 2) | (val >> 28)) & 0xFF;
        }
        
        barrier = i; /* Use volatile to prevent dead code elimination */
    }
    
    /* Mix results */
    return sum_int + (int)sum_float + barrier;
}

/* Test with nested loops - Test 2 */
__attribute__((noinline, optimize("O3"), target("arch=haswell")))
int test_nested_loops(int* matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            /* 2D array access with complex index */
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Mixed 32-bit and 64-bit operations */
            int64_t wide_val = (int64_t)val * j;
            row_sum += (int)(wide_val & 0xFFFFFFFF) + (int)(wide_val >> 32);
            
            /* More conditional operations */
            row_sum += (val % 2 == 0) ? val : -val;
            
            /* Another scheduling barrier */
            asm volatile("" : : : "memory");
        }
        total += row_sum;
        
        /* Outer loop has its own operations */
        if (i % 4 == 0) {
            total = helper_mul(total, 3);
        }
    }
    
    return total;
}

/* Test with pointer chasing - Test 3 */
__attribute__((noinline, optimize("O2")))
int test_pointer_chasing(int* data, int size, int iterations) {
    int sum = 0;
    int* current = data;
    
    for (int i = 0; i < iterations; i++) {
        /* Pointer dereference */
        int val = *current;
        
        /* Complex computation chain */
        int t1 = val * 1103515245 + 12345;
        int t2 = (t1 >> 16) & 32767;
        sum += t1 ^ t2;
        
        /* Update pointer with wrap-around */
        current = data + ((val + i) % size);
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* Use builtin for bit manipulation */
        sum += __builtin_ctz(val | 1);  /* Count trailing zeros */
    }
    
    return sum;
}

/* Test with SIMD-like operations - Test 4 */
__attribute__((noinline, optimize("O3"), target("arch=core2")))
int test_simd_patterns(short* shorts, int count) {
    int sum[4] = {0, 0, 0, 0};
    
    for (int i = 0; i < count; i += 4) {
        /* Process multiple elements - encourages vectorization attempts */
        for (int j = 0; j < 4 && (i + j) < count; j++) {
            short val = shorts[i + j];
            
            /* Different operations for different lanes */
            switch (j % 4) {
                case 0: sum[0] += val * 3; break;
                case 1: sum[1] += (val << 1) | (val >> 15); break;
                case 2: sum[2] += val > 0 ? val : -val; break;
                case 3: sum[3] += __builtin_popcount(val); break;
            }
        }
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum[0] + sum[1] + sum[2] + sum[3];
}

/* Main driver */
int main() {
    const int SIZE = 256;
    const int MATRIX_SIZE = 16;
    
    /* Initialize test data */
    int* data = (int*)malloc(SIZE * sizeof(int));
    int* matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    short* shorts = (short*)malloc(SIZE * sizeof(short));
    
    /* Fill with pseudo-random but deterministic data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        shorts[i] = (short)(data[i] & 0xFFFF);
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = (i * 1664525 + 1013904223) & 0xFF;
    }
    
    /* Run all tests */
    int result1 = test_complex_loop(data, SIZE);
    int result2 = test_nested_loops(matrix, MATRIX_SIZE, MATRIX_SIZE);
    int result3 = test_pointer_chasing(data, SIZE, 1000);
    int result4 = test_simd_patterns(shorts, SIZE);
    
    /* Combine results (prevents optimization) */
    int final_result = result1 + result2 + result3 + result4;
    
    printf("Test results:\n");
    printf("  Complex loop: %d\n", result1);
    printf("  Nested loops: %d\n", result2);
    printf("  Pointer chase: %d\n", result3);
    printf("  SIMD pattern: %d\n", result4);
    printf("  Final result: %d\n", final_result);
    
    free(data);
    free(matrix);
    free(shorts);
    
    return (final_result > 0) ? 0 : 1;
}
