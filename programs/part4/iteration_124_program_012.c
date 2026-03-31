/* Test for selective scheduler debug dumping in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper to force call RTL */
static int __attribute__((noinline)) helper_multiply(int a, int b) {
    return a * b;
}

/* Non-inline helper with floating point */
static float __attribute__((noinline)) helper_float(float a, float b) {
    return a * b + a / b;
}

/* Test function with mixed operations to generate diverse RTL */
__attribute__((noinline, optimize("O3")))
int test_selective_sched_1(int* array, int size) {
    int sum = 0;
    int product = 1;
    float fsum = 0.0f;
    
    /* Create scheduling barrier */
    asm volatile("" : : : "memory");
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent computation with ILP opportunities */
        int val = array[i];
        
        /* Integer operations */
        sum += val * i;
        product *= (val > 0) ? val : 1;  /* Conditional move pattern */
        
        /* Floating point operations */
        float fval = (float)val;
        fsum += fval * i + fval / (i + 1);
        
        /* Built-in function for complex RTL */
        sum += __builtin_popcount(val);
        
        /* Memory access pattern */
        if (i % 2 == 0) {
            array[i] = sum;  /* Store operation */
        } else {
            array[i] = product;  /* Different store operation */
        }
        
        /* Call to non-inline function */
        sum += helper_multiply(val, i);
        
        /* Another scheduling barrier */
        if (i % 8 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Final computation with conditional */
    int result = (sum > product) ? sum : product;
    result += (int)fsum;
    
    return result;
}

/* Second test with different patterns */
__attribute__((noinline, optimize("O3"), target("arch=haswell")))
int test_selective_sched_2(int* data, int n) {
    volatile int barrier = 0;  /* Prevent optimization */
    int64_t big_sum = 0;
    int32_t small_sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Mixed 32-bit and 64-bit operations */
        int32_t val32 = data[i];
        int64_t val64 = (int64_t)val32;
        
        /* Complex expression with multiple dependencies */
        big_sum += val64 * i * (i + 1) / 2;
        small_sum += val32 << (i % 16);
        
        /* Conditional with side effects */
        if (val32 > barrier) {
            data[i] = val32 * 2;
            barrier = val32;
        } else {
            data[i] = val32 / 2;
        }
        
        /* Use ternary operator for conditional move */
        int temp = (i % 3 == 0) ? 
                   helper_multiply(val32, i) : 
                   __builtin_ffs(val32);
        
        small_sum += temp;
        
        /* Floating point in the mix */
        if (i % 4 == 0) {
            float ftemp = helper_float((float)val32, (float)(i + 1));
            small_sum += (int)ftemp;
        }
    }
    
    /* Combine results */
    int result = (int)(big_sum % 0x7FFFFFFF) + small_sum;
    
    /* Force memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Third test with nested loops */
__attribute__((noinline, optimize("O3")))
int test_selective_sched_3(int* matrix, int rows, int cols) {
    int total = 0;
    
    for (int r = 0; r < rows; r++) {
        int row_sum = 0;
        
        /* Inner loop with pipelining opportunities */
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            int val = matrix[idx];
            
            /* Complex data-dependent computation */
            row_sum += val * (r + 1) * (c + 1);
            
            /* Bit manipulation operations */
            row_sum ^= (val << (c % 8));
            row_sum |= __builtin_popcount(val);
            
            /* Conditional store */
            matrix[idx] = (val > row_sum) ? val : row_sum;
            
            /* Periodic scheduling barrier */
            if (c % 16 == 0) {
                asm volatile("" : : : "memory");
            }
        }
        
        total += row_sum;
        
        /* Call external function periodically */
        if (r % 5 == 0) {
            total = helper_multiply(total, r + 1);
        }
    }
    
    return total;
}

/* Main function to run all tests */
int main() {
    const int SIZE = 256;
    const int ROWS = 16;
    const int COLS = 16;
    
    /* Initialize test data */
    int* array1 = (int*)malloc(SIZE * sizeof(int));
    int* array2 = (int*)malloc(SIZE * sizeof(int));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    if (!array1 || !array2 || !matrix) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 13 + 7) % 100;
        array2[i] = (i * 17 + 11) % 100;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = (i * 19 + 13) % 100;
    }
    
    /* Run tests */
    int result1 = test_selective_sched_1(array1, SIZE);
    int result2 = test_selective_sched_2(array2, SIZE);
    int result3 = test_selective_sched_3(matrix, ROWS, COLS);
    
    /* Combine results */
    int final_result = result1 + result2 + result3;
    
    printf("Test results:\n");
    printf("  Test 1: %d\n", result1);
    printf("  Test 2: %d\n", result2);
    printf("  Test 3: %d\n", result3);
    printf("  Final: %d\n", final_result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(matrix);
    
    return 0;
}
