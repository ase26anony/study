/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_result = 0;

/* Non-inline helper functions to generate call RTL */
__attribute__((noinline, optimize("O2")))
int helper_multiply(int a, int b) {
    return a * b;
}

__attribute__((noinline, optimize("O2")))
float helper_float_multiply(float a, float b) {
    return a * b;
}

__attribute__((noinline, optimize("O2")))
int helper_conditional(int a, int b) {
    return (a > b) ? a : b;
}

/* Test 1: Mixed integer and floating-point operations in a loop */
__attribute__((noinline, optimize("O2")))
int test_mixed_operations(int* array, int size) {
    int int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < size; i++) {
        /* Integer operations with data dependency */
        int val = array[i];
        int product = val * i;
        int_sum += product;
        
        /* Floating-point operations */
        float fval = (float)val;
        float fproduct = fval * (float)i;
        float_sum += fproduct;
        
        /* Conditional move operation */
        int max_val = (val > int_sum) ? val : int_sum;
        
        /* Built-in function for complex RTL */
        int popcnt = __builtin_popcount(val);
        
        /* Memory barrier to create scheduling region */
        asm volatile ("" : : : "memory");
        
        /* Mix 32-bit and 64-bit operations */
        int64_t big_val = (int64_t)val * (int64_t)i;
        int_sum += (int)(big_val & 0xFFFFFFFF);
        
        /* Call helper functions */
        int_sum += helper_multiply(val, i);
        float_sum += helper_float_multiply(fval, (float)i);
        
        /* Complex conditional with multiple basic blocks */
        if (i % 3 == 0) {
            int_sum += val * 2;
            float_sum += fval * 2.0f;
        } else if (i % 3 == 1) {
            int_sum -= val;
            float_sum -= fval;
        } else {
            int_sum = helper_conditional(int_sum, val);
        }
        
        /* Prevent loop unrolling from simplifying too much */
        g_volatile_counter++;
    }
    
    /* Use both results to prevent dead code elimination */
    return int_sum + (int)float_sum;
}

/* Test 2: Nested loops with array computations */
__attribute__((noinline, optimize("O2")))
int test_nested_loops(int rows, int cols) {
    int total = 0;
    
    /* Create small 2D array on stack */
    int array[10][10];
    
    /* Initialize with pattern */
    for (int i = 0; i < rows && i < 10; i++) {
        for (int j = 0; j < cols && j < 10; j++) {
            array[i][j] = i * 100 + j;
        }
    }
    
    /* Complex nested loop computation */
    for (int i = 1; i < rows && i < 9; i++) {
        for (int j = 1; j < cols && j < 9; j++) {
            /* Stencil computation (3x3 convolution-like) */
            int sum = 0;
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    sum += array[i + di][j + dj];
                }
            }
            
            /* Mixed operations */
            float avg = (float)sum / 9.0f;
            int rounded = (int)(avg + 0.5f);
            
            /* Conditional update */
            if (rounded > array[i][j]) {
                total += rounded * 2;
            } else {
                total += array[i][j];
            }
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
        }
    }
    
    return total;
}

/* Test 3: Pointer chasing and indirect memory access */
__attribute__((noinline, optimize("O2")))
int test_pointer_chasing(int* data, int size) {
    if (size <= 0) return 0;
    
    int sum = 0;
    int* current = data;
    int steps = 0;
    
    /* Create linked-list like access pattern */
    while (steps < size && steps < 100) {
        int val = *current;
        
        /* Complex computation with the value */
        int transformed = val ^ (val >> 3);
        transformed = (transformed * 0x5bd1e995) & 0x7FFFFFFF;
        
        sum += transformed;
        
        /* Move to next position (simulate pointer chasing) */
        int offset = (val % 16) + 1;
        if ((current - data) + offset >= size) {
            current = data;
        } else {
            current += offset;
        }
        
        steps++;
        
        /* Use builtin for bit manipulation */
        sum += __builtin_ctz(val | 1);
        
        /* Create scheduling barrier periodically */
        if (steps % 7 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    return sum;
}

/* Test 4: Switch statement with multiple cases */
__attribute__((noinline, optimize("O2")))
int test_switch_case(int value, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int op = (value + i) % 8;
        
        switch (op) {
            case 0:
                result += i * 2;
                break;
            case 1:
                result -= i * 3;
                break;
            case 2:
                result ^= i;
                break;
            case 3:
                result |= (i << 3);
                break;
            case 4:
                result = (result > i) ? result : i;
                break;
            case 5:
                result *= (i + 1);
                break;
            case 6:
                result = helper_multiply(result, i);
                break;
            case 7:
                result = __builtin_popcount(result);
                break;
        }
        
        /* Prevent optimization */
        g_volatile_counter += op;
    }
    
    return result;
}

/* Main function that runs all tests */
int main(int argc, char** argv) {
    int total_result = 0;
    
    /* Initialize test data */
    int data_size = 100;
    int* test_data = (int*)malloc(data_size * sizeof(int));
    
    if (!test_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with pseudo-random but deterministic values */
    for (int i = 0; i < data_size; i++) {
        test_data[i] = (i * 37 + 123) & 0xFFF;
    }
    
    printf("Running selective scheduler tests...\n");
    
    /* Run test 1: Mixed operations */
    int result1 = test_mixed_operations(test_data, data_size);
    printf("Test 1 result: %d\n", result1);
    total_result += result1;
    
    /* Run test 2: Nested loops */
    int result2 = test_nested_loops(8, 8);
    printf("Test 2 result: %d\n", result2);
    total_result += result2;
    
    /* Run test 3: Pointer chasing */
    int result3 = test_pointer_chasing(test_data, data_size);
    printf("Test 3 result: %d\n", result3);
    total_result += result3;
    
    /* Run test 4: Switch case */
    int result4 = test_switch_case(42, 50);
    printf("Test 4 result: %d\n", result4);
    total_result += result4;
    
    /* Final result */
    printf("Total result: %d\n", total_result);
    
    /* Store in volatile to ensure computation is used */
    g_volatile_result = total_result;
    
    free(test_data);
    
    return (total_result != 0) ? 0 : 1;
}
