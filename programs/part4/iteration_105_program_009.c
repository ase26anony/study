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

/* Function with complex SIMD loops that should trigger SIMT transformation */
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + (iter * 0.1f);
    float sum = 0.0f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with reduction and conditional */
    #pragma omp simd reduction(+:sum) simdlen(4) aligned(c, d: 64)
    for (int i = 0; i < n; i++) {
        float val = c[i] * 2.0f;
        if (val > 1.0f) {  /* Non-trivial conditional */
            d[i] = val - 1.0f;
            sum += d[i];
        } else {
            d[i] = val;
        }
    }
    
    /* Loop 3: Nested SIMD context */
    for (int block = 0; block < 4; block++) {
        int start = block * (n / 4);
        int end = (block + 1) * (n / 4);
        
        #pragma omp simd simdlen(8) linear(start:1) aligned(a, d: 64)
        for (int i = start; i < end; i++) {
            /* Mixed operations to prevent optimization */
            a[i] = d[i] * 0.5f + (float)(i % 8) * 0.125f;
        }
    }
    
    /* Loop 4: Double precision SIMD with different simdlen */
    double dp_sum = 0.0;
    #pragma omp simd reduction(+:dp_sum) simdlen(4) aligned(a, b: 64)
    for (int i = 0; i < n/2; i++) {
        double da = (double)a[i*2];
        double db = (double)b[i*2];
        dp_sum += da * db;
    }
    
    /* Prevent dead code elimination */
    volatile float sink = sum + (float)dp_sum;
    (void)sink;
}

/* Function called from both SIMD and non-SIMD contexts */
void mixed_context_computation(float *arr1, float *arr2, int n, int mode) {
    if (mode == 0) {
        /* SIMD context */
        #pragma omp simd simdlen(8) aligned(arr1, arr2: 64)
        for (int i = 0; i < n; i++) {
            arr1[i] = arr2[i] * 2.0f - arr1[i];
        }
    } else {
        /* Non-SIMD context - but still contains SIMD loop */
        #pragma omp simd safelen(32) aligned(arr1: 64)
        for (int i = 0; i < n; i++) {
            arr1[i] = arr1[i] * 0.5f + (float)i / n;
        }
    }
}

/* Target offload region for additional SIMT context */
#ifdef USE_TARGET
void target_computation(float *a, float *b, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: a[0:n]) \
        simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        a[i] = sinf(a[i]) * expf(b[i]);
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
        a[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        b[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    /* Multiple iterations to create runtime context */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, SIZE, iter);
        
        /* Call with different modes to create mixed contexts */
        mixed_context_computation(a, b, SIZE, iter % 2);
        mixed_context_computation(c, d, SIZE, (iter + 1) % 2);
        
        #ifdef USE_TARGET
        if (iter % 3 == 0) {
            target_computation(a, b, SIZE);
        }
        #endif
    }
    
    /* Final checksum to prevent optimization */
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
