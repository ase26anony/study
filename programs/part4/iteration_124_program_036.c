/* Test program to trigger selective scheduler debug dumping in GCC */
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
__attribute__((noinline, optimize("O3")))
int test_function_1(int* arr, int n, int seed) {
    volatile int barrier; /* Prevent optimization */
    int sum = seed;
    float fsum = seed * 0.5f;
    
    /* Mixed integer and FP operations */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with memory access */
        int val = arr[i];
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : 100;
        
        /* Integer multiplication */
        int prod = helper_mul(val, i);
        
        /* Built-in function for complex RTL */
        int popcnt = __builtin_popcount(val);
        
        /* Floating point operation */
        float fval = val * 0.3f;
        fsum += helper_fmul(fval, i * 0.1f);
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Complex expression with multiple operations */
        sum += (prod + cond_val - popcnt) * (i % 8);
        
        /* Branch to create control flow */
        if (i % 3 == 0) {
            sum += arr[i % n] * 2;
        } else if (i % 3 == 1) {
            sum -= arr[i % n] / 2;
        } else {
            sum ^= arr[i % n];
        }
        
        barrier = i; /* Use volatile to prevent dead code elimination */
    }
    
    /* Final mixing */
    return sum + (int)fsum;
}

/* Another test function with different patterns */
__attribute__((noinline, optimize("O3"), target("arch=core2")))
int64_t test_function_2(int64_t* arr, int n, int64_t seed) {
    int64_t sum = seed;
    double dsum = seed * 0.25;
    
    /* Unrolled loop with mixed operations */
    for (int i = 0; i < n; i += 2) {
        /* 64-bit operations */
        int64_t a = arr[i];
        int64_t b = arr[i + 1];
        
        /* Complex 64-bit expression */
        int64_t diff = (a > b) ? a - b : b - a;
        
        /* Mixed 32/64 bit operations */
        int32_t low_a = a & 0xFFFFFFFF;
        int32_t low_b = b & 0xFFFFFFFF;
        
        /* Built-in for 64-bit popcount */
        int popcnt = __builtin_popcountll(a ^ b);
        
        /* Double precision operations */
        double da = a * 0.01;
        double db = b * 0.02;
        dsum += da * db;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* More complex computation */
        sum += diff * (low_a + low_b) / (popcnt + 1);
        
        /* Conditional with side effect */
        if ((a + b) % 7 == 0) {
            arr[i] = sum % 1000;
        }
    }
    
    return sum + (int64_t)dsum;
}

/* Function with pointer chasing to create memory dependencies */
__attribute__((noinline, optimize("O2")))
int test_function_3(int** matrix, int rows, int cols, int init) {
    int result = init;
    
    for (int i = 0; i < rows; i++) {
        int* row = matrix[i];
        int row_sum = 0;
        
        for (int j = 0; j < cols; j++) {
            /* Memory access with stride */
            row_sum += row[j] * (i + j);
            
            /* Conditional store */
            if (row_sum > 1000000) {
                row[j] = row_sum % 256;
                row_sum = 0;
            }
            
            /* Another scheduling barrier */
            if (j % 4 == 0) {
                asm volatile("" : : : "memory");
            }
        }
        
        /* Use built-in for parity */
        result ^= __builtin_parity(row_sum) ? row_sum : ~row_sum;
    }
    
    return result;
}

/* Main driver that calls all test functions */
int main(void) {
    const int SIZE = 256;
    const int MATRIX_SIZE = 32;
    
    /* Allocate and initialize test data */
    int* array1 = (int*)malloc(SIZE * sizeof(int));
    int64_t* array2 = (int64_t*)malloc(SIZE * sizeof(int64_t));
    int** matrix = (int**)malloc(MATRIX_SIZE * sizeof(int*));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 37 + 123) % 1000;
        array2[i] = (i * 73 + 456) % 2000;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        matrix[i] = (int*)malloc(MATRIX_SIZE * sizeof(int));
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix[i][j] = (i * MATRIX_SIZE + j * 19) % 500;
        }
    }
    
    /* Call test functions with different characteristics */
    int result1 = test_function_1(array1, SIZE, 42);
    int64_t result2 = test_function_2(array2, SIZE, 12345LL);
    int result3 = test_function_3(matrix, MATRIX_SIZE, MATRIX_SIZE, 999);
    
    /* Combine results to produce final output */
    int final_result = result1 + (int)(result2 % 1000000) + result3;
    
    printf("Test Results:\n");
    printf("  Function 1: %d\n", result1);
    printf("  Function 2: %lld\n", (long long)result2);
    printf("  Function 3: %d\n", result3);
    printf("  Final: %d\n", final_result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    for (int i = 0; i < MATRIX_SIZE; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    return (final_result > 0) ? 0 : 1;
}
