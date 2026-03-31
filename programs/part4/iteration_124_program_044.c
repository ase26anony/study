/* Test to trigger selective scheduler debug dumping in GCC */
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

/* Complex function with mixed operations to generate diverse RTL */
__attribute__((noinline, optimize("O3")))
int test_selective_scheduler(int* array, int size, int seed) {
    volatile int barrier;  /* Prevent optimization */
    int sum_int = 0;
    float sum_float = 0.0f;
    int conditional_sum = 0;
    
    /* Create complex loop with data-dependent computations */
    for (int i = 0; i < size; i++) {
        /* Memory access - generates mem RTL */
        int val = array[i];
        
        /* Integer operations with builtin */
        int popcnt = __builtin_popcount(val);
        
        /* Mixed integer/float operations */
        float fval = (float)val * 1.5f;
        
        /* Conditional move/ternary - may generate if_then_else RTL */
        int cond_val = (val > seed) ? val : seed;
        
        /* Function calls - generate call RTL */
        int mul_result = helper_mul(val, i);
        float fmul_result = helper_fmul(fval, 2.0f);
        
        /* Complex expression with multiple dependencies */
        sum_int += (popcnt * mul_result) / (cond_val + 1);
        sum_float += fmul_result - (float)cond_val;
        
        /* Conditional branch creates control flow */
        if (i % 3 == 0) {
            conditional_sum += val * 2;
            /* Inline asm creates scheduling barrier */
            asm volatile ("" : : : "memory");
        } else if (i % 3 == 1) {
            conditional_sum -= val;
            /* Another scheduling barrier */
            asm volatile ("" : : : "memory");
        } else {
            conditional_sum ^= val;
        }
        
        /* Prevent loop unrolling from simplifying too much */
        barrier = i;
    }
    
    /* Final computation mixing types */
    int result = sum_int + (int)sum_float + conditional_sum;
    
    /* Use result to prevent dead code elimination */
    asm volatile ("" : "+r" (result) : : "memory");
    return result;
}

/* Another test with different patterns */
__attribute__((noinline, optimize("O3")))
int test_scheduler_patterns(int n) {
    int a = n;
    int b = n * 2;
    int c = 0;
    
    /* Loop with complex data flow */
    for (int i = 0; i < 100; i++) {
        /* Use 32-bit and 64-bit operations */
        int64_t wide_op = (int64_t)a * (int64_t)b;
        
        /* Bit manipulation operations */
        int rotated = (a << 3) | (a >> 29);
        
        /* Conditional with side effects */
        c += (a > b) ? rotated : (rotated ^ 0x55555555);
        
        /* Update with dependency chain */
        a = (a * 1103515245 + 12345) & 0x7fffffff;
        b = (b * 1664525 + 1013904223) & 0x7fffffff;
        
        /* Memory barrier every 7 iterations */
        if (i % 7 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use builtin for complex RTL pattern */
    int popcnt_total = __builtin_popcount(c);
    
    /* Final conditional return */
    return (popcnt_total > 16) ? c : (c ^ 0xAAAAAAAA);
}

/* Test with array operations */
__attribute__((noinline, optimize("O3")))
float test_array_operations(float* data, int count) {
    float sum = 0.0f;
    float prod = 1.0f;
    
    for (int i = 0; i < count; i++) {
        /* Load with potential aliasing */
        float val = data[i];
        
        /* Complex FP operations */
        float squared = val * val;
        float cubed = squared * val;
        
        /* Conditional FP operations */
        float processed = (val > 0.0f) ? squared : cubed;
        
        /* Accumulate with different patterns */
        sum += processed;
        prod *= (processed + 1.0f);
        
        /* Branch with FP condition */
        if (processed > 100.0f) {
            /* Scheduling barrier in hot path */
            asm volatile ("" : : : "memory");
            sum -= 50.0f;
        }
    }
    
    /* Avoid division by zero */
    if (prod != 0.0f) {
        return sum / prod;
    }
    return sum;
}

/* Main driver that calls all tests */
int main() {
    const int SIZE = 256;
    int* array = (int*)malloc(SIZE * sizeof(int));
    float* farray = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize with pseudo-random data */
    int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 1103515245 + 12345) & 0x7fff;
        farray[i] = (float)array[i] / 1000.0f;
    }
    
    /* Run tests with different patterns */
    int result1 = test_selective_scheduler(array, SIZE, seed);
    printf("Test 1 result: %d\n", result1);
    
    int result2 = test_scheduler_patterns(SIZE);
    printf("Test 2 result: %d\n", result2);
    
    float result3 = test_array_operations(farray, SIZE);
    printf("Test 3 result: %f\n", result3);
    
    /* Final checksum */
    int final_result = result1 + result2 + (int)result3;
    printf("Final result: %d\n", final_result);
    
    free(array);
    free(farray);
    
    return (final_result != 0) ? 0 : 1;
}
