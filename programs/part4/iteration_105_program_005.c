#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 512

/* Simple LCG for deterministic pseudo-random values */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function containing multiple SIMD loops with various clauses */
void compute(float *a, float *b, float *c, int n, int m) {
    float k = 2.5f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with linear clause and conditional */
    #pragma omp simd linear(k:1) simdlen(4)
    for (int i = 0; i < n; i++) {
        if (a[i] > 0.5f) {
            c[i] = b[i] / (a[i] + k);
        } else {
            c[i] = a[i] * b[i] - k;
        }
        k += 0.001f;  /* Modified by linear clause */
    }
    
    /* Loop 3: Nested loops - outer regular, inner SIMD */
    for (int j = 0; j < m; j++) {
        float sum = 0.0f;
        /* Inner SIMD loop with reduction */
        #pragma omp simd reduction(+:sum) simdlen(8)
        for (int i = 0; i < n; i++) {
            sum += a[i] * c[i] * (j + 1);
        }
        b[j] = sum / n;
    }
    
    /* Loop 4: Mixed data types in SIMD context */
    double d[N];
    #pragma omp simd simdlen(4)  /* 4 for double, 8 for float */
    for (int i = 0; i < n; i++) {
        d[i] = (double)a[i] * 1.5 + (double)b[i % m];
    }
    
    /* Loop 5: SIMD with function calls to builtins */
    #pragma omp simd safelen(32)
    for (int i = 0; i < n; i++) {
        /* Use builtin functions that may have SIMD versions */
        c[i] = __builtin_sinf(a[i]) * __builtin_expf(-b[i % m]);
    }
}

/* Another function that calls compute, creating different contexts */
void compute_wrapper(float *a, float *b, float *c, int n, int m, int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        compute(a, b, c, n, m);
        
        /* Additional SIMD loop in wrapper */
        #pragma omp simd simdlen(16)
        for (int i = 0; i < n; i++) {
            a[i] = c[i] * 0.9f + a[i] * 0.1f;
        }
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

int main() {
    /* Allocate aligned memory for better SIMD performance */
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c = (float*)aligned_alloc(64, N * sizeof(float));
    float *d = (float*)aligned_alloc(64, M * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < N; i++) {
        a[i] = (float)lcg_rand() / (float)UINT_MAX;
        b[i] = (float)lcg_rand() / (float)UINT_MAX;
        c[i] = 0.0f;
    }
    for (int j = 0; j < M; j++) {
        d[j] = (float)lcg_rand() / (float)UINT_MAX;
    }
    
    /* Call compute multiple times to create runtime context */
    for (int outer = 0; outer < 10; outer++) {
        compute_wrapper(a, b, c, N, M, 5);
        
        /* Mix in some non-SIMD loops to create context switches */
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 0.5f;  /* Non-SIMD loop */
        }
        
        #ifdef USE_OFFLOAD
        if (outer % 3 == 0) {
            compute_target(a, b, c, N);
        }
        #endif
    }
    
    /* Final verification: compute checksum */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
    for (int i = 0; i < N; i++) {
        checksum += (double)c[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
