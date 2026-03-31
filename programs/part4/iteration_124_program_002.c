/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions to prevent optimization */
static int volatile sink;

/* Non-inline helper to generate call RTL */
__attribute__((noinline, optimize("O2")))
int helper_multiply(int a, int b) {
    return a * b;
}

/* Non-inline helper with floating point */
__attribute__((noinline, optimize("O2")))
float helper_float(float a, float b) {
    return a * b + a / b;
}

/* Test 1: Complex integer loop with mixed operations */
__attribute__((noinline, optimize("O2")))
int test_complex_loop(int* array, int size) {
    int sum = 0;
    int product = 1;
    int conditional_sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Memory access - generates mem RTL */
        int val = array[i];
        
        /* Data-dependent computation */
        sum += val * i;
        
        /* Conditional operation - may generate cond_exec RTL */
        product *= (val > 0) ? val : 1;
        
        /* Built-in function - generates specific RTL pattern */
        conditional_sum += __builtin_popcount(val);
        
        /* Inline assembly as scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Mix with floating point in same loop */
        float ftemp = (float)val * 1.5f;
        sink = (int)ftemp;
    }
    
    return sum + product + conditional_sum;
}

/* Test 2: Nested loops with outer loop pipelining opportunities */
__attribute__((noinline, optimize("O2")))
int test_nested_loops(int* matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            /* 2D array access */
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Complex expression with multiple dependencies */
            row_sum += val * (i + j) / (j + 1);
            
            /* Conditional move pattern */
            int max_val = (val > row_sum) ? val : row_sum;
            
            /* Function call - generates call RTL */
            row_sum = helper_multiply(row_sum, max_val % 256);
            
            /* Floating point interleaved */
            if (j % 3 == 0) {
                float fval = helper_float((float)val, (float)(j + 1));
                sink = (int)fval;
            }
        }
        
        /* Branch with side effect */
        if (i % 2 == 0) {
            total += row_sum;
        } else {
            total -= row_sum / 2;
        }
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

/* Test 3: Mixed 32-bit and 64-bit operations */
__attribute__((noinline, optimize("O2")))
long long test_mixed_bitwidth(int* data, int n) {
    long long big_sum = 0;
    int small_sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* 32-bit operations */
        int val32 = data[i];
        small_sum += val32 << (i % 16);
        
        /* 64-bit operations */
        long long val64 = (long long)val32 * i;
        big_sum += val64;
        
        /* Complex condition with arithmetic */
        if ((val32 & 0xF) == 0) {
            big_sum -= small_sum;
        } else if ((val32 & 0x1) == 0) {
            big_sum += val64 / 3;
        } else {
            big_sum ^= val64;
        }
        
        /* Memory barrier every 8 iterations */
        if (i % 8 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return big_sum + small_sum;
}

/* Test 4: Pointer chasing and indirect calls */
__attribute__((noinline, optimize("O2")))
int test_pointer_chasing(int** ptr_array, int size) {
    int result = 0;
    int* current = ptr_array[0];
    
    for (int i = 0; i < size; i++) {
        /* Pointer dereference */
        int val = *current;
        
        /* Complex address calculation */
        current = ptr_array[(val + i) % size];
        if (!current) current = &result;
        
        /* Mixed operations */
        result ^= val * i;
        result += __builtin_ctz(val | 1);  /* Count trailing zeros */
        
        /* Floating point conversion */
        double dval = (double)val * 0.5;
        sink = (int)dval;
    }
    
    return result;
}

/* Main driver */
int main() {
    const int SIZE = 256;
    const int MATRIX_SIZE = 16;
    
    /* Initialize test data */
    int* array = (int*)malloc(SIZE * sizeof(int));
    int* matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int** ptr_array = (int**)malloc(SIZE * sizeof(int*));
    
    /* Fill with pseudo-random but deterministic data */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 13 + 7) % 100;
        ptr_array[i] = &array[(i * 17) % SIZE];
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = (i * 29 + 11) % 200;
    }
    
    /* Run all tests */
    int result1 = test_complex_loop(array, SIZE);
    int result2 = test_nested_loops(matrix, MATRIX_SIZE, MATRIX_SIZE);
    long long result3 = test_mixed_bitwidth(array, SIZE);
    int result4 = test_pointer_chasing(ptr_array, SIZE);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2 + result3 + result4;
    
    printf("Test results:\n");
    printf("  Test 1: %d\n", result1);
    printf("  Test 2: %d\n", result2);
    printf("  Test 3: %lld\n", result3);
    printf("  Test 4: %d\n", result4);
    printf("  Final: %d\n", final_result);
    
    /* Cleanup */
    free(array);
    free(matrix);
    free(ptr_array);
    
    return (final_result != 0) ? 0 : 1;
}
