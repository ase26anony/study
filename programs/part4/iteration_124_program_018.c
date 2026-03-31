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
    volatile int barrier; /* Prevent optimization */
    int sum = seed;
    float fsum = seed * 0.5f;
    
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
        float fval = val * 0.01f;
        fsum += helper_fmul(fval, fval);
        
        /* Mixed-type computation */
        sum += cond_val + popcnt;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Another memory write */
        arr[i] = (sum & 0xFF);
        
        /* Branch to create control flow */
        if (i % 3 == 0) {
            sum += i * 2;
        } else if (i % 3 == 1) {
            sum -= i;
        } else {
            sum ^= i;
        }
        
        barrier = sum; /* Use volatile to prevent dead code elimination */
    }
    
    return sum + (int)fsum;
}

/* Function with outer loop pipelining opportunities */
__attribute__((noinline, optimize("O3")))
int test_function_2(int* arr, int n) {
    int total = 0;
    
    /* Outer loop for pipelining */
    for (int outer = 0; outer < 10; outer++) {
        int inner_sum = 0;
        
        /* Inner loop with complex computations */
        for (int i = 0; i < n; i++) {
            /* Multiple independent computations */
            int a = arr[i] * 3;
            int b = arr[i] + i;
            int c = arr[i] ^ 0x55;
            
            /* SIMD-like operations */
            int d = (a << 2) | (b >> 1);
            int e = (c * 7) & 0xFF;
            
            /* Complex expression with multiple operators */
            inner_sum += ((a * b) + (c * d) - (e * 2)) / (i + 1);
            
            /* Memory barrier every 8 iterations */
            if ((i & 7) == 0) {
                asm volatile("" : : : "memory");
            }
        }
        
        total += inner_sum;
        
        /* Modify array for next iteration */
        for (int i = 0; i < n; i++) {
            arr[i] = (arr[i] + 1) & 0x7F;
        }
    }
    
    return total;
}

/* Function targeting specific architecture features */
__attribute__((target("arch=haswell"), noinline, optimize("O3")))
int test_function_3(int* arr, int n) {
    int64_t big_sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* 64-bit operations */
        int64_t val64 = (int64_t)arr[i] * 1000000LL;
        
        /* Complex 64-bit computation */
        big_sum += val64 * i;
        
        /* Conditional with both 32 and 64 bit */
        if (big_sum > 1000000000LL) {
            big_sum >>= 1;
            arr[i] = (int)(big_sum & 0xFFFFFFFF);
        }
        
        /* Another builtin */
        int leading_zeros = __builtin_clz(arr[i] | 1);
        
        /* Use result */
        big_sum += leading_zeros;
    }
    
    /* Mix 32-bit and 64-bit results */
    return (int)big_sum + (int)(big_sum >> 32);
}

/* Main test driver */
int main(void) {
    const int SIZE = 1024;
    int* array = (int*)malloc(SIZE * sizeof(int));
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Run test functions */
    int result1 = test_function_1(array, SIZE, 42);
    printf("Result 1: %d\n", result1);
    
    /* Re-initialize array */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 1103515245 + 54321) & 0x7FFF;
    }
    
    int result2 = test_function_2(array, SIZE);
    printf("Result 2: %d\n", result2);
    
    /* Re-initialize array */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 1103515245 + 98765) & 0x7FFF;
    }
    
    int result3 = test_function_3(array, SIZE);
    printf("Result 3: %d\n", result3);
    
    /* Final checksum */
    int final_sum = result1 + result2 + result3;
    printf("Final checksum: %d\n", final_sum);
    
    free(array);
    return 0;
}
