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

/* Function with complex loop for selective scheduling */
__attribute__((noinline, optimize("O3")))
int test_selective_scheduling(int* array, int size) {
    int sum = 0;
    float fsum = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Mixed integer and floating point operations */
    for (int i = 0; i < size; i++) {
        /* Memory access - generates mem RTL */
        int val = array[i];
        
        /* Data-dependent computation with conditional */
        int temp = (val > 0) ? val : -val;
        
        /* Integer multiplication */
        int imul = helper_mul(temp, i);
        
        /* Floating point operation */
        float fmul = helper_fmul((float)temp, (float)i);
        
        /* Built-in function - generates specific RTL */
        int popcnt = __builtin_popcount(val);
        
        /* Complex expression with multiple operations */
        sum += (imul * popcnt) / (i + 1);
        fsum += fmul;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 3 == 0) {
            sum += val * 2;
            /* Another memory access */
            array[i] = sum % 256;
        } else if (i % 3 == 1) {
            /* Different computation path */
            sum -= val / 2;
            fsum *= 1.01f;
        } else {
            /* Third computation path */
            sum ^= val;
            fsum = fsum - (float)val;
        }
        
        /* 64-bit operations for different machine modes */
        uint64_t big_val = (uint64_t)val * (uint64_t)i;
        sum += (int)(big_val >> 32);
        
        /* Prevent loop unrolling from simplifying too much */
        barrier = i;
    }
    
    /* Use results to prevent dead code elimination */
    return sum + (int)fsum + barrier;
}

/* Second test function with different patterns */
__attribute__((noinline, optimize("O3"), target("arch=core2")))
float test_vectorized_ops(float* farr, int* iarr, int n) {
    float result = 0.0f;
    int checksum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Mixed-type operations */
        float f = farr[i];
        int ival = iarr[i];
        
        /* Conditional move pattern */
        float f_abs = (f < 0) ? -f : f;
        int i_abs = (ival < 0) ? -ival : ival;
        
        /* Complex floating point expression */
        result += f_abs * (float)i_abs + (float)(i % 8);
        
        /* Integer computation with shift */
        checksum ^= (ival << (i % 16)) | (ival >> (32 - (i % 16)));
        
        /* Memory store */
        farr[i] = result;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Nested conditionals */
        if (i % 5 == 0) {
            checksum += __builtin_ctz(ival | 1);
        }
    }
    
    /* Volatile write to ensure side effects */
    volatile float dummy = result;
    return result + (float)checksum;
}

/* Third test with outer loop pipelining opportunities */
__attribute__((noinline, optimize("O3")))
int test_outer_loop_pipelining(int** matrix, int rows, int cols) {
    int total = 0;
    
    for (int r = 0; r < rows; r++) {
        int row_sum = 0;
        for (int c = 0; c < cols; c++) {
            /* 2D array access */
            int val = matrix[r][c];
            
            /* Complex computation */
            int comp = val * r + c * c;
            
            /* Ternary operator with side effect */
            matrix[r][c] = (comp > 1000) ? comp % 1000 : comp;
            
            row_sum += comp;
            
            /* Inline assembly with clobber */
            if (c % 7 == 0) {
                asm volatile("" : : : "memory", "eax", "ebx");
            }
        }
        total += row_sum;
        
        /* Function call in outer loop */
        total = helper_mul(total, r + 1);
    }
    
    return total;
}

/* Main driver */
int main() {
    const int SIZE = 256;
    const int ROWS = 64;
    const int COLS = 64;
    
    /* Initialize test data */
    int* array = (int*)malloc(SIZE * sizeof(int));
    float* farray = (float*)malloc(SIZE * sizeof(float));
    int** matrix = (int**)malloc(ROWS * sizeof(int*));
    
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 37 + 123) % 1000;
        farray[i] = (float)((i * 51 + 456) % 1000) / 10.0f;
    }
    
    for (int r = 0; r < ROWS; r++) {
        matrix[r] = (int*)malloc(COLS * sizeof(int));
        for (int c = 0; c < COLS; c++) {
            matrix[r][c] = (r * c * 73) % 1000;
        }
    }
    
    /* Run all test functions */
    int result1 = test_selective_scheduling(array, SIZE);
    float result2 = test_vectorized_ops(farray, array, SIZE);
    int result3 = test_outer_loop_pipelining(matrix, ROWS, COLS);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + (int)result2 + result3;
    
    printf("Test Results:\n");
    printf("  test_selective_scheduling: %d\n", result1);
    printf("  test_vectorized_ops: %.2f\n", result2);
    printf("  test_outer_loop_pipelining: %d\n", result3);
    printf("  Final combined result: %d\n", final_result);
    
    /* Cleanup */
    for (int r = 0; r < ROWS; r++) {
        free(matrix[r]);
    }
    free(matrix);
    free(array);
    free(farray);
    
    return (final_result > 0) ? 0 : 1;
}
