/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;

/* Non-inline helper functions to generate call RTL */
static int __attribute__((noinline, optimize("O2"))) 
helper_compute(int a, int b) {
    return (a * b) ^ (a + b);
}

static float __attribute__((noinline, optimize("O2")))
helper_float_compute(float x, float y) {
    return x * y + x / (y + 1.0f);
}

/* Function with complex mixed operations */
__attribute__((noinline, optimize("O2")))
int test_mixed_operations(int* arr, int n) {
    int sum_int = 0;
    float sum_float = 0.0f;
    int temp_result;
    float float_result;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing computation */
        int val = arr[i];
        
        /* Integer operations with data dependency */
        sum_int += val * i;
        sum_int ^= (val << 3);
        
        /* Conditional operation (may generate if_then_else RTL) */
        int cond_val = (val > 100) ? val : (val * 2);
        sum_int += cond_val;
        
        /* Built-in function for complex RTL pattern */
        sum_int += __builtin_popcount(val);
        
        /* Floating point operations */
        float fval = (float)val * g_volatile_float;
        sum_float += fval * i;
        
        /* Inline assembly as scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* Mix integer and float in same expression */
        if (i % 2 == 0) {
            sum_float += (float)(sum_int % 256);
        } else {
            sum_float -= (float)(sum_int % 128);
        }
        
        /* Call to non-inline helper */
        temp_result = helper_compute(val, i);
        sum_int += temp_result;
        
        /* Another helper call for float */
        float_result = helper_float_compute(fval, sum_float);
        sum_float = float_result * 0.5f;
        
        /* Complex conditional with multiple branches */
        if (val > 500) {
            sum_int *= 2;
            sum_float *= 1.5f;
        } else if (val > 200) {
            sum_int += val >> 2;
            sum_float += 0.25f;
        }
        
        /* Update volatile to prevent dead code elimination */
        g_volatile_counter++;
    }
    
    /* Final computation mixing types */
    return sum_int + (int)sum_float;
}

/* Function with nested loops for outer-loop pipelining */
__attribute__((noinline, optimize("O2")))
int test_nested_loops(int* matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            /* 2D array access with complex addressing */
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Data-dependent computation */
            row_sum += val * (i + j);
            
            /* Conditional move-like operation */
            int adjusted = (j % 3 == 0) ? val * 2 : val / 2;
            row_sum ^= adjusted;
            
            /* Use of builtin for bit manipulation */
            row_sum += __builtin_ctz(val | 1);
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Cross-iteration dependency */
        total += row_sum * i;
        
        /* Volatile update */
        g_volatile_counter += row_sum % 7;
    }
    
    return total;
}

/* Function with pointer arithmetic and memory operations */
__attribute__((noinline, optimize("O2")))
void test_pointer_ops(int* dest, int* src, int n) {
    int* end = src + n;
    
    while (src < end) {
        /* Pointer dereference and computation */
        int val = *src++;
        
        /* Complex addressing mode */
        *dest++ = val * 3 + (int)(g_volatile_float * 2.0f);
        
        /* Bitfield operations */
        int masked = val & 0xFF;
        int shifted = (val >> 8) & 0xFF;
        *(dest - 1) += masked * shifted;
        
        /* Conditional store */
        if (val > 1000) {
            *(dest - 1) = -val;
        }
        
        /* Another scheduling barrier */
        asm volatile ("" : : : "memory");
    }
}

/* Main test driver */
int main() {
    const int SIZE = 512;
    const int MATRIX_SIZE = 32;
    
    /* Allocate and initialize test data */
    int* array = (int*)malloc(SIZE * sizeof(int));
    int* matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int* dest = (int*)malloc(SIZE * sizeof(int));
    
    if (!array || !matrix || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = (i * 1664525 + 1013904223) & 0x3FFF;
    }
    
    /* Run tests */
    int result1 = test_mixed_operations(array, SIZE);
    printf("Test 1 result: %d\n", result1);
    
    int result2 = test_nested_loops(matrix, MATRIX_SIZE, MATRIX_SIZE);
    printf("Test 2 result: %d\n", result2);
    
    test_pointer_ops(dest, array, SIZE);
    printf("Test 3 completed, dest[0] = %d\n", dest[0]);
    
    /* Verify volatile was updated */
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(array);
    free(matrix);
    free(dest);
    
    return 0;
}
