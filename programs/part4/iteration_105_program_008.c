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

/* Function containing multiple SIMD loops with various clauses */
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + iter * 0.1f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with linear clause and conditional */
    #pragma omp simd linear(k:1) reduction(+:k) safelen(32)
    for (int i = 0; i < n; i++) {
        if (c[i] > 0.0f) {
            d[i] = c[i] * 0.5f;
        } else {
            d[i] = c[i] * 2.0f;
        }
        k += 0.001f;  /* linear increment */
    }
    
    /* Loop 3: Mixed operations with different data types */
    #pragma omp simd simdlen(4) aligned(d: 32)
    for (int i = 0; i < n; i++) {
        /* Mixed operations to prevent trivial optimization */
        float temp = d[i];
        d[i] = temp * temp - 1.0f;
        if (i % 2 == 0) {
            d[i] += 0.25f;
        }
    }
    
    /* Loop 4: Nested SIMD context simulation */
    for (int block = 0; block < 4; block++) {
        int start = block * (n / 4);
        int end = (block + 1) * (n / 4);
        
        #pragma omp simd safelen(8) simdlen(4) linear(start:1)
        for (int i = start; i < end; i++) {
            a[i] = b[i] + d[i] * k;
        }
    }
}

/* Function with target offload to trigger SIMT in different context */
#ifdef USE_TARGET
void compute_target(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * sinf((float)i * 0.01f);
    }
}
#endif

int main() {
    /* Allocate aligned memory for better vectorization */
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c = (float*)aligned_alloc(64, N * sizeof(float));
    float *d = (float*)aligned_alloc(64, N * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 100.0f;
        b[i] = (float)(lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    /* Multiple iterations to create runtime context */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, N, iter);
        
        /* Alternate between different loop patterns */
        if (iter % 2 == 0) {
            #pragma omp simd simdlen(16) reduction(+:a[0:N])
            for (int i = 0; i < N; i++) {
                a[i] = a[i] * 0.99f + 0.01f * b[i];
            }
        }
    }
    
#ifdef USE_TARGET
    /* Optional target offload section */
    compute_target(a, b, c, N);
#endif
    
    /* Final verification checksum */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) safelen(8)
    for (int i = 0; i < N; i++) {
        checksum += (double)c[i] + (double)d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
