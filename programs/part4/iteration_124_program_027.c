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

/* Complex function with mixed operations - target for selective scheduling */
__attribute__((noinline, optimize("O3")))
int test_selective_scheduling(int* arr, int n, float* farr) {
    volatile int barrier; /* Prevent optimization */
    int sum_int = 0;
    float sum_float = 0.0f;
    int i;
    
    /* Complex loop with data-dependent computations */
    for (i = 0; i < n; i++) {
        /* Memory access - generates mem RTL */
        int val = arr[i];
        
        /* Integer operations with builtin - generates specific RTL patterns */
        int popcnt = __builtin_popcount(val);
        
        /* Conditional move/ternary - may generate cond_exec RTL */
        int cond_val = (val > 100) ? val : 100;
        
        /* Mixed integer/float operations */
        float fval = farr[i];
        float scaled = fval * (float)i;
        
        /* Function calls - generate call RTL */
        int mul_result = helper_mul(val, i);
        float fmul_result = helper_fmul(fval, scaled);
        
        /* Branch with different basic blocks */
        if (i % 3 == 0) {
            /* Block 1: Integer-heavy path */
            sum_int += val * popcnt + cond_val;
            sum_float += scaled;
            
            /* Inline assembly as scheduling barrier */
            asm volatile("" : : : "memory");
        } else if (i % 3 == 1) {
            /* Block 2: Float-heavy path */
            sum_int += mul_result;
            sum_float += fmul_result * 2.0f;
            
            /* Complex expression with multiple operations */
            int temp = (val << 3) | (val >> 5);
            sum_int ^= temp;
        } else {
            /* Block 3: Mixed operations path */
            sum_int += helper_mul(val, cond_val);
            sum_float += helper_fmul(scaled, fmul_result);
            
            /* 32-bit and 64-bit operations */
            int64_t big_val = (int64_t)val * (int64_t)i;
            sum_int += (int)(big_val & 0xFFFFFFFF);
        }
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Volatile read to prevent dead code elimination */
        barrier = i;
    }
    
    /* Final computation mixing results */
    int final_result = sum_int + (int)sum_float;
    
    /* Prevent tail optimization */
    asm volatile("" : : "r"(final_result) : "memory");
    
    return final_result;
}

/* Second test function with different patterns */
__attribute__((noinline, optimize("O3"), target("arch=core2")))
float test_vector_ops(float* a, float* b, int n) {
    float sum = 0.0f;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Potential vectorization candidates */
        float diff = a[i] - b[i];
        float prod = a[i] * b[i];
        
        /* Conditional with complex RTL */
        float result = (diff > 0.0f) ? prod : -prod;
        
        /* Builtin math function */
        float abs_val = __builtin_fabsf(result);
        
        sum += abs_val * (float)i;
        
        /* Memory store operation */
        a[i] = result;  /* Generates store RTL */
    }
    
    return sum;
}

/* Third test with nested loops */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int size) {
    int total = 0;
    int i, j;
    
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            /* Complex index calculation */
            int idx = (i * 17 + j * 13) % size;
            
            /* Bit manipulation operations */
            int rotated = (idx << 4) | (idx >> 28);
            int masked = rotated & 0x0F0F0F0F;
            
            /* Conditional with multiple branches */
            if (masked > 0x01010101) {
                total += masked * i;
            } else if (masked < 0) {
                total -= j;
            } else {
                total ^= (i * j);
            }
            
            /* Mix with float */
            float ftemp = (float)masked * 0.5f;
            total += (int)ftemp;
        }
        
        /* Outer loop scheduling barrier */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

/* Main driver */
int main() {
    const int SIZE = 256;
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    float* float_array2 = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = (i * 37 + 123) % 1000;
        float_array[i] = (float)(i * 51) * 0.1f;
        float_array2[i] = (float)(i * 73) * 0.05f;
    }
    
    /* Call test functions */
    int result1 = test_selective_scheduling(int_array, SIZE, float_array);
    float result2 = test_vector_ops(float_array, float_array2, SIZE);
    int result3 = test_nested_loops(64);
    
    /* Combine results to produce final output */
    int final_result = result1 + (int)result2 + result3;
    
    printf("Test Results:\n");
    printf("  Function 1: %d\n", result1);
    printf("  Function 2: %f\n", result2);
    printf("  Function 3: %d\n", result3);
    printf("  Final: %d\n", final_result);
    
    /* Verify with expected value (computed from deterministic inputs) */
    if (final_result == 105739776) {  /* Pre-computed expected value */
        printf("SUCCESS: All tests passed\n");
    } else {
        printf("WARNING: Result mismatch (expected 105739776, got %d)\n", final_result);
    }
    
    free(int_array);
    free(float_array);
    free(float_array2);
    
    return 0;
}
