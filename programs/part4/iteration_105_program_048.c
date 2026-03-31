#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ITERATIONS 10

/* Simple LCG to generate pseudo-random data without library calls */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function containing multiple SIMD loops with various clauses */
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + iter * 0.1f;
    float sum = 0.0f;
    
    /* Loop 1: Basic SIMD with simdlen and safelen */
    #pragma omp simd simdlen(8) safelen(16) aligned(a, b, c: 64) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with reduction and conditional */
    #pragma omp simd simdlen(4) reduction(+:sum) aligned(c, d: 32)
    for (int i = 0; i < n; i++) {
        if (c[i] > 0.5f) {
            d[i] = c[i] * 0.8f;
            sum += d[i];
        } else {
            d[i] = c[i] * 1.2f;
        }
    }
    
    /* Loop 3: Nested context - outer loop with inner SIMD */
    for (int j = 0; j < 4; j++) {
        float scale = 1.0f + j * 0.25f;
        
        #pragma omp simd simdlen(16) linear(i:1) aligned(d: 64)
        for (int i = 0; i < n; i++) {
            d[i] = d[i] * scale + (float)(i % 8);
        }
    }
    
    /* Loop 4: Mixed data types within SIMD region */
    #pragma omp simd simdlen(8)
    for (int i = 0; i < n; i++) {
        int idx = i;
        float temp = d[i];
        /* Mixed operations to prevent simple optimization */
        d[i] = temp * (float)(idx % 16) - (float)(iter);
    }
    
    /* Loop 5: SIMD with function calls to builtins */
    #pragma omp simd simdlen(4) safelen(8)
    for (int i = 0; i < n; i += 2) {
        /* Using __builtin_fabsf to avoid external library dependency */
        c[i] = __builtin_fabsf(d[i]) + __builtin_sqrtf(__builtin_fabsf(b[i]));
        if (i + 1 < n) {
            c[i + 1] = __builtin_fabsf(d[i + 1]) - __builtin_sqrtf(__builtin_fabsf(b[i + 1]));
        }
    }
}

/* Another function to create calling context */
void process_chunk(float *a, float *b, float *c, float *d, int start, int end, int iter) {
    /* SIMD loop with different clauses */
    #pragma omp simd simdlen(8) linear(i:1) aligned(a, b, c, d: 64)
    for (int i = start; i < end; i++) {
        a[i] = (float)(i % 256) * 0.01f;
        b[i] = (float)((i + iter) % 128) * 0.02f;
    }
    
    compute(a + start, b + start, c + start, d + start, end - start, iter);
}

int main(void) {
    /* Allocate aligned memory for better SIMD performance */
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c = (float*)aligned_alloc(64, N * sizeof(float));
    float *d = (float*)aligned_alloc(64, N * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        b[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    /* Multiple iterations to create runtime context */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Process in chunks to create different calling patterns */
        for (int chunk = 0; chunk < N; chunk += 256) {
            int end = chunk + 256;
            if (end > N) end = N;
            process_chunk(a, b, c, d, chunk, end, iter);
        }
        
        /* Additional direct SIMD loop in main */
        #pragma omp simd simdlen(16) reduction(+:a[0:N]) aligned(a: 64)
        for (int i = 0; i < N; i++) {
            a[i] = a[i] * 0.99f + (float)iter * 0.001f;
        }
    }
    
    /* Final verification checksum */
    double checksum = 0.0;
    #pragma omp simd simdlen(8) reduction(+:checksum) aligned(c, d: 64)
    for (int i = 0; i < N; i++) {
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
