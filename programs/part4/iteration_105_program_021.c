#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 10

// Simple LCG for deterministic pseudo-random values
static unsigned int seed = 123456789;
static inline float rand_float(void) {
    seed = seed * 1103515245 + 12345;
    return (float)(seed % 1000) / 1000.0f;
}

// Function containing multiple SIMD loops with various clauses
void compute(float *a, float *b, float *c, int n, int iter) {
    float k = 1.5f + (float)iter * 0.1f;
    float sum = 0.0f;
    
    // Loop 1: Basic SIMD with safelen and simdlen
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    // Loop 2: SIMD with linear clause and conditional
    #pragma omp simd linear(k:1) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        float temp = a[i] * 2.0f + b[i];
        if (temp > 0.5f) {
            c[i] = temp * k;
            sum += c[i];
        } else {
            c[i] = temp / k;
        }
        k += 0.001f;  // Linear increment
    }
    
    // Loop 3: Nested context - outer loop with inner SIMD
    for (int j = 0; j < 4; j++) {
        float scale = 1.0f + j * 0.25f;
        #pragma omp simd simdlen(4) aligned(a, c: 32)
        for (int i = 0; i < n; i += 2) {
            // Mixed-width operations
            c[i] = a[i] * scale + (float)j;
            if (i + 1 < n) {
                c[i + 1] = a[i + 1] * scale * 2.0f - (float)j;
            }
        }
    }
    
    // Loop 4: SIMD with double precision mixed in
    double dsum = 0.0;
    #pragma omp simd simdlen(4) reduction(+:dsum)
    for (int i = 0; i < n; i++) {
        double dval = (double)a[i] * 1.5;
        dsum += dval;
        c[i] = (float)dval * b[i];
    }
    
    // Loop 5: Another SIMD with different clauses
    #pragma omp simd safelen(32) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = c[i] * 0.9f + a[i] * 0.1f;
    }
}

// Wrapper function to create calling context
void compute_wrapper(float *a, float *b, float *c, int n) {
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, n, iter);
        
        // Additional SIMD loop in wrapper
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            b[i] = c[i] * 0.5f + b[i] * 0.5f;
        }
    }
}

// Target offload version for additional SIMT context
#ifdef USE_TARGET
#pragma omp declare target
void target_compute(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
}
#pragma omp end declare target
#endif

int main(void) {
    float *a = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    // Initialize with pseudo-random pattern
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand_float();
        b[i] = rand_float();
        c[i] = 0.0f;
    }
    
    // Multiple calls to create different contexts
    compute_wrapper(a, b, c, SIZE);
    
    #ifdef USE_TARGET
    #pragma omp target data map(to: a[0:SIZE], b[0:SIZE]) map(from: c[0:SIZE])
    {
        target_compute(a, b, c, SIZE);
    }
    #endif
    
    // Verification checksum
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < SIZE; i++) {
        checksum += (double)c[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    
    return 0;
}
