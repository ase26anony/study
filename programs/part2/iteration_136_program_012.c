#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int dynamic_n, float alpha, float beta) {
    /* First target region: compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
                device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = alpha * a[i] + b[i];
        } else {
            c[i] = beta * a[i] - b[i];
        }
        
        /* Additional computation to prevent optimization */
        c[i] = c[i] * (1.0f + sinf(i * 0.001f));
    }
    
    /* Second target region: dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: d[0:dynamic_n]) \
                device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int j = 0; j < dynamic_n; j++) {
        /* Different conditional pattern */
        if (j < dynamic_n / 2) {
            d[j] = sqrtf(d[j]) + j * 0.01f;
        } else {
            d[j] = logf(fabsf(d[j]) + 1.0f) - j * 0.005f;
        }
        
        /* Nested conditionals increase complexity */
        if (d[j] > 100.0f) {
            d[j] = 100.0f;
        } else if (d[j] < -100.0f) {
            d[j] = -100.0f;
        }
    }
    
    /* Third loop: reduction pattern */
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
                map(to: c[0:N]) reduction(+:sum) \
                device(0) num_teams(16) thread_limit(64) simdlen(8)
    for (int k = 0; k < N; k += 2) {
        sum += c[k] * c[k + 1];
    }
    
    /* Use the result to prevent dead code elimination */
    d[0] += sum * 0.001f;
}

/* Wrapper function called from host OpenMP parallel region */
static void gpu_wrapper(float* a, float* b, float* c, float* d, int n, int tid) {
    /* Each thread gets different parameters */
    float alpha = 1.5f + tid * 0.1f;
    float beta = 0.8f - tid * 0.05f;
    
    /* Call the target offloading function */
    process_on_gpu(a, b, c, d, n, alpha, beta);
}

int main() {
    float *a, *b, *c, *d;
    float *a_host, *b_host, *c_host, *d_host;
    int i;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(M * sizeof(float));
    
    a_host = (float*)malloc(N * sizeof(float));
    b_host = (float*)malloc(N * sizeof(float));
    c_host = (float*)malloc(N * sizeof(float));
    d_host = (float*)malloc(M * sizeof(float));
    
    srand(42);
    for (i = 0; i < N; i++) {
        a[i] = (float)rand() / RAND_MAX * 100.0f;
        b[i] = (float)rand() / RAND_MAX * 50.0f;
        c[i] = 0.0f;
        a_host[i] = a[i];
        b_host[i] = b[i];
        c_host[i] = 0.0f;
    }
    
    for (i = 0; i < M; i++) {
        d[i] = (float)rand() / RAND_MAX * 200.0f - 100.0f;
        d_host[i] = d[i];
    }
    
    /* Host-side OpenMP parallel region creating nested parallelism */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU wrapper with its thread ID */
        gpu_wrapper(a, b, c, d, M - tid * 100, tid);
        
        /* Host-side computation for verification */
        #pragma omp for simd
        for (i = 0; i < N; i++) {
            float alpha = 1.5f + tid * 0.1f;
            float beta = 0.8f - tid * 0.05f;
            
            if (i % 2 == 0) {
                c_host[i] = alpha * a_host[i] + b_host[i];
            } else {
                c_host[i] = beta * a_host[i] - b_host[i];
            }
            c_host[i] = c_host[i] * (1.0f + sinf(i * 0.001f));
        }
    }
    
    /* Additional host-side target region with different parameters */
    float final_result = 0.0f;
    #pragma omp target teams distribute parallel for simd \
                map(to: c[0:N]) map(from: final_result) \
                device(0) num_teams(8) thread_limit(32) simdlen(4) \
                reduction(+:final_result)
    for (i = 0; i < N; i++) {
        /* Complex conditional chain */
        if (c[i] > 0.0f) {
            if (c[i] < 50.0f) {
                final_result += c[i] * 0.5f;
            } else if (c[i] < 100.0f) {
                final_result += c[i] * 0.3f;
            } else {
                final_result += c[i] * 0.1f;
            }
        } else {
            final_result += fabsf(c[i]) * 0.2f;
        }
    }
    
    /* Verification */
    int errors = 0;
    float tolerance = 1e-4f;
    
    for (i = 0; i < N; i++) {
        if (fabsf(c[i] - c_host[i]) > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: device=%f, host=%f\n", 
                       i, c[i], c_host[i]);
            }
        }
    }
    
    /* Verify dynamic loop results */
    for (i = 0; i < M; i++) {
        float expected = d_host[i];
        if (i < M / 2) {
            expected = sqrtf(expected) + i * 0.01f;
        } else {
            expected = logf(fabsf(expected) + 1.0f) - i * 0.005f;
        }
        if (expected > 100.0f) expected = 100.0f;
        else if (expected < -100.0f) expected = -100.0f;
        
        if (fabsf(d[i] - expected) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Dynamic loop mismatch at %d: %f vs %f\n", 
                       i, d[i], expected);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All computations match between host and device\n");
        printf("Final reduction result: %f\n", final_result);
    } else {
        printf("FAILURE: Found %d errors\n", errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(a_host); free(b_host); free(c_host); free(d_host);
    
    return errors > 0 ? 1 : 0;
}
