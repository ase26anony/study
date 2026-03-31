/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Non-inline helper functions to generate call RTL */
static int __attribute__((noinline)) helper_mul(int a, int b) {
    return a * b;
}

static float __attribute__((noinline)) helper_fmul(float a, float b) {
    return a * b;
}

/* Complex loop with mixed operations - Test 1 */
__attribute__((noinline, optimize("O2")))
int test_complex_loop(int* array, int size) {
    int sum = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < size; i++) {
        /* Integer operations with data dependency */
        int val = array[i];
        int scaled = val * i;
        
        /* Conditional move/ternary operation */
        int threshold = (i > size/2) ? scaled : val;
        
        /* Mixed floating point operations */
        float fval = (float)val;
        float fscaled = fval * (i * 0.5f);
        
        /* Memory barrier to create scheduling regions */
        asm volatile ("" : : : "memory");
        
        /* Built-in function for complex RTL */
        int popcnt = __builtin_popcount(val);
        
        /* More operations with dependencies */
        sum += threshold + popcnt;
        fsum += fscaled;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 3 == 0) {
            sum += helper_mul(val, i);
        } else if (i % 3 == 1) {
            fsum += helper_fmul(fval, fval);
        } else {
            /* Array access with different pattern */
            array[i] = (sum & 0xFF) + i;
        }
        
        /* Another scheduling barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Combine results to prevent elimination */
    return sum + (int)fsum;
}

/* Test with nested loops - Test 2 */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int* data, int width, int height) {
    int total = 0;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            
            /* Complex addressing calculation */
            int neighbor_sum = 0;
            if (x > 0) neighbor_sum += data[idx - 1];
            if (x < width - 1) neighbor_sum += data[idx + 1];
            if (y > 0) neighbor_sum += data[idx - width];
            if (y < height - 1) neighbor_sum += data[idx + width];
            
            /* Conditional computation */
            int new_val = (data[idx] > neighbor_sum) ? 
                         data[idx] * 2 : 
                         neighbor_sum / 2;
            
            /* Mixed 32/64 bit operations */
            int64_t big_val = (int64_t)new_val * (int64_t)idx;
            total += (int)(big_val & 0xFFFFFFFF) + (int)(big_val >> 32);
            
            /* Update array */
            data[idx] = new_val & 0xFF;
        }
        
        /* Scheduling barrier between rows */
        asm volatile ("" : : : "memory");
    }
    
    return total;
}

/* Test with vectorizable pattern - Test 3 */
__attribute__((noinline, target("arch=haswell")))
float test_vector_ops(float* a, float* b, int n) {
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Operations that could vectorize */
        float prod = a[i] * b[i];
        float diff = a[i] - b[i];
        
        /* Conditional based on computation */
        float result = (prod > 0.0f) ? 
                      sqrtf(prod) : 
                      fabsf(diff);
        
        /* Complex dependency chain */
        sum += result * i;
        
        /* Cross-iteration dependency every 4 iterations */
        if (i % 4 == 0) {
            a[i] = sum * 0.1f;
            asm volatile ("" : : : "memory");
        }
    }
    
    return sum;
}

/* Test with switch statement - Test 4 */
__attribute__((noinline))
int test_switch_pattern(int* values, int n, int mode) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        int val = values[i];
        
        switch (mode) {
            case 0:
                result += val * 2;
                break;
            case 1:
                result += val >> 1;
                /* Fall through */
            case 2:
                result += __builtin_ctz(val | 1);
                break;
            case 3:
                result += helper_mul(val, i);
                asm volatile ("" : : : "memory");
                break;
            default:
                result += val;
        }
        
        /* Additional computation */
        if (val % 2 == 0) {
            result += (val * 3) / 2;
        } else {
            result -= val / 2;
        }
    }
    
    return result;
}

/* Main driver */
int main() {
    const int SIZE = 256;
    const int WIDTH = 16;
    const int HEIGHT = 16;
    
    /* Initialize test data */
    int* array1 = (int*)malloc(SIZE * sizeof(int));
    int* array2 = (int*)malloc(WIDTH * HEIGHT * sizeof(int));
    float* farray1 = (float*)malloc(SIZE * sizeof(float));
    float* farray2 = (float*)malloc(SIZE * sizeof(float));
    
    /* Use volatile to prevent optimization */
    volatile int seed = 42;
    
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 17 + seed) % 100;
        farray1[i] = (float)(i * 23 + seed) / 100.0f;
        farray2[i] = (float)(i * 29 + seed) / 100.0f;
    }
    
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        array2[i] = (i * 13 + seed) % 200;
    }
    
    /* Run all tests */
    int result1 = test_complex_loop(array1, SIZE);
    int result2 = test_nested_loops(array2, WIDTH, HEIGHT);
    float result3 = test_vector_ops(farray1, farray2, SIZE);
    int result4 = test_switch_pattern(array1, SIZE, 2);
    
    /* Combine results to ensure all code is used */
    int final_result = result1 + result2 + (int)result3 + result4;
    
    printf("Test Results:\n");
    printf("  Test 1: %d\n", result1);
    printf("  Test 2: %d\n", result2);
    printf("  Test 3: %f\n", result3);
    printf("  Test 4: %d\n", result4);
    printf("  Final: %d\n", final_result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(farray1);
    free(farray2);
    
    return (final_result > 0) ? 0 : 1;
}
