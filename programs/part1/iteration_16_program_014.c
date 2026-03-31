#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, optimize("O3"))) 
static int hot_function(int *arr, int n) {
    volatile int result = 0;
    for (int i = 0; i < n; i++) {
        // Mixed integer operations with dependencies
        arr[i] = arr[i] * 3 + 7;
        result += arr[i];
        
        // Memory barrier to split scheduling regions
        asm volatile("" ::: "memory");
        
        // More dependent operations
        arr[i] = (arr[i] << 2) | (arr[i] >> 30);
        result ^= arr[i];
    }
    return result;
}

__attribute__((cold, noinline))
static float cold_function(float *farr, int n) {
    float sum = 0.0f;
    
    // Complex control flow with mixed operations
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            farr[i] = sinf(farr[i]) * 2.0f;
        } else if (i % 3 == 1) {
            farr[i] = cosf(farr[i]) / 1.5f;
        } else {
            farr[i] = sqrtf(fabsf(farr[i]));
        }
        
        // Conditional move pattern
        sum += (farr[i] > 0.0f) ? farr[i] : -farr[i];
        
        // Another scheduling barrier
        asm volatile("" ::: "memory");
    }
    return sum;
}

__attribute__((optimize("sched-pressure")))
static void pointer_chasing(int **ptr_arr, int *data, int n) {
    // Pointer chasing with WAR/WAW hazards
    int *current = &data[0];
    for (int i = 0; i < n; i++) {
        int idx = *current % n;
        ptr_arr[i] = &data[idx];
        
        // RAW hazard
        data[idx] = data[idx] * 2 + i;
        
        // Update current pointer
        current = ptr_arr[i];
        
        // Assembly with register clobber
        asm volatile("" : "=r"(current) : "0"(current) : "eax", "memory");
    }
}

__attribute__((optimize("O3")))
static void vectorized_loop(double *darr, int n) {
    // SIMD-friendly loop that should vectorize
    #pragma GCC unroll 4
    for (int i = 0; i < n; i += 4) {
        // Mixed FP operations
        darr[i] = darr[i] * 1.1 + 0.5;
        darr[i+1] = darr[i+1] * 0.9 - 0.3;
        darr[i+2] = darr[i+2] * 1.2 + darr[i];
        darr[i+3] = darr[i+3] * 0.8 - darr[i+1];
        
        // Cross-iteration dependency
        if (i > 0) {
            darr[i] += darr[i-1] * 0.1;
        }
    }
}

__attribute__((noinline))
static int complex_control_flow(int *arr, int n) {
    int result = 0;
    
    // Nested loops with multiple exit points
    for (int i = 0; i < n; i++) {
        if (arr[i] < 0) {
            // Early continue
            continue;
        }
        
        for (int j = 0; j < 8; j++) {
            // Switch statement with sparse cases
            switch ((arr[i] + j) % 7) {
                case 0:
                    arr[i] += j * 3;
                    break;
                case 1:
                    arr[i] -= j * 2;
                    break;
                case 3:  // Sparse case
                    arr[i] *= j;
                    break;
                case 6:  // Another sparse case
                    arr[i] /= (j + 1);
                    break;
                default:
                    arr[i] ^= j;
                    // Fall through
            }
            
            // Early exit from inner loop
            if (arr[i] > 1000) {
                result++;
                break;
            }
        }
        
        // Memory barrier
        asm volatile("" ::: "memory");
        
        // Another hazard
        result += arr[i];
    }
    
    return result;
}

__attribute__((optimize("O3")))
static void mixed_operations(int *int_arr, float *float_arr, double *double_arr, int n) {
    // Heterogeneous instruction stream
    for (int i = 0; i < n; i++) {
        // Integer operations
        int_arr[i] = (int_arr[i] * 3 + 7) % 256;
        
        // Floating point operations
        float_arr[i] = sinf(float_arr[i]) * 2.0f + 1.0f;
        
        // Double precision operations
        double_arr[i] = double_arr[i] * 1.5 + cos(double_arr[i]);
        
        // Pointer arithmetic with dependency
        if (i > 0) {
            int_arr[i] += int_arr[i-1];
            float_arr[i] += float_arr[i-1] * 0.5f;
        }
        
        // Scheduling barrier every 8 iterations
        if (i % 8 == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

// Function with computed goto (challenges scheduler)
__attribute__((optimize("O2")))
static int computed_goto_pattern(int *arr, int n) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        int selector = arr[i] % 5;
        goto *labels[selector];
        
    label0:
        arr[i] += 1;
        result ^= arr[i];
        continue;
        
    label1:
        arr[i] *= 2;
        result += arr[i];
        continue;
        
    label2:
        arr[i] -= 3;
        result |= arr[i];
        continue;
        
    label3:
        arr[i] /= 2;
        result &= arr[i];
        continue;
        
    label4:
        arr[i] <<= 1;
        result -= arr[i];
        continue;
    }
    
    return result;
}

int main() {
    // Allocate arrays with different alignments
    int *int_arr = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    float *float_arr = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double *double_arr = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int **ptr_arr = aligned_alloc(64, ARRAY_SIZE * sizeof(int*));
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = (i * 17) % 256;
        float_arr[i] = (i % 100) * 0.01f;
        double_arr[i] = (i % 200) * 0.005;
    }
    
    long long total_result = 0;
    
    // Run multiple iterations to ensure execution
    for (int iter = 0; iter < ITERATIONS; iter++) {
        // Call all test functions in sequence
        total_result += hot_function(int_arr, ARRAY_SIZE);
        total_result += (int)cold_function(float_arr, ARRAY_SIZE);
        
        pointer_chasing(ptr_arr, int_arr, ARRAY_SIZE / 8);
        
        vectorized_loop(double_arr, ARRAY_SIZE);
        
        total_result += complex_control_flow(int_arr, ARRAY_SIZE);
        
        mixed_operations(int_arr, float_arr, double_arr, ARRAY_SIZE);
        
        total_result += computed_goto_pattern(int_arr, ARRAY_SIZE / 4);
        
        // Modify arrays slightly each iteration
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int_arr[i] = (int_arr[i] + iter) % 1024;
            float_arr[i] = fmodf(float_arr[i] + 0.1f, 1.0f);
            double_arr[i] = fmod(double_arr[i] + 0.05, 2.0);
        }
    }
    
    // Print result to prevent dead code elimination
    printf("Total result: %lld\n", total_result);
    
    // Cleanup
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(ptr_arr);
    
    return 0;
}
