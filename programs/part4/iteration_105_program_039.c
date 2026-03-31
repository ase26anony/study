#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 10

// Simple LCG for deterministic pseudo-random values
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

// Function containing multiple SIMD loops with various clauses
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + iter * 0.1f;
    float sum = 0.0f;
    
    // Loop 1: Basic SIMD with simdlen and safelen
    #pragma omp simd simdlen(8) safelen(16) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    // Loop 2: SIMD with linear clause and conditional
    #pragma omp simd simdlen(4) linear(k:1) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        float temp = a[i] + b[i];
        if (temp > 0.5f) {
            d[i] = temp * k;
            sum += d[i];
        } else {
            d[i] = temp / k;
        }
        k += 0.001f;  // Modified by linear clause
    }
    
    // Loop 3: Mixed operations with different data types
    #pragma omp simd simdlen(8) aligned(a, c, d: 64)
    for (int i = 0; i < n; i++) {
        // Mix float and double operations
        double temp_dbl = (double)a[i] * 1.5;
        c[i] = (float)temp_dbl + d[i] * 2.0f;
    }
    
    // Nested SIMD context
    for (int outer = 0; outer < 4; outer++) {
        float scale = 1.0f + outer * 0.25f;
        
        // Loop 4: SIMD inside outer loop
        #pragma omp simd simdlen(4) linear(scale:0.1) safelen(8)
        for (int i = 0; i < n/2; i++) {
            int idx = i * 2;
            a[idx] = b[idx] * scale;
            a[idx + 1] = b[idx + 1] / scale;
            scale += 0.001f;
        }
    }
    
    // Loop 5: SIMD with function calls to built-ins
    #pragma omp simd simdlen(4)
    for (int i = 0; i < n; i++) {
        // Use built-in math functions
        c[i] = __builtin_sinf(a[i]) + __builtin_expf(b[i]);
    }
    
    // Prevent dead code elimination
    volatile float dummy = sum;
    (void)dummy;
}

// Wrapper function to create calling context
void compute_wrapper(float *a, float *b, float *c, float *d, int n) {
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, n, iter);
    }
}

int main() {
    // Allocate aligned memory
    float *a = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *d = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with pseudo-random pattern
    for (int i = 0; i < SIZE; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        b[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Call computation multiple times
    compute_wrapper(a, b, c, d, SIZE);
    
    // Additional context: target offload region
    #ifdef _OPENMP
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:SIZE], b[0:SIZE]) map(from: c[0:SIZE]) \
        simdlen(8) safelen(16)
    for (int i = 0; i < SIZE; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
    #endif
    
    // Calculate checksum to prevent optimization
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
    for (int i = 0; i < SIZE; i++) {
        checksum += (double)c[i] + (double)d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    // Free memory
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
