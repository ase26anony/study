/* Test program to trigger selective scheduler debug dumping in GCC */
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
__attribute__((noinline, optimize("O2")))
int test_selective_sched_loop(int* arr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing calculation */
        int val = arr[i];
        
        /* Integer operations with data dependency */
        int prod = helper_mul(val, i);
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : prod;
        
        /* Floating point operations */
        float fval = (float)val;
        float fprod = helper_fmul(fval, fval);
        
        /* Built-in function for complex RTL */
        int popcnt = __builtin_popcount(val);
        
        /* Mixed-type computation */
        sum += cond_val + popcnt;
        fsum += fprod;
        
        /* Scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* Another memory write */
        arr[i] = (sum & 0xFF);
    }
    
    /* Final computation to prevent elimination */
    return sum + (int)fsum;
}

/* Test with nested loops - Test 2 */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int* arr, int n, int m) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_sum = 0;
        for (int j = 0; j < m; j++) {
            /* Complex addressing */
            int idx = (i * m + j) % n;
            int val = arr[idx];
            
            /* Bit manipulation operations */
            int rotated = (val << 3) | (val >> 29);
            int masked = rotated & 0x7FFFFFFF;
            
            /* Conditional based on computation */
            if (masked > 1000) {
                inner_sum += masked * j;
            } else {
                inner_sum += masked / (j + 1);
            }
            
            /* Another scheduling barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Store result with memory barrier */
        arr[i] = inner_sum;
        total += inner_sum;
    }
    
    return total;
}

/* Test with control flow - Test 3 */
__attribute__((noinline, optimize("O2"), target("arch=core2")))
int test_control_flow(int* arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        int val = arr[i];
        
        /* Multiple conditional paths */
        if (i % 3 == 0) {
            /* Path 1: Integer operations */
            result += val * i;
            result ^= 0xABCD;
        } else if (i % 3 == 1) {
            /* Path 2: Shift operations */
            result += (val << (i % 8)) | (val >> (32 - (i % 8)));
            asm volatile ("" : : : "memory");
        } else {
            /* Path 3: Division and remainder */
            result += val / ((i % 7) + 1);
            result += val % ((i % 5) + 1);
        }
        
        /* 64-bit operations on 32-bit system */
        long long big_val = (long long)val * val;
        result += (int)(big_val & 0xFFFFFFFF);
        
        /* Another memory access pattern */
        arr[(i + 1) % n] = result & 0xFF;
    }
    
    return result;
}

/* Test with floating point intensive operations - Test 4 */
__attribute__((noinline, optimize("O3")))
float test_float_ops(float* arr, int n) {
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        float x = arr[i];
        float y = arr[(i + 1) % n];
        
        /* Mixed precision operations */
        double dbl = (double)x * (double)y;
        float flt = (float)dbl;
        
        /* Conditional floating point operation */
        float cond = (x > y) ? x * x : y * y;
        
        /* Complex floating point expression */
        sum += flt + cond + (float)__builtin_sqrtf(fabsf(x));
        
        /* Memory barrier between FP operations */
        asm volatile ("" : : : "memory");
        
        /* Store intermediate result */
        arr[i] = sum / (i + 2);
    }
    
    return sum;
}

/* Main driver that uses all test functions */
int main(int argc, char** argv) {
    const int SIZE = 256;
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = (i * 13 + 7) % 1000;
        float_array[i] = (float)((i * 17 + 11) % 100) / 10.0f;
    }
    
    /* Run all tests to exercise selective scheduler */
    int result1 = test_selective_sched_loop(int_array, SIZE);
    printf("Test 1 result: %d\n", result1);
    
    int result2 = test_nested_loops(int_array, SIZE / 4, 4);
    printf("Test 2 result: %d\n", result2);
    
    int result3 = test_control_flow(int_array, SIZE);
    printf("Test 3 result: %d\n", result3);
    
    float result4 = test_float_ops(float_array, SIZE);
    printf("Test 4 result: %f\n", result4);
    
    /* Final checksum to ensure all computations matter */
    int final_sum = result1 + result2 + result3 + (int)result4;
    printf("Final checksum: %d\n", final_sum);
    
    free(int_array);
    free(float_array);
    
    return final_sum != 0 ? 0 : 1;
}
