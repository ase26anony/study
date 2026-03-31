/* Test program to trigger SIMT transformation in omp-low.cc (lines 2941-2975) */
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

/* Function with multiple SIMD loops to trigger transformation */
#pragma omp declare simd notinbranch
static float simd_sinf(float x) {
    /* Taylor approximation for sinf to avoid libm */
    float x2 = x * x;
    float x3 = x2 * x;
    float x5 = x3 * x2;
    return x - x3/6.0f + x5/120.0f;
}

void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + iter * 0.1f;
    float sum = 0.0f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with reduction and conditional */
    #pragma omp simd reduction(+:sum) simdlen(4) aligned(c, d: 32)
    for (int i = 0; i < n; i++) {
        float val = c[i];
        if (val > 0.5f) {
            d[i] = simd_sinf(val);
            sum += d[i];
        } else {
            d[i] = val * 0.5f;
            sum += d[i];
        }
    }
    
    /* Loop 3: Nested context - outer loop with inner SIMD */
    for (int j = 0; j < 4; j++) {
        float scale = 1.0f + j * 0.25f;
        #pragma omp simd simdlen(8) linear(i:1) aligned(d: 64)
        for (int i = 0; i < n/2; i++) {
            d[i] = d[i] * scale + (float)j;
        }
    }
    
    /* Loop 4: Mixed data types */
    int *indices = (int*)malloc(n * sizeof(int));
    #pragma omp simd simdlen(16) aligned(indices: 64)
    for (int i = 0; i < n; i++) {
        indices[i] = i * 2;
    }
    
    /* Loop 5: SIMD with linear clause on reference */
    float accum = 0.0f;
    #pragma omp simd linear(accum:1) simdlen(4)
    for (int i = 0; i < n; i++) {
        accum += 0.01f;
        a[i] += accum;
    }
    
    free(indices);
}

/* Target region to potentially trigger different SIMT context */
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
    float *a = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *d = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        b[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    /* Multiple calls to create different contexts */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, SIZE, iter);
        
        #ifdef USE_TARGET
        if (iter % 3 == 0) {
            compute_target(a, b, c, SIZE);
        }
        #endif
    }
    
    /* Verification checksum */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
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
