#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ITERATIONS 10

/* Simple LCG to avoid library calls */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function with multiple SIMD loops - target for SIMT transformation */
void compute(float *a, float *b, float *c, int n, int offset) {
    float k = 1.5f + offset * 0.1f;
    float sum = 0.0f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with linear clause and conditional */
    #pragma omp simd linear(k:1) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        if (c[i] > 0.0f) {
            sum += c[i];
        } else {
            sum -= c[i];
        }
        k += 0.001f;  /* Linear increment */
    }
    
    /* Loop 3: SIMD with mixed operations */
    #pragma omp simd simdlen(4) aligned(a, b: 32)
    for (int i = 0; i < n; i++) {
        /* Mixed operations to prevent trivial optimization */
        float temp = a[i] * 2.0f;
        b[i] = temp - (c[i] / (k + 1.0f));
        a[i] = b[i] * 0.5f + temp * 0.5f;
    }
    
    /* Nested loop structure */
    for (int outer = 0; outer < 4; outer++) {
        float scale = 1.0f + outer * 0.25f;
        
        /* Loop 4: SIMD inside outer loop */
        #pragma omp simd safelen(8) linear(scale:0.1)
        for (int i = 0; i < n/2; i++) {
            int idx = i * 2;
            c[idx] = a[idx] * scale;
            c[idx + 1] = b[idx + 1] / scale;
            scale += 0.1f;  /* Linear increment */
        }
    }
    
    /* Loop 5: Double precision SIMD with different simdlen */
    double *d = (double*)malloc(n * sizeof(double));
    if (d) {
        #pragma omp simd simdlen(4) aligned(d: 64)
        for (int i = 0; i < n; i++) {
            d[i] = (double)a[i] * 2.5 + (double)b[i] * 1.5;
        }
        
        /* Loop 6: SIMD with function-like builtin */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            /* Use __builtin_fabsf to simulate builtin call */
            c[i] = __builtin_fabsf(a[i] - b[i]) * d[i];
        }
        
        free(d);
    }
}

/* Another function that calls compute, creating different contexts */
void process_chunks(float *a, float *b, float *c, int total, int chunk_size) {
    for (int start = 0; start < total; start += chunk_size) {
        int end = start + chunk_size;
        if (end > total) end = total;
        
        /* Target construct that may trigger SIMT transformation */
        #pragma omp target teams distribute parallel for simd \
                map(to: a[start:end-start], b[start:end-start]) \
                map(from: c[start:end-start])
        for (int i = start; i < end; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
        
        /* Call compute function on the chunk */
        compute(&a[start], &b[start], &c[start], end - start, start);
    }
}

int main() {
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c = (float*)aligned_alloc(64, N * sizeof(float));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 100.0f;
        b[i] = (float)(lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
    }
    
    /* Multiple calls to create different compilation contexts */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call with different parameters */
        compute(a, b, c, N, iter);
        
        /* Process in chunks */
        process_chunks(a, b, c, N, 256);
        
        /* Modify data slightly */
        #pragma omp simd
        for (int i = 0; i < N; i++) {
            a[i] += 0.01f;
            b[i] -= 0.005f;
        }
    }
    
    /* Final verification checksum */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < N; i++) {
        checksum += (double)c[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
