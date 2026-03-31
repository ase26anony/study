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

/* Complex function with mixed operations - Test 1 */
__attribute__((noinline, optimize("O3")))
int test_selective_sched_1(int* arr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing computation */
        int val = arr[i];
        
        /* Integer operations with data dependency */
        int prod = helper_mul(val, i);
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : prod;
        
        /* Built-in function for complex RTL */
        int popcnt = __builtin_popcount(val);
        
        /* Floating point operations */
        float fval = (float)val;
        float fprod = helper_fmul(fval, fval * 0.5f);
        
        /* Mix integer and float results */
        sum += cond_val + popcnt + (int)fprod;
        fsum += fprod;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Another memory write */
        arr[i] = (val + i) & 0xFF;
        
        /* Branch to create control flow */
        if (i % 3 == 0) {
            sum += barrier;
        } else if (i % 3 == 1) {
            sum -= barrier;
        } else {
            sum ^= barrier;
        }
    }
    
    /* Use both results to prevent dead code elimination */
    return sum + (int)fsum;
}

/* Function with nested loops - Test 2 */
__attribute__((noinline, optimize("O3")))
int test_selective_sched_2(int* arr, int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_sum = 0;
        
        /* Inner loop with unroll hint */
        #pragma GCC unroll 4
        for (int j = 0; j < 8; j++) {
            /* Complex addressing */
            int idx = (i * 8 + j) % n;
            int val = arr[idx];
            
            /* Mixed-width operations */
            int64_t wide_val = (int64_t)val * (int64_t)j;
            int32_t narrow_val = (int32_t)(wide_val >> 16);
            
            /* Bit manipulation operations */
            int rotated = (narrow_val << 3) | (narrow_val >> 29);
            
            /* Conditional based on computation */
            inner_sum += (rotated > 0) ? rotated : -rotated;
            
            /* Memory store with barrier */
            arr[idx] = rotated & 0xFF;
            asm volatile("" : : : "memory");
        }
        
        total += inner_sum;
        
        /* Function call in loop */
        total = helper_mul(total, 1);
    }
    
    return total;
}

/* Function with pointer chasing - Test 3 */
__attribute__((noinline, optimize("O3")))
int test_selective_sched_3(int* arr, int n) {
    int result = 0;
    int* ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Pointer arithmetic and dereference */
        int val = *ptr;
        
        /* Complex expression with multiple operations */
        int comp = ((val * 1103515245) + 12345) & 0x7fffffff;
        
        /* Use builtin for conditional */
        int leading_zeros = __builtin_clz(val | 1);
        
        /* Floating point conversion and operation */
        double dval = (double)comp / 2147483647.0;
        double dresult = dval * dval * 3.14159;
        
        /* Convert back and accumulate */
        result += (int)(dresult * 1000.0) + leading_zeros;
        
        /* Update pointer with wrap-around */
        ptr = arr + ((ptr - arr + 1) % n);
        
        /* Scheduling barrier every 4 iterations */
        if (i % 4 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return result;
}

/* Main driver */
int main(int argc, char** argv) {
    const int SIZE = 1024;
    int* array = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Run all tests */
    int result1 = test_selective_sched_1(array, SIZE);
    int result2 = test_selective_sched_2(array, SIZE);
    int result3 = test_selective_sched_3(array, SIZE);
    
    /* Combine results to ensure all code is used */
    int final_result = result1 + result2 + result3;
    
    printf("Selective scheduler test result: %d\n", final_result);
    
    /* Verify with simple computation */
    int verify = 0;
    for (int i = 0; i < SIZE; i++) {
        verify += array[i] & 0xFF;
    }
    printf("Array checksum: %d\n", verify);
    
    free(array);
    return 0;
}
