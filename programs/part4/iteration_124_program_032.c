/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Non-inline helper functions to generate call RTL */
static int __attribute__((noinline)) helper_mul(int a, int b) {
    return a * b;
}

static float __attribute__((noinline)) helper_fmul(float a, float b) {
    return a * b;
}

/* Complex function with mixed operations - Test 1 */
int __attribute__((noinline, optimize("O3"))) 
test_mixed_operations(int* arr, int n, float* farr) {
    volatile int barrier; /* Prevent optimization */
    int int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Memory access - generates mem RTL */
        int val = arr[i];
        
        /* Data-dependent computation with conditional */
        int temp = (val > 100) ? val * 2 : val / 2;
        
        /* Mixed integer operations */
        int_sum += temp * i;
        int_sum += helper_mul(val, i);
        
        /* Floating point operations */
        float fval = farr[i];
        float ftemp = (fval > 50.0f) ? fval * 1.5f : fval / 1.5f;
        float_sum += ftemp * i;
        float_sum += helper_fmul(fval, (float)i);
        
        /* Built-in function - generates specific RTL */
        int_sum += __builtin_popcount(val);
        
        /* Inline assembly as scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 3 == 0) {
            int_sum += val << 2;
            barrier = int_sum; /* Volatile write */
        } else if (i % 3 == 1) {
            int_sum -= val >> 1;
        } else {
            int_sum ^= val;
        }
        
        /* Another memory write */
        arr[i] = int_sum % 256;
    }
    
    return int_sum + (int)float_sum;
}

/* Function with nested loops - Test 2 */
int __attribute__((noinline, optimize("O3")))
test_nested_loops(int* matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Complex addressing calculation */
            int idx = i * cols + j;
            
            /* Mixed-width operations (32-bit and 64-bit) */
            long long big_val = (long long)matrix[idx] * i * j;
            
            /* Conditional move simulation */
            int sign = (big_val > 0) ? 1 : -1;
            
            /* Bit manipulation */
            total += sign * (int)(big_val & 0xFFFFFFFF);
            total ^= (int)(big_val >> 32);
            
            /* Another scheduling barrier */
            asm volatile ("" : : : "memory");
            
            /* Store with different pattern */
            matrix[idx] = total % 1000;
        }
    }
    
    return total;
}

/* Function with pointer chasing - Test 3 */
int __attribute__((noinline, optimize("O3")))
test_pointer_chasing(int* data, int size, int iterations) {
    int sum = 0;
    int* ptr = data;
    
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < size - 1; i++) {
            /* Pointer arithmetic and dereference */
            int curr = *ptr;
            int next = *(ptr + 1);
            
            /* Complex dependency chain */
            int diff = abs(curr - next);
            sum += diff * i;
            
            /* Floating point conversion and operation */
            float fdiff = (float)diff;
            sum += (int)(fdiff * fdiff);
            
            /* Move pointer */
            ptr = (ptr + 1) % (data + size);
            
            /* Conditional with side effect */
            if (sum & 1) {
                asm volatile ("" : : : "memory");
                *ptr = sum % 256;
            }
        }
    }
    
    return sum;
}

/* Function using SIMD-like operations - Test 4 */
int __attribute__((noinline, optimize("O3"), target("arch=haswell")))
test_simd_patterns(short* shorts, int count) {
    int sum[4] = {0, 0, 0, 0};
    
    for (int i = 0; i < count - 3; i += 4) {
        /* Process 4 elements at a time */
        for (int j = 0; j < 4; j++) {
            /* Mixed operations */
            int val = shorts[i + j];
            sum[j] += val * val;
            sum[j] -= val >> 2;
            sum[j] ^= val << 3;
        }
        
        /* Cross-element dependencies */
        sum[0] += sum[1] * sum[2];
        sum[3] ^= sum[0] & sum[1];
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Store results back */
        for (int j = 0; j < 4; j++) {
            shorts[i + j] = (short)(sum[j] % 32768);
        }
    }
    
    return sum[0] + sum[1] + sum[2] + sum[3];
}

/* Main driver */
int main() {
    const int SIZE = 256;
    const int MATRIX_SIZE = 16;
    
    /* Allocate and initialize test data */
    int* array = (int*)malloc(SIZE * sizeof(int));
    float* farray = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    short* shorts = (short*)malloc(SIZE * sizeof(short));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 37 + 123) % 1000;
        farray[i] = (float)((i * 51 + 456) % 500) / 10.0f;
        shorts[i] = (short)((i * 73 + 789) % 1000);
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = (i * 29 + 321) % 800;
    }
    
    /* Run all tests */
    int result1 = test_mixed_operations(array, SIZE, farray);
    int result2 = test_nested_loops(matrix, MATRIX_SIZE, MATRIX_SIZE);
    int result3 = test_pointer_chasing(array, SIZE, 3);
    int result4 = test_simd_patterns(shorts, SIZE);
    
    /* Combine results */
    int final_result = result1 + result2 + result3 + result4;
    
    printf("Test Results:\n");
    printf("  Test 1: %d\n", result1);
    printf("  Test 2: %d\n", result2);
    printf("  Test 3: %d\n", result3);
    printf("  Test 4: %d\n", result4);
    printf("Final Result: %d\n", final_result);
    
    /* Cleanup */
    free(array);
    free(farray);
    free(matrix);
    free(shorts);
    
    return (final_result != 0) ? 0 : 1;
}
