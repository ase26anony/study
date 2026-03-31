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

/* Complex function with mixed operations */
__attribute__((noinline, optimize("O3")))
int test_function_1(int* arr, int n, int seed) {
    volatile int barrier;  /* Prevent optimization */
    int sum = seed;
    float fsum = seed * 0.5f;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing computation */
        int val = arr[i];
        
        /* Integer operations */
        int prod = helper_mul(val, i);
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : (val * 2);
        
        /* Built-in function for complex RTL */
        int popcnt = __builtin_popcount(val);
        
        /* Floating point operations */
        float fval = (float)val;
        float fprod = helper_fmul(fval, fval * 0.1f);
        
        /* Mixed-type computation */
        sum += prod + cond_val + popcnt;
        fsum += fprod;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Branch with different operations in each path */
        if (i % 3 == 0) {
            sum += __builtin_ctz(val | 1);  /* Avoid undefined behavior */
            arr[i] = sum & 0xFF;
        } else if (i % 3 == 1) {
            /* Complex FP chain */
            fsum = fsum * 1.1f - 0.5f;
            arr[i] = (int)fsum;
        } else {
            /* Bit manipulation */
            arr[i] = (arr[i] ^ (arr[i] >> 1)) + i;
        }
        
        barrier = i;  /* Volatile write to prevent dead code elimination */
    }
    
    /* Final computation mixing int and float */
    return sum + (int)fsum;
}

/* Function with outer loop pipelining opportunities */
__attribute__((noinline, optimize("O3")))
int test_function_2(int* arr, int n) {
    int total = 0;
    
    /* Outer loop - target for -fsel-sched-pipelining-outer-loops */
    for (int outer = 0; outer < 10; outer++) {
        int inner_sum = 0;
        
        /* Inner loop with data-dependent computation */
        for (int i = 0; i < n; i++) {
            /* Complex addressing with multiple uses */
            int idx = (i + outer) % n;
            int val = arr[idx];
            
            /* Multiple independent operations for ILP */
            int a = val * 3;
            int b = val + 7;
            int c = val ^ 0x55;
            float d = (float)val * 3.14f;
            
            /* Cross-dependent operations */
            a = a + b;
            c = c * 2;
            d = d + 1.0f;
            
            /* Memory store with computation */
            arr[idx] = (a + c + (int)d) & 0xFF;
            
            inner_sum += arr[idx];
            
            /* Another scheduling barrier */
            asm volatile("" : : : "memory");
        }
        
        total += inner_sum;
        
        /* Conditional branch affecting control flow */
        if (outer % 2 == 0) {
            total = total * 2 - 1;
        }
    }
    
    return total;
}

/* Function with vectorizable patterns */
__attribute__((noinline, target("arch=haswell")))
int test_function_3(short* data, int len) {
    int checksum = 0;
    
    for (int i = 0; i < len; i++) {
        /* Operations on different data widths */
        short s = data[i];
        int extended = (int)s;
        
        /* Bit manipulation operations */
        int reversed = __builtin_bswap32(extended);
        int rotated = (extended << 4) | (extended >> 28);
        
        /* Conditional based on computation */
        int result = (reversed > rotated) ? reversed : rotated;
        
        /* Mixed 32-bit and 64-bit operations */
        long long big_val = (long long)result * 1000000LL;
        result = (int)(big_val >> 16);
        
        checksum ^= result;
        data[i] = (short)(checksum & 0xFFFF);
        
        /* Memory barrier every 8 iterations */
        if (i % 8 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return checksum;
}

/* Main driver that calls all test functions */
int main(int argc, char** argv) {
    int size = 256;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size < 16) size = 16;
        if (size > 1024) size = 1024;
    }
    
    /* Allocate and initialize test arrays */
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    short* arr3 = (short*)malloc(size * sizeof(short));
    
    for (int i = 0; i < size; i++) {
        arr1[i] = (i * 37 + 123) % 1000;
        arr2[i] = (i * 51 + 456) % 1000;
        arr3[i] = (short)((i * 73 + 789) % 1000);
    }
    
    /* Call test functions with different patterns */
    int result1 = test_function_1(arr1, size, 42);
    int result2 = test_function_2(arr2, size);
    int result3 = test_function_3(arr3, size);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2 + result3;
    
    /* Use results to ensure code isn't dead */
    printf("Test results: %d, %d, %d\n", result1, result2, result3);
    printf("Final checksum: %d\n", final_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return (final_result != 0) ? 0 : 1;
}
