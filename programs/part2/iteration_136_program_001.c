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
    /* First target region with compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N], alpha) map(tofrom: c[0:N]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
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
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n], beta) map(tofrom: d[0:n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = beta * a[i] - b[i];
        } else if (i % 3 == 1) {
            d[i] = beta * b[i] - a[i];
        } else {
            d[i] = (a[i] + b[i]) * beta;
        }
        
        /* More computation to ensure non-trivial loop */
        d[i] *= cosf((float)i * 0.005f);
    }
    
    /* Third loop with reduction - adds complexity */
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
        map(to: c[0:N]) map(tofrom: sum) reduction(+:sum) \
        device(0) num_teams(16) thread_limit(64) simdlen(8)
    for (int i = 0; i < N; i++) {
        sum += c[i] * c[i];
    }
    
    /* Use the result to prevent dead code elimination */
    if (sum > 1e9f) {
        printf("Intermediate sum: %f\n", sum);
    }
}

/* Wrapper function called from host OpenMP parallel region */
static void compute_wrapper(float* a, float* b, float* c, float* d, 
                           int dynamic_size, int thread_id) {
    float alpha = 1.5f + thread_id * 0.1f;
    float beta = 0.8f - thread_id * 0.05f;
    
    process_on_gpu(a, b, c, d, dynamic_size, alpha, beta);
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
        a[i] = (float)i / N;
        b[i] = (float)(i * i) / (N * N);
        c[i] = 0.0f;
        d[i] = 0.0f;
        c_ref[i] = 0.0f;
        d_ref[i] = 0.0f;
    }
    
    int dynamic_size = M;
    
    /* Host-side OpenMP parallel region creating nested parallelism */
    #pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread calls the offloading function */
        compute_wrapper(a, b, c, d, dynamic_size, thread_id);
        
        /* Host-side reference computation for verification */
        #pragma omp for simd
        for (int i = 0; i < N; i++) {
            float alpha = 1.5f + thread_id * 0.1f;
            if (i % 2 == 0) {
                c_ref[i] = alpha * a[i] + b[i];
            } else {
                c_ref[i] = alpha * b[i] + a[i];
            }
            c_ref[i] += sinf((float)i * 0.01f);
        }
        
        #pragma omp for simd
        for (int i = 0; i < dynamic_size; i++) {
            float beta = 0.8f - thread_id * 0.05f;
            if (i % 3 == 0) {
                d_ref[i] = beta * a[i] - b[i];
            } else if (i % 3 == 1) {
                d_ref[i] = beta * b[i] - a[i];
            } else {
                d_ref[i] = (a[i] + b[i]) * beta;
            }
            d_ref[i] *= cosf((float)i * 0.005f);
        }
    }
    
    /* Verify results */
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
    
    for (int i = 0; i < dynamic_size; i++) {
        if (fabsf(d[i] - d_ref[i]) > tolerance) {
            errors++;
            if (errors <= 10) {
                printf("Mismatch at d[%d]: device=%f, host=%f\n", 
                       i, d[i], d_ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All GPU computations match host reference\n");
    } else {
        printf("FAILURE: Found %d errors\n", errors);
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
