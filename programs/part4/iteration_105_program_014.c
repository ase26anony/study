#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
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
    float sum1 = 0.0f, sum2 = 0.0f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with reduction and conditional */
    #pragma omp simd reduction(+:sum1) simdlen(4) aligned(c, d: 32)
    for (int i = 0; i < n; i++) {
        float temp = c[i] * 2.0f;
        if (temp > 10.0f) {
            d[i] = temp - 5.0f;
        } else {
            d[i] = temp + 5.0f;
        }
        sum1 += d[i];
    }
    
    /* Loop 3: SIMD with mixed operations and linear clause */
    #pragma omp simd linear(ref:1) simdlen(8) aligned(a, d: 64)
    for (int i = 0; i < n; i++) {
        float x = a[i];
        float y = d[i];
        /* Use built-in functions to prevent optimization */
        a[i] = __builtin_sinf(x) * __builtin_cosf(y);
        d[i] = __builtin_expf(x * 0.01f);
    }
    
    /* Loop 4: Nested SIMD context */
    for (int block = 0; block < 4; block++) {
        int start = block * (n / 4);
        int end = (block + 1) * (n / 4);
        
        #pragma omp simd reduction(+:sum2) safelen(8) simdlen(4) \
                aligned(b, c: 32) linear(i:1)
        for (int i = start; i < end; i++) {
            b[i] = b[i] + c[i] * 0.5f;
            sum2 += b[i];
        }
    }
    
    /* Loop 5: SIMD with different data type (double) */
    double *da = (double*)a;
    double *db = (double*)b;
    double dsum = 0.0;
    
    #pragma omp simd reduction(+:dsum) simdlen(4) aligned(da, db: 64)
    for (int i = 0; i < n/2; i++) {
        da[i] = db[i] * 1.5 + da[i];
        dsum += da[i];
    }
    
    /* Prevent dead code elimination */
    volatile float dummy = sum1 + sum2 + (float)dsum;
    (void)dummy;
}

/* Function that creates SIMD/non-SIMD calling context */
void process_arrays(float *a, float *b, float *c, float *d, int n) {
    /* Call compute multiple times to create different contexts */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        if (iter % 2 == 0) {
            /* Even iterations: direct call */
            compute(a, b, c, d, n, iter);
        } else {
            /* Odd iterations: call through pointer (different context) */
            void (*func_ptr)(float*, float*, float*, float*, int, int) = compute;
            func_ptr(a, b, c, d, n, iter);
        }
    }
}

int main() {
    /* Allocate aligned memory for better SIMD performance */
    float *a = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *d = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 100.0f;
        b[i] = (float)(lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    /* Process arrays with SIMD loops */
    process_arrays(a, b, c, d, SIZE);
    
    /* Additional context: target offload region */
    #ifdef _OPENMP
    #pragma omp target teams distribute parallel for simd \
            map(tofrom: a[0:SIZE], b[0:SIZE]) simdlen(8)
    for (int i = 0; i < SIZE; i++) {
        a[i] = a[i] + b[i] * 0.3f;
    }
    #endif
    
    /* Calculate checksum to verify computation */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
    for (int i = 0; i < SIZE; i++) {
        checksum += (double)a[i] + (double)b[i] + (double)c[i] + (double)d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Free allocated memory */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
