/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ITERATIONS 10

/* Simple PRNG to avoid library calls */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function with multiple SIMD loops - target for transformation */
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + iter * 0.1f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with linear clause and conditional */
    #pragma omp simd linear(k:1) simdlen(4) aligned(d: 64)
    for (int i = 0; i < n; i++) {
        if (c[i] > 0.0f) {
            d[i] = c[i] * k;
        } else {
            d[i] = c[i] / k;
        }
        k += 0.001f;  /* linear increment */
    }
    
    /* Loop 3: SIMD reduction with mixed operations */
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += a[i] * d[i] - b[i];
    }
    
    /* Loop 4: Nested SIMD-like pattern */
    for (int j = 0; j < 4; j++) {
        float scale = 1.0f + j * 0.25f;
        #pragma omp simd simdlen(16) aligned(a, c: 64)
        for (int i = 0; i < n; i += 2) {
            /* Vectorizable but non-trivial pattern */
            c[i] = a[i] * scale + __builtin_sinf(k * i);
            if (i + 1 < n) {
                c[i + 1] = a[i + 1] * scale - __builtin_cosf(k * i);
            }
        }
    }
    
    /* Loop 5: SIMD with different data type */
    double dsum = 0.0;
    #pragma omp simd reduction(+:dsum) simdlen(4) aligned(a: 64)
    for (int i = 0; i < n; i++) {
        dsum += (double)a[i] * 0.5;
    }
}

/* Wrapper that might create SIMT context */
void compute_wrapper(float *a, float *b, float *c, float *d, int n) {
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, n, iter);
        
        /* Additional SIMD loop in wrapper context */
        #pragma omp simd simdlen(8) aligned(c, d: 64)
        for (int i = 0; i < n; i++) {
            c[i] = c[i] + d[i] * 0.5f;
        }
    }
}

/* Target offload version - creates different SIMT context */
#ifdef USE_TARGET
void compute_target(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + __builtin_expf(a[i] * 0.1f);
    }
}
#endif

int main() {
    /* Allocate with alignment hint */
    float *a = (float*)__builtin_assume_aligned(malloc(N * sizeof(float)), 64);
    float *b = (float*)__builtin_assume_aligned(malloc(N * sizeof(float)), 64);
    float *c = (float*)__builtin_assume_aligned(malloc(N * sizeof(float)), 64);
    float *d = (float*)__builtin_assume_aligned(malloc(N * sizeof(float)), 64);
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        a[i] = (lcg_rand() % 1000) / 100.0f;
        b[i] = (lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    /* Call compute multiple times to create runtime context */
    for (int outer = 0; outer < 5; outer++) {
        compute_wrapper(a, b, c, d, N);
        
        /* Mix in some scalar computation between SIMD calls */
        for (int i = 0; i < N; i++) {
            a[i] = a[i] * 0.99f;  /* Prevent pattern recognition */
        }
    }
    
#ifdef USE_TARGET
    /* Optional target offload section */
    compute_target(a, b, c, N);
#endif
    
    /* Final verification checksum */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
    for (int i = 0; i < N; i++) {
        checksum += c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
