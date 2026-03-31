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
        float temp = c[i] * 2.0f;
        if (temp > 1.0f) {  /* Non-trivial conditional */
            d[i] = temp - 1.0f;
            sum += d[i];
        } else {
            d[i] = temp;
        }
    }
    
    /* Loop 3: Nested context - outer loop with inner SIMD */
    for (int j = 0; j < 4; j++) {
        float scale = 1.0f + j * 0.25f;
        #pragma omp simd simdlen(8) linear(i:1)
        for (int i = 0; i < n; i++) {
            a[i] = b[i] * scale + d[i] * 0.5f;
        }
    }
    
    /* Loop 4: Mixed data types within SIMD */
    #pragma omp simd simdlen(4)
    for (int i = 0; i < n; i++) {
        /* Mixed operations encouraging vectorization */
        int idx = i & 0xFF;
        float fval = (float)idx * 0.01f;
        double dval = (double)fval * 0.5;
        c[i] = (float)dval + b[i];
    }
}

/* Another function creating SIMD context */
void process_arrays(float *a, float *b, int n) {
    /* SIMD with linear clause on reference */
    #pragma omp simd linear(a, b:1)
    for (int i = 0; i < n; i++) {
        a[i] = a[i] * 0.99f + b[i] * 0.01f;
    }
    
    /* Sequential SIMD loops */
    #pragma omp simd simdlen(16)
    for (int i = 0; i < n; i += 2) {
        b[i] = a[i] * a[i + 1];
    }
    
    #pragma omp simd simdlen(8)
    for (int i = 1; i < n; i += 2) {
        b[i] = a[i - 1] + a[i];
    }
}

/* Function that could be called from different contexts */
static void simd_kernel(float *restrict out, const float *restrict in, int n) {
    #pragma omp simd safelen(32) aligned(out, in: 32)
    for (int i = 0; i < n; i++) {
        /* Use built-in function to encourage vectorization */
        out[i] = __builtin_sinf(in[i]) * 2.0f;
    }
}

int main(void) {
    /* Allocate aligned memory for better vectorization */
    float *a = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *d = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *temp = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    if (!a || !b || !c || !d || !temp) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        b[i] = (float)(lcg_rand() % 1000) / 1000.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
        temp[i] = (float)i / SIZE;
    }
    
    /* Multiple calls to create context for SIMT transformation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, SIZE, iter);
        process_arrays(a, b, SIZE);
        
        /* Call SIMD kernel function */
        simd_kernel(temp, a, SIZE);
        
        /* Mix data back */
        #pragma omp simd
        for (int i = 0; i < SIZE; i++) {
            a[i] = a[i] * 0.8f + temp[i] * 0.2f;
        }
    }
    
    /* Final verification checksum */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
    for (int i = 0; i < SIZE; i++) {
        checksum += (double)a[i] + (double)b[i] + (double)c[i] + (double)d[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(temp);
    
    return 0;
}
