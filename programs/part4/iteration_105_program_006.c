/* simt_coverage.c - Test program for GCC SIMT transformation coverage */
#include <stdio.h>
#include <stdlib.h>

/* Simple PRNG to avoid library calls */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(float *a, float *b, float *c, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 100.0f;
        b[i] = (float)(lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
    }
}

/* Complex computation with multiple SIMD loops */
__attribute__((noinline))
static void compute(float *a, float *b, float *c, float *d, 
                    int n, int iter, float k) {
    float sum = 0.0f;
    
    /* Loop 1: SIMD with safelen and simdlen clauses */
    #pragma omp simd safelen(16) simdlen(8) aligned(a,b,c:64) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with reduction and conditional */
    #pragma omp simd reduction(+:sum) aligned(d:64)
    for (int i = 0; i < n; i++) {
        float temp = c[i] * 2.0f;
        if (temp > 10.0f) {
            d[i] = temp - 5.0f;
        } else {
            d[i] = temp + 5.0f;
        }
        sum += d[i];
    }
    
    /* Loop 3: Nested loops with outer SIMD */
    for (int j = 0; j < iter; j++) {
        float factor = (float)j * 0.1f;
        
        #pragma omp simd simdlen(4) linear(i:1)
        for (int i = 0; i < n; i++) {
            a[i] = b[i] * factor + c[i];
        }
    }
    
    /* Loop 4: Mixed data types in SIMD context */
    double dsum = 0.0;
    #pragma omp simd reduction(+:dsum) simdlen(4)
    for (int i = 0; i < n; i++) {
        double val = (double)a[i] * 0.5;
        dsum += val;
        c[i] = (float)val;
    }
    
    /* Loop 5: SIMD with function calls to builtins */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Use builtin functions that might be vectorized */
        c[i] = __builtin_sinf(c[i]) + __builtin_expf(b[i]);
    }
}

/* Wrapper function to create calling context */
__attribute__((noinline))
static void compute_wrapper(float *a, float *b, float *c, float *d,
                           int n, int outer_iters) {
    for (int iter = 0; iter < outer_iters; iter++) {
        float k = (float)iter * 0.01f;
        compute(a, b, c, d, n, 3, k);
        
        /* Additional SIMD loop in wrapper context */
        #pragma omp simd simdlen(8) aligned(a,b:64)
        for (int i = 0; i < n; i++) {
            a[i] = a[i] + b[i] * 0.5f;
        }
    }
}

/* Target offload version for additional SIMT context */
#ifdef USE_TARGET
__attribute__((noinline))
static void target_compute(float *a, float *b, int n) {
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:n], b[0:n]) map(from: a[0:n]) \
                simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        a[i] = a[i] * b[i] + __builtin_sinf((float)i * 0.01f);
    }
}
#endif

int main(void) {
    const int N = 1024;
    const int OUTER_ITERS = 10;
    
    /* Allocate with alignment for aligned clauses */
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c = (float*)aligned_alloc(64, N * sizeof(float));
    float *d = (float*)aligned_alloc(64, N * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_arrays(a, b, c, N);
    
    /* Main computation with SIMD loops */
    compute_wrapper(a, b, c, d, N, OUTER_ITERS);
    
#ifdef USE_TARGET
    /* Optional target offload computation */
    target_compute(a, b, N);
#endif
    
    /* Verification checksum */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < N; i++) {
        checksum += (double)a[i] + (double)b[i] + (double)c[i] + (double)d[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    
    return 0;
}
