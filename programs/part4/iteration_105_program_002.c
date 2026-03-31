/* Test program to trigger SIMT transformation in omp-low.cc */
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
    float k = (float)iter * 0.1f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with linear clause and conditional */
    #pragma omp simd linear(k:1) safelen(8) simdlen(4)
    for (int i = 0; i < n; i++) {
        if (c[i] > 0.5f) {
            d[i] = c[i] * 0.9f + k;
        } else {
            d[i] = c[i] * 1.1f - k;
        }
        k += 0.001f;
    }
    
    /* Loop 3: SIMD reduction with mixed operations */
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum) aligned(a, d: 64)
    for (int i = 0; i < n; i++) {
        sum += a[i] * d[i];
        /* Use built-in function to encourage vectorization */
        d[i] = __builtin_sinf(d[i] * 0.01f);
    }
    
    /* Loop 4: Nested context - outer loop with inner SIMD */
    for (int j = 0; j < 4; j++) {
        float scale = 1.0f + j * 0.25f;
        #pragma omp simd simdlen(8) linear(scale:0.1)
        for (int i = 0; i < n; i++) {
            c[i] = c[i] * scale + d[i];
            scale += 0.0001f;
        }
    }
    
    /* Loop 5: SIMD with different data types */
    double dsum = 0.0;
    #pragma omp simd reduction(+:dsum) simdlen(4)
    for (int i = 0; i < n; i++) {
        double val = (double)c[i] * 1.5;
        dsum += val;
        /* Conditional store with different type */
        if (i % 2 == 0) {
            d[i] = (float)val;
        }
    }
}

/* Function called from both SIMD and non-SIMD contexts */
void process_chunk(float *a, float *b, float *c, float *d, int start, int end, int mode) {
    if (mode == 0) {
        /* Non-SIMD path */
        for (int i = start; i < end; i++) {
            c[i] = a[i] + b[i];
        }
    } else {
        /* SIMD path - this creates context for conditional SIMD execution */
        #pragma omp simd safelen(8) simdlen(4)
        for (int i = start; i < end; i++) {
            c[i] = a[i] * b[i] - d[i];
        }
    }
}

int main() {
    /* Allocate aligned memory for better vectorization */
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
        a[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        b[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        c[i] = 0.0f;
        d[i] = (float)(lcg_rand() % 1000) / 1000.0f;
    }
    
    /* Multiple iterations to create runtime context */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call compute function with SIMD loops */
        compute(a, b, c, d, SIZE, iter);
        
        /* Alternate between SIMD and non-SIMD contexts */
        for (int chunk = 0; chunk < 4; chunk++) {
            int start = chunk * (SIZE / 4);
            int end = (chunk + 1) * (SIZE / 4);
            process_chunk(a, b, c, d, start, end, iter % 2);
        }
        
        /* Additional SIMD loop with complex clauses */
        float accum = 0.0f;
        #pragma omp simd reduction(+:accum) linear(accum:0.01) \
                    aligned(a, c: 64) safelen(16) simdlen(8)
        for (int i = 0; i < SIZE; i++) {
            accum += a[i] * c[i];
            c[i] = c[i] * 0.95f + accum * 0.05f;
        }
    }
    
    /* Calculate checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(4)
    for (int i = 0; i < SIZE; i++) {
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
