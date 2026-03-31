#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int n, int dynamic_n, float scale) {
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
                device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < n; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] * scale + b[i];
        } else {
            c[i] = a[i] * (scale * 2.0f) - b[i];
        }
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:dynamic_n], b[0:dynamic_n]) map(from: d[0:dynamic_n]) \
                device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = sqrtf(a[i] * b[i]);
        } else if (i % 3 == 1) {
            d[i] = a[i] * a[i] + b[i] * b[i];
        } else {
            d[i] = 2.0f * a[i] - 3.0f * b[i];
        }
    }
}

/* Wrapper function called from host-side OpenMP parallel region */
static void compute_wrapper(float* a, float* b, float* c, float* d, 
                           int n, int dynamic_n, float scale) {
    process_on_gpu(a, b, c, d, n, dynamic_n, scale);
}

int main() {
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *d = (float*)malloc(N * sizeof(float));
    float *c_ref = (float*)malloc(N * sizeof(float));
    float *d_ref = (float*)malloc(N * sizeof(float));
    
    /* Initialize arrays */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i + 1) * 0.01f;
        b[i] = (float)(N - i) * 0.005f;
    }
    
    float scale = 2.5f;
    int dynamic_n = M;  /* Dynamic iteration count */
    
    /* Host-side OpenMP parallel region creating nested parallelism */
    #pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread calls the wrapper which will offload to GPU */
        #pragma omp master
        {
            compute_wrapper(a, b, c, d, N, dynamic_n, scale);
        }
        
        #pragma omp barrier
        
        /* Host-side reference computation for validation */
        #pragma omp for simd
        for (int i = 0; i < N; i++) {
            if (i % 2 == 0) {
                c_ref[i] = a[i] * scale + b[i];
            } else {
                c_ref[i] = a[i] * (scale * 2.0f) - b[i];
            }
        }
        
        #pragma omp for simd
        for (int i = 0; i < dynamic_n; i++) {
            if (i % 3 == 0) {
                d_ref[i] = sqrtf(a[i] * b[i]);
            } else if (i % 3 == 1) {
                d_ref[i] = a[i] * a[i] + b[i] * b[i];
            } else {
                d_ref[i] = 2.0f * a[i] - 3.0f * b[i];
            }
        }
    }
    
    /* Validate results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    for (int i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
            if (errors <= 5) {
                printf("Mismatch at c[%d]: device=%f, host=%f\n", 
                       i, c[i], c_ref[i]);
            }
        }
    }
    
    for (int i = 0; i < dynamic_n; i++) {
        if (fabsf(d[i] - d_ref[i]) > tolerance) {
            errors++;
            if (errors <= 5) {
                printf("Mismatch at d[%d]: device=%f, host=%f\n", 
                       i, d[i], d_ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All GPU computations match host reference\n");
    } else {
        printf("FAILURE: Found %d mismatches\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(c_ref);
    free(d_ref);
    
    return errors > 0 ? 1 : 0;
}
