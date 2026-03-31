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

static inline float rand_float(void) {
    return (float)(lcg_rand() & 0xFFFF) / 65536.0f;
}

/* Function containing multiple SIMD loops with various clauses */
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + (float)iter * 0.1f;
    float sum1 = 0.0f, sum2 = 0.0f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with linear clause and conditional */
    #pragma omp simd linear(i:1) reduction(+:sum1)
    for (int i = 0; i < n; i++) {
        float temp = a[i] * k - b[i];
        if (temp > 0.0f) {
            d[i] = temp;
            sum1 += temp;
        } else {
            d[i] = -temp;
        }
    }
    
    /* Loop 3: Nested SIMD-like structure */
    for (int block = 0; block < n; block += 64) {
        int limit = (block + 64 < n) ? block + 64 : n;
        
        #pragma omp simd simdlen(4) aligned(c, d: 64) reduction(+:sum2)
        for (int i = block; i < limit; i++) {
            c[i] = c[i] * 0.5f + d[i];
            sum2 += c[i];
        }
    }
    
    /* Loop 4: Mixed data types within SIMD region */
    #pragma omp simd simdlen(8)
    for (int i = 0; i < n; i++) {
        int idx = i;
        float val = c[i];
        /* Mixed operations to prevent simple optimization */
        c[i] = (val > 1.0f) ? val * 0.8f : val * 1.2f;
        b[i] = b[i] + (float)idx * 0.01f;
    }
    
    /* Loop 5: SIMD with multiple clauses */
    #pragma omp simd safelen(32) simdlen(16) linear(k:0) aligned(a, d: 64) reduction(+:sum1)
    for (int i = 0; i < n; i += 2) {
        d[i] = a[i] * k + d[i];
        d[i+1] = a[i+1] * k * 0.5f + d[i+1];
        sum1 += d[i] + d[i+1];
    }
}

/* Secondary function to create calling context */
void process_chunk(float *a, float *b, float *c, float *d, int start, int end) {
    for (int i = start; i < end; i += 128) {
        int chunk_size = (i + 128 <= end) ? 128 : end - i;
        
        /* SIMD loop inside function call context */
        #pragma omp simd simdlen(8) safelen(16)
        for (int j = 0; j < chunk_size; j++) {
            int idx = i + j;
            a[idx] = a[idx] * 1.1f + (float)j * 0.01f;
        }
        
        compute(a + i, b + i, c + i, d + i, chunk_size, i);
    }
}

/* Target offload version for additional SIMT context */
#ifdef USE_OFFLOAD
void compute_target(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
}
#endif

int main(void) {
    /* Allocate aligned memory for better SIMD performance */
    float *a = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *d = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterned data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand_float();
        b[i] = rand_float() * 2.0f - 1.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    /* Multiple iterations to create runtime context */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call compute function directly */
        compute(a, b, c, d, SIZE, iter);
        
        /* Call through process_chunk for different context */
        process_chunk(a, b, c, d, 0, SIZE);
        
        #ifdef USE_OFFLOAD
        /* Offload version if enabled */
        compute_target(a, b, c, SIZE);
        #endif
        
        /* Modify parameters slightly each iteration */
        for (int i = 0; i < SIZE; i += 4) {
            b[i] *= 1.01f;
        }
    }
    
    /* Calculate checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
    for (int i = 0; i < SIZE; i++) {
        checksum += (double)a[i] + (double)b[i] + (double)c[i] + (double)d[i];
    }
    
    printf("Checksum: %.6f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
