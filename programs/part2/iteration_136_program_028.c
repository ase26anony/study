#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <string.h>

#define N 10000
#define M 5000

/* Non-inlineable helper function with target offloading */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float* restrict d, int dynamic_n, float alpha, float beta) {
    /* First target region: compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
        device(0) num_teams(64) thread_limit(256) simdlen(32)
    for (int i = 0; i < N; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = alpha * a[i] + beta * b[i];
        } else {
            c[i] = alpha * a[i] - beta * b[i];
        }
        
        /* Additional computation to prevent optimization */
        c[i] += sinf((float)i * 0.01f);
    }
    
    /* Second target region: dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_n], b[0:dynamic_n]) map(tofrom: d[0:dynamic_n]) \
        device(0) num_teams(32) thread_limit(128) simdlen(16)
    for (int i = 0; i < dynamic_n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = a[i] * b[i] + c[i % N];
        } else if (i % 3 == 1) {
            d[i] = a[i] / (b[i] + 1.0f);
        } else {
            d[i] = sqrtf(fabsf(a[i] - b[i]));
        }
        
        /* Nested conditionals increase complexity */
        if (d[i] > 100.0f) {
            d[i] = 100.0f;
        } else if (d[i] < -100.0f) {
            d[i] = -100.0f;
        }
    }
    
    /* Third loop: reduction pattern */
    float sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
        map(to: c[0:N]) map(tofrom: sum) reduction(+:sum) \
        device(0) num_teams(16) thread_limit(64) simdlen(8)
    for (int i = 0; i < N; i++) {
        sum += c[i] * c[i];
    }
    
    /* Use the result to prevent dead code elimination */
    d[0] += sum * 0.001f;
}

/* Wrapper function called from host-side parallel region */
static void compute_wrapper(float* a, float* b, float* c, float* d, 
                           int dynamic_n, int thread_id) {
    float alpha = 1.5f + thread_id * 0.1f;
    float beta = 0.5f - thread_id * 0.05f;
    
    process_on_gpu(a, b, c, d, dynamic_n, alpha, beta);
}

int main() {
    float *a, *b, *c, *d;
    float *a_ref, *b_ref, *c_ref, *d_ref;
    int i;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * sizeof(float));
    
    a_ref = (float*)malloc(N * sizeof(float));
    b_ref = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    d_ref = (float*)malloc(N * sizeof(float));
    
    srand(42);
    for (i = 0; i < N; i++) {
        a[i] = (float)rand() / RAND_MAX * 100.0f;
        b[i] = (float)rand() / RAND_MAX * 100.0f;
        c[i] = 0.0f;
        d[i] = 0.0f;
        
        a_ref[i] = a[i];
        b_ref[i] = b[i];
        c_ref[i] = c[i];
        d_ref[i] = d[i];
    }
    
    /* Host-side OpenMP parallel region with nested offloading */
    #pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread calls the wrapper which contains target regions */
        compute_wrapper(a, b, c, d, M + thread_id * 100, thread_id);
        
        /* Host-side computation for verification */
        #pragma omp for simd
        for (i = 0; i < N; i++) {
            if (i % 2 == 0) {
                c_ref[i] = 1.5f * a_ref[i] + 0.5f * b_ref[i];
            } else {
                c_ref[i] = 1.5f * a_ref[i] - 0.5f * b_ref[i];
            }
            c_ref[i] += sinf((float)i * 0.01f);
        }
    }
    
    /* Verify results */
    int errors = 0;
    float tolerance = 1e-4f;
    
    for (i = 0; i < N; i++) {
        if (fabsf(c[i] - c_ref[i]) > tolerance) {
            errors++;
            if (errors < 10) {
                printf("Mismatch at index %d: device=%f, host=%f\n", 
                       i, c[i], c_ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All computations match between host and device.\n");
        
        /* Additional verification for second loop */
        float host_sum = 0.0f;
        for (i = 0; i < M; i++) {
            host_sum += c_ref[i] * c_ref[i];
        }
        host_sum *= 0.001f;
        
        if (fabsf(d[0] - host_sum) < tolerance) {
            printf("SUCCESS: Reduction result verified.\n");
        } else {
            printf("WARNING: Reduction mismatch: device=%f, host=%f\n", 
                   d[0], host_sum);
        }
    } else {
        printf("FAILURE: Found %d mismatches in %d elements.\n", errors, N);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(a_ref); free(b_ref); free(c_ref); free(d_ref);
    
    return errors == 0 ? 0 : 1;
}
