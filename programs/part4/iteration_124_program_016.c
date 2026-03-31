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
__attribute__((noinline, optimize("O2")))
int test_complex_loop(int* arr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Mixed integer and floating point operations */
        int val = arr[i];
        
        /* Data-dependent computation with multiple ILP opportunities */
        int prod = val * i;
        int shifted = prod >> 3;
        
        /* Conditional operation (may generate cond_exec RTL) */
        int cond_val = (val > 100) ? val : shifted;
        
        /* Call to non-inline function (generates call RTL) */
        int helper_result = helper_mul(cond_val, i);
        
        /* Floating point operations */
        float fval = (float)val;
        float fprod = fval * (float)i;
        
        /* Another non-inline call */
        float fhelper_result = helper_fmul(fprod, 2.0f);
        
        /* Memory barrier to create scheduling regions */
        asm volatile("" : : : "memory");
        
        /* Accumulate results */
        sum += helper_result;
        fsum += fhelper_result;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 3 == 0) {
            /* Additional computation in this path */
            sum += __builtin_popcount(val);
        } else if (i % 3 == 1) {
            /* Different computation */
            sum -= (val & 0xFF);
        } else {
            /* Yet another path */
            sum ^= val;
        }
        
        /* Builtin function for complex RTL pattern */
        sum += __builtin_clz(val | 1);
    }
    
    /* Use both results to prevent elimination */
    return sum + (int)fsum;
}

/* Function with nested loops for outer-loop pipelining */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int* arr, int n, int m) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_sum = 0;
        
        /* Inner loop with data-dependent access pattern */
        for (int j = 0; j < m; j++) {
            int idx = (i * j) % n;
            int val = arr[idx];
            
            /* Complex expression with multiple operations */
            int result = ((val * j) + (i << 2)) / (val + 1);
            
            /* Conditional move-like operation */
            int final_val = (result > 0) ? result : -result;
            
            inner_sum += final_val;
            
            /* Memory access pattern */
            arr[idx] = val + 1;  /* Generates mem RTL */
        }
        
        total += inner_sum;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

/* Function using 64-bit operations */
__attribute__((noinline, target("arch=core2")))
int64_t test_64bit_ops(int64_t* arr, int n) {
    int64_t sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Mix of 32-bit and 64-bit operations */
        int64_t val = arr[i];
        int32_t low = val & 0xFFFFFFFF;
        int32_t high = (val >> 32) & 0xFFFFFFFF;
        
        /* Cross-type operations */
        int64_t prod = (int64_t)low * (int64_t)high;
        int64_t shifted = prod << (i % 8);
        
        /* Complex conditional */
        int64_t result = (prod > shifted) ? prod : shifted;
        
        sum += result;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

/* Main test driver */
int main() {
    const int SIZE = 256;
    int* data = (int*)malloc(SIZE * sizeof(int));
    int64_t* data64 = (int64_t*)malloc(SIZE * sizeof(int64_t));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 13 + 7) % 1000;
        data64[i] = ((int64_t)i * 17 + 11) % 10000;
    }
    
    /* Call all test functions */
    int result1 = test_complex_loop(data, SIZE);
    printf("Result 1: %d\n", result1);
    
    int result2 = test_nested_loops(data, 64, 32);
    printf("Result 2: %d\n", result2);
    
    int64_t result3 = test_64bit_ops(data64, SIZE);
    printf("Result 3: %lld\n", (long long)result3);
    
    /* Final computation using all results */
    int final_result = result1 + result2 + (int)result3;
    printf("Final result: %d\n", final_result);
    
    free(data);
    free(data64);
    
    return 0;
}
