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
    
    /* Memory barrier to create scheduling region */
    asm volatile ("" : : : "memory");
    
    for (int i = 0; i < n; i++) {
        /* Mixed integer and FP operations */
        int val = arr[i];
        int squared = val * val;
        float fval = (float)val;
        
        /* Conditional operation creating if_then_else RTL */
        int cond_val = (val > 100) ? squared : val;
        
        /* Use builtin for complex RTL pattern */
        int popcnt = __builtin_popcount(val);
        
        /* Memory store operation */
        arr[i] = cond_val + popcnt;
        
        /* Mixed-type accumulation */
        sum += arr[i];
        fsum += fval * i;
        
        /* Another barrier mid-loop */
        if (i % 8 == 0) {
            asm volatile ("" : : : "memory");
        }
        
        /* Call to non-inline function */
        sum += helper_mul(val, i);
    }
    
    /* Final volatile write to prevent dead code elimination */
    volatile int final_sum = sum + (int)fsum;
    return final_sum;
}

/* Test with nested loops - Test 2 */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int size) {
    int total = 0;
    int* matrix = (int*)malloc(size * size * sizeof(int));
    
    if (!matrix) return 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < size * size; i++) {
        matrix[i] = i % 256;
    }
    
    /* Complex nested loop with data dependencies */
    for (int i = 0; i < size; i++) {
        int row_sum = 0;
        for (int j = 0; j < size; j++) {
            int idx = i * size + j;
            int val = matrix[idx];
            
            /* Complex conditional expression */
            int processed = (val & 1) ? 
                __builtin_ffs(val) : 
                __builtin_clz(val);
                
            /* Floating point in inner loop */
            float fprocessed = helper_fmul((float)processed, 1.5f);
            
            row_sum += processed + (int)fprocessed;
            
            /* Store back with transformation */
            matrix[idx] = (val * 1103515245 + 12345) & 0x7fffffff;
        }
        total ^= row_sum; /* Non-linear accumulation */
    }
    
    free(matrix);
    return total;
}

/* Test with switch statement - Test 3 */
__attribute__((noinline, optimize("O3")))
int test_switch_pattern(int* data, int len) {
    int result = 0;
    
    for (int i = 0; i < len; i++) {
        int val = data[i] % 5;
        
        /* Switch creates complex control flow */
        switch (val) {
            case 0:
                result += data[i] * 2;
                asm volatile ("" : : : "memory");
                break;
            case 1:
                result += __builtin_popcount(data[i]);
                break;
            case 2:
                result += helper_mul(data[i], i);
                break;
            case 3:
                /* Mixed operations in case */
                float ftmp = (float)data[i];
                result += (int)(ftmp * 3.14159f);
                break;
            default:
                result += data[i] >> 3;
                break;
        }
        
        /* Conditional store */
        if (i % 3 == 0) {
            data[i] = result & 0xFF;
        }
    }
    
    return result;
}

/* Test with 64-bit operations - Test 4 */
__attribute__((target("arch=haswell")))
__attribute__((noinline, optimize("O3")))
int64_t test_64bit_ops(int64_t* arr, int n) {
    int64_t sum = 0;
    uint64_t usum = 0;
    
    for (int i = 0; i < n; i++) {
        /* 64-bit operations */
        int64_t val = arr[i];
        int64_t squared = val * val;
        
        /* 32/64-bit mix */
        int32_t low = (int32_t)(val & 0xFFFFFFFF);
        int32_t high = (int32_t)(val >> 32);
        
        /* Complex 64-bit expression */
        int64_t combined = ((int64_t)low * high) + squared;
        
        /* Bit manipulation */
        uint64_t rotated = (val << 13) | (val >> (64 - 13));
        
        sum += combined;
        usum += rotated;
        
        /* Memory barrier every 16 iterations */
        if (i % 16 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    return sum + (int64_t)usum;
}

/* Main driver */
int main() {
    const int SIZE = 256;
    int* data = (int*)malloc(SIZE * sizeof(int));
    int64_t* data64 = (int64_t*)malloc(SIZE * sizeof(int64_t));
    
    if (!data || !data64) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7FFF;
        data64[i] = (int64_t)data[i] * 1000000;
    }
    
    printf("Starting selective scheduler tests...\n");
    
    /* Run all tests to trigger scheduling */
    int result1 = test_selective_sched_loop(data, SIZE);
    printf("Test 1 result: %d\n", result1);
    
    int result2 = test_nested_loops(32);
    printf("Test 2 result: %d\n", result2);
    
    int result3 = test_switch_pattern(data, SIZE);
    printf("Test 3 result: %d\n", result3);
    
    int64_t result4 = test_64bit_ops(data64, SIZE);
    printf("Test 4 result: %ld\n", (long)result4);
    
    /* Final checksum */
    int final_sum = result1 + result2 + result3 + (int)result4;
    printf("Final checksum: %d\n", final_sum);
    
    free(data);
    free(data64);
    
    return 0;
}
