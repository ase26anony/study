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
int test_mixed_operations(int* arr, int n) {
    int sum_int = 0;
    float sum_float = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Integer operations with data dependency */
        int val = arr[i];
        int squared = val * val;
        
        /* Conditional move/ternary operation */
        int abs_val = (val < 0) ? -val : val;
        
        /* Mixed float/int operations */
        float fval = (float)val;
        float fresult = fval * 3.14159f;
        
        /* Built-in function for complex RTL */
        int popcnt = __builtin_popcount(abs_val);
        
        /* Memory access pattern */
        arr[i] = squared + popcnt;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Accumulate results */
        sum_int += helper_mul(abs_val, i);
        sum_float += helper_fmul(fresult, fval);
        
        /* Branch to create control flow */
        if (i % 3 == 0) {
            sum_int += barrier;
        } else if (i % 5 == 0) {
            sum_float += (float)barrier;
        }
        
        /* Another scheduling barrier */
        barrier = i;
    }
    
    return sum_int + (int)sum_float;
}

/* Test with nested loops - Test 2 */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int* matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Complex addressing mode */
            int elem = matrix[idx];
            
            /* Bit manipulation operations */
            int rotated = (elem << 3) | (elem >> 29);
            int masked = rotated & 0x7FFFFFFF;
            
            /* Conditional based on multiple factors */
            int processed = (i > j) ? masked : -masked;
            
            /* Use both 32-bit and 64-bit operations */
            int64_t wide = (int64_t)processed * (int64_t)(i + j);
            processed = (int)(wide >> 16);
            
            row_sum += processed;
            
            /* Store back with different pattern */
            matrix[idx] = (i % 2 == 0) ? processed : -processed;
            
            /* Scheduling barrier every 8 iterations */
            if (j % 8 == 0) {
                asm volatile("" : : : "memory");
            }
        }
        total += row_sum;
    }
    
    return total;
}

/* Test with pointer chasing - Test 3 */
__attribute__((noinline, optimize("O3")))
int test_pointer_chasing(int* data, int size, int stride) {
    int sum = 0;
    int* ptr = data;
    int* end = data + size;
    
    while (ptr < end) {
        /* Load with potential aliasing */
        int val = *ptr;
        
        /* Complex computation chain */
        int t1 = val * 1103515245 + 12345;
        int t2 = (t1 >> 16) & 32767;
        int t3 = t2 * val;
        int t4 = __builtin_bswap32(t3);
        
        /* Store with offset */
        *(ptr + stride) = t4;
        
        /* Update sum with conditional */
        sum += (t4 > 0) ? t4 : -t4;
        
        /* Pointer arithmetic with barrier */
        ptr += stride;
        asm volatile("" : : : "memory");
        
        /* Branch with unpredictable pattern */
        if ((sum & 0xFF) < 128) {
            ptr += 1;
        }
    }
    
    return sum;
}

/* Test with SIMD-like operations - Test 4 */
__attribute__((target("arch=haswell"), noinline, optimize("O3")))
int test_simd_patterns(short* shorts, int count) {
    int sum[4] = {0, 0, 0, 0};
    
    for (int i = 0; i < count; i += 4) {
        /* Manual unrolling for ILP */
        short s0 = shorts[i];
        short s1 = shorts[i + 1];
        short s2 = shorts[i + 2];
        short s3 = shorts[i + 3];
        
        /* Independent computations */
        int p0 = s0 * s0;
        int p1 = s1 * s1 * 2;
        int p2 = s2 * s2 * 3;
        int p3 = s3 * s3 * 4;
        
        /* Reduction with memory barriers */
        sum[0] += p0;
        asm volatile("" : : : "memory");
        sum[1] += p1;
        asm volatile("" : : : "memory");
        sum[2] += p2;
        asm volatile("" : : : "memory");
        sum[3] += p3;
        
        /* Conditional store */
        if (p0 > p1) {
            shorts[i] = (short)(p0 >> 16);
        }
    }
    
    return sum[0] + sum[1] + sum[2] + sum[3];
}

/* Main driver */
int main() {
    const int SIZE = 256;
    const int MATRIX_SIZE = 16;
    
    /* Allocate and initialize test data */
    int* data1 = (int*)malloc(SIZE * sizeof(int));
    int* data2 = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int* data3 = (int*)malloc(SIZE * sizeof(int));
    short* data4 = (short*)malloc(SIZE * sizeof(short));
    
    for (int i = 0; i < SIZE; i++) {
        data1[i] = (i * 37 + 123) & 0xFFF;
        data3[i] = (i * 73 + 456) & 0xFFF;
        data4[i] = (short)((i * 59 + 789) & 0x7FFF);
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        data2[i] = (i * 91 + 321) & 0xFFF;
    }
    
    /* Run all tests */
    int result1 = test_mixed_operations(data1, SIZE);
    int result2 = test_nested_loops(data2, MATRIX_SIZE, MATRIX_SIZE);
    int result3 = test_pointer_chasing(data3, SIZE, 3);
    int result4 = test_simd_patterns(data4, SIZE);
    
    /* Final computation to ensure all results are used */
    int final_result = result1 + result2 + result3 + result4;
    
    printf("Test Results:\n");
    printf("  Test 1: %d\n", result1);
    printf("  Test 2: %d\n", result2);
    printf("  Test 3: %d\n", result3);
    printf("  Test 4: %d\n", result4);
    printf("Final Result: %d\n", final_result);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    
    return (final_result > 0) ? 0 : 1;
}
