#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int n, int dynamic_n, float alpha) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
                device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < n; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] + b[i] * alpha;
        } else {
            c[i] = a[i] - b[i] * alpha;
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:dynamic_n], b[0:dynamic_n]) map(from: d[0:dynamic_n]) \
                device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
        } else if (i % 3 == 1) {
            d[i] = a[i] * b[i] * alpha;
        } else {
            d[i] = (a[i] + b[i]) * alpha;
        }
    }
}

/* Host-side computation for verification */
static void compute_reference(float* a, float* b, float* c_ref, float* d_ref, 
                              int n, int dynamic_n, float alpha) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = a[i] + b[i] * alpha;
        } else {
            c_ref[i] = a[i] - b[i] * alpha;
        }
    }
    
    for (int i = 0; i < dynamic_n; i++) {
        if (i % 3 == 0) {
            d_ref[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
        } else if (i % 3 == 1) {
            d_ref[i] = a[i] * b[i] * alpha;
        } else {
            d_ref[i] = (a[i] + b[i]) * alpha;
        }
    }
}

int main(int argc, char** argv) {
    float *a, *b, *c, *d, *c_ref, *d_ref;
    float alpha = 2.5f;
    int dynamic_n = M;
    
    if (argc > 1) {
        dynamic_n = atoi(argv[1]);
        if (dynamic_n > N) dynamic_n = N;
    }
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    d_ref = (float*)malloc(N * sizeof(float));
    
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a[i] = (float)i / N;
        b[i] = (float)(N - i) / N;
        c[i] = 0.0f;
        d[i] = 0.0f;
        c_ref[i] = 0.0f;
        d_ref[i] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region calling the offloading function */
    #pragma omp parallel num_threads(4)
    {
        /* Each thread calls the offloading function */
        process_on_gpu(a, b, c, d, N, dynamic_n, alpha);
    }
    
    /* Compute reference on host */
    compute_reference(a, b, c_ref, d_ref, N, dynamic_n, alpha);
    
    /* Verify results */
    int errors = 0;
    float tolerance = 1e-5f;
    
    for (int i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
            if (errors <= 5) {
                printf("Error at c[%d]: device=%f, host=%f\n", i, c[i], c_ref[i]);
            }
        }
    }
    
    for (int i = 0; i < dynamic_n; i++) {
        if (fabsf(d[i] - d_ref[i]) > tolerance) {
            errors++;
            if (errors <= 5) {
                printf("Error at d[%d]: device=%f, host=%f\n", i, d[i], d_ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All computations match between device and host\n");
        printf("Processed %d elements with compile-time constant loop\n", N);
        printf("Processed %d elements with dynamic loop\n", dynamic_n);
    } else {
        printf("FAILURE: Found %d errors\n", errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(c_ref); free(d_ref);
    
    return errors == 0 ? 0 : 1;
}
