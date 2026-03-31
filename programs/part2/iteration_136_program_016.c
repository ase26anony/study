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
        map(to: a[0:N], b[0:N], alpha) map(tofrom: c[0:N]) \
        device(0) num_teams(64) thread_limit(128) simdlen(8)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = alpha * a[i] + b[i];
        } else {
            c[i] = alpha * b[i] + a[i];
        }
        
        /* Additional computation to prevent optimization */
        c[i] += sinf((float)i * 0.01f);
    }
    
    /* Second target region: dynamic iteration count based on parameter */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n], beta) map(tofrom: d[0:n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(4)
    for (int i = 0; i < n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = beta * a[i] - b[i];
        } else if (i % 3 == 1) {
            d[i] = beta * b[i] - a[i];
        } else {
            d[i] = a[i] * b[i] * beta;
        }
        
        /* More complex computation */
        d[i] *= cosf((float)i * 0.02f);
    }
}

/* Host-side reference computation */
static void compute_reference(float* a, float* b, float* c_ref, float* d_ref, 
                             int n, float alpha, float beta) {
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + b[i];
        } else {
            c_ref[i] = alpha * b[i] + a[i];
        }
        c_ref[i] += sinf((float)i * 0.01f);
    }
    
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            d_ref[i] = beta * a[i] - b[i];
        } else if (i % 3 == 1) {
            d_ref[i] = beta * b[i] - a[i];
        } else {
            d_ref[i] = a[i] * b[i] * beta;
        }
        d_ref[i] *= cosf((float)i * 0.02f);
    }
}

/* Function called from host-side OpenMP parallel region */
static void parallel_offload_wrapper(float* a, float* b, float* c, float* d, 
                                    int n, float alpha, float beta) {
    int thread_id = omp_get_thread_num();
    
    /* Add thread-specific offset to prevent identical computations */
    float thread_alpha = alpha + thread_id * 0.001f;
    float thread_beta = beta - thread_id * 0.001f;
    
    /* Call the GPU offloading function */
    process_on_gpu(a, b, c, d, n, thread_alpha, thread_beta);
}

int main() {
    float *a, *b, *c, *d, *c_ref, *d_ref;
    float alpha = 2.5f, beta = 1.5f;
    int dynamic_n = M;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    d_ref = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i / N;
        b[i] = (float)(N - i) / N;
        c[i] = 0.0f;
        d[i] = 0.0f;
        c_ref[i] = 0.0f;
        d_ref[i] = 0.0f;
    }
    
    /* Host-side reference computation */
    compute_reference(a, b, c_ref, d_ref, dynamic_n, alpha, beta);
    
    /* Host-side OpenMP parallel region wrapping target offloading */
    #pragma omp parallel num_threads(2)
    {
        parallel_offload_wrapper(a, b, c, d, dynamic_n, alpha, beta);
    }
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    for (int i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at c[%d]: device=%f, host=%f\n", 
                       i, c[i], c_ref[i]);
            }
        }
    }
    
    for (int i = 0; i < dynamic_n; i++) {
        if (fabsf(d[i] - d_ref[i]) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at d[%d]: device=%f, host=%f\n", 
                       i, d[i], d_ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All computations match between host and device.\n");
    } else {
        printf("FAILURE: Found %d mismatches between host and device.\n", errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(c_ref); free(d_ref);
    
    return errors > 0 ? 1 : 0;
}
