#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

static void __attribute__((noinline,noipa))
process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
               float* restrict d, int n, int dynamic_n)
{
    // First target region with compile-time constant iteration count
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < n; i++) {
        // Conditional inside loop body - influences SIMT transformation
        if (i % 2 == 0) {
            c[i] = a[i] * 2.0f + b[i];
        } else {
            c[i] = a[i] * 3.0f - b[i];
        }
    }
    
    // Second target region with dynamic iteration count
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_n], c[0:dynamic_n]) map(from: d[0:dynamic_n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_n; i++) {
        // Different conditional pattern
        if (i % 3 == 0) {
            d[i] = sqrtf(a[i]) + c[i];
        } else if (i % 3 == 1) {
            d[i] = a[i] * c[i] - 1.0f;
        } else {
            d[i] = 2.0f * a[i] / (c[i] + 0.001f);
        }
    }
    
    // Third loop with reduction for additional complexity
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
        map(to: c[0:n]) map(tofrom: sum) reduction(+:sum) \
        device(0) num_teams(16) thread_limit(64) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += c[i] * (i % 10);
    }
    
    // Store reduction result
    c[0] = sum;
}

static void __attribute__((noinline,noipa))
host_reference(float* restrict a, float* restrict b, float* restrict c,
               float* restrict d, int n, int dynamic_n)
{
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c[i] = a[i] * 2.0f + b[i];
        } else {
            c[i] = a[i] * 3.0f - b[i];
        }
    }
    
    for (int i = 0; i < dynamic_n; i++) {
        if (i % 3 == 0) {
            d[i] = sqrtf(a[i]) + c[i];
        } else if (i % 3 == 1) {
            d[i] = a[i] * c[i] - 1.0f;
        } else {
            d[i] = 2.0f * a[i] / (c[i] + 0.001f);
        }
    }
    
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += c[i] * (i % 10);
    }
    c[0] = sum;
}

int main(void)
{
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c_gpu = (float*)malloc(N * sizeof(float));
    float *c_host = (float*)malloc(N * sizeof(float));
    float *d_gpu = (float*)malloc(N * sizeof(float));
    float *d_host = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c_gpu || !c_host || !d_gpu || !d_host) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i + 1) * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
        c_gpu[i] = 0.0f;
        c_host[i] = 0.0f;
        d_gpu[i] = 0.0f;
        d_host[i] = 0.0f;
    }
    
    // Host-side OpenMP parallel region wrapping the GPU call
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        // Each thread calls the GPU processing function
        // This creates nested parallelism scenario
        if (tid == 0) {
            process_on_gpu(a, b, c_gpu, d_gpu, N, M);
        }
        
        #pragma omp barrier
        
        // Compute reference on host
        if (tid == 1) {
            host_reference(a, b, c_host, d_host, N, M);
        }
    }
    
    // Verify results
    int errors = 0;
    float tolerance = 1e-4f;
    
    for (int i = 0; i < N; i++) {
        if (fabsf(c_gpu[i] - c_host[i]) > tolerance) {
            if (errors < 10) {
                printf("Mismatch at c[%d]: GPU=%f, Host=%f\n", 
                       i, c_gpu[i], c_host[i]);
            }
            errors++;
        }
    }
    
    for (int i = 0; i < M; i++) {
        if (fabsf(d_gpu[i] - d_host[i]) > tolerance) {
            if (errors < 10) {
                printf("Mismatch at d[%d]: GPU=%f, Host=%f\n", 
                       i, d_gpu[i], d_host[i]);
            }
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All GPU computations match host reference\n");
        printf("Reduction result: %f\n", c_gpu[0]);
    } else {
        printf("FAILURE: %d mismatches found\n", errors);
    }
    
    free(a);
    free(b);
    free(c_gpu);
    free(c_host);
    free(d_gpu);
    free(d_host);
    
    return errors > 0 ? 1 : 0;
}
