#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ITERATIONS 10

/* Simple LCG to avoid library calls */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function with multiple SIMD loops - target for SIMT transformation */
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + iter * 0.1f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with linear clause and conditional */
    #pragma omp simd linear(k:1) reduction(+:k)
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            d[i] = c[i] * 2.0f - b[i];
        } else {
            d[i] = c[i] * 0.5f + a[i];
        }
        k += 0.001f;
    }
    
    /* Loop 3: Mixed operations with different data types */
    #pragma omp simd simdlen(4) aligned(d: 32)
    for (int i = 0; i < n; i++) {
        /* Use built-in function to encourage vectorization */
        float temp = __builtin_sinf(d[i] * 0.01f);
        c[i] = temp * temp + d[i];
    }
    
    /* Loop 4: Nested SIMD context simulation */
    for (int outer = 0; outer < 4; outer++) {
        float scale = 1.0f + outer * 0.25f;
        #pragma omp simd safelen(8) linear(scale:0.1)
        for (int i = 0; i < n/4; i++) {
            int idx = outer * (n/4) + i;
            b[idx] = a[idx] * scale + c[idx];
            scale += 0.001f;
        }
    }
}

/* Another function to create calling context */
void process_chunk(float *a, float *b, float *c, float *d, int start, int end) {
    /* SIMD loop with multiple clauses */
    #pragma omp simd simdlen(16) aligned(a, b: 64) linear(start:1)
    for (int i = start; i < end; i++) {
        float t = __builtin_expf(a[i] * 0.01f);
        b[i] = t * c[i] - d[i];
        start += 1; /* Use of linear variable */
    }
}

int main() {
    /* Allocate aligned memory for better vectorization */
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c = (float*)aligned_alloc(64, N * sizeof(float));
    float *d = (float*)aligned_alloc(64, N * sizeof(float));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        a[i] = (lcg_rand() % 1000) / 100.0f;
        b[i] = (lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    /* Multiple iterations to create runtime context */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, N, iter);
        
        /* Process in chunks to create different calling contexts */
        for (int chunk = 0; chunk < 4; chunk++) {
            int start = chunk * (N/4);
            int end = start + (N/4);
            process_chunk(a, b, c, d, start, end);
        }
    }
    
    /* Calculate checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
    for (int i = 0; i < N; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Free allocated memory */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
