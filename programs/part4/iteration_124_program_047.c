/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;

/* Non-inline helper functions to generate call RTL */
__attribute__((noinline, optimize("O2")))
int helper_compute(int a, int b) {
    return (a * b) + (a ^ b);
}

__attribute__((noinline, optimize("O2")))
float helper_float(float x, float y) {
    return x * y + x / (y + 1.0f);
}

/* Test 1: Mixed integer and FP operations with memory accesses */
__attribute__((noinline, optimize("O2")))
int test_mixed_operations(int* array, int size) {
    int int_sum = 0;
    float fp_sum = 0.0f;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Data-dependent integer computation */
        int val = array[i] + g_volatile_counter;
        int_sum += val * i;
        
        /* Floating-point computation */
        float fp_val = (float)val * g_volatile_float;
        fp_sum += fp_val * (i % 10);
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : (val * 2);
        int_sum += cond_val;
        
        /* Built-in function for complex RTL */
        int_sum += __builtin_popcount(val);
        
        /* Memory barrier to create scheduling region */
        asm volatile("" : : : "memory");
        
        /* Another memory access */
        if (i % 2 == 0) {
            array[i] = int_sum % 256;
        } else {
            array[i] = fp_sum;
        }
    }
    
    return int_sum + (int)fp_sum;
}

/* Test 2: Nested loops with complex control flow */
__attribute__((noinline, optimize("O2")))
int test_nested_loops(int* data, int rows, int cols) {
    int total = 0;
    int i, j;
    
    for (i = 0; i < rows; i++) {
        int row_sum = 0;
        for (j = 0; j < cols; j++) {
            int idx = i * cols + j;
            int val = data[idx];
            
            /* Complex conditional with multiple branches */
            if (val > 0) {
                row_sum += val * j;
                /* Call to non-inline function */
                row_sum += helper_compute(val, j);
            } else if (val < 0) {
                row_sum -= (-val) * i;
                /* Floating point in integer loop */
                float temp = helper_float((float)val, (float)i);
                row_sum += (int)temp;
            } else {
                row_sum += i + j;
            }
            
            /* Bit manipulation operations */
            row_sum ^= (row_sum << 3);
            row_sum &= 0xFFFF;
            
            /* Another scheduling barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Conditional store */
        data[i] = (row_sum > 1000) ? row_sum : (row_sum * 2);
        total += row_sum;
    }
    
    return total;
}

/* Test 3: SIMD-like operations using 32-bit and 64-bit types */
__attribute__((noinline, optimize("O2")))
long long test_mixed_width(uint32_t* arr32, uint64_t* arr64, int n) {
    long long total = 0;
    uint64_t acc64 = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        /* 32-bit operations */
        uint32_t val32 = arr32[i];
        uint32_t rotated = (val32 << 5) | (val32 >> 27);
        arr32[i] = rotated ^ 0xDEADBEEF;
        
        /* 64-bit operations */
        uint64_t val64 = arr64[i];
        val64 = val64 * 6364136223846793005ULL + 1;
        acc64 ^= val64;
        
        /* Mixed-width computation */
        total += (long long)val32 * (long long)val64;
        
        /* Complex conditional with multiple operations */
        if (total & 1) {
            total += acc64;
            /* Use builtin for population count on 64-bit */
            total += __builtin_popcountll(acc64);
        } else {
            total -= (acc64 >> 32);
        }
        
        /* Memory barrier every 8 iterations */
        if (i % 8 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return total + acc64;
}

/* Test 4: Function with switch statement for varied control flow */
__attribute__((noinline, optimize("O2")))
int test_switch_case(int mode, int iterations) {
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        switch ((mode + i) % 7) {
            case 0:
                result += i * 3;
                /* Call helper */
                result += helper_compute(i, mode);
                break;
            case 1:
                result -= i * 2;
                /* Floating point in switch */
                result += (int)helper_float((float)i, (float)mode);
                break;
            case 2:
                result ^= (i << 4);
                /* Memory access */
                g_volatile_counter = result;
                break;
            case 3:
                result = (result > 1000) ? result / 2 : result * 3;
                /* Builtin */
                result += __builtin_clz(i | 1);
                break;
            case 4:
                result = ~result;
                asm volatile("" : : : "memory");
                break;
            case 5:
                result += __builtin_popcount(i);
                /* Complex expression */
                result += (i % 3 == 0) ? (result >> 2) : (result << 2);
                break;
            default:
                result += mode + i;
                break;
        }
        
        /* Prevent simple loop optimization */
        if (i % 16 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return result;
}

/* Main driver that runs all tests */
int main(int argc, char** argv) {
    const int SIZE = 256;
    const int ROWS = 16;
    const int COLS = 16;
    const int ITERS = 1000;
    
    /* Allocate and initialize test data */
    int* array1 = (int*)malloc(SIZE * sizeof(int));
    int* array2 = (int*)malloc(ROWS * COLS * sizeof(int));
    uint32_t* array32 = (uint32_t*)malloc(SIZE * sizeof(uint32_t));
    uint64_t* array64 = (uint64_t*)malloc(SIZE * sizeof(uint64_t));
    
    if (!array1 || !array2 || !array32 || !array64) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 37 + 123) % 1000;
        array32[i] = (uint32_t)(i * 7919);
        array64[i] = (uint64_t)(i * 104729);
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        array2[i] = (i * 53 + 456) % 500;
    }
    
    /* Run all tests */
    int result1 = test_mixed_operations(array1, SIZE);
    printf("Test 1 result: %d\n", result1);
    
    int result2 = test_nested_loops(array2, ROWS, COLS);
    printf("Test 2 result: %d\n", result2);
    
    long long result3 = test_mixed_width(array32, array64, SIZE);
    printf("Test 3 result: %lld\n", result3);
    
    int result4 = test_switch_case(argc > 1 ? atoi(argv[1]) : 3, ITERS);
    printf("Test 4 result: %d\n", result4);
    
    /* Final checksum */
    int final_sum = result1 + result2 + (int)result3 + result4;
    printf("Final checksum: %d\n", final_sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array32);
    free(array64);
    
    return 0;
}
