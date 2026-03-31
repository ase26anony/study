#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int n, float alpha, float beta) {
    /* First target region: compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:N], b[0:N], alpha) \
                map(tofrom: c[0:N]) \
                device(0) num_teams(64) thread_limit(256) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = alpha * a[i] + b[i];
        } else {
            c[i] = alpha * b[i] + a[i];
        }
    }
    
    /* Second target region: dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:n], b[0:n], beta) \
                map(tofrom: d[0:n]) \
                device(0) num_teams(32) thread_limit(128) simdlen(16)
    for (int i = 0; i < n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = beta * a[i] - b[i];
        } else if (i % 3 == 1) {
            d[i] = beta * b[i] - a[i];
        } else {
            d[i] = sqrtf(fabsf(a[i] * b[i]));
        }
    }
}

/* Host-side computation for verification */
static void compute_reference(float* a, float* b, float* c_ref, float* d_ref, 
                             int n, float alpha, float beta) {
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + b[i];
        } else {
            c_ref[i] = alpha * b[i] + a[i];
        }
    }
    
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            d_ref[i] = beta * a[i] - b[i];
        } else if (i % 3 == 1) {
            d_ref[i] = beta * b[i] - a[i];
        } else {
            d_ref[i] = sqrtf(fabsf(a[i] * b[i]));
        }
    }
}

int main() {
    float *a, *b, *c, *d, *c_ref, *d_ref;
    float alpha = 2.5f, beta = 1.5f;
    int dynamic_n = M;
    
    /* Allocate memory */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    d_ref = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c || !d || !c_ref || !d_ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 1) % 100) * 0.2f;
        c[i] = 0.0f;
        d[i] = 0.0f;
        c_ref[i] = 0.0f;
        d_ref[i] = 0.0f;
    }
    
    /* Host-side reference computation */
    compute_reference(a, b, c_ref, d_ref, dynamic_n, alpha, beta);
    
    /* Call GPU processing from within host OpenMP parallel region */
    #pragma omp parallel num_threads(4)
    {
        /* Each thread calls the offloading function */
        #pragma omp master
        {
            process_on_gpu(a, b, c, d, dynamic_n, alpha, beta);
        }
        #pragma omp barrier
        
        /* Verify results in parallel */
        #pragma omp for
        for (int i = 0; i < N; i++) {
            if (i < dynamic_n) {
                if (fabsf(c[i] - c_ref[i]) > 1e-6f || 
                    fabsf(d[i] - d_ref[i]) > 1e-6f) {
                    #pragma omp critical
                    {
                        printf("Mismatch at index %d: c=%f vs %f, d=%f vs %f\n",
                               i, c[i], c_ref[i], d[i], d_ref[i]);
                    }
                }
            } else {
                if (fabsf(c[i] - c_ref[i]) > 1e-6f) {
                    #pragma omp critical
                    {
                        printf("Mismatch at index %d: c=%f vs %f\n",
                               i, c[i], c_ref[i]);
                    }
                }
            }
        }
    }
    
    /* Final verification */
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > 1e-6f) errors++;
        if (i < dynamic_n && fabsf(d[i] - d_ref[i]) > 1e-6f) errors++;
    }
    
    if (errors == 0) {
        printf("SUCCESS: All GPU computations match host reference\n");
    } else {
        printf("FAILURE: %d mismatches found\n", errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(c_ref); free(d_ref);
    
    return errors > 0 ? 1 : 0;
}
