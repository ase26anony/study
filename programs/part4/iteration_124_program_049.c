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

/* Complex function with mixed operations */
__attribute__((noinline, optimize("O3")))
int test_function_1(int* arr, int n, int seed) {
    volatile int barrier;  /* Prevent optimization */
    int sum = seed;
    float fsum = seed * 0.5f;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing calculation */
        int val = arr[i];
        
        /* Integer operations with data dependency */
        int prod = helper_mul(val, i);
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : (val * 2);
        
        /* Built-in function for complex RTL */
        int bit_count = __builtin_popcount(val);
        
        /* Floating point operations */
        float fval = (float)val;
        fsum += helper_fmul(fval, fval * 0.1f);
        
        /* Mixed-type computation */
        sum += prod + cond_val + bit_count;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Another memory write */
        arr[i] = sum & 0xFF;
        
        /* Branch to create control flow */
        if (i % 3 == 0) {
            sum += i * 2;
        } else if (i % 3 == 1) {
            sum -= i;
        } else {
            sum ^= i;
        }
        
        barrier = i;  /* Volatile use */
    }
    
    /* Final computation with both int and float */
    return sum + (int)fsum;
}

/* Function with outer loop pipelining opportunities */
__attribute__((noinline, optimize("O3")))
int test_function_2(int* arr1, int* arr2, int n) {
    int total = 0;
    
    /* Outer loop for pipelining */
    for (int i = 0; i < n - 1; i++) {
        /* Inner computation with data dependencies */
        int a = arr1[i];
        int b = arr2[i];
        int c = arr1[i + 1];
        int d = arr2[i + 1];
        
        /* Complex expression with multiple operations */
        int temp = (a * b) + (c * d);
        temp = (temp > 0) ? temp : -temp;
        
        /* Using different bit widths */
        int64_t wide = (int64_t)a * (int64_t)b;
        total += (int)wide + temp;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Store result */
        arr1[i] = total & 0xFFFF;
    }
    
    return total;
}

/* Function with pointer chasing and complex addressing */
__attribute__((noinline, optimize("O3")))
int test_function_3(int** matrix, int rows, int cols) {
    int checksum = 0;
    
    for (int i = 0; i < rows; i++) {
        int* row = matrix[i];
        
        for (int j = 0; j < cols; j++) {
            /* Complex addressing mode */
            int val = row[j];
            
            /* Data-dependent computation */
            if (val % 2 == 0) {
                checksum += val * j;
            } else {
                checksum -= val * (i + 1);
            }
            
            /* Memory write with offset */
            row[j] = checksum & 0xFF;
            
            /* Periodic scheduling barrier */
            if (j % 4 == 0) {
                asm volatile("" : : : "memory");
            }
        }
    }
    
    return checksum;
}

/* Main driver */
int main() {
    const int SIZE = 256;
    const int MATRIX_SIZE = 16;
    
    /* Allocate and initialize test data */
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    int** matrix = (int**)malloc(MATRIX_SIZE * sizeof(int*));
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 37) % 101;
        arr2[i] = (i * 53) % 97;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        matrix[i] = (int*)malloc(MATRIX_SIZE * sizeof(int));
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix[i][j] = (i * MATRIX_SIZE + j) % 127;
        }
    }
    
    /* Call test functions with different patterns */
    int result1 = test_function_1(arr1, SIZE, 42);
    int result2 = test_function_2(arr1, arr2, SIZE);
    int result3 = test_function_3(matrix, MATRIX_SIZE, MATRIX_SIZE);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2 + result3;
    
    printf("Test results: %d, %d, %d\n", result1, result2, result3);
    printf("Final checksum: %d\n", final_result);
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(arr1);
    free(arr2);
    
    return (final_result != 0) ? 0 : 1;
}
