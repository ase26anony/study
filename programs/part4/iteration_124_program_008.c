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

/* Function with complex loop to create ILP opportunities */
__attribute__((noinline, optimize("O3")))
int test_complex_loop(int* array, int size) {
    int sum_int = 0;
    float sum_float = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < size; i++) {
        /* Mixed integer operations */
        int temp = array[i];
        int squared = temp * temp;
        int cubed = squared * temp;
        
        /* Conditional move/ternary operation */
        int select = (temp > 0) ? squared : cubed;
        
        /* Memory access pattern */
        array[i] = select + i;
        
        /* Floating point operations */
        float f_temp = (float)temp;
        float f_squared = f_temp * f_temp;
        float f_cubed = f_squared * f_temp;
        
        /* Built-in function */
        int popcnt = __builtin_popcount(temp);
        
        /* Inline assembly as scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Accumulate results */
        sum_int += select + popcnt;
        sum_float += f_squared - f_cubed;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 3 == 0) {
            sum_int += helper_mul(temp, i);
        } else if (i % 3 == 1) {
            sum_float += helper_fmul(f_temp, (float)i);
        } else {
            /* Complex expression with multiple operations */
            sum_int ^= (temp << (i & 7)) | (temp >> (8 - (i & 7)));
        }
        
        barrier = i; /* Volatile write to prevent dead code elimination */
    }
    
    return sum_int + (int)sum_float + barrier;
}

/* Function with nested loops for outer-loop pipelining */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int rows, int cols, int* matrix) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Data-dependent computation */
            int val = matrix[idx];
            int transformed = (val * 1103515245 + 12345) & 0x7fffffff;
            
            /* Conditional operation */
            int masked = (j % 2 == 0) ? transformed : ~transformed;
            
            /* Mixed-width operations */
            int64_t wide = (int64_t)val * (int64_t)transformed;
            int narrow = (int)(wide >> 16);
            
            row_sum += masked ^ narrow;
            
            /* Store back with computation */
            matrix[idx] = (masked + narrow) & 0xFF;
            
            /* Another scheduling barrier */
            asm volatile("" : : : "memory");
        }
        total += row_sum;
    }
    
    return total;
}

/* Function with switch statement for control flow complexity */
__attribute__((noinline, optimize("O3")))
int test_control_flow(int x, int y) {
    int result = 0;
    
    switch (x % 5) {
        case 0:
            result = x * y + __builtin_clz(x);
            break;
        case 1:
            result = (x > y) ? x - y : y - x;
            result += __builtin_ctz(y | 1);
            break;
        case 2:
            result = x ^ y;
            for (int i = 0; i < 8; i++) {
                result = (result << 1) | ((result >> 31) & 1);
            }
            break;
        case 3:
            result = helper_mul(x, y);
            result += __builtin_parity(result);
            break;
        case 4:
            result = x / (y | 1);
            result *= __builtin_ffs(x);
            break;
    }
    
    /* Additional computation to create more scheduling opportunities */
    float f_result = (float)result;
    for (int i = 0; i < 4; i++) {
        f_result = f_result * 1.5f - 0.25f;
    }
    
    return result + (int)f_result;
}

/* Main test driver */
int main() {
    const int SIZE = 256;
    const int ROWS = 16;
    const int COLS = 16;
    
    /* Initialize test data */
    int* array = (int*)malloc(SIZE * sizeof(int));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    if (!array || !matrix) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with pseudo-random but deterministic data */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 1103515245 + 12345) & 0x7fff;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = (i * 1664525 + 1013904223) & 0xffff;
    }
    
    /* Run tests */
    int result1 = test_complex_loop(array, SIZE);
    int result2 = test_nested_loops(ROWS, COLS, matrix);
    int result3 = test_control_flow(result1, result2);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 + result2 + result3;
    
    printf("Test results: %d, %d, %d\n", result1, result2, result3);
    printf("Final checksum: %d\n", final_result);
    
    /* Cleanup */
    free(array);
    free(matrix);
    
    return 0;
}
