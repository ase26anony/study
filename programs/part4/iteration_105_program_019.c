/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 10

/* Simple PRNG to avoid library calls */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function with multiple SIMD loops - target for SIMT transformation */
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = (float)iter * 0.1f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with reduction and conditional */
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum) simdlen(4) aligned(d: 64)
    for (int i = 0; i < n; i++) {
        d[i] = c[i] * 2.0f - b[i];
        if (d[i] > 0.5f) {
            sum += d[i];
        }
    }
    
    /* Loop 3: Mixed operations with different data types */
    int *indices = (int*)malloc(n * sizeof(int));
    #pragma omp simd simdlen(8) linear(i:1)
    for (int i = 0; i < n; i++) {
        indices[i] = i * 2;
    }
    
    /* Loop 4: Nested SIMD context */
    for (int outer = 0; outer < 4; outer++) {
        float scale = 1.0f + outer * 0.25f;
        #pragma omp simd safelen(8) simdlen(4) aligned(a, c: 32)
        for (int i = 0; i < n/2; i++) {
            int idx = indices[i];
            if (idx < n) {
                a[idx] = c[idx] * scale + (float)outer;
            }
        }
    }
    
    free(indices);
    
    /* Loop 5: SIMD with built-in function calls */
    #pragma omp simd simdlen(4)
    for (int i = 0; i < n; i++) {
        /* Use built-in math functions that may have SIMD versions */
        c[i] = __builtin_sinf(c[i]) + __builtin_expf(b[i] * 0.1f);
    }
}

/* Another function to create calling context */
void process_chunks(float *a, float *b, float *c, float *d, int n) {
    for (int chunk = 0; chunk < 4; chunk++) {
        int start = chunk * (n / 4);
        int end = (chunk + 1) * (n / 4);
        
        /* This creates a context where SIMD loops might need SIMT transformation */
        #pragma omp target teams distribute parallel for simd if(target:0) map(tofrom: a[start:end-start], b[start:end-start]) map(from: c[start:end-start])
        for (int i = start; i < end; i++) {
            c[i] = a[i] + b[i] * 0.5f;
        }
        
        compute(a + start, b + start, c + start, d + start, end - start, chunk);
    }
}

int main(void) {
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
    
    /* Multiple iterations to increase chance of hitting the transformation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, SIZE, iter);
        
        /* Alternate between direct compute and chunked processing */
        if (iter % 2 == 0) {
            process_chunks(a, b, c, d, SIZE);
        }
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
