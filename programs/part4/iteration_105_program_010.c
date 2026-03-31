#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ITERATIONS 10

/* Simple LCG for deterministic pseudo-random values */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function containing multiple SIMD loops with various clauses */
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + iter * 0.1f;
    float sum = 0.0f;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with linear clause and conditional */
    #pragma omp simd linear(k:1) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        float temp = a[i] * 2.0f + b[i];
        if (temp > 0.0f) {
            d[i] = temp * k;
            sum += d[i];
        } else {
            d[i] = 0.0f;
        }
        k += 0.001f;  /* Modified by linear clause */
    }
    
    /* Loop 3: Mixed data types and different simdlen */
    #pragma omp simd simdlen(4) aligned(a, c: 32)
    for (int i = 0; i < n; i++) {
        /* Mixed operations encouraging SIMT transformation */
        double tmp_double = (double)a[i] * 0.5;
        c[i] = (float)tmp_double + b[i] * 0.3f;
    }
    
    /* Loop 4: Nested SIMD context simulation */
    for (int block = 0; block < 4; block++) {
        int start = block * (n / 4);
        int end = start + (n / 4);
        
        #pragma omp simd safelen(8) linear(start:1)
        for (int i = start; i < end; i++) {
            /* Complex enough to not be optimized away */
            a[i] = b[i] * c[i] - d[i] + sinf((float)i * 0.01f);
        }
    }
    
    /* Prevent dead code elimination */
    volatile float sink = sum;
    (void)sink;
}

/* Function called from both SIMD and non-SIMD contexts */
void process_chunk(float *a, float *b, float *c, float *d, int start, int end, int mode) {
    if (mode == 0) {
        /* SIMD context */
        #pragma omp simd
        for (int i = start; i < end; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    } else {
        /* Non-SIMD context - scalar fallback */
        for (int i = start; i < end; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    /* Another SIMD loop with reduction */
    float local_sum = 0.0f;
    #pragma omp simd reduction(+:local_sum)
    for (int i = start; i < end; i++) {
        d[i] = c[i] * 0.5f;
        local_sum += d[i];
    }
    
    volatile float sink = local_sum;
    (void)sink;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    float *a = (float*)aligned_alloc(64, N * sizeof(float));
    float *b = (float*)aligned_alloc(64, N * sizeof(float));
    float *c = (float*)aligned_alloc(64, N * sizeof(float));
    float *d = (float*)aligned_alloc(64, N * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random pattern */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 100.0f;
        b[i] = (float)(lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    /* Multiple calls to create different contexts */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, N, iter);
        
        /* Call from different contexts to encourage IFN_GOMP_USE_SIMT */
        for (int mode = 0; mode < 2; mode++) {
            for (int chunk = 0; chunk < 4; chunk++) {
                int start = chunk * (N / 4);
                int end = start + (N / 4);
                process_chunk(a, b, c, d, start, end, mode);
            }
        }
        
        /* Target construct for offload context */
        #ifdef _OPENMP
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
            if(iter % 2 == 0)  /* Conditional offload */
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i] + (float)iter;
        }
        #endif
    }
    
    /* Final verification checksum */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < N; i++) {
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
