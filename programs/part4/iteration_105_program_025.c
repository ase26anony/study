/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
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

/* Initialize arrays with pseudo-random data */
void init_arrays(float *a, float *b, float *c, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (float)(lcg_rand() % 1000) / 100.0f;
        b[i] = (float)(lcg_rand() % 1000) / 100.0f;
        c[i] = 0.0f;
    }
}

/* Complex computation with multiple SIMD loops */
#pragma omp declare simd notinbranch linear(k:1)
static float simd_safe_func(float x, float y, int k) {
    return x * y + (float)k * 0.5f;
}

/* Main computation function with various SIMD constructs */
void compute(float *a, float *b, float *c, float *d, int n, int iter) {
    float sum1 = 0.0f, sum2 = 0.0f;
    int k = iter;
    
    /* Loop 1: Basic SIMD with safelen and simdlen */
    #pragma omp simd safelen(16) simdlen(8) aligned(a, b, c: 64) linear(k:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + (float)k;
        k += 1; /* linear clause test */
    }
    
    /* Loop 2: SIMD with reduction and conditional */
    #pragma omp simd reduction(+:sum1) simdlen(4)
    for (int i = 0; i < n; i++) {
        float val = a[i] + b[i];
        if (val > 10.0f) { /* Non-trivial conditional */
            sum1 += val * c[i];
        }
    }
    
    /* Loop 3: Mixed data types and function calls */
    #pragma omp simd simdlen(8) aligned(d: 64)
    for (int i = 0; i < n; i++) {
        /* Use built-in function to prevent optimization */
        d[i] = __builtin_sinf(a[i]) * __builtin_cosf(b[i]);
    }
    
    /* Loop 4: Nested SIMD-like pattern */
    for (int block = 0; block < 4; block++) {
        int start = block * (n / 4);
        int end = (block + 1) * (n / 4);
        
        #pragma omp simd safelen(32) reduction(+:sum2) linear(start:1)
        for (int i = start; i < end; i++) {
            sum2 += a[i] * d[i] - b[i];
            start += 1; /* linear clause */
        }
    }
    
    /* Loop 5: SIMD with multiple clauses */
    #pragma omp simd simdlen(16) aligned(a, b, c, d: 64) \
                     linear(k:1) reduction(+:sum1, sum2)
    for (int i = 0; i < n; i += 2) {
        c[i] = simd_safe_func(a[i], b[i], k);
        d[i] = simd_safe_func(b[i], c[i], k + 1);
        sum1 += c[i];
        sum2 += d[i];
        k += 2;
    }
    
    /* Prevent dead code elimination */
    a[0] = sum1 + sum2;
}

/* Function called from both SIMD and non-SIMD contexts */
#pragma omp declare simd
float process_element(float a, float b, float c) {
    return (a > b) ? a * c : b * c;
}

/* Additional computation with target offload context */
void target_computation(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
                simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        c[i] = process_element(a[i], b[i], 2.0f);
    }
}

int main() {
    float *a = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *d = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with different patterns */
    init_arrays(a, b, c, SIZE);
    
    /* Multiple calls to create runtime context */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute(a, b, c, d, SIZE, iter);
        
        /* Alternate between standard and target computation */
        if (iter % 2 == 0) {
            target_computation(a, b, d, SIZE);
        }
        
        /* Modify data slightly each iteration */
        #pragma omp simd
        for (int i = 0; i < SIZE; i++) {
            a[i] += 0.1f;
            b[i] -= 0.05f;
        }
    }
    
    /* Final checksum to prevent optimization */
    double checksum = 0.0;
    #pragma omp simd reduction(+:checksum) simdlen(8)
    for (int i = 0; i < SIZE; i++) {
        checksum += (double)c[i] + (double)d[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Verification */
    if (checksum != 0.0) {
        printf("Computation completed successfully\n");
    }
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
