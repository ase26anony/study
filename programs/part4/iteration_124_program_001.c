/* sel-sched-test.c
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fdump-rtl-sched -fdump-rtl-sched2 -fdump-rtl-all -o sel-sched-test sel-sched-test.c
 */

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

/* Test function 1: Mixed integer/floating point operations with memory accesses */
__attribute__((noinline, optimize("O2")))
int test_mixed_operations(int* arr, int n) {
    int int_sum = 0;
    float float_sum = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Integer operations with data dependency */
        int val = arr[i];
        int_sum += val * i;
        
        /* Floating point operations */
        float fval = (float)val;
        float_sum += fval * (float)i;
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : 100;
        int_sum += cond_val;
        
        /* Built-in function for complex RTL */
        int_sum += __builtin_popcount(val);
        
        /* Memory barrier to create scheduling region */
        asm volatile("" : : : "memory");
        
        /* Another memory access */
        arr[i] = val + 1;
        
        /* Use helper functions for call RTL */
        int_sum += helper_mul(val, i);
        float_sum += helper_fmul(fval, (float)i);
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 3 == 0) {
            int_sum += val * 2;
            barrier = int_sum; /* Volatile write */
        } else if (i % 3 == 1) {
            int_sum -= val;
            barrier = int_sum;
        } else {
            int_sum ^= val;
            barrier = int_sum;
        }
    }
    
    return int_sum + (int)float_sum;
}

/* Test function 2: Nested loops with pointer arithmetic */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int* matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int* row = matrix + i * cols;
        int row_sum = 0;
        
        for (int j = 0; j < cols; j++) {
            /* Complex addressing mode */
            int elem = row[j];
            
            /* Mixed operations */
            row_sum += elem * (i + j);
            row_sum ^= elem << (j % 8);
            
            /* Floating point in inner loop */
            float felem = (float)elem;
            total += (int)(felem * 0.5f);
            
            /* Conditional based on multiple variables */
            int tmp = (i > j) ? elem : row_sum;
            total += tmp;
        }
        
        /* Outer loop computation */
        total += row_sum * i;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

/* Test function 3: 64-bit operations and complex control flow */
__attribute__((noinline, target("arch=haswell")))
int64_t test_64bit_ops(int64_t* data, int size) {
    int64_t sum = 0;
    int32_t partial = 0;
    
    for (int i = 0; i < size; i++) {
        int64_t val = data[i];
        
        /* 64-bit operations */
        sum += val * i;
        sum ^= val >> 32;
        
        /* 32-bit operations mixed with 64-bit */
        partial += (int32_t)val;
        
        /* Complex conditional with side effects */
        if (val & 1) {
            sum += partial;
            partial = 0;
        } else {
            sum -= val;
        }
        
        /* Memory store with different type */
        data[i] = sum;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Use both 32-bit and 64-bit builtins */
        sum += __builtin_popcountll(val);
        partial += __builtin_popcount((uint32_t)val);
    }
    
    return sum + partial;
}

/* Test function 4: Array reduction with unpredictable access pattern */
__attribute__((noinline))
int test_unpredictable_access(int* arr, int* indices, int n) {
    int sum = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Unpredictable memory access */
        int idx = indices[i] % n;
        int val = arr[idx];
        
        /* Data-dependent computation */
        sum += val * (i % 16);
        
        /* Type conversion and FP operation */
        fsum += (float)val * 1.5f;
        
        /* Complex expression with multiple operators */
        sum += (val > 0) ? (val << 2) : (-val >> 1);
        
        /* Periodic scheduling barrier */
        if (i % 8 == 0) {
            asm volatile("" : : : "memory");
        }
        
        /* Update array element */
        arr[idx] = val + i;
    }
    
    /* Final mixed-type computation */
    return sum + (int)fsum;
}

/* Main driver that calls all test functions */
int main() {
    const int SIZE = 256;
    const int MATRIX_SIZE = 16;
    
    /* Allocate and initialize test data */
    int* array1 = (int*)malloc(SIZE * sizeof(int));
    int* array2 = (int*)malloc(SIZE * sizeof(int));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    int64_t* array64 = (int64_t*)malloc(SIZE * sizeof(int64_t));
    int* matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 37 + 123) % 1000;
        array2[i] = (i * 51 + 456) % 1000;
        indices[i] = (i * 29 + 789) % SIZE;
        array64[i] = (int64_t)(i * 73 + 321) * 1000000;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = (i * 13 + 654) % 500;
    }
    
    /* Run all test functions */
    int result1 = test_mixed_operations(array1, SIZE);
    int result2 = test_nested_loops(matrix, MATRIX_SIZE, MATRIX_SIZE);
    int64_t result3 = test_64bit_ops(array64, SIZE);
    int result4 = test_unpredictable_access(array2, indices, SIZE);
    
    /* Combine results to produce final output */
    int final_result = result1 + result2 + (int)result3 + result4;
    
    printf("Test Results:\n");
    printf("  Mixed operations: %d\n", result1);
    printf("  Nested loops: %d\n", result2);
    printf("  64-bit operations: %lld\n", (long long)result3);
    printf("  Unpredictable access: %d\n", result4);
    printf("  Final combined result: %d\n", final_result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(indices);
    free(array64);
    free(matrix);
    
    return (final_result != 0) ? 0 : 1;
}
