/* Test for selective scheduler debug dumping in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline helper functions to generate call RTL */
static int __attribute__((noinline)) helper_mul(int a, int b) {
    return a * b;
}

static float __attribute__((noinline)) helper_fmul(float a, float b) {
    return a * b;
}

/* Complex loop with mixed operations - should create ILP opportunities */
__attribute__((noinline, optimize("O3")))
int test_selective_scheduling(int* array, int size) {
    int sum_int = 0;
    float sum_float = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < size; i++) {
        /* Memory access with addressing computation */
        int base_val = array[i];
        
        /* Integer operations with data dependency */
        int temp1 = base_val * i;
        int temp2 = helper_mul(temp1, i + 1);
        
        /* Conditional operation - may generate cond_exec RTL */
        int temp3 = (temp2 > 1000) ? temp2 : temp2 * 2;
        
        /* Floating point operations mixed in */
        float fval = (float)base_val * 0.5f;
        float ftemp = helper_fmul(fval, (float)i);
        
        /* Built-in function for complex RTL pattern */
        int popcnt = __builtin_popcount(temp3);
        
        /* Memory barrier to create scheduling regions */
        asm volatile ("" : : : "memory");
        
        /* Accumulate results with different operations */
        sum_int += temp3 + popcnt;
        sum_float += ftemp;
        
        /* Conditional branch to create control flow */
        if (i % 3 == 0) {
            sum_int -= array[i % size];
        } else if (i % 5 == 0) {
            sum_int += (int)ftemp;
        }
        
        /* Another scheduling barrier */
        barrier = i;
    }
    
    /* Final computation with ternary operator */
    int final_result = (sum_int > 0) ? sum_int : (int)sum_float;
    return final_result + barrier;
}

/* Another test with different patterns */
__attribute__((noinline, optimize("O3"), target("arch=core2")))
int test_vectorized_ops(int* data, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing with multiple computations */
        int idx1 = (i * 3) % n;
        int idx2 = (i * 7) % n;
        
        /* Mixed 32-bit and 64-bit operations */
        long long prod = (long long)data[idx1] * data[idx2];
        int hi_part = (int)(prod >> 32);
        int lo_part = (int)(prod & 0xFFFFFFFF);
        
        /* Bit manipulation operations */
        int rotated = (lo_part << 3) | (lo_part >> 29);
        int masked = rotated & 0x7F;
        
        /* Conditional move pattern */
        int selected = (hi_part > 0) ? hi_part : masked;
        
        /* Another scheduling barrier */
        asm volatile ("" : : : "memory");
        
        result ^= selected;
        
        /* Store back to memory */
        data[i % n] = result;
    }
    
    return result;
}

/* Test with nested loops for outer loop pipelining */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int rows, int cols, int* matrix) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Complex computation with multiple dependencies */
            int val = matrix[idx];
            int scaled = val * (i + 1) * (j + 1);
            
            /* Use of builtin for complex RTL */
            int parity = __builtin_parity(scaled);
            
            /* Floating point in inner loop */
            float fval = (float)scaled * 0.123f;
            
            row_sum += scaled + parity + (int)fval;
            
            /* Periodic scheduling barrier */
            if (j % 8 == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        
        /* Conditional update based on row_sum */
        total += (row_sum > 0) ? row_sum : -row_sum;
    }
    
    return total;
}

/* Main driver that calls all test functions */
int main() {
    const int SIZE = 256;
    int* array = (int*)malloc(SIZE * sizeof(int));
    int* data = (int*)malloc(SIZE * sizeof(int));
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 13 + 7) % 100;
        data[i] = (i * 17 + 11) % 100;
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = (i * 19 + 13) % 100;
    }
    
    /* Call test functions with different patterns */
    int result1 = test_selective_scheduling(array, SIZE);
    int result2 = test_vectorized_ops(data, SIZE);
    int result3 = test_nested_loops(16, 16, matrix);
    
    /* Use results to prevent dead code elimination */
    int final_result = result1 + result2 + result3;
    
    printf("Selective scheduling test result: %d\n", final_result);
    
    free(array);
    free(data);
    free(matrix);
    
    return 0;
}
