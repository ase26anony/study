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

/* Complex function with mixed operations to generate diverse RTL */
__attribute__((noinline, optimize("O3")))
int test_function_1(int* arr, int n, int seed) {
    volatile int barrier;  /* Prevent optimization */
    int sum = seed;
    float fsum = seed * 0.5f;
    
    /* Mix integer and floating point operations */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with memory access */
        int val = arr[i];
        
        /* Complex conditional with ternary operator */
        int cond_val = (val > 100) ? val * 2 : val / 2;
        
        /* Mixed integer operations */
        sum += cond_val * i;
        sum ^= (val << 3) | (val >> 5);
        
        /* Floating point operations */
        fsum += (float)val * 0.123f;
        fsum = fsum * 1.01f - 0.5f;
        
        /* Built-in function call */
        sum += __builtin_popcount(val);
        
        /* Inline assembly as scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* Conditional branch creating basic blocks */
        if (i % 7 == 0) {
            /* Call to non-inline function */
            sum = helper_mul(sum, 3);
            fsum = helper_fmul(fsum, 1.5f);
        } else if (i % 3 == 0) {
            /* Different computation path */
            sum -= val * val;
            fsum /= 1.1f;
        }
        
        /* Memory write */
        arr[i] = sum & 0xFF;
        
        /* Another scheduling barrier */
        barrier = sum;
    }
    
    /* Final computation mixing types */
    return sum + (int)fsum;
}

/* Function with outer loop pipelining opportunities */
__attribute__((noinline, optimize("O3")))
int test_function_2(int* arr, int n) {
    int total = 0;
    
    /* Outer loop with complex inner computation */
    for (int outer = 0; outer < 3; outer++) {
        int inner_sum = 0;
        
        /* Inner loop with data dependencies */
        for (int i = 0; i < n; i++) {
            /* Complex addressing pattern */
            int idx = (i * 13 + outer) % n;
            int val = arr[idx];
            
            /* SIMD-like operations */
            int a = val * 3;
            int b = val + 7;
            int c = val ^ 0x55;
            
            /* Conditional move pattern */
            int max_val = (a > b) ? a : b;
            max_val = (max_val > c) ? max_val : c;
            
            inner_sum += max_val * i;
            
            /* Mixed width operations */
            int64_t wide_val = (int64_t)val * 0x100000001LL;
            inner_sum += (int)(wide_val >> 32);
            
            /* Another scheduling barrier */
            asm volatile ("" : : : "memory");
        }
        
        total += inner_sum * (outer + 1);
    }
    
    return total;
}

/* Function with pointer chasing and complex control flow */
__attribute__((noinline, optimize("O3")))
int test_function_3(int* arr, int n) {
    int result = 0;
    int* ptr = arr;
    int count = n;
    
    while (count-- > 0) {
        /* Pointer arithmetic and dereference */
        int val = *ptr++;
        
        /* Complex bit manipulation */
        int rotated = (val << 4) | (val >> 28);
        int masked = rotated & 0x0F0F0F0F;
        
        /* Table lookup simulation */
        int table[8] = {1, 3, 5, 7, 11, 13, 17, 19};
        int index = masked % 8;
        result += table[index] * val;
        
        /* Conditional with side effect */
        if (val > result) {
            result = result ^ val;
            asm volatile ("" : : : "memory");
        }
        
        /* Nested ternary operations */
        int adjust = (val < 0) ? -val : 
                    (val > 1000) ? val / 10 :
                    (val > 100) ? val / 5 : val;
        
        result += adjust;
        
        /* Prevent loop unrolling from simplifying too much */
        if (count % 16 == 0) {
            result = helper_mul(result, 2);
        }
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    const int SIZE = 256;
    int* array = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 13 + 7) ^ 0x12345678;
    }
    
    /* Call test functions with different patterns */
    int result1 = test_function_1(array, SIZE, argc);
    int result2 = test_function_2(array, SIZE);
    int result3 = test_function_3(array, SIZE);
    
    /* Combine results to ensure all code is live */
    int final_result = result1 + result2 + result3;
    
    /* Use result to prevent dead code elimination */
    printf("Test result: %d\n", final_result);
    
    /* Additional volatile write to ensure side effects */
    volatile int check = final_result;
    
    free(array);
    return final_result != 0 ? 0 : 1;
}
