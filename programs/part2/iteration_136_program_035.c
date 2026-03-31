/* This program is designed to trigger the SIMT transformation in GCC's omp-low.cc
   Specifically targeting lines 2941-2975 which handle SIMT variant generation
   for OpenMP target offloading to GPU architectures. */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define M 5000
#define VALIDATE_EPS 1e-6f

/* Non-inlineable helper function containing target offloading regions */
static __attribute__((noinline)) 
void process_on_gpu(float* restrict a, float* restrict b, float* restrict c, 
                    float alpha, int dynamic_n, int use_conditional) {
    /* First target region: compile-time constant iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
        device(0) num_teams(64) thread_limit(128) simdlen(32)
    for (int i = 0; i < N; ++i) {
        /* Conditional inside loop body - influences SIMT transformation */
        if (use_conditional && (i % 2 == 0)) {
            c[i] = alpha * a[i] + b[i] * 2.0f;
        } else {
            c[i] = alpha * a[i] + b[i];
        }
    }
    
    /* Second target region: dynamic iteration count */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_n], b[0:dynamic_n]) map(from: c[0:dynamic_n]) \
        device(0) num_teams(32) thread_limit(256) simdlen(16)
    for (int i = 0; i < dynamic_n; ++i) {
        /* Different computation pattern */
        if (i % 3 == 0) {
            c[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
        } else {
            c[i] = a[i] * b[i] / (alpha + 1.0f);
        }
    }
}

/* Host-side reference computation for validation */
static void compute_reference(float* restrict a, float* restrict b, 
                              float* restrict ref, float alpha, 
                              int n, int use_conditional) {
    for (int i = 0; i < n; ++i) {
        if (use_conditional && (i % 2 == 0)) {
            ref[i] = alpha * a[i] + b[i] * 2.0f;
        } else {
            ref[i] = alpha * a[i] + b[i];
        }
    }
}

int main() {
    float *a, *b, *c, *ref;
    int i;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    ref = (float*)malloc(N * sizeof(float));
    
    if (!a || !b || !c || !ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterned data */
    #pragma omp parallel for simd
    for (i = 0; i < N; ++i) {
        a[i] = (float)i / 100.0f;
        b[i] = (float)(N - i) / 50.0f;
        c[i] = 0.0f;
        ref[i] = 0.0f;
    }
    
    /* Host-side OpenMP parallel region wrapping the GPU call */
    #pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread calls the GPU processing function with different parameters */
        #pragma omp master
        {
            printf("Starting GPU offloading from %d host threads\n", omp_get_num_threads());
        }
        
        /* Call GPU function - this should trigger SIMT transformation */
        process_on_gpu(a, b, c, 2.5f, M, thread_id % 2);
    }
    
    /* Compute reference on host */
    compute_reference(a, b, ref, 2.5f, N, 1);
    
    /* Validate results */
    int errors = 0;
    #pragma omp parallel for reduction(+:errors)
    for (i = 0; i < N; ++i) {
        if (fabsf(c[i] - ref[i]) > VALIDATE_EPS) {
            errors++;
            #pragma omp critical
            if (errors < 10) {
                printf("Mismatch at index %d: GPU=%f, Host=%f\n", i, c[i], ref[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("SUCCESS: All %d elements computed correctly on GPU\n", N);
        printf("SIMT transformation should have been triggered for target teams distribute parallel for simd\n");
    } else {
        printf("FAILURE: %d mismatches found\n", errors);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(ref);
    
    return errors > 0 ? 1 : 0;
}
