#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ITERATIONS 10

// Simple LCG for deterministic pseudo-random values
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

// Function containing multiple SIMD loops with various clauses
void compute(float *a, float *b, float *c, int n, int iter) {
    float k = (float)iter * 0.1f;
    
    // Loop 1: Basic SIMD with safelen and simdlen
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    // Loop 2: SIMD with linear clause and conditional
    #pragma omp simd linear(k:1) reduction(+:k) safelen(32)
    for (int i = 0; i < n; i++) {
        if (c[i] > 0.5f) {
            a[i] = b[i] * 2.0f - c[i];
        } else {
            a[i] = b[i] * 0.5f + c[i];
        }
        k += 0.001f;
    }
    
    // Loop 3: Mixed operations with different data types
    double sum = 0.0;
    #pragma omp simd simdlen(4) reduction(+:sum) aligned(a, b: 64)
    for (int i = 0; i < n; i++) {
        double temp = (double)a[i] * (double)b[i];
        sum += temp;
        // Mixed type operation
        c[i] = (float)temp * 0.5f;
    }
    
    // Loop 4: Nested SIMD context simulation
    for (int block = 0; block < 4; block++) {
        int start = block * (n / 4);
        int end = start + (n / 4);
        
        #pragma omp simd simdlen(16) linear(start:1) safelen(64)
        for (int i = start; i < end; i++) {
            // Use built-in function to prevent optimization
            c[i] = __builtin_sinf(a[i]) * __builtin_expf(b[i]);
        }
    }
}

// Function that creates SIMD/non-SIMD context switching
void compute_wrapper(float *a, float *b, float *c, int n, int mode) {
    if (mode == 0) {
        // SIMD context
        #pragma omp target teams distribute parallel for simd \
                map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
                simdlen(8) safelen(16)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    } else {
        // Non-SIMD context but with SIMD loops inside
        compute(a, b, c, n, mode);
    }
}

int main() {
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c = (float*)aligned_alloc(64, N * sizeof(float));
    
    // Initialize with pseudo-random pattern
    for (int i = 0; i < N; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        b[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        c[i] = 0.0f;
    }
    
    // Multiple iterations to create runtime context
    for (int iter = 0; iter < ITERATIONS; iter++) {
        // Alternate between SIMD and non-SIMD contexts
        int mode = iter % 2;
        compute_wrapper(a, b, c, N, mode);
        
        // Additional direct SIMD computation
        compute(a, b, c, N, iter);
    }
    
    // Verification checksum
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
    for (int i = 0; i < N; i++) {
        checksum += (double)c[i];
    }
    
    printf("Checksum: %.6f\n", checksum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    
    return 0;
}
