#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ITERATIONS 10

// Simple LCG for reproducible pseudo-random values
static unsigned int seed = 123456789;
static inline float rand_float() {
    seed = seed * 1103515245 + 12345;
    return (float)(seed % 1000) / 1000.0f;
}

// Function containing multiple SIMD loops with various clauses
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + (float)iter * 0.1f;
    float sum = 0.0f;
    
    // Loop 1: Basic SIMD with safelen and simdlen
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    // Loop 2: SIMD with linear clause and conditional
    #pragma omp simd linear(i:1) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        if (c[i] > 0.5f) {
            sum += c[i] * 2.0f;
        } else {
            sum += c[i];
        }
    }
    
    // Loop 3: Nested context - outer loop with inner SIMD
    for (int j = 0; j < 4; j++) {
        float scale = 1.0f + j * 0.25f;
        #pragma omp simd simdlen(4) aligned(d: 32)
        for (int i = 0; i < n; i++) {
            d[i] = c[i] * scale + (float)j;
        }
    }
    
    // Loop 4: Mixed operations with different data widths
    double dsum = 0.0;
    #pragma omp simd simdlen(4) reduction(+:dsum)
    for (int i = 0; i < n; i++) {
        double val = (double)c[i] * 1.5;
        dsum += val;
        if (i % 2 == 0) {
            d[i] = (float)val;
        }
    }
    
    // Loop 5: SIMD with function calls to built-ins
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        // Use built-in math functions that may have SIMD versions
        c[i] = __builtin_sinf(a[i]) + __builtin_expf(b[i] * 0.1f);
    }
    
    printf("Iter %d: sum = %.3f, dsum = %.3f\n", iter, sum, (float)dsum);
}

// Function that creates context for SIMT transformation
void process_arrays(int n) {
    float *a = (float*)aligned_alloc(64, n * sizeof(float));
    float *b = (float*)aligned_alloc(64, n * sizeof(float));
    float *c = (float*)aligned_alloc(64, n * sizeof(float));
    float *d = (float*)aligned_alloc(64, n * sizeof(float));
    
    // Initialize with patterned data
    for (int i = 0; i < n; i++) {
        a[i] = rand_float();
        b[i] = rand_float();
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Multiple calls to create runtime context
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, n, iter);
    }
    
    // Final checksum
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < n; i++) {
        checksum += (double)c[i] + (double)d[i];
    }
    
    printf("Final checksum: %.6f\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
}

// Target offload version for additional SIMT context
#ifdef USE_TARGET
void target_computation(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
}
#endif

int main() {
    printf("Starting SIMD transformation test...\n");
    
    // Process with standard SIMD
    process_arrays(N);
    
    // Additional test with different size
    process_arrays(N/2);
    
    #ifdef USE_TARGET
    // Optional target offload section
    float *ta = (float*)malloc(N * sizeof(float));
    float *tb = (float*)malloc(N * sizeof(float));
    float *tc = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        ta[i] = rand_float();
        tb[i] = rand_float();
    }
    
    target_computation(ta, tb, tc, N);
    
    float tsum = 0.0f;
    #pragma omp simd reduction(+:tsum)
    for (int i = 0; i < N; i++) {
        tsum += tc[i];
    }
    printf("Target computation sum: %.3f\n", tsum);
    
    free(ta);
    free(tb);
    free(tc);
    #endif
    
    return 0;
}
