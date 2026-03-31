#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 10

/* Simple LCG to generate pseudo-random data without library calls */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function containing multiple SIMD loops with various clauses */
void compute(float *a, float *b, float *c, int n, int iter) {
    float k = 1.5f + iter * 0.1f;
    float sum = 0.0f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with linear clause and conditional */
    #pragma omp simd linear(i:1) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        if (c[i] > 0.0f) {
            sum += c[i];
        } else {
            sum -= c[i];
        }
    }
    
    /* Loop 3: SIMD with mixed operations */
    #pragma omp simd simdlen(4) aligned(a, b: 32)
    for (int i = 0; i < n; i++) {
        /* Mixed operations to prevent trivial optimization */
        float temp = a[i] * 2.0f;
        b[i] = temp - c[i];
        a[i] = b[i] + k;
    }
    
    /* Loop 4: Nested context - outer loop with inner SIMD */
    for (int j = 0; j < 4; j++) {
        float scale = 1.0f + j * 0.25f;
        #pragma omp simd safelen(8) linear(scale:0.25)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] * scale + b[i];
        }
    }
    
    /* Loop 5: SIMD with different data type */
    double dsum = 0.0;
    #pragma omp simd simdlen(4) reduction(+:dsum)
    for (int i = 0; i < n; i++) {
        dsum += (double)c[i] * 0.5;
    }
    
    /* Prevent dead code elimination */
    volatile float dummy = sum + (float)dsum;
    (void)dummy;
}

/* Another function to create calling context */
void compute_wrapper(float *a, float *b, float *c, int n) {
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, n, iter);
        
        /* Additional SIMD loop in wrapper */
        #pragma omp simd simdlen(16) aligned(a, c: 64)
        for (int i = 0; i < n; i += 2) {
            a[i] = c[i] * 0.8f;
            if (i + 1 < n) {
                a[i + 1] = c[i + 1] * 1.2f;
            }
        }
    }
}

/* Target offload version for additional SIMT context */
#pragma omp declare target
void target_compute(float *a, float *b, float *c, int n) {
    #pragma omp teams distribute parallel for simd simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
    
    #pragma omp simd simdlen(4)
    for (int i = 0; i < n; i++) {
        a[i] = c[i] * 0.5f;
    }
}
#pragma omp end declare target

int main(void) {
    /* Allocate aligned memory for better vectorization */
    float *a = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 100.0f;
        b[i] = (float)(lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
    }
    
    /* Call compute function multiple times */
    compute_wrapper(a, b, c, SIZE);
    
    /* Optional target offload section */
    #ifdef USE_OFFLOAD
    #pragma omp target teams distribute parallel for simd map(tofrom: a[0:SIZE], b[0:SIZE], c[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        c[i] = a[i] * b[i];
    }
    
    #pragma omp target map(tofrom: a[0:SIZE], b[0:SIZE], c[0:SIZE])
    {
        target_compute(a, b, c, SIZE);
    }
    #endif
    
    /* Calculate checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
    for (int i = 0; i < SIZE; i++) {
        checksum += (double)a[i] + (double)b[i] + (double)c[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
