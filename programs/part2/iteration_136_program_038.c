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
                device(0) num_teams(32) thread_limit(256) simdlen(32)
    for (int i = 0; i < n; i++) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (i % 2 == 0) {
            c[i] = a[i] * alpha + b[i];
        } else {
            c[i] = a[i] * 2.0f + b[i] * 0.5f;
        }
        
        /* Additional computation to prevent optimization */
        c[i] += sinf((float)i * 0.01f);
    }
    
    /* Second target region with dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
                map(to: a[0:dynamic_n], b[0:dynamic_n]) map(from: d[0:dynamic_n]) \
                device(0) num_teams(16) thread_limit(128) simdlen(16)
    for (int i = 0; i < dynamic_n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = a[i] * 2.0f * alpha + b[i];
        } else if (i % 3 == 1) {
            d[i] = a[i] * alpha * alpha + sqrtf(b[i]);
        } else {
            d[i] = a[i] * 0.5f + b[i] * 2.0f;
        }
        
        /* More complex computation */
        d[i] += cosf((float)i * 0.02f) * (i % 10);
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* c_ref, float* d_ref, 
                             int n, int dynamic_n, float alpha) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = a[i] * alpha + b[i];
        } else {
            c_ref[i] = a[i] * 2.0f + b[i] * 0.5f;
        }
        c_ref[i] += sinf((float)i * 0.01f);
    }
    
    for (int i = 0; i < dynamic_n; i++) {
        if (i % 3 == 0) {
            d_ref[i] = a[i] * 2.0f * alpha + b[i];
        } else if (i % 3 == 1) {
            d_ref[i] = a[i] * alpha * alpha + sqrtf(b[i]);
        } else {
            d_ref[i] = a[i] * 0.5f + b[i] * 2.0f;
        }
        d_ref[i] += cosf((float)i * 0.02f) * (i % 10);
    }
}

int main() {
    float *a, *b, *c, *d, *c_ref, *d_ref;
    float alpha = 1.5f;
    int dynamic_n = M;
    
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
        int thread_id = omp_get_thread_num();
        
        /* Each thread calls the offloading function with different data sections */
        int chunk_size = N / omp_get_num_threads();
        int start = thread_id * chunk_size;
        int end = (thread_id == omp_get_num_threads() - 1) ? N : start + chunk_size;
        
        /* Adjust dynamic_n per thread to create variation */
        int local_dynamic_n = dynamic_n - thread_id * 100;
        if (local_dynamic_n < 100) local_dynamic_n = 100;
        
        process_on_gpu(&a[start], &b[start], &c[start], &d[start], 
                      end - start, local_dynamic_n, alpha + thread_id * 0.1f);
    }
    
    /* Compute reference on host */
    compute_reference(a, b, c_ref, d_ref, N, dynamic_n, alpha);
    
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
            if (errors <= 10) {
                printf("Mismatch at d[%d]: device=%f, host=%f\n", 
                       i, d[i], d_ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All computations match between device and host\n");
    } else {
        printf("FAILURE: Found %d mismatches\n", errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(c_ref); free(d_ref);
    
    return errors > 0 ? 1 : 0;
}
