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
int test_selective_sched_loop(int* arr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing computation */
        int val = arr[i];
        
        /* Integer operations with data dependency */
        int prod = helper_mul(val, i);
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : prod;
        
        /* Mixed floating point operations */
        float fval = (float)val;
        float fprod = helper_fmul(fval, fval);
        
        /* Branch with different basic blocks */
        if (i % 3 == 0) {
            /* Block 1: Integer heavy */
            sum += cond_val * 2;
            fsum += fprod * 0.5f;
            
            /* Builtin function for complex RTL */
            sum += __builtin_popcount(val);
        } else if (i % 3 == 1) {
            /* Block 2: Memory and float heavy */
            arr[i] = cond_val + 1;
            fsum += fval * 3.14f;
            
            /* Inline assembly as scheduling barrier */
            asm volatile ("" : : : "memory");
        } else {
            /* Block 3: Mixed operations */
            sum += val ^ i;
            fsum += fprod / 2.0f;
            
            /* Another builtin */
            sum += __builtin_ffs(val);
        }
        
        /* Prevent loop unrolling from simplifying too much */
        barrier = i;
    }
    
    /* Combine results to prevent dead code elimination */
    return sum + (int)fsum + barrier;
}

/* Test with 64-bit operations - Test 2 */
__attribute__((noinline, optimize("O3"), target("arch=core2")))
int64_t test_64bit_ops(int64_t* arr, int n) {
    int64_t sum = 0;
    int64_t prod = 1;
    
    for (int i = 0; i < n; i++) {
        /* 64-bit memory access and arithmetic */
        int64_t val = arr[i];
        
        /* Complex 64-bit computation */
        int64_t rotated = (val << 5) | (val >> 59);
        
        /* Conditional based on value */
        if (val & 1) {
            sum += rotated * i;
            prod *= (val % 256) + 1;
        } else {
            sum += val / (i + 1);
            prod ^= rotated;
        }
        
        /* Memory store with computed address */
        arr[(i + 1) % n] = sum;
    }
    
    return sum + prod;
}

/* Nested loops for outer loop pipelining - Test 3 */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int** matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        float row_fsum = 0.0f;
        
        for (int j = 0; j < cols; j++) {
            /* 2D array access */
            int val = matrix[i][j];
            
            /* Mixed computations */
            row_sum += val * (i + j);
            row_fsum += (float)val * 1.5f;
            
            /* Periodic scheduling barrier */
            if ((j & 7) == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        
        /* Combine results with non-trivial operation */
        total += row_sum + (int)(row_fsum * 2.0f);
        
        /* Store back to prevent optimization */
        matrix[i][0] = row_sum;
    }
    
    return total;
}

/* Function with switch statement for varied control flow - Test 4 */
__attribute__((noinline, optimize("O3")))
int test_switch_case(int x, int y) {
    int result = 0;
    
    switch (x % 5) {
        case 0:
            result = y * 2;
            /* Builtin with side effect */
            result += __builtin_clz(y);
            break;
        case 1:
            result = y / 3;
            /* Memory barrier */
            asm volatile ("" : : : "memory");
            break;
        case 2:
            result = y ^ 0xABCD;
            /* Float conversion */
            result += (int)((float)y * 1.1f);
            break;
        case 3:
            result = helper_mul(y, y);
            /* Another builtin */
            result += __builtin_ctz(y | 1);
            break;
        default:
            result = y + 100;
            /* Complex computation */
            for (int i = 0; i < 4; i++) {
                result = (result << 3) | (result >> 29);
            }
            break;
    }
    
    return result;
}

/* Main driver that calls all test functions */
int main() {
    const int SIZE = 256;
    const int ROWS = 32;
    const int COLS = 32;
    
    /* Allocate and initialize test data */
    int* array = (int*)malloc(SIZE * sizeof(int));
    int64_t* array64 = (int64_t*)malloc(SIZE * sizeof(int64_t));
    int** matrix = (int**)malloc(ROWS * sizeof(int*));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 37 + 123) % 1000;
        array64[i] = (int64_t)(i * 73 + 456) % 5000;
    }
    
    for (int i = 0; i < ROWS; i++) {
        matrix[i] = (int*)malloc(COLS * sizeof(int));
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = (i * COLS + j * 19) % 800;
        }
    }
    
    /* Call test functions with different patterns */
    int result1 = test_selective_sched_loop(array, SIZE);
    int64_t result2 = test_64bit_ops(array64, SIZE);
    int result3 = test_nested_loops(matrix, ROWS, COLS);
    int result4 = test_switch_case(SIZE, 42);
    
    /* Combine results to ensure all code is live */
    int final_result = result1 + (int)result2 + result3 + result4;
    
    printf("Selective Scheduler Test Result: %d\n", final_result);
    
    /* Cleanup */
    for (int i = 0; i < ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(array);
    free(array64);
    
    return 0;
}
