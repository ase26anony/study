/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline helper functions to generate call RTL */
static int __attribute__((noinline)) helper_mul(int a, int b) {
    return a * b;
}

static float __attribute__((noinline)) helper_fmul(float a, float b) {
    return a * b;
}

/* Complex loop with mixed operations - Test 1 */
__attribute__((noinline, optimize("O3")))
int test_selective_sched_loop(int* array, int size) {
    int sum = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < size; i++) {
        /* Integer operations with data dependency */
        int val = array[i];
        int scaled = val * i;
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : scaled;
        
        /* Mixed floating point operations */
        float fval = (float)val;
        float fscaled = fval * (i * 0.5f);
        
        /* Memory access pattern */
        array[i] = cond_val + i;
        
        /* Builtin function for complex RTL */
        int popcnt = __builtin_popcount(val);
        
        /* Inline assembly as scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* Accumulate results with different types */
        sum += cond_val + popcnt;
        fsum += fscaled;
        
        /* Branch to create control flow */
        if (i % 3 == 0) {
            sum += helper_mul(val, i);
        } else if (i % 5 == 0) {
            fsum += helper_fmul(fval, fval);
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : "+r"(sum), "+r"(fsum));
    return sum + (int)fsum;
}

/* Test with nested loops - Test 2 */
__attribute__((noinline, optimize("O3"), target("arch=core2")))
int test_nested_loops(int* data, int width, int height) {
    int total = 0;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            
            /* Complex addressing mode */
            int val = data[idx];
            
            /* Mixed 32/64 bit operations */
            long long big_val = (long long)val * idx;
            
            /* Bit manipulation operations */
            int rotated = (val << 3) | (val >> 29);
            
            /* Conditional with side effects */
            int new_val;
            if (big_val > 1000LL) {
                new_val = rotated + idx;
            } else {
                new_val = val - idx;
            }
            
            /* Store with potential aliasing */
            data[idx] = new_val;
            
            /* Complex accumulation */
            total += new_val + (int)(big_val & 0xFFFFFFFF);
            
            /* Another scheduling barrier */
            asm volatile ("" : : : "memory");
        }
    }
    
    return total;
}

/* Test with pointer chasing - Test 3 */
__attribute__((noinline, optimize("O2")))
int test_pointer_chasing(int* base, int steps) {
    int* current = base;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        /* Load from unpredictable address */
        int val = *current;
        
        /* Complex computation */
        int transformed = ((val & 0xFF) << 16) | 
                         ((val & 0xFF00) >> 8) | 
                         (val & 0xFF0000);
        
        /* Update pointer with stride */
        current = base + (transformed % 256);
        
        /* Use builtin for conditional */
        int leading_zeros = __builtin_clz(val | 1);
        
        sum += transformed + leading_zeros;
        
        /* Barrier every 8 iterations */
        if (i % 8 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    return sum;
}

/* Test with SIMD-like operations - Test 4 */
__attribute__((noinline, optimize("O3"), target("arch=haswell")))
int test_simd_patterns(short* shorts, int count) {
    int sum[4] = {0, 0, 0, 0};
    
    for (int i = 0; i < count; i += 4) {
        /* Manual unrolling for ILP */
        short s0 = shorts[i];
        short s1 = shorts[i + 1];
        short s2 = shorts[i + 2];
        short s3 = shorts[i + 3];
        
        /* Parallel computations */
        int p0 = s0 * i;
        int p1 = s1 * (i + 1);
        int p2 = s2 * (i + 2);
        int p3 = s3 * (i + 3);
        
        /* Conditional updates */
        p0 = (p0 > 0) ? p0 : -p0;
        p1 = (p1 > 0) ? p1 : -p1;
        p2 = (p2 > 0) ? p2 : -p2;
        p3 = (p3 > 0) ? p3 : -p3;
        
        /* Accumulate in different elements */
        sum[0] += p0;
        sum[1] += p1;
        sum[2] += p2;
        sum[3] += p3;
        
        /* Memory store */
        shorts[i] = (short)(p0 & 0xFFFF);
        shorts[i + 1] = (short)(p1 & 0xFFFF);
    }
    
    /* Reduce results */
    return sum[0] + sum[1] + sum[2] + sum[3];
}

/* Main driver */
int main() {
    const int SIZE = 1024;
    const int WIDTH = 32;
    const int HEIGHT = 32;
    
    /* Allocate and initialize test data */
    int* array1 = (int*)malloc(SIZE * sizeof(int));
    int* array2 = (int*)malloc(WIDTH * HEIGHT * sizeof(int));
    short* array3 = (short*)malloc(SIZE * sizeof(short));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        array3[i] = (short)(array1[i] & 0xFFFF);
    }
    
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        array2[i] = (i * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Run all tests */
    int result1 = test_selective_sched_loop(array1, SIZE);
    int result2 = test_nested_loops(array2, WIDTH, HEIGHT);
    int result3 = test_pointer_chasing(array1, 256);
    int result4 = test_simd_patterns(array3, SIZE);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2 + result3 + result4;
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int output = final_result;
    
    printf("Test results: %d\n", output);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
