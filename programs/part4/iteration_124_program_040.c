/* sel-sched-test.c - Test program to trigger selective scheduler debug dumping */
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

/* Complex function with mixed operations and control flow */
__attribute__((noinline, optimize("O2")))
int test_function_1(int* array, int n, int seed) {
    volatile int barrier;  /* Prevent optimization */
    int sum = 0;
    int i;
    
    /* Mixed integer operations with data dependencies */
    for (i = 0; i < n; i++) {
        int idx = (i + seed) % n;
        int val = array[idx];
        
        /* Complex computation with multiple operations */
        int temp = val * i;
        temp += (val > 100) ? val : (val * 2);  /* Conditional move pattern */
        temp = helper_mul(temp, i + 1);         /* Function call */
        
        /* Memory barrier to create scheduling region */
        asm volatile("" : : : "memory");
        
        /* Floating point operations mixed in */
        float fval = (float)val;
        float ftemp = fval * 0.5f;
        ftemp = helper_fmul(ftemp, fval);
        
        /* Use builtin for complex RTL pattern */
        int bits = __builtin_popcount(val);
        temp += bits;
        
        /* Another barrier */
        asm volatile("" : : : "memory");
        
        /* Store result with conditional */
        if (i % 3 == 0) {
            array[idx] = temp;
        } else if (i % 3 == 1) {
            array[idx] = temp / 2;
        } else {
            array[idx] = temp + (int)ftemp;
        }
        
        sum += temp;
        barrier = sum;  /* Volatile write to prevent dead code elimination */
    }
    
    return sum;
}

/* Function with 64-bit operations and more complex control flow */
__attribute__((noinline, optimize("O3")))
int64_t test_function_2(int64_t* data, int size, int64_t init) {
    int64_t result = init;
    int i;
    
    for (i = 0; i < size; i++) {
        /* 64-bit operations */
        int64_t val = data[i];
        int64_t squared = val * val;
        
        /* Complex conditional with multiple branches */
        if (val & 1) {
            /* Odd path */
            squared = squared >> 2;
            result += squared * 3;
        } else {
            /* Even path */
            squared = squared << 1;
            result += squared / 5;
        }
        
        /* Mixed width operations */
        int32_t low = (int32_t)val;
        int32_t high = (int32_t)(val >> 32);
        int32_t mixed = low * high;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Use result in memory store */
        data[i] = result + mixed;
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return result;
}

/* Function with pointer arithmetic and memory accesses */
__attribute__((noinline, target("arch=haswell")))
float test_function_3(float* a, float* b, float* c, int n) {
    float sum = 0.0f;
    int i;
    
    for (i = 0; i < n; i++) {
        /* SIMD-friendly pattern that might generate interesting RTL */
        float aval = a[i];
        float bval = b[i];
        float cval = c[i];
        
        /* Complex FP expression */
        float temp = aval * bval + cval;
        temp = temp / (aval + 1.0f);
        
        /* Conditional store */
        if (temp > 0.0f) {
            c[i] = temp;
        } else {
            c[i] = -temp;
        }
        
        /* Accumulate with dependency */
        sum += temp;
        
        /* Barrier every 4 iterations */
        if (i % 4 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return sum;
}

/* Function with switch statement for varied control flow */
__attribute__((noinline))
int test_function_4(int mode, int iterations) {
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        switch (mode) {
            case 0:
                result += i * 2;
                break;
            case 1:
                result += i * i;
                /* Builtin with side effect */
                result += __builtin_ffs(i);
                break;
            case 2:
                result += i >> 1;
                /* Memory barrier */
                asm volatile("" : : : "memory");
                result += i & 0xFF;
                break;
            default:
                result += i % 10;
                break;
        }
        
        /* Change mode periodically */
        if (i % 100 == 0) {
            mode = (mode + 1) % 4;
        }
    }
    
    return result;
}

/* Main driver that calls all test functions */
int main(void) {
    const int SIZE = 1024;
    int* array1 = malloc(SIZE * sizeof(int));
    int64_t* array2 = malloc(SIZE * sizeof(int64_t));
    float* array3a = malloc(SIZE * sizeof(float));
    float* array3b = malloc(SIZE * sizeof(float));
    float* array3c = malloc(SIZE * sizeof(float));
    
    /* Initialize data with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 37 + 123) % 1000;
        array2[i] = (int64_t)i * 7919;
        array3a[i] = (float)(i % 100) * 0.1f;
        array3b[i] = (float)(i % 50) * 0.2f;
        array3c[i] = (float)(i % 25) * 0.4f;
    }
    
    printf("Starting selective scheduler test...\n");
    
    /* Call test functions with different patterns */
    int result1 = test_function_1(array1, SIZE, 42);
    printf("Test 1 result: %d\n", result1);
    
    int64_t result2 = test_function_2(array2, SIZE / 2, 1000);
    printf("Test 2 result: %lld\n", (long long)result2);
    
    float result3 = test_function_3(array3a, array3b, array3c, SIZE);
    printf("Test 3 result: %f\n", result3);
    
    int result4 = test_function_4(0, 1000);
    printf("Test 4 result: %d\n", result4);
    
    /* Final checksum to verify computation */
    int final_sum = result1 + (int)result2 + (int)result3 + result4;
    printf("Final checksum: %d\n", final_sum);
    
    free(array1);
    free(array2);
    free(array3a);
    free(array3b);
    free(array3c);
    
    return 0;
}
