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

/* Function with complex loop body to create ILP opportunities */
__attribute__((noinline, optimize("O2")))
int test_loop_ilp(int* arr, int n) {
    int sum_int = 0;
    float sum_float = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Mixed integer operations */
        int temp = arr[i];
        int squared = temp * temp;
        int cubed = squared * temp;
        
        /* Data-dependent computation */
        sum_int += (i % 2) ? squared : cubed;
        
        /* Floating-point operations */
        float f_temp = (float)temp;
        sum_float += f_temp * i;
        
        /* Conditional move/ternary operation */
        int max_val = (temp > i) ? temp : i;
        sum_int += max_val;
        
        /* Memory barrier to create scheduling regions */
        asm volatile("" : : : "memory");
        
        /* Use helper functions */
        sum_int += helper_mul(temp, i);
        sum_float += helper_fmul(f_temp, (float)i);
        
        /* Built-in function */
        sum_int += __builtin_popcount(temp);
        
        /* Complex conditional with multiple basic blocks */
        if (temp > 1000) {
            sum_int += temp / 2;
            barrier = temp; /* Use volatile */
        } else if (temp < -1000) {
            sum_int -= temp * 2;
        } else {
            sum_int += temp;
        }
    }
    
    /* Mix results to prevent dead code elimination */
    return sum_int + (int)sum_float + barrier;
}

/* Function with nested loops for outer loop pipelining */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int** matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            /* Access 2D array with stride */
            int val = matrix[i][j];
            
            /* Mixed-width operations */
            int64_t wide_val = (int64_t)val * val;
            row_sum += (int)(wide_val & 0xFFFFFFFF) + (int)(wide_val >> 32);
            
            /* More complex RTL patterns */
            row_sum += (val > 0) ? val : -val; /* Absolute value */
            
            /* Memory operation with different addressing */
            if (j > 0) {
                matrix[i][j] = matrix[i][j-1] + val;
            }
        }
        total += row_sum;
        
        /* Scheduling barrier between outer loop iterations */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

/* Function with switch statement for control flow complexity */
__attribute__((noinline, target("arch=core2")))
int test_control_flow(int x, int y) {
    int result = 0;
    
    switch (x % 4) {
        case 0:
            result = y * 2;
            /* Complex expression */
            result += (y > 0) ? __builtin_clz(y) : __builtin_ctz(-y);
            break;
        case 1:
            result = y / 3;
            result += helper_mul(y, y);
            break;
        case 2:
            result = y << 2;
            /* Floating point in switch case */
            result += (int)((float)y * 1.5f);
            break;
        case 3:
            result = y ^ 0xAAAA;
            result = (result * 3) / 2;
            break;
    }
    
    /* Additional loop with early exit */
    for (int i = 0; i < 100; i++) {
        result += i;
        if (result > 10000) {
            result -= 5000;
            break;
        }
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    const int SIZE = 1000;
    const int ROWS = 50;
    const int COLS = 20;
    
    /* Allocate and initialize test data */
    int* array = (int*)malloc(SIZE * sizeof(int));
    int** matrix = (int**)malloc(ROWS * sizeof(int*));
    
    if (!array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 37 + 123) % 1000 - 500;
    }
    
    for (int i = 0; i < ROWS; i++) {
        matrix[i] = (int*)malloc(COLS * sizeof(int));
        if (!matrix[i]) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = (i * 50 + j * 7) % 2000 - 1000;
        }
    }
    
    /* Run all test functions */
    int result1 = test_loop_ilp(array, SIZE);
    int result2 = test_nested_loops(matrix, ROWS, COLS);
    int result3 = test_control_flow(123, 456);
    
    /* Combine results to ensure all code is live */
    int final_result = result1 + result2 + result3;
    
    /* Print result to prevent optimization */
    printf("Test results: %d, %d, %d\n", result1, result2, result3);
    printf("Final checksum: %d\n", final_result);
    
    /* Cleanup */
    for (int i = 0; i < ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(array);
    
    return 0;
}
