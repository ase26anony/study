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
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + iter * 0.1f;
    float sum = 0.0f;
    
    // Loop 1: Basic SIMD with safelen and simdlen
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    // Loop 2: SIMD with linear clause and conditional
    #pragma omp simd linear(k:1) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        if (c[i] > 0.0f) {
            sum += c[i] * 0.5f;
        }
        d[i] = sum + i * 0.01f;
    }
    
    // Loop 3: Mixed operations with different data types
    #pragma omp simd simdlen(4) aligned(d: 32)
    for (int i = 0; i < n; i++) {
        // Mixed precision operations
        float temp = d[i];
        double dtemp = (double)temp * 1.25;
        d[i] = (float)dtemp + (i % 8) * 0.1f;
    }
    
    // Loop 4: Nested SIMD-like pattern
    for (int block = 0; block < 4; block++) {
        int start = block * (n / 4);
        int end = (block + 1) * (n / 4);
        
        #pragma omp simd safelen(8) linear(start:1)
        for (int i = start; i < end; i++) {
            a[i] = b[i] + c[i] - d[i];
            // Use built-in function to prevent optimization
            a[i] = __builtin_sinf(a[i] * 0.01f);
        }
    }
    
    // Loop 5: SIMD with explicit vector length for float
    #pragma omp simd simdlen(16) reduction(+:sum)
    for (int i = 0; i < n; i += 2) {
        float val1 = a[i] * 2.0f;
        float val2 = a[i + 1] * 3.0f;
        c[i] = __builtin_expf(val1);
        c[i + 1] = __builtin_expf(val2);
        sum += val1 + val2;
    }
}

// Function that creates context for SIMT transformation
void process_arrays(int n) {
    float *a = (float*)aligned_alloc(64, n * sizeof(float));
    float *b = (float*)aligned_alloc(64, n * sizeof(float));
    float *c = (float*)aligned_alloc(64, n * sizeof(float));
    float *d = (float*)aligned_alloc(64, n * sizeof(float));
    
    // Initialize with pseudo-random pattern
    for (int i = 0; i < n; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 100.0f;
        b[i] = (float)(lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    // Multiple calls to create different contexts
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, n, iter);
        
        // Additional SIMD loop in calling context
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            a[i] = a[i] * 0.99f + 0.01f * b[i];
        }
    }
    
    // Verification checksum
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < n; i++) {
        checksum += (double)a[i] + (double)b[i] + (double)c[i] + (double)d[i];
    }
    
    printf("Checksum: %.6f\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
}

// Target offload version for additional SIMT context
#ifdef USE_OFFLOAD
void compute_offload(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 2.0f;
        // Conditional to prevent simple optimization
        if (c[i] > 100.0f) {
            c[i] = 100.0f;
        }
    }
}
#endif

int main() {
    int sizes[] = {256, 512, 1024, 2048};
    
    // Process with different sizes
    for (int s = 0; s < 4; s++) {
        printf("Processing size %d:\n", sizes[s]);
        process_arrays(sizes[s]);
        
        #ifdef USE_OFFLOAD
        // Offload version if enabled
        float *a = (float*)malloc(sizes[s] * sizeof(float));
        float *b = (float*)malloc(sizes[s] * sizeof(float));
        float *c = (float*)malloc(sizes[s] * sizeof(float));
        
        for (int i = 0; i < sizes[s]; i++) {
            a[i] = (float)i;
            b[i] = (float)(sizes[s] - i);
        }
        
        compute_offload(a, b, c, sizes[s]);
        
        float sum = 0.0f;
        #pragma omp simd reduction(+:sum)
        for (int i = 0; i < sizes[s]; i++) {
            sum += c[i];
        }
        printf("Offload sum: %.2f\n", sum);
        
        free(a);
        free(b);
        free(c);
        #endif
    }
    
    return 0;
}
