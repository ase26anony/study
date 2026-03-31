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

/* Complex loop with mixed operations - Test 1 */
__attribute__((noinline, optimize("O3")))
int test_selective_sched_loop(int* arr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Memory access - generates mem RTL */
        int val = arr[i];
        
        /* Integer operations with data dependency */
        int prod = helper_mul(val, i);
        
        /* Conditional move/ternary - may generate if_then_else RTL */
        int cond_val = (val > 100) ? val : prod;
        
        /* Builtin function - generates specific RTL pattern */
        int popcnt = __builtin_popcount(val);
        
        /* Floating point operations */
        float fval = (float)val;
        float fprod = helper_fmul(fval, fval * 0.5f);
        
        /* Mixed type accumulation with barrier */
        sum += cond_val + popcnt;
        fsum += fprod;
        
        /* Scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* Branch with side effects */
        if (i % 3 == 0) {
            sum -= val;
            asm volatile ("" : : : "memory");
        } else if (i % 7 == 0) {
            sum += val * 2;
        }
    }
    
    /* Use both results to prevent optimization */
    return sum + (int)fsum;
}

/* Test with nested loops - Test 2 */
__attribute__((noinline, optimize("O3"), target("arch=core2")))
int test_nested_loops(int* arr, int n, int m) {
    volatile int total = 0;  /* volatile to prevent optimization */
    
    for (int i = 0; i < n; i++) {
        int inner_sum = 0;
        
        /* Inner loop with complex operations */
        for (int j = 0; j < m; j++) {
            /* Memory access with stride */
            int idx = (i * m + j) % n;
            int val = arr[idx];
            
            /* 32-bit and 64-bit operations */
            int32_t i32_op = val * j;
            int64_t i64_op = (int64_t)val * (int64_t)j;
            
            /* Bit manipulation */
            int rotated = (val << 4) | (val >> 28);
            
            /* Complex expression with multiple dependencies */
            inner_sum += i32_op + (int)(i64_op & 0xFFFFFFFF) + rotated;
            
            /* Another scheduling barrier */
            if (j % 5 == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        
        total += inner_sum;
        
        /* Conditional store */
        if (i % 2 == 0) {
            arr[i % n] = inner_sum & 0xFF;
        }
    }
    
    return total;
}

/* Test with pointer chasing - Test 3 */
__attribute__((noinline, optimize("O2")))
int test_pointer_chasing(int* arr, int n) {
    int sum = 0;
    int* ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Pointer access with offset */
        int val = *ptr;
        
        /* Complex address calculation */
        int offset = val % 16;
        int* next_ptr = ptr + offset;
        
        /* Ensure pointer stays within bounds */
        if (next_ptr >= arr && next_ptr < arr + n) {
            ptr = next_ptr;
        } else {
            ptr = arr + (i % n);
        }
        
        /* Multiple independent operations */
        int a = val * 3;
        int b = val / 2;
        int c = val + a - b;
        
        /* Use builtin for complex RTL */
        int parity = __builtin_parity(val);
        
        sum += c + parity;
        
        /* Frequent barrier */
        asm volatile ("" : : : "memory");
    }
    
    return sum;
}

/* Test with SIMD-like operations - Test 4 */
__attribute__((noinline, optimize("O3"), target("arch=haswell")))
int test_simd_patterns(short* arr, int n) {
    int sum[4] = {0, 0, 0, 0};
    
    for (int i = 0; i < n - 3; i += 4) {
        /* Load multiple values */
        short v0 = arr[i];
        short v1 = arr[i + 1];
        short v2 = arr[i + 2];
        short v3 = arr[i + 3];
        
        /* Parallel operations */
        int p0 = v0 * v0;
        int p1 = v1 * v1;
        int p2 = v2 * v2;
        int p3 = v3 * v3;
        
        /* Reduction */
        sum[0] += p0;
        sum[1] += p1;
        sum[2] += p2;
        sum[3] += p3;
        
        /* Cross-lane operations */
        int cross = (v0 * v1) + (v2 * v3);
        sum[0] += cross;
        
        /* Barrier every 8 iterations */
        if ((i / 4) % 8 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Final reduction */
    return sum[0] + sum[1] + sum[2] + sum[3];
}

/* Main driver */
int main() {
    const int SIZE = 1024;
    int* array = (int*)malloc(SIZE * sizeof(int));
    short* sarray = (short*)malloc(SIZE * sizeof(short));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 1103515245 + 12345) & 0x7FFF;
        sarray[i] = (short)(array[i] & 0xFFFF);
    }
    
    /* Run all tests */
    int result1 = test_selective_sched_loop(array, SIZE);
    int result2 = test_nested_loops(array, SIZE / 4, 8);
    int result3 = test_pointer_chasing(array, SIZE);
    int result4 = test_simd_patterns(sarray, SIZE);
    
    /* Use results to prevent dead code elimination */
    int total = result1 + result2 + result3 + result4;
    
    printf("Selective scheduler test result: %d\n", total);
    printf("Individual results: %d, %d, %d, %d\n", 
           result1, result2, result3, result4);
    
    free(array);
    free(sarray);
    
    return 0;
}
