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
int test_mixed_operations(int* arr, int n) {
    int sum_int = 0;
    float sum_float = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing computation */
        int val = arr[i];
        
        /* Integer operations with data dependency */
        int temp = val * i;
        temp += (i & 1) ? temp >> 2 : temp << 2; /* Conditional shift */
        
        /* Built-in function for complex RTL */
        int popcnt = __builtin_popcount(val);
        
        /* Floating point operations */
        float fval = (float)val;
        float ftemp = fval * (float)i;
        
        /* Inline assembly as scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : 100;
        
        /* Function calls for call RTL */
        sum_int += helper_mul(temp, popcnt);
        sum_float += helper_fmul(ftemp, (float)cond_val);
        
        /* Another barrier */
        asm volatile("" : : : "memory");
        
        /* Complex conditional with branching */
        if (i % 3 == 0) {
            sum_int += __builtin_ctz(val | 1); /* Count trailing zeros */
        } else if (i % 3 == 1) {
            sum_int -= __builtin_clz(val); /* Count leading zeros */
        } else {
            sum_int ^= val;
        }
        
        barrier = i; /* Use volatile to prevent dead code elimination */
    }
    
    return sum_int + (int)sum_float + barrier;
}

/* Function with outer loop pipelining opportunities - Test 2 */
__attribute__((noinline, optimize("O3")))
int test_outer_loop_pipelining(int* arr1, int* arr2, int n) {
    int total = 0;
    
    /* Outer loop with complex inner loop */
    for (int outer = 0; outer < 10; outer++) {
        int inner_sum = 0;
        
        /* Inner loop with data-dependent computations */
        for (int i = 0; i < n; i++) {
            /* Multiple memory accesses */
            int a = arr1[i];
            int b = arr2[i];
            
            /* Complex integer arithmetic */
            int prod = a * b;
            int div = (b != 0) ? a / (b | 1) : 0; /* Avoid division by zero */
            
            /* Bit manipulation */
            int rotated = (prod << 3) | (prod >> 29);
            
            /* Conditional based on multiple variables */
            int select = (a > b) ? 
                         ((a & 1) ? a : b) : 
                         ((b & 1) ? b : a);
            
            inner_sum += rotated + div + select;
            
            /* Memory store */
            arr1[i] = (inner_sum & 0xFF);
        }
        
        total += inner_sum;
        
        /* Scheduling barrier between outer loop iterations */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

/* Function targeting specific architecture features - Test 3 */
__attribute__((noinline, target("arch=haswell")))
int test_arch_specific(int* arr, int n) {
    long long sum64 = 0;
    int sum32 = 0;
    
    for (int i = 0; i < n; i++) {
        /* 64-bit operations */
        long long val64 = (long long)arr[i] * i;
        
        /* Mixed 32/64 bit operations */
        sum64 += val64;
        sum32 += (int)(sum64 >> 32); /* Extract high bits */
        
        /* SIMD-like operations manually */
        int a = arr[i];
        int b = arr[(i + 1) % n];
        int c = arr[(i + 2) % n];
        int d = arr[(i + 3) % n];
        
        /* Parallel computations */
        int ab = a * b;
        int cd = c * d;
        int ac = a + c;
        int bd = b + d;
        
        sum32 += ab + cd + ac + bd;
        
        /* Complex conditional with side effects */
        if ((a ^ b) > (c ^ d)) {
            sum64 -= val64;
            asm volatile("" : : : "memory");
        }
    }
    
    return sum32 + (int)sum64;
}

/* Function with unpredictable control flow - Test 4 */
__attribute__((noinline, optimize("O2")))
int test_control_flow(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    while (i < n) {
        /* Unpredictable branch */
        if (arr[i] & 1) {
            /* Complex path 1 */
            int x = arr[i];
            for (int j = 0; j < 4; j++) {
                x = (x * 1103515245 + 12345) & 0x7FFFFFFF;
                result ^= x;
            }
            i += 2;
        } else {
            /* Complex path 2 */
            int y = arr[i];
            y = __builtin_bswap32(y); /* Byte swap */
            result += y * i;
            i += 1;
        }
        
        /* Another level of conditionals */
        switch (result & 3) {
            case 0:
                result += helper_mul(arr[i % n], i);
                break;
            case 1:
                result -= arr[i % n];
                break;
            case 2:
                result ^= ~arr[i % n];
                break;
            case 3:
                result = (result << 3) | (result >> 29);
                break;
        }
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Main driver */
int main() {
    const int SIZE = 256;
    int* array1 = (int*)malloc(SIZE * sizeof(int));
    int* array2 = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        array2[i] = (i * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Run all test functions */
    int result1 = test_mixed_operations(array1, SIZE);
    int result2 = test_outer_loop_pipelining(array1, array2, SIZE);
    int result3 = test_arch_specific(array1, SIZE);
    int result4 = test_control_flow(array2, SIZE);
    
    /* Combine results to ensure all code is live */
    int final_result = result1 + result2 + result3 + result4;
    
    printf("Test Results:\n");
    printf("  Mixed Operations: %d\n", result1);
    printf("  Outer Loop Pipelining: %d\n", result2);
    printf("  Arch Specific: %d\n", result3);
    printf("  Control Flow: %d\n", result4);
    printf("  Final Result: %d\n", final_result);
    
    free(array1);
    free(array2);
    
    return (final_result != 0) ? 0 : 1; /* Non-zero result indicates execution */
}
