/* test_sel_sched_dump.c
 * 
 * This test is designed to trigger GCC's selective scheduler debug dumping
 * functionality, specifically the lines in sel-sched-dump.cc that dump
 * RTL representations of scheduled instructions.
 *
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops \
 *                -fdump-rtl-sched -fdump-rtl-sched2 -fdump-rtl-all \
 *                -fdump-noaddr -da test_sel_sched_dump.c -o test_sel_sched_dump
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;

/* Non-inline helper functions to generate call RTL */
int __attribute__((noinline, optimize("O2"))) helper_multiply(int a, int b) {
    return a * b + (a ^ b);
}

float __attribute__((noinline, optimize("O2"))) helper_float_op(float x, float y) {
    return x * y - x / (y + 1.0f);
}

/* Test function 1: Mixed integer and floating-point operations in a loop
 * Creates diverse RTL patterns including memory accesses, arithmetic, and conditionals */
int __attribute__((noinline, optimize("O2"))) test_mixed_operations(int* array, int size) {
    int int_sum = 0;
    float float_sum = 0.0f;
    
    /* Create data-dependent computation with ILP opportunities */
    for (int i = 0; i < size; i++) {
        /* Integer operations with data dependency */
        int val = array[i];
        int_sum += val * i;
        int_sum ^= helper_multiply(val, i);
        
        /* Floating-point operations - creates different RTL patterns */
        float fval = (float)val * 0.5f;
        float_sum += fval * i;
        float_sum -= helper_float_op(fval, (float)i);
        
        /* Conditional move/ternary operation - may generate if_then_else RTL */
        int conditional = (val > 100) ? val : (val * 2);
        int_sum += conditional;
        
        /* Built-in function for complex RTL pattern */
        int_sum += __builtin_popcount(val);
        
        /* Memory barrier to create scheduling boundaries */
        asm volatile ("" : : : "memory");
        
        /* Another conditional with different pattern */
        if (i % 3 == 0) {
            float_sum *= 1.01f;
            int_sum -= val;
        } else if (i % 3 == 1) {
            float_sum /= 1.02f;
            int_sum |= val;
        } else {
            float_sum += 0.5f;
            int_sum &= ~val;
        }
        
        /* Use volatile to prevent dead code elimination */
        g_volatile_counter += i;
    }
    
    /* Mix results */
    return int_sum + (int)float_sum;
}

/* Test function 2: Nested loops with pointer arithmetic
 * Generates complex addressing modes and memory RTL */
int __attribute__((noinline, optimize("O2"))) test_nested_loops(int* matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int* row_start = matrix + i * cols;
        
        for (int j = 0; j < cols; j++) {
            /* Complex addressing calculation */
            int idx = (i * 17 + j * 13) % (rows * cols);
            int val = matrix[idx];
            
            /* Mixed operations */
            total += val * (i + j);
            total ^= (val << (j % 16));
            
            /* Conditional store */
            if (val > total) {
                row_start[j] = total;
            } else {
                row_start[j] = val + 1;
            }
            
            /* Floating-point in nested loop */
            float ftemp = (float)val * g_volatile_float;
            g_volatile_float += 0.1f;
            
            /* Another scheduling barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Outer loop computation */
        total += __builtin_ctz(i + 1);  /* Count trailing zeros */
    }
    
    return total;
}

/* Test function 3: Switch statement with computed goto-like behavior
 * Creates complex control flow for scheduler to analyze */
int __attribute__((noinline, optimize("O2"))) test_switch_pattern(int mode, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        switch ((mode + i) % 5) {
            case 0:
                result += i * 2;
                /* Generate mem RTL with store */
                g_volatile_counter = result;
                break;
            case 1:
                result -= i * 3;
                /* Generate float RTL */
                result += (int)((float)i * 1.5f);
                break;
            case 2:
                result ^= i;
                /* Call RTL generation */
                result += helper_multiply(result, i);
                break;
            case 3:
                result |= (i << 4);
                /* Complex builtin */
                result += __builtin_parity(result);
                break;
            case 4:
                result &= ~i;
                /* Another memory barrier */
                asm volatile ("" : : : "memory");
                result += g_volatile_counter;
                break;
        }
        
        /* Additional computation after switch */
        result += (result > 0) ? result : -result;  /* Absolute value via ternary */
    }
    
    return result;
}

/* Test function 4: SIMD-like operations manually unrolled
 * Encourages instruction-level parallelism */
int __attribute__((noinline, optimize("O2"), target("arch=haswell"))) 
test_simd_pattern(int* data, int size) {
    int sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Manual unrolling for ILP */
    for (int i = 0; i < size; i += 4) {
        /* Four independent computation chains */
        int val0 = (i < size) ? data[i] : 0;
        int val1 = (i + 1 < size) ? data[i + 1] : 0;
        int val2 = (i + 2 < size) ? data[i + 2] : 0;
        int val3 = (i + 3 < size) ? data[i + 3] : 0;
        
        /* Independent operations that can be scheduled in parallel */
        sum0 += val0 * val0 + i;
        sum1 += val1 * val1 + (i + 1);
        sum2 += val2 * val2 + (i + 2);
        sum3 += val3 * val3 + (i + 3);
        
        /* Cross-dependent operation to create some serialization */
        int cross = sum0 ^ sum1;
        sum2 += cross;
        sum3 ^= cross;
        
        /* Mixed float/int */
        float fcross = (float)cross * 0.25f;
        sum0 += (int)(fcross * 100.0f);
        
        /* Memory barrier every 8 iterations */
        if ((i / 4) % 8 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Combine results */
    return sum0 + sum1 + sum2 + sum3;
}

/* Test function 5: Recursive pattern with tail operations
 * Creates complex live range analysis requirements */
int __attribute__((noinline, optimize("O2"))) 
test_recursive_pattern(int* arr, int start, int end, int depth) {
    if (start >= end || depth <= 0) {
        return arr[start % (end + 1)];
    }
    
    int mid = (start + end) / 2;
    
    /* Process both halves - creates branching pattern */
    int left = test_recursive_pattern(arr, start, mid, depth - 1);
    int right = test_recursive_pattern(arr, mid + 1, end, depth - 1);
    
    /* Complex combination with mixed operations */
    int result = (left * right) + (left ^ right);
    
    /* Floating-point conversion and back */
    float fresult = (float)result * 0.618f;  /* Golden ratio */
    result = (int)fresult + (int)(fresult * 1000.0f) % 256;
    
    /* Conditional based on result */
    result = (result > 1000000) ? result >> 4 : result << 2;
    
    /* Use volatile */
    g_volatile_counter += result;
    
    return result;
}

/* Main function that runs all tests */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int DATA_SIZE = 256;
    int* test_array = (int*)malloc(DATA_SIZE * sizeof(int));
    int* matrix = (int*)malloc(DATA_SIZE * DATA_SIZE * sizeof(int));
    
    if (!test_array || !matrix) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < DATA_SIZE; i++) {
        test_array[i] = (i * 13 + 7) % 1000;
    }
    
    for (int i = 0; i < DATA_SIZE * DATA_SIZE; i++) {
        matrix[i] = (i * 17 + 11) % 500;
    }
    
    int total_result = 0;
    
    /* Run test 1: Mixed operations */
    printf("Running test_mixed_operations...\n");
    total_result += test_mixed_operations(test_array, DATA_SIZE);
    
    /* Run test 2: Nested loops */
    printf("Running test_nested_loops...\n");
    total_result += test_nested_loops(matrix, 16, 16);
    
    /* Run test 3: Switch pattern */
    printf("Running test_switch_pattern...\n");
    total_result += test_switch_pattern(2, 100);
    
    /* Run test 4: SIMD pattern */
    printf("Running test_simd_pattern...\n");
    total_result += test_simd_pattern(test_array, DATA_SIZE);
    
    /* Run test 5: Recursive pattern */
    printf("Running test_recursive_pattern...\n");
    total_result += test_recursive_pattern(test_array, 0, DATA_SIZE - 1, 5);
    
    /* Additional volatile operations to ensure all code is live */
    total_result += g_volatile_counter;
    total_result += (int)g_volatile_float;
    
    printf("Final result: %d\n", total_result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    printf("Volatile float: %f\n", g_volatile_float);
    
    /* Verify result is non-zero (sanity check) */
    if (total_result == 0) {
        printf("WARNING: Result is zero - possible optimization issue\n");
    }
    
    free(test_array);
    free(matrix);
    
    return 0;
}
