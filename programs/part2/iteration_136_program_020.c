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
            c[i] = alpha * a[i] + b[i];
        } else {
            c[i] = alpha * b[i] - a[i];
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_n], c[0:dynamic_n]) map(from: d[0:dynamic_n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = sqrtf(a[i] * a[i] + c[i] * c[i]);
        } else if (i % 3 == 1) {
            d[i] = a[i] * c[i] / (a[i] + c[i] + 1.0f);
        } else {
            d[i] = sinf(a[i]) * cosf(c[i]);
        }
    }
}

/* Host-side computation for validation */
static void compute_reference(float* a, float* b, float* c_ref, float* d_ref, 
                             int n, int dynamic_n, float alpha) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + b[i];
        } else {
            c_ref[i] = alpha * b[i] - a[i];
        }
    }
    
    for (int i = 0; i < dynamic_n; i++) {
        if (i % 3 == 0) {
            d_ref[i] = sqrtf(a[i] * a[i] + c_ref[i] * c_ref[i]);
        } else if (i % 3 == 1) {
            d_ref[i] = a[i] * c_ref[i] / (a[i] + c_ref[i] + 1.0f);
        } else {
            d_ref[i] = sinf(a[i]) * cosf(c_ref[i]);
        }
    }
}

int main() {
    float *a, *b, *c, *d, *c_ref, *d_ref;
    int dynamic_n = M;
    float alpha = 2.5f;
    
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
    
    /* Host-side OpenMP parallel region containing target offload */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU processing function */
        #pragma omp master
        {
            printf("Starting GPU offload from thread %d\n", tid);
            process_on_gpu(a, b, c, d, N, dynamic_n, alpha);
        }
        
        #pragma omp barrier
        
        /* Host-side computation for validation (done by all threads) */
        #pragma omp for
        for (int i = 0; i < N; i++) {
            if (i < dynamic_n) {
                if (i % 3 == 0) {
                    d_ref[i] = sqrtf(a[i] * a[i] + c_ref[i] * c_ref[i]);
                }
            }
        }
    }
    
    /* Master thread computes full reference */
    compute_reference(a, b, c_ref, d_ref, N, dynamic_n, alpha);
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-5f;
    
    for (int i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
            if (errors <= 5) {
                printf("Error at c[%d]: GPU=%f, CPU=%f\n", i, c[i], c_ref[i]);
            }
        }
        if (i < dynamic_n && fabsf(d[i] - d_ref[i]) > tolerance) {
            errors++;
            if (errors <= 5) {
                printf("Error at d[%d]: GPU=%f, CPU=%f\n", i, d[i], d_ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All GPU computations match CPU reference\n");
    } else {
        printf("FAILURE: %d errors found\n", errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(c_ref); free(d_ref);
    
    return errors > 0 ? 1 : 0;
}
