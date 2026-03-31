#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <assert.h>

#define N 10000
#define M 5000
#define VALIDATE_EPSILON 1e-6

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
            c[i] = alpha * b[i] + a[i];
        }
        
        /* Additional computation to prevent optimization */
        c[i] += sinf((float)i * 0.01f);
    }
    
    /* Second target region: dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_n], c[0:dynamic_n]) map(from: d[0:dynamic_n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_n; i++) {
        /* Different conditional pattern */
        if (i % 3 == 0) {
            d[i] = beta * a[i] + c[i];
        } else if (i % 3 == 1) {
            d[i] = beta * c[i] + a[i];
        } else {
            d[i] = sqrtf(fabsf(a[i] * c[i]));
        }
        
        /* More complex computation */
        d[i] += cosf((float)i * 0.02f);
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* a, float* b, float* c_ref, float* d_ref, 
                             int n, int dynamic_n, float alpha, float beta) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            c_ref[i] = alpha * a[i] + b[i];
        } else {
            c_ref[i] = alpha * b[i] + a[i];
        }
        c_ref[i] += sinf((float)i * 0.01f);
    }
    
    for (int i = 0; i < dynamic_n; i++) {
        if (i % 3 == 0) {
            d_ref[i] = beta * a[i] + c_ref[i];
        } else if (i % 3 == 1) {
            d_ref[i] = beta * c_ref[i] + a[i];
        } else {
            d_ref[i] = sqrtf(fabsf(a[i] * c_ref[i]));
        }
        d_ref[i] += cosf((float)i * 0.02f);
    }
}

/* Validation function */
static int validate_results(float* dev, float* ref, int n, const char* name) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (fabsf(dev[i] - ref[i]) > VALIDATE_EPSILON) {
            if (errors < 5) {
                printf("Mismatch at %s[%d]: device=%.6f, host=%.6f\n", 
                       name, i, dev[i], ref[i]);
            }
            errors++;
        }
    }
    return errors;
}

int main(int argc, char** argv) {
    float *a, *b, *c, *d;
    float *c_ref, *d_ref;
    int dynamic_n = M;
    
    if (argc > 1) {
        dynamic_n = atoi(argv[1]);
        dynamic_n = (dynamic_n < 1) ? M : dynamic_n;
        dynamic_n = (dynamic_n > N) ? N : dynamic_n;
    }
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * sizeof(float));
    c_ref = (float*)malloc(N * sizeof(float));
    d_ref = (float*)malloc(N * sizeof(float));
    
    assert(a && b && c && d && c_ref && d_ref);
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(N - i) * 0.05f;
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    float alpha = 2.5f;
    float beta = 1.5f;
    
    /* Host-side reference computation */
    compute_reference(a, b, c_ref, d_ref, N, dynamic_n, alpha, beta);
    
    /* Host-side OpenMP parallel region wrapping target offload */
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread calls the GPU processing function */
        #pragma omp barrier
        
        if (tid == 0) {
            process_on_gpu(a, b, c, d, dynamic_n, alpha, beta);
        }
        
        #pragma omp barrier
    }
    
    /* Validate results */
    int c_errors = validate_results(c, c_ref, N, "c");
    int d_errors = validate_results(d, d_ref, dynamic_n, "d");
    
    if (c_errors == 0 && d_errors == 0) {
        printf("SUCCESS: All GPU computations match host reference\n");
        printf("  Array c[%d]: 0 errors\n", N);
        printf("  Array d[%d]: 0 errors\n", dynamic_n);
    } else {
        printf("FAILURE: Found mismatches\n");
        printf("  Array c: %d errors\n", c_errors);
        printf("  Array d: %d errors\n", d_errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(c_ref); free(d_ref);
    
    return (c_errors == 0 && d_errors == 0) ? 0 : 1;
}
