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

/* Initialize arrays with pseudo-random data */
void init_arrays(float *a, float *b, float *c, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 100.0f;
        b[i] = (float)(lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
    }
}

/* Function containing multiple SIMD loops with various clauses */
void compute(float *a, float *b, float *c, int n, float k) {
    float sum1 = 0.0f, sum2 = 0.0f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with linear clause and conditional */
    #pragma omp simd linear(i:1) reduction(+:sum1)
    for (int i = 0; i < n; i++) {
        float temp = a[i] + b[i];
        if (temp > 10.0f) {
            sum1 += temp;
            c[i] = c[i] * 0.5f;
        }
    }
    
    /* Loop 3: SIMD with mixed operations */
    #pragma omp simd simdlen(4) aligned(a, b: 32)
    for (int i = 0; i < n; i++) {
        /* Use builtin-like operations */
        c[i] = c[i] * c[i] - a[i] * b[i];
    }
    
    /* Loop 4: Nested SIMD context */
    for (int block = 0; block < 4; block++) {
        int start = block * (n / 4);
        int end = start + (n / 4);
        
        #pragma omp simd safelen(8) reduction(+:sum2)
        for (int i = start; i < end; i++) {
            sum2 += c[i];
            /* Conditional store */
            if (c[i] < 0.0f) {
                c[i] = -c[i];
            }
        }
    }
    
    /* Loop 5: Double precision SIMD */
    double dsum = 0.0;
    #pragma omp simd simdlen(4) reduction(+:dsum)
    for (int i = 0; i < n; i++) {
        double dval = (double)c[i];
        dsum += dval * dval;
    }
    
    /* Prevent dead code elimination */
    volatile float dummy = sum1 + sum2 + (float)dsum;
    (void)dummy;
}

/* Function with target offload to create SIMT context */
#ifdef _OPENMP
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
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_arrays(a, b, c, SIZE);
    
    /* Multiple calls to create runtime context */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        float k = (float)iter * 0.1f;
        
        /* Call compute multiple times with different parameters */
        compute(a, b, c, SIZE, k);
        
        #ifdef _OPENMP
        /* Alternate with target offload version */
        if (iter % 2 == 0) {
            compute_target(a, b, c, SIZE);
        }
        #endif
        
        /* Modify input slightly each iteration */
        #pragma omp simd
        for (int i = 0; i < SIZE; i++) {
            a[i] += 0.01f;
        }
    }
    
    /* Final verification checksum */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < SIZE; i++) {
        checksum += (double)c[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
