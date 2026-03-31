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

/* Complex computation function with multiple SIMD loops */
void compute(float *a, float *b, float *c, int n, float k) {
    float sum = 0.0f;
    
    /* First SIMD loop with multiple clauses */
    #pragma omp simd simdlen(8) safelen(16) aligned(a, b, c: 64) linear(i:1)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + k;
    }
    
    /* Second SIMD loop with reduction and conditional */
    #pragma omp simd simdlen(4) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        if (c[i] > 50.0f) {
            sum += c[i] * 0.5f;
        } else {
            sum += c[i];
        }
    }
    
    /* Third SIMD loop with mixed operations */
    #pragma omp simd simdlen(8) aligned(a, b: 64)
    for (int i = 0; i < n; i++) {
        float temp = a[i] + b[i];
        /* Use conditional store */
        if (temp > 100.0f) {
            c[i] = temp * 0.8f;
        } else {
            c[i] = temp * 1.2f;
        }
    }
    
    /* Nested loop structure */
    for (int iter = 0; iter < 3; iter++) {
        float factor = 1.0f + iter * 0.1f;
        
        /* SIMD loop inside outer loop */
        #pragma omp simd simdlen(4) safelen(8) linear(i:1)
        for (int i = 0; i < n; i++) {
            a[i] = b[i] * factor + c[i];
        }
    }
}

/* Function with double precision SIMD loops */
void compute_double(double *a, double *b, double *c, int n) {
    /* SIMD loop with different simdlen for doubles */
    #pragma omp simd simdlen(4) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i] + sin(a[i]) * cos(b[i]);
    }
    
    /* Another SIMD loop with reduction */
    double sum = 0.0;
    #pragma omp simd simdlen(4) reduction(+:sum)
    for (int i = 0; i < n; i += 2) {
        sum += c[i] + c[i+1];
    }
}

/* Target offload version */
#ifdef USE_TARGET
void compute_target(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * expf(a[i] * 0.01f);
    }
}
#endif

int main() {
    float a[SIZE], b[SIZE], c[SIZE];
    double da[SIZE], db[SIZE], dc[SIZE];
    
    /* Initialize data */
    init_arrays(a, b, c, SIZE);
    for (int i = 0; i < SIZE; i++) {
        da[i] = (double)a[i];
        db[i] = (double)b[i];
        dc[i] = 0.0;
    }
    
    /* Multiple calls to create runtime context */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        float k = 1.0f + iter * 0.5f;
        
        /* Call compute function multiple times */
        compute(a, b, c, SIZE, k);
        
        /* Call double precision version */
        compute_double(da, db, dc, SIZE);
        
        #ifdef USE_TARGET
        /* Call target offload version */
        compute_target(a, b, c, SIZE);
        #endif
        
        /* Modify data slightly for next iteration */
        for (int i = 0; i < SIZE; i++) {
            a[i] += 0.1f;
            b[i] -= 0.05f;
        }
    }
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum_float = 0.0f;
    double checksum_double = 0.0;
    
    #pragma omp simd reduction(+:checksum_float)
    for (int i = 0; i < SIZE; i++) {
        checksum_float += c[i];
    }
    
    #pragma omp simd simdlen(4) reduction(+:checksum_double)
    for (int i = 0; i < SIZE; i++) {
        checksum_double += dc[i];
    }
    
    printf("Float checksum: %f\n", checksum_float);
    printf("Double checksum: %lf\n", checksum_double);
    
    return 0;
}
