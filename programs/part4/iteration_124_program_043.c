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

/* Function with complex loop to maximize ILP opportunities */
__attribute__((noinline, optimize("O3")))
int test_complex_loop(int* arr, int n) {
    int sum_int = 0;
    float sum_float = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Mixed integer operations */
        int a = arr[i];
        int b = i * 2;
        int c = (a > b) ? a : b;  /* Conditional move pattern */
        sum_int += helper_mul(a, c);
        
        /* Memory barrier to create scheduling region */
        asm volatile("" : : : "memory");
        
        /* Floating point operations */
        float x = (float)a * 0.5f;
        float y = (float)b * 1.5f;
        sum_float += helper_fmul(x, y);
        
        /* Built-in function for complex RTL */
        sum_int += __builtin_popcount(a);
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 3 == 0) {
            sum_int += arr[i] * 3;
            barrier = sum_int; /* Volatile write */
        } else if (i % 3 == 1) {
            sum_int -= arr[i] * 2;
            /* Another scheduling barrier */
            asm volatile("" : : : "memory");
        } else {
            sum_int ^= arr[i];
        }
        
        /* Mixed 32/64 bit operations */
        int64_t wide = (int64_t)a * (int64_t)b;
        sum_int += (int)(wide >> 16);
    }
    
    return sum_int + (int)sum_float;
}

/* Outer loop with pipelining opportunities */
__attribute__((noinline, optimize("O3")))
int test_outer_loop_pipelining(int* arr, int n, int m) {
    int total = 0;
    
    for (int j = 0; j < m; j++) {
        int local_sum = 0;
        volatile int dummy = j;
        
        /* Inner loop with data-dependent computation */
        for (int i = 0; i < n; i++) {
            int idx = (i + j) % n;
            int val = arr[idx];
            
            /* Complex expression with multiple dependencies */
            local_sum += val * (i + 1) - (val >> 2) + (val & 0xFF);
            
            /* Memory access pattern */
            arr[idx] = val + 1;
            
            /* Periodic scheduling barrier */
            if (i % 8 == 0) {
                asm volatile("" : : : "memory");
            }
        }
        
        total += local_sum * (dummy + 1);
    }
    
    return total;
}

/* Function targeting specific microarchitecture */
__attribute__((target("arch=haswell"), noinline, optimize("O3")))
int test_haswell_specific(int* arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* FMA-like operations */
        int a = arr[i];
        int b = i * 3;
        int c = a * b;
        
        /* Conditional with side effects */
        sum += (a > 100) ? c : -c;
        
        /* Complex bit manipulation */
        sum ^= __builtin_bswap32(a);
        
        /* Another scheduling region */
        asm volatile("" : : : "memory");
        
        /* SIMD-friendly pattern */
        sum += arr[(i + 1) % n] + arr[(i + 2) % n];
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main() {
    const int SIZE = 1024;
    const int OUTER_ITER = 10;
    int* data = (int*)malloc(SIZE * sizeof(int));
    int total_result = 0;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Call test functions with different patterns */
    total_result += test_complex_loop(data, SIZE);
    
    /* Modify data slightly */
    for (int i = 0; i < SIZE; i++) {
        data[i] ^= 0x55555555;
    }
    
    total_result += test_outer_loop_pipelining(data, SIZE, OUTER_ITER);
    
    /* More modifications */
    for (int i = 0; i < SIZE; i++) {
        data[i] = data[i] * 3 + 1;
    }
    
    total_result += test_haswell_specific(data, SIZE);
    
    printf("Total result: %d\n", total_result);
    
    free(data);
    return 0;
}
