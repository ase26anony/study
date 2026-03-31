#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 10

// Simple LCG for deterministic pseudo-random values
static unsigned int seed = 123456789;
static inline float rand_float() {
    seed = seed * 1103515245 + 12345;
    return (float)(seed % 1000) / 1000.0f;
}

// Function containing multiple SIMD loops with various clauses
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + (float)iter * 0.1f;
    float sum1 = 0.0f, sum2 = 0.0f;
    
    // Loop 1: Basic SIMD with safelen and simdlen
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    // Loop 2: SIMD with linear clause and conditional
    #pragma omp simd linear(i:1) reduction(+:sum1)
    for (int i = 0; i < n; i++) {
        if (c[i] > 0.5f) {
            sum1 += c[i] * 2.0f;
        } else {
            sum1 += c[i] * 0.5f;
        }
    }
    
    // Loop 3: SIMD with different data type (double) and simdlen
    #pragma omp simd simdlen(4) aligned(d: 64)
    for (int i = 0; i < n; i++) {
        double temp = (double)a[i] * 1.5;
        d[i] = (float)temp + (float)(i % 8) * 0.1f;
    }
    
    // Loop 4: Nested SIMD-like pattern
    for (int j = 0; j < 4; j++) {
        float offset = (float)j * 0.25f;
        #pragma omp simd safelen(8) linear(ref:1) reduction(+:sum2)
        for (int i = 0; i < n; i++) {
            float val = a[i] + b[i] * offset;
            if (val > 0.0f) {
                c[i] += val * d[i];
                sum2 += val;
            }
        }
    }
    
    // Loop 5: SIMD with mixed operations
    #pragma omp simd simdlen(16)
    for (int i = 0; i < n; i += 2) {
        // Vectorizable pattern with stride
        c[i] = a[i] * b[i] - d[i];
        if (i + 1 < n) {
            c[i + 1] = a[i + 1] + b[i + 1] * d[i + 1];
        }
    }
    
    // Use results to prevent dead code elimination
    a[0] = sum1 + sum2;
}

// Wrapper function to create calling context
void compute_wrapper(float *a, float *b, float *c, float *d, int n) {
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, n, iter);
        
        // Additional SIMD loop in wrapper context
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            b[i] = c[i] * 0.9f + d[i] * 0.1f;
        }
    }
}

// Target offload version (triggers different code paths)
#ifdef USE_TARGET
void compute_target(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + sinf(a[i]) * cosf(b[i]);
    }
}
#endif

int main() {
    // Allocate aligned memory for better vectorization
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
        a[i] = rand_float();
        b[i] = rand_float();
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Call compute function multiple times
    compute_wrapper(a, b, c, d, SIZE);
    
#ifdef USE_TARGET
    // Also call target version if enabled
    compute_target(a, b, c, SIZE);
#endif
    
    // Calculate checksum to verify computation
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < SIZE; i++) {
        checksum += (double)a[i] + (double)b[i] + (double)c[i] + (double)d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
