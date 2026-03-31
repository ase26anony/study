#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
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
    float sum = 0.0f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with reduction and conditional */
    #pragma omp simd reduction(+:sum) simdlen(4) aligned(d: 64)
    for (int i = 0; i < n; i++) {
        float temp = c[i] * 0.5f;
        if (temp > 1.0f) {
            d[i] = temp;
            sum += temp;
        } else {
            d[i] = 0.0f;
        }
    }
    
    /* Loop 3: Nested SIMD-like pattern */
    for (int block = 0; block < 4; block++) {
        int start = block * (n / 4);
        int end = (block + 1) * (n / 4);
        
        #pragma omp simd simdlen(8) linear(start:1) aligned(a, d: 64)
        for (int i = start; i < end; i++) {
            a[i] = d[i] * 0.25f + (float)(i % 8);
        }
    }
    
    /* Loop 4: Mixed operations with different data widths */
    #pragma omp simd simdlen(16) aligned(b, c: 64)
    for (int i = 0; i < n - 1; i += 2) {
        /* Mixed-width operations */
        b[i] = c[i] * 2.0f;
        b[i + 1] = c[i + 1] * 3.0f;
    }
    
    /* Prevent dead code elimination */
    volatile float dummy = sum;
    (void)dummy;
}

/* Another function that calls SIMD loops from different contexts */
void compute_wrapper(float *a, float *b, float *c, float *d, int n) {
    for (int iter = 0; iter < 2; iter++) {
        /* This outer loop creates context for conditional SIMD execution */
        compute(a, b, c, d, n, iter);
        
        /* Additional SIMD loop in wrapper */
        #pragma omp simd safelen(32) simdlen(16) aligned(a, d: 64)
        for (int i = 0; i < n; i++) {
            d[i] = a[i] + d[i] * 0.1f;
        }
    }
}

/* Target offload version - triggers different SIMT paths */
#ifdef USE_TARGET
void compute_target(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
}
#endif

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
    
    /* Initialize with pseudo-random pattern */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 100.0f;
        b[i] = (float)(lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    /* Multiple calls to create runtime context for SIMT decisions */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        compute_wrapper(a, b, c, d, SIZE);
        
        /* Vary parameters to affect SIMD decisions */
        if (outer % 3 == 0) {
            compute(a, b, c, d, SIZE, outer);
        }
        
        #ifdef USE_TARGET
        if (outer % 5 == 0) {
            compute_target(a, b, c, SIZE);
        }
        #endif
    }
    
    /* Final verification checksum */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
    for (int i = 0; i < SIZE; i++) {
        checksum += (double)a[i] + (double)b[i] + (double)c[i] + (double)d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
