#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 10

/* Simple PRNG to avoid library calls */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function with multiple SIMD loops and clauses */
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float k = 1.5f + (iter * 0.1f);
    float sum1 = 0.0f, sum2 = 0.0f;
    
    /* Loop 1: Basic SIMD with simdlen and safelen */
    #pragma omp simd simdlen(8) safelen(16) aligned(a, b, c: 64) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Loop 2: SIMD with reduction and conditional */
    #pragma omp simd reduction(+:sum1) simdlen(4) aligned(a, c: 32)
    for (int i = 0; i < n; i++) {
        float temp = a[i] + c[i];
        if (temp > 0.0f) {
            sum1 += temp;
        } else {
            sum1 += temp * 0.5f;
        }
    }
    
    /* Loop 3: Mixed operations with different data types */
    #pragma omp simd simdlen(8) linear(i:1)
    for (int i = 0; i < n; i++) {
        /* Mixed operations to prevent simple optimization */
        float val = a[i];
        for (int j = 0; j < 2; j++) {  /* Small inner loop */
            val = val * 0.9f + b[i] * 0.1f;
        }
        d[i] = val;
        
        /* Conditional store */
        if (d[i] < 0.0f) {
            d[i] = -d[i];
        }
    }
    
    /* Loop 4: Nested SIMD context */
    for (int outer = 0; outer < 4; outer++) {
        float offset = outer * 0.25f;
        
        #pragma omp simd simdlen(4) reduction(+:sum2) aligned(d: 64)
        for (int i = 0; i < n/2; i++) {
            /* Use built-in function to encourage vectorization */
            d[i] = d[i] + offset;
            sum2 += d[i];
        }
    }
    
    /* Prevent dead code elimination */
    a[0] = sum1 + sum2;
}

/* Function that creates SIMD/non-SIMD context switching */
void process_data(float *a, float *b, float *c, float *d, int n, int mode) {
    if (mode == 0) {
        /* SIMD context */
        for (int iter = 0; iter < 4; iter++) {
            compute(a, b, c, d, n, iter);
        }
    } else {
        /* Non-SIMD context with manual loop */
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
        compute(a, b, c, d, n, 1);
    }
}

int main() {
    /* Allocate aligned memory */
    float *a = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *d = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (lcg_rand() % 1000) / 100.0f;
        b[i] = (lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    /* Create context for SIMT transformation */
    for (int mode = 0; mode < 2; mode++) {
        for (int repeat = 0; repeat < ITERATIONS; repeat++) {
            process_data(a, b, c, d, SIZE, mode);
            
            /* Additional SIMD loop in main */
            float checksum = 0.0f;
            #pragma omp simd reduction(+:checksum) simdlen(8) aligned(a, b, c, d: 64)
            for (int i = 0; i < SIZE; i++) {
                checksum += a[i] + b[i] + c[i] + d[i];
            }
            
            /* Use result to prevent optimization */
            if (repeat % 100 == 0) {
                a[0] = checksum * 0.0001f;
            }
        }
    }
    
    /* Final verification */
    double total = 0.0;
    #pragma omp simd reduction(+:total) simdlen(4)
    for (int i = 0; i < SIZE; i++) {
        total += (double)c[i] + (double)d[i];
    }
    
    printf("Result checksum: %f\n", total);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
