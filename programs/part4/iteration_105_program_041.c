#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ITERATIONS 10

// Simple LCG for reproducible pseudo-random values
static unsigned int seed = 123456789;
static inline float lcg_rand_float() {
    seed = seed * 1103515245 + 12345;
    return (float)(seed % 1000) / 1000.0f;
}

// Function containing multiple SIMD loops with various clauses
__attribute__((noinline))
void compute(float *a, float *b, float *c, int n, float k) {
    float sum1 = 0.0f, sum2 = 0.0f;
    int i;
    
    // Loop 1: Basic SIMD with safelen and simdlen
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64) linear(i:1)
    for (i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    // Loop 2: SIMD with reduction and conditional
    #pragma omp simd reduction(+:sum1) simdlen(4) aligned(a, b: 32)
    for (i = 0; i < n; i++) {
        float temp = a[i] + b[i];
        if (temp > 1.0f) {
            sum1 += temp;
        }
    }
    
    // Loop 3: Nested SIMD context - outer loop
    for (int j = 0; j < 4; j++) {
        float scale = 1.0f + j * 0.1f;
        
        // Inner SIMD loop with linear clause
        #pragma omp simd linear(scale:0.1) simdlen(8) aligned(c: 64)
        for (i = 0; i < n; i++) {
            c[i] = c[i] * scale + b[i];
        }
    }
    
    // Loop 4: SIMD with mixed operations
    #pragma omp simd reduction(+:sum2) safelen(32) aligned(a, c: 64)
    for (i = 0; i < n; i++) {
        // Use built-in function to prevent optimization
        c[i] = __builtin_sinf(a[i]) + __builtin_expf(b[i]);
        sum2 += c[i];
    }
    
    // Prevent dead code elimination
    volatile float dummy = sum1 + sum2;
    (void)dummy;
}

// Another function with different SIMD characteristics
__attribute__((noinline))
void compute_double(double *a, double *b, double *c, int n) {
    double sum = 0.0;
    
    // Loop with different simdlen for doubles
    #pragma omp simd simdlen(4) reduction(+:sum) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        sum += c[i];
    }
    
    volatile double dummy = sum;
    (void)dummy;
}

// Function that could be called from target region
__attribute__((noinline))
void target_compatible_compute(float *a, float *b, float *c, int n) {
    #pragma omp simd simdlen(16) safelen(32) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        // Complex enough to require vectorization
        c[i] = a[i] * a[i] + b[i] * b[i] + 2.0f * a[i] * b[i];
    }
}

int main() {
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c = (float*)aligned_alloc(64, N * sizeof(float));
    
    double *da = (double*)aligned_alloc(64, N * sizeof(double));
    double *db = (double*)aligned_alloc(64, N * sizeof(double));
    double *dc = (double*)aligned_alloc(64, N * sizeof(double));
    
    // Initialize with pseudo-random data
    for (int i = 0; i < N; i++) {
        a[i] = lcg_rand_float();
        b[i] = lcg_rand_float();
        da[i] = (double)lcg_rand_float();
        db[i] = (double)lcg_rand_float();
    }
    
    // Multiple calls to create different contexts
    for (int iter = 0; iter < ITERATIONS; iter++) {
        float k = 1.0f + iter * 0.1f;
        
        // Call compute function multiple times
        compute(a, b, c, N, k);
        
        // Call double version
        compute_double(da, db, dc, N);
        
        // Call target-compatible version
        target_compatible_compute(a, b, c, N);
    }
    
    // Optional: Use target pragma for offload context
    #ifdef USE_OFFLOAD
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
        simdlen(8) safelen(16)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
    #endif
    
    // Calculate checksum to prevent optimization
    float checksum = 0.0f;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < N; i++) {
        checksum += c[i] + (float)dc[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(da);
    free(db);
    free(dc);
    
    return 0;
}
