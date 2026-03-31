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

/* Complex function with mixed operations and control flow */
__attribute__((noinline, optimize("O3")))
int test_function_1(int* arr, int n, int seed) {
    volatile int barrier; /* Prevent optimization */
    int sum_int = 0;
    float sum_float = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Create ILP opportunities with mixed operations */
        int idx = (i + seed) % n;
        int val = arr[idx];
        
        /* Integer operations with data dependencies */
        int prod = helper_mul(val, i);
        int shifted = prod << 2;
        int masked = shifted & 0xFF;
        
        /* Floating point operations */
        float fval = (float)val;
        float fprod = helper_fmul(fval, (float)i);
        sum_float += fprod;
        
        /* Conditional operations - may generate cond_exec RTL */
        int cond_val = (val > 100) ? val : (val * 2);
        
        /* Builtin function for complex RTL pattern */
        int popcnt = __builtin_popcount(val);
        
        /* Memory access pattern */
        arr[(i + 1) % n] = cond_val + popcnt;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Complex accumulation with branching */
        if (i % 3 == 0) {
            sum_int += masked * 3;
        } else if (i % 3 == 1) {
            sum_int += masked * 5;
        } else {
            sum_int += masked * 7;
        }
        
        /* Another barrier to create scheduling regions */
        barrier = i;
    }
    
    /* Mix results */
    return sum_int + (int)sum_float;
}

/* Function with 64-bit operations */
__attribute__((noinline, optimize("O3"), target("arch=core2")))
int64_t test_function_2(int64_t* arr, int n, int64_t seed) {
    int64_t sum = 0;
    volatile int64_t barrier;
    
    for (int i = 0; i < n; i++) {
        /* Mixed 32/64-bit operations */
        int64_t val = arr[i] ^ seed;
        int32_t low = (int32_t)val;
        int32_t high = (int32_t)(val >> 32);
        
        /* Complex 64-bit arithmetic */
        int64_t prod = val * (i + 1);
        int64_t shifted = prod >> (i % 8);
        
        /* Conditional move pattern */
        int64_t result = (prod > shifted) ? prod : shifted;
        
        /* Memory store with computation */
        arr[(i + 2) % n] = result + low - high;
        
        /* Barrier */
        asm volatile("" : : : "memory");
        
        /* Accumulate with branching */
        if (val & 1) {
            sum += result * 2;
        } else {
            sum += result / 2;
        }
        
        barrier = i;
    }
    
    return sum;
}

/* Function with pointer chasing and complex addressing */
__attribute__((noinline, optimize("O3")))
int test_function_3(int** matrix, int rows, int cols) {
    int total = 0;
    volatile int barrier;
    
    for (int i = 0; i < rows; i++) {
        int* row = matrix[i];
        for (int j = 0; j < cols; j++) {
            /* Complex addressing mode */
            int val = row[j] + row[(j + 1) % cols];
            
            /* Mixed operations */
            float fval = (float)val * 1.5f;
            int ival = (int)fval;
            
            /* Bit manipulation */
            int rotated = (ival << 4) | (ival >> 28);
            
            /* Update with barrier */
            row[j] = rotated;
            asm volatile("" : : : "memory");
            
            /* Conditional accumulation */
            total += (val > 0) ? rotated : -rotated;
        }
        barrier = i;
    }
    
    return total;
}

/* Main test driver */
int main() {
    const int SIZE = 256;
    const int MATRIX_SIZE = 16;
    
    /* Initialize test data */
    int* array1 = (int*)malloc(SIZE * sizeof(int));
    int64_t* array2 = (int64_t*)malloc(SIZE * sizeof(int64_t));
    int** matrix = (int**)malloc(MATRIX_SIZE * sizeof(int*));
    
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 37 + 123) % 1000;
        array2[i] = (int64_t)i * 7919;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        matrix[i] = (int*)malloc(MATRIX_SIZE * sizeof(int));
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix[i][j] = (i * MATRIX_SIZE + j) * 13;
        }
    }
    
    /* Call test functions to generate scheduling opportunities */
    int result1 = test_function_1(array1, SIZE, 42);
    int64_t result2 = test_function_2(array2, SIZE, 0x123456789ABCDEFLL);
    int result3 = test_function_3(matrix, MATRIX_SIZE, MATRIX_SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %lld, %d\n", 
           result1, (long long)result2, result3);
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(array1);
    free(array2);
    
    return 0;
}
