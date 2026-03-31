#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 10

/* Simple LCG to avoid library calls */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(float *a, float *b, float *c, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        b[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        c[i] = 0.0f;
    }
}

/* Complex computation function with multiple SIMD loops */
#pragma omp declare simd notinbranch
static inline float custom_func(float x, float y) {
    return x * x + y * y - 2.0f * x * y;
}

/* Main computation with various SIMD constructs */
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float sum1 = 0.0f, sum2 = 0.0f;
    float k = (float)iter * 0.1f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with reduction and conditional */
    #pragma omp simd reduction(+:sum1) simdlen(4) aligned(d: 64)
    for (int i = 0; i < n; i++) {
        float val = custom_func(a[i], b[i]);
        d[i] = val;
        if (val > 0.5f) {
            sum1 += val;
        }
    }
    
    /* Loop 3: SIMD with linear clause on reference */
    float ref = 1.0f;
    #pragma omp simd linear(ref:1) simdlen(8)
    for (int i = 0; i < n; i++) {
        c[i] = c[i] * ref;
        ref += 0.001f;
    }
    
    /* Nested loops to create complex context */
    for (int outer = 0; outer < 4; outer++) {
        float factor = (float)outer * 0.25f;
        
        /* Loop 4: SIMD inside outer loop */
        #pragma omp simd simdlen(16) aligned(a, b, d: 64)
        for (int i = 0; i < n/2; i++) {
            d[i] = a[i] * factor + b[i] * (1.0f - factor);
        }
        
        /* Loop 5: Another SIMD with mixed operations */
        #pragma omp simd reduction(+:sum2) safelen(32)
        for (int i = n/2; i < n; i++) {
            float t1 = a[i] * a[i];
            float t2 = b[i] * b[i];
            d[i] = t1 - t2;
            sum2 += (t1 + t2);
        }
    }
    
    /* Final verification value */
    c[0] = sum1 + sum2;
}

/* Target offload version for additional SIMT context */
#ifdef USE_TARGET
void compute_target(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 2.0f;
        /* Conditional store to prevent optimization */
        if (c[i] < 0.0f) {
            c[i] = -c[i];
        }
    }
}
#endif

int main(void) {
    float *a = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *d = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with different patterns */
    init_arrays(a, b, c, SIZE);
    
    /* Multiple calls to create runtime context */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, SIZE, iter);
        
        #ifdef USE_TARGET
        if (iter % 3 == 0) {
            compute_target(a, b, d, SIZE);
        }
        #endif
        
        /* Modify inputs slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            a[i] += 0.01f;
            b[i] -= 0.005f;
        }
    }
    
    /* Calculate checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
    for (int i = 0; i < SIZE; i++) {
        checksum += (double)c[i] + (double)d[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
