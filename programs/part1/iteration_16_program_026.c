#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

// Function with complex scheduling patterns
__attribute__((hot, optimize("O3"))) 
static float hot_function(float* restrict a, float* restrict b, float* restrict c, int n) {
    float sum = 0.0f;
    
    // Mixed integer and floating point operations with dependencies
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        // RAW hazard: b depends on a
        float temp = a[i] * 2.0f;
        
        // WAR hazard: temp is written then read
        b[i] = temp + 1.0f;
        
        // WAW hazard: sum is written multiple times
        sum += b[i] * c[i];
        
        // Pointer chasing pattern
        if (i > 0) {
            // Memory barrier to force scheduling decisions
            asm volatile("" ::: "memory");
            
            // Complex dependency chain
            a[i] = a[i-1] * 0.5f + b[i];
        }
    }
    
    // Conditional move vs branching
    return (sum > 0.0f) ? sum : -sum;
}

// Cold function with different scheduling characteristics
__attribute__((cold, noinline, optimize("sched-pressure")))
static double cold_function(double* restrict arr, int* restrict indices, int n) {
    double result = 0.0;
    
    // Nested loops with mixed control flow
    for (int i = 0; i < n; i += 2) {
        // Early exit condition
        if (indices[i] >= n) break;
        
        // Pointer chasing with computed index
        int idx = indices[i];
        
        // Switch statement with sparse values
        switch (idx % 7) {
            case 0:
                arr[idx] = sin(arr[idx]);
                break;
            case 1:
                arr[idx] = cos(arr[idx]);
                break;
            case 3:  // Note: case 2 is missing
                arr[idx] = sqrt(fabs(arr[idx]));
                break;
            case 5:
                // Assembly with register clobber
                asm volatile (
                    "movq %1, %%rax\n\t"
                    "addq $1, %%rax\n\t"
                    "movq %%rax, %0"
                    : "=r" (arr[idx])
                    : "r" ((int64_t)arr[idx])
                    : "rax"
                );
                break;
            default:
                arr[idx] = arr[idx] * 0.99;
                break;
        }
        
        // Continue condition
        if (arr[idx] < 0) continue;
        
        result += arr[idx];
        
        // Another memory barrier
        asm volatile("" ::: "memory");
    }
    
    return result;
}

// Function with vectorization opportunities
__attribute__((optimize("O3"), always_inline))
static inline void vectorized_loop(float* restrict src, float* restrict dst, int n) {
    // SIMD-friendly loop
    #pragma GCC unroll 8
    for (int i = 0; i < n; i++) {
        // Multiple independent operations for vectorization
        float x = src[i];
        float y = x * x;
        float z = y + x;
        dst[i] = z * 0.5f;
        
        // Create WAW hazard
        if (i % 4 == 0) {
            dst[i] += 1.0f;
        }
    }
}

// Complex control flow function
__attribute__((noinline))
static int complex_control_flow(int* data, int n) {
    int total = 0;
    int i = 0;
    
    // Loop with multiple exit points
    while (1) {
        if (i >= n) break;
        
        // Nested if-else chain
        if (data[i] < 0) {
            total -= data[i];
            i += 2;
            continue;
        } else if (data[i] > 100) {
            total += data[i] * 2;
            i += 3;
            
            // Memory barrier in middle of dependency chain
            asm volatile("" ::: "memory");
            
            if (total > 1000) {
                break;
            }
            continue;
        } else {
            total += data[i];
        }
        
        // Computed goto-like pattern using switch
        switch (i % 5) {
            case 0: i += 1; break;
            case 1: i += 2; break;
            case 2: i += 3; break;
            case 3: i += 4; break;
            case 4: i += 5; break;
        }
        
        // Another scheduling barrier
        asm volatile("" ::: "memory");
    }
    
    return total;
}

// Main test function that calls all patterns
__attribute__((optimize("O3")))
static double run_scheduling_tests(void) {
    // Allocate arrays
    float* fa = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* fb = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* fc = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double* da = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* indices = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* idata = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    // Initialize data
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)(i % 100) * 0.1f;
        fb[i] = (float)(i % 50) * 0.2f;
        fc[i] = (float)(i % 25) * 0.3f;
        da[i] = (double)(i % 200) * 0.05;
        indices[i] = (i * 13) % ARRAY_SIZE;  // Scattered access pattern
        idata[i] = (i * 7) % 150;
    }
    
    double total_result = 0.0;
    
    // Run multiple iterations to ensure execution
    for (int iter = 0; iter < ITERATIONS; iter++) {
        // Call hot function (should trigger selective scheduling)
        float hot_result = hot_function(fa, fb, fc, ARRAY_SIZE);
        total_result += hot_result;
        
        // Call cold function
        double cold_result = cold_function(da, indices, ARRAY_SIZE);
        total_result += cold_result;
        
        // Vectorized operations
        vectorized_loop(fa, fb, ARRAY_SIZE);
        total_result += fb[ARRAY_SIZE/2];
        
        // Complex control flow
        int cf_result = complex_control_flow(idata, ARRAY_SIZE);
        total_result += cf_result;
        
        // Modify data for next iteration
        for (int i = 0; i < ARRAY_SIZE; i++) {
            fa[i] += 0.01f;
            da[i] += 0.005;
            idata[i] = (idata[i] + 1) % 200;
        }
    }
    
    // Cleanup
    free(fa);
    free(fb);
    free(fc);
    free(da);
    free(indices);
    free(idata);
    
    return total_result;
}

int main(void) {
    clock_t start = clock();
    
    // Run the scheduling tests
    double result = run_scheduling_tests();
    
    clock_t end = clock();
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Print result to prevent dead code elimination
    printf("Total result: %f\n", result);
    printf("Time used: %f seconds\n", cpu_time_used);
    
    // Additional print to ensure all code paths are considered
    printf("Test completed successfully.\n");
    
    return 0;
}
